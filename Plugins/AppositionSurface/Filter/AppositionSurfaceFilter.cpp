/*
 * AppositionSurfaceFilter.cpp
 *
 *  Created on: Jan 18, 2013
 *      Author: Félix de las Pozas Álvarez
 */

// plugin
#include "AppositionSurfaceFilter.h"
#include <Core/ASMeshProxy.h>
#include <Core/ASVolumeProxy.h>
#include <Core/Model/Segmentation.h>
#include <Core/Outputs/MeshType.h>
#include <Core/Outputs/RawMesh.h>
#include <Core/Outputs/RasterizedVolume.h>
#include <GUI/Representations/SliceRepresentation.h>
#include <GUI/Representations/MeshRepresentation.h>

// Qt
#include <QtGlobal>
#include <QDebug>
#include <QMessageBox>

// VTK
#include <vtkMath.h>
#include <vtkTransform.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkImplicitVolume.h>
#include <vtkClipPolyData.h>
#include <vtkTriangleFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkMeshQuality.h>
#include <vtkMutableUndirectedGraph.h>
#include <vtkIdList.h>
#include <vtkBoostConnectedComponents.h>
#include <vtkIdTypeArray.h>
#include <vtkEdgeListIterator.h>
#include <vtkGenericDataObjectWriter.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkPlane.h>


const double UNDEFINED = -1.;
const char SEP = ',';

using namespace EspINA;

const QString AppositionSurfaceFilter::INPUTLINK = "Input";
const QString AppositionSurfaceFilter::SAS = "SAS";

const ModelItem::ArgumentId AppositionSurfaceFilter::ORIGIN = "Origin Segmentation";

//----------------------------------------------------------------------------
AppositionSurfaceFilter::AppositionSurfaceFilter(NamedInputs inputs, Arguments args, FilterType type)
: SegmentationFilter(inputs, args, type)
, m_resolution(50)
, m_iterations(10)
, m_converge(true)
, m_ap(NULL)
, m_originSegmentation(NULL)
, m_origin(args[ORIGIN])
, m_area(UNDEFINED)
, m_perimeter(UNDEFINED)
, m_tortuosity(UNDEFINED)
, m_alreadyFetchedData(false)
{
  m_templateOrigin = new double[3];
  m_templateNormal = new double[3];

  if (m_origin == QString())
    m_origin = QString("Unspecified origin");
}

//----------------------------------------------------------------------------
AppositionSurfaceFilter::~AppositionSurfaceFilter()
{
  if (m_ap != NULL)
  {
    disconnect(m_originSegmentation, SIGNAL(outputModified()), this, SLOT(inputModified()));
    m_ap->Delete();
  }

  delete[] m_templateNormal;
  delete[] m_templateOrigin;
}

//----------------------------------------------------------------------------
void AppositionSurfaceFilter::createDummyOutput(FilterOutputId id, const FilterOutput::OutputRepresentationName &type)
{
  if (SegmentationVolume::TYPE == type)
    createOutput(id, VolumeProxySPtr(new ASVolumeProxy()));
  else if (MeshType::TYPE == type)
    createOutput(id, MeshProxySPtr(new ASMeshProxy()));
  else
    Q_ASSERT(false);
}

//----------------------------------------------------------------------------
void AppositionSurfaceFilter::createOutputRepresentations(SegmentationOutputSPtr output)
{
  SegmentationVolumeSPtr volumeRep = segmentationVolume(output);
  MeshTypeSPtr           meshRep   = meshOutput(output);
  output->addGraphicalRepresentation(GraphicalRepresentationSPtr(new SegmentationSliceRepresentation(volumeRep, NULL)));
  output->addGraphicalRepresentation(GraphicalRepresentationSPtr(new MeshRepresentation(meshRep, NULL)));
}

//----------------------------------------------------------------------------
bool AppositionSurfaceFilter::fetchSnapshot(FilterOutputId oId)
{
  return false;
}

//----------------------------------------------------------------------------
bool AppositionSurfaceFilter::needUpdate() const
{
  return needUpdate(0);
}

//----------------------------------------------------------------------------
bool AppositionSurfaceFilter::needUpdate(FilterOutputId oId) const
{
  bool update = SegmentationFilter::needUpdate(oId);

  if (!update && !m_inputs.isEmpty())
  {
    Q_ASSERT(m_inputs.size() == 1);

    SegmentationVolumeSPtr inputVolume  = segmentationVolume(m_inputs[0] );
    MeshTypeSPtr           outputMesh   = meshOutput        (m_outputs[0]);

    update = outputMesh->timeStamp() < inputVolume->timeStamp();
  }

  return update;
}

//----------------------------------------------------------------------------
void AppositionSurfaceFilter::upkeeping()
{
  if (!m_originSegmentation)
  {
    const QString namedInput = m_args[Filter::INPUTS];
    QStringList list = namedInput.split(QChar('_'));
    const int outputId = list[1].toInt();

    int i = 0;
    FilterSPtr segFilter = m_namedInputs[AppositionSurfaceFilter::INPUTLINK];

    ModelItemSList items = segFilter->relatedItems(EspINA::OUT, Filter::CREATELINK);
    while(!m_originSegmentation && i < items.size())
    {
      SegmentationPtr segmentation = segmentationPtr(items[i].data());
      if (segmentation->outputId() == outputId)
      {
        m_originSegmentation = segmentation;

        connect(m_originSegmentation, SIGNAL(outputModified()),
                this, SLOT(inputModified()));
      }
      ++i;
    }
  }
  Q_ASSERT(m_originSegmentation);

  //FIXME
  m_input = segmentationVolume(m_originSegmentation->output())->toITK();

  // need to get the polydata as soon as possible to show the correct mesh
  fetchCachePolyDatas();

  ModelItemSList items = relatedItems(EspINA::OUT, Filter::CREATELINK);
  if (items.size() == 1)
  {
    SegmentationPtr segmentation = segmentationPtr(items.first().data());
    segmentation->setInputSegmentationDependent(true);
    items.first().data()->notifyModification();
  }
}

//----------------------------------------------------------------------------
void AppositionSurfaceFilter::run()
{
  run(0);
}

//----------------------------------------------------------------------------
void AppositionSurfaceFilter::run(FilterOutputId oId)
{
  Q_ASSERT(0 == oId);

  upkeeping();

  //m_input = m_originSegmentation->volume()->toITK();
  m_input->SetBufferedRegion(m_input->GetLargestPossibleRegion());

  itkVolumeType::SizeType bounds;
  bounds[0] = bounds[1] = bounds[2] = 1;
  PadFilterType::Pointer padder = PadFilterType::New();
  padder->SetInput(m_input);
  padder->SetPadLowerBound(bounds);
  padder->SetPadUpperBound(bounds);
  padder->SetConstant(0); // extend with black pixels
  padder->Update();
  itkVolumeType::Pointer padImage = padder->GetOutput();

  itkVolumeType::RegionType region = padImage->GetLargestPossibleRegion();
  region.SetIndex(region.GetIndex() + bounds);
  padImage->SetRegions(region);

  ItkToVtkFilterType::Pointer itk2vtk_filter = ItkToVtkFilterType::New();
  itk2vtk_filter->SetInput(padImage);
  itk2vtk_filter->Update();
  vtkSmartPointer<vtkImageData> vtk_padImage = itk2vtk_filter->GetOutput();

  double *spacing = vtk_padImage->GetSpacing();

  //qDebug() << "Computing Distance Map";
  Points points = segmentationPoints(padImage);
  //qDebug() << points->GetNumberOfPoints() << " segmentation points");

  double corner[3], max[3], mid[3], min[3], size[3];
  OBBTreeType obbTree = OBBTreeType::New();
  obbTree->ComputeOBB(points, corner, max, mid, min, size);
  Points obbCorners = corners(corner, max, mid, min);
  DistanceMapType::Pointer distanceMap = computeDistanceMap(padImage, DISTANCESMOOTHSIGMAFACTOR);

  //   qDebug() << "Build and move the plane to Avg Max Distance";
  double avgMaxDistPoint[3];
  double maxDistance;
  maxDistancePoint(distanceMap, avgMaxDistPoint, maxDistance);

  int xResolution = m_resolution;
  int yResolution = m_resolution;

  computeResolution(max, mid, vtk_padImage->GetSpacing(), xResolution, yResolution);

  PlaneSourceType planeSource = PlaneSourceType::New();
  planeSource->SetOrigin(obbCorners->GetPoint(0));
  planeSource->SetPoint1(obbCorners->GetPoint(1));
  planeSource->SetPoint2(obbCorners->GetPoint(2));
  planeSource->SetResolution(xResolution, yResolution);
  planeSource->Update();

  double *normal = planeSource->GetNormal();
  vtkMath::Normalize(normal);

  double v[3], displacement[3];
  for (int i = 0; i < 3; i++) {
    v[i] = avgMaxDistPoint[i] - obbCorners->GetPoint(0)[i];
    m_templateOrigin[i] = obbCorners->GetPoint(0)[i];
    m_templateNormal[i] = normal[i];
  }

  project(v, normal, displacement);

  if (vtkMath::Dot(displacement, normal) > 0)
    planeSource->Push(vtkMath::Norm(displacement));
  else
    planeSource->Push(- vtkMath::Norm(displacement));

  PolyData sourcePlane = planeSource->GetOutput();

  // Plane is only transformed in its normal direction
  //   qDebug() << "Compute transformation matrix from distance map gradient";

  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  gradientFilter->SetInput(distanceMap);
  gradientFilter->SetUseImageSpacingOn();
  gradientFilter->Update();

  vtkSmartPointer<vtkImageData> gradientVectorGrid = vtkSmartPointer<vtkImageData>::New();
  vectorImageToVTKImage(gradientFilter->GetOutput(), gradientVectorGrid);
  //gradientVectorGrid->Print(std::cout);

  projectVectors(gradientVectorGrid, normal);

  GridTransform grid_transform = GridTransform::New();
  grid_transform->SetDisplacementGrid(gradientVectorGrid);
  grid_transform->SetInterpolationModeToCubic();
  grid_transform->SetDisplacementScale(DISPLACEMENTSCALE);

  TransformPolyDataFilter transformer = TransformPolyDataFilter::New();
  PolyData auxPlane = sourcePlane;

  int numIterations = m_iterations;
  double thresholdError = 0;
  if (m_converge)
  {
    computeIterationLimits(min, spacing, numIterations, thresholdError);
  }

  //   qDebug() << "Number of iterations:" << m_iterations;

  transformer->SetTransform(grid_transform);
  PointsListType pointsList;
  for (int i =0; i <= numIterations; i++) {
    transformer->SetInput(auxPlane);
    transformer->Modified();
    transformer->Update();

    auxPlane->DeepCopy(transformer->GetOutput());
    if (m_converge) {
      if (hasConverged(auxPlane->GetPoints(), pointsList, thresholdError)) {
        //   qDebug() << "Total iterations: " << i << std::endl;
        break;
      }
      else {
        pointsList.push_front(auxPlane->GetPoints());
        if (pointsList.size() > MAXSAVEDSTATUSES)
          pointsList.pop_back();
      }
    }
  }
  pointsList.clear();

  PolyData clippedPlane = clipPlane(transformer->GetOutput(), vtk_padImage);
  //ESPINA_DEBUG(clippedPlane->GetNumberOfCells() << " cells after clip");

  /**
   * Traslate
   */
  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  transform->Translate (-spacing[0]*bounds[0], -spacing[1]*bounds[0], -spacing[2]*bounds[0]);
  transformFilter->SetTransform(transform);
  transformFilter->SetInput(clippedPlane);
  transformFilter->Update();

  PolyData appositionSurface = transformFilter->GetOutput();
  // PolyData appositionSurface = clippedPlane;
  //ESPINA_DEBUG(appositionSurface->GetNumberOfCells() << " cells in apppositionPlane");

  // qDebug() << "Create Mesh";
  //m_ap->DeepCopy(appositionSurface);
  m_ap = PolyData::New();
  m_ap->Initialize();
  m_ap->SetPoints(appositionSurface->GetPoints());
  m_ap->SetPolys(appositionSurface->GetPolys());
  m_ap->SetLines(appositionSurface->GetLines());
  m_ap->Modified();

  SegmentationVolumeSPtr volume = segmentationVolume(m_originSegmentation->output());

  SegmentationRepresentationSList repList;
  repList << RawMeshSPtr(new RawMesh(m_ap));
  repList << RasterizedVolumeSPtr(new RasterizedVolume(m_ap, volume->spacing()));

  createOutput(0, repList);

  emit modified(this);
}

//----------------------------------------------------------------------------
AppositionSurfaceFilter::Points AppositionSurfaceFilter::segmentationPoints(itkVolumeType::Pointer seg) const
{
  itkVolumeType::PointType   origin  = seg->GetOrigin();
  itkVolumeType::SpacingType spacing = seg->GetSpacing();

  Points points = Points::New();

  itkVolumeIterator it(seg, seg->GetLargestPossibleRegion());
  while (!it.IsAtEnd())
  {
    itkVolumeType::PixelType val = it.Get();
    itkVolumeType::IndexType index = it.GetIndex();
    if (val != 0)
    {
      double segPoint[3];
      for (int i=0; i<3; i++)
        segPoint[i] = origin[i]+index[i]*spacing[i];
      points->InsertNextPoint(segPoint);
    }
    ++it;
  }
  return points;
}

//----------------------------------------------------------------------------
AppositionSurfaceFilter::Points AppositionSurfaceFilter::corners(double corner[3], double max[3], double mid[3], double min[3]) const
{
  Points points = Points::New();
  double x[3];
  
  // {0,0,0}  <- in a cube
  x[0] = corner[0];
  x[1] = corner[1];
  x[2] = corner[2];
  points->InsertNextPoint(x);
  // {1,0,0}  <- in a cube
  x[0] = corner[0] + mid[0];
  x[1] = corner[1] + mid[1];
  x[2] = corner[2] + mid[2];
  points->InsertNextPoint(x);
  // {0,1,0}  <- in a cube
  x[0] = corner[0] + max[0];
  x[1] = corner[1] + max[1];
  x[2] = corner[2] + max[2];
  points->InsertNextPoint(x);
  // {1,1,0}  <- in a cube
  x[0] = corner[0] + max[0] + mid[0];
  x[1] = corner[1] + max[1] + mid[1];
  x[2] = corner[2] + max[2] + mid[2];
  points->InsertNextPoint(x);
  // {0,0,1}  <- in a cube
  x[0] = corner[0] + min[0];
  x[1] = corner[1] + min[1];
  x[2] = corner[2] + min[2];
  points->InsertNextPoint(x);
  // {1,0,1}  <- in a cube
  x[0] = corner[0] + mid[0] + min[0];
  x[1] = corner[1] + mid[1] + min[1];
  x[2] = corner[2] + mid[2] + min[2];
  points->InsertNextPoint(x);
  // {0,1,1}  <- in a cube
  x[0] = corner[0] + max[0] + min[0];
  x[1] = corner[1] + max[1] + min[1];
  x[2] = corner[2] + max[2] + min[2];
  points->InsertNextPoint(x);
  // {1,1,1}  <- in a cube
  x[0] = corner[0] + max[0] + mid[0] + min[0];
  x[1] = corner[1] + max[1] + mid[1] + min[1];
  x[2] = corner[2] + max[2] + mid[2] + min[2];
  points->InsertNextPoint(x);
  
  return points;
}

//----------------------------------------------------------------------------
AppositionSurfaceFilter::DistanceMapType::Pointer AppositionSurfaceFilter::computeDistanceMap(itkVolumeType::Pointer volume, float sigma) const
{
  SMDistanceMapFilterType::Pointer smdm_filter = SMDistanceMapFilterType::New();
  smdm_filter->InsideIsPositiveOn();
  smdm_filter->UseImageSpacingOn();
  smdm_filter->SquaredDistanceOff();
  smdm_filter->SetInput(volume);
  smdm_filter->Update();
  
  double avgMaxDistPoint[3];
  double max_distance;
  maxDistancePoint(smdm_filter->GetOutput(), avgMaxDistPoint, max_distance);
  
  SmoothingFilterType::Pointer smoothingRecursiveGaussianImageFilter = SmoothingFilterType::New();
  smoothingRecursiveGaussianImageFilter->SetSigma(sigma * max_distance);
  smoothingRecursiveGaussianImageFilter->SetInput(smdm_filter->GetOutput());
  
  SmoothingFilterType::OutputImageRegionType::SizeType regionSize = smdm_filter->GetOutput()->GetLargestPossibleRegion().GetSize();
  if (((sigma * max_distance) > 0) && (regionSize[0] >= 4) && (regionSize[1] >= 4) && (regionSize[2] >= 4))
  {
    smoothingRecursiveGaussianImageFilter->Update();
    return smoothingRecursiveGaussianImageFilter->GetOutput();
  }
  else
  {
    return smdm_filter->GetOutput();
  }
}

//----------------------------------------------------------------------------
void AppositionSurfaceFilter::maxDistancePoint(AppositionSurfaceFilter::DistanceMapType::Pointer map, 
                                               double avgMaxDistPoint[3],
                                               double & maxDist) const
{
  maxDist = 0;
  DistanceMapType::PointType origin = map->GetOrigin();
  DistanceMapType::SpacingType spacing = map->GetSpacing();
  Points points = Points::New();
  
  DistanceIterator it(map, map->GetLargestPossibleRegion());
  
  // #ifdef DEBUG_AP_FILES
  //     ofstream distanceFile;
  //     distanceFile.open("decDistFile");
  // #endif
  
  while (!it.IsAtEnd())
  {
    DistanceType dist = it.Get();
    // #ifdef DEBUG_AP_FILES
    //     distanceFile << dist << std::endl;
    // #endif
    if (dist > maxDist)
    {
      DistanceMapType::IndexType index = it.GetIndex();
      maxDist = dist;
      for (unsigned int i = 0; i < 3; i++)
        avgMaxDistPoint[i] = origin[i] + index[i]*spacing[i];
      
      points->Initialize();
      points->InsertNextPoint(avgMaxDistPoint);
    }
    else if (dist == maxDist)
    {
      DistanceMapType::IndexType index = it.GetIndex();
      for (unsigned int i = 0; i < 3; i++)
        avgMaxDistPoint[i] += origin[i] + index[i]*spacing[i];
      
      points->InsertNextPoint(origin[0] + index[0]*spacing[0],
                              origin[1] + index[1]*spacing[1],
                              origin[2] + index[2]*spacing[2]);
    }
    ++it;
  }
  //   #ifdef DEBUG_AP_FILES
  //   distanceFile.close();
  //   #endif
  
  for (unsigned int i = 0; i < 3; i++)
    avgMaxDistPoint[i] /= points->GetNumberOfPoints();
  
}

//----------------------------------------------------------------------------
void AppositionSurfaceFilter::computeResolution(double * max, double * mid, double * spacing, int & xResolution, int & yResolution) const
{
  double max_in_pixels[3] = {0,0,0};
  double mid_in_pixels[3] = {0,0,0};
  for (unsigned int i=0; i < 3; i++)
  {
    max_in_pixels[i] = max[i] / spacing[i];
    mid_in_pixels[i] = mid[i] / spacing[i];
  }
  yResolution = vtkMath::Norm(max_in_pixels);  // Heads up: Max - y
  xResolution = vtkMath::Norm(mid_in_pixels);  // Heads up: Mid - x
}

//----------------------------------------------------------------------------
void AppositionSurfaceFilter::project(const double* A, const double* B, double* Projection) const
{
  double scale = vtkMath::Dot(A,B)/pow(vtkMath::Norm(B), 2);
  for(unsigned int i = 0; i < 3; i++)
    Projection[i] = scale * B[i];
}

//----------------------------------------------------------------------------
void AppositionSurfaceFilter::vectorImageToVTKImage(CovariantVectorImageType::Pointer vectorImage,
                                                    vtkSmartPointer<vtkImageData> image) const
{
  CovariantVectorImageType::PointType origin = vectorImage->GetOrigin();
  // ESPINA_DEBUG("CovariantVectorMap Origin " << origin[0] << " " << origin[1] << " " << origin[2]);
  CovariantVectorImageType::RegionType region = vectorImage->GetLargestPossibleRegion();
  // region.Print(std::cout);
  CovariantVectorImageType::SpacingType spacing = vectorImage->GetSpacing();
  CovariantVectorImageType::SizeType imageSize = region.GetSize();
  CovariantVectorImageType::IndexType originIndex = region.GetIndex();
  
  image->SetOrigin(origin[0], origin[1], origin[2]);
  image->SetExtent(originIndex[0], originIndex[0] + imageSize[0] - 1,
                   originIndex[1], originIndex[1] + imageSize[1] - 1,
                   originIndex[2], originIndex[2] + imageSize[2] - 1);
  image->SetSpacing(spacing[0], spacing[1], spacing[2]);
  // image->SetSpacing(vectorImage->GetSpacing()[0], vectorImage->GetSpacing()[1], vectorImage->GetSpacing()[2]);
  
  // image->Print(std::cout);
  vtkSmartPointer<vtkFloatArray> vectors = vtkSmartPointer<vtkFloatArray>::New();
  vectors->SetNumberOfComponents(3);
  vectors->SetNumberOfTuples(imageSize[0] * imageSize[1] * imageSize[2]);
  vectors->SetName("GradientVectors");
  
  // #ifdef DEBUG_AP_FILES
  //   std::ofstream covarianceFile;
  //   covarianceFile.open("decCovarianceFile");
  // #endif
  
  int counter = 0;
  for (unsigned int k = originIndex[2]; k < originIndex[2] + imageSize[2]; k++)
    for (unsigned int j = originIndex[1]; j < originIndex[1] + imageSize[1]; j++)
      for (unsigned int i = originIndex[0]; i < originIndex[0] + imageSize[0]; i++)
      {
        CovariantVectorImageType::IndexType index;
        index[0] = i;
        index[1] = j;
        index[2] = k;
        
        CovariantVectorType pixel = vectorImage->GetPixel(index);
        float val[3];
        val[0] = pixel[0];
        val[1] = pixel[1];
        val[2] = pixel[2];
        
        // #ifdef DEBUG_AP_FILES
        //  covarianceFile << val[0] << " " << val[1] << " " << val[2] << std::endl;
        // #endif
        vectors->InsertTupleValue(counter, val);
        counter++;
      }
      
      // #ifdef DEBUG_AP_FILES
      //   covarianceFile.close();
      // #endif
      
      image->GetPointData()->SetVectors(vectors);
      image->GetPointData()->SetScalars(vectors);
}

//----------------------------------------------------------------------------
void AppositionSurfaceFilter::projectVectors(vtkImageData* vectors_image, double* unitary) const
{
  vtkSmartPointer<vtkDataArray> vectors = vectors_image->GetPointData()->GetVectors();
  int numTuples = vectors->GetNumberOfTuples();
  
  // #ifdef DEBUG_AP_FILES
  //   std::ofstream gradientFile;
  //   gradientFile.open("decGradientFile");
  //   gradientFile << "Unitary: " << unitary[0] << " " << unitary[1] << " " << unitary[2] << std::endl;
  // #endif
  
  double projv[3];
  for (int i = 0; i < numTuples; i++)
  {
    double *v = vectors->GetTuple(i);
    project(v, unitary, projv);
    vectors->SetTuple(i, projv);
    
    // #ifdef DEBUG_AP_FILES
    //     gradientFile << projv[0] << " " << projv[1] << " " << projv[2] << std::endl;
    // #endif
  }
  
  // #ifdef DEBUG_AP_FILES
  //   gradientFile.close();
  // #endif
}

//------------------------------------------------------------------------
/**
 * Normal's magnitude is 1
 */
void AppositionSurfaceFilter::projectPolyDataToPlane(double* origin, double* normal, vtkPolyData* meshIn, vtkPolyData* meshOut) const
{
  
  vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
  plane->SetOrigin(origin);
  plane->SetNormal(normal);
  
  int pointsCount = 0;
  double projected[3], p[3];
  
  vtkSmartPointer<vtkPoints> pointsIn, pointsOut;
  pointsOut = vtkSmartPointer<vtkPoints>::New();
  pointsIn = meshIn->GetPoints();
  
  pointsCount = pointsIn->GetNumberOfPoints();
  pointsOut->SetNumberOfPoints(pointsCount);
  for (int i = 0; i < pointsCount; i++) {
    pointsIn->GetPoint(i, p);
    plane->ProjectPoint(p, projected);
    pointsOut->SetPoint(i, projected);
  }
  
  vtkSmartPointer<vtkPolyData> auxMesh = vtkSmartPointer<vtkPolyData>::New();
  auxMesh->DeepCopy(meshIn);
  auxMesh->SetPoints(pointsOut);
  
  vtkSmartPointer<vtkPolyDataNormals> normals =
  vtkSmartPointer<vtkPolyDataNormals>::New();
  normals->SetInput(auxMesh);
  normals->SplittingOff();
  normals->Update();
  
  
  meshOut->ShallowCopy(normals->GetOutput());
  
}

//----------------------------------------------------------------------------
void AppositionSurfaceFilter::computeIterationLimits(double * min, double * spacing, int & iterations, double & thresholdError) const
{
  double min_in_pixels[3] = { 0, 0, 0 };
  double step[3] = { 0, 0, 0 };
  
  for (unsigned int i = 0; i < 3; i++)
    step[i] = min[i];
  
  vtkMath::Normalize(step);
  for (unsigned int i = 0; i < 3; i++)
  {
    min_in_pixels[i] = min[i] / spacing[i];
    step[i] = step[i] * spacing[i];
  }
  
  iterations = MAXITERATIONSFACTOR * std::max(1, int(floor(vtkMath::Norm(min_in_pixels))));
  thresholdError = THRESHOLDFACTOR * vtkMath::Norm(step);
}

//----------------------------------------------------------------------------
bool AppositionSurfaceFilter::hasConverged( vtkPoints * lastPlanePoints, PointsListType & pointsList, double threshold) const
{
  double error = 0;
  
  for (PointsListType::iterator it = pointsList.begin();
       it != pointsList.end(); ++it) {
    computeMeanEuclideanError(lastPlanePoints, *it, error);
  if (error <= threshold) return true;
       }
       return false;
}

//----------------------------------------------------------------------------
int AppositionSurfaceFilter::computeMeanEuclideanError(vtkPoints * pointsA, vtkPoints * pointsB, double & euclideanError) const
{
  double pointA[3], pointB[3];
  euclideanError = 0;
  int pointsCount = 0;
  
  if (pointsA->GetNumberOfPoints() != pointsB->GetNumberOfPoints())
    return -1;
  
  pointsCount = pointsA->GetNumberOfPoints();
  
  for (int i = 0; i < pointsCount; i++)
  {
    pointsA->GetPoint(i, pointA);
    pointsB->GetPoint(i, pointB);
    euclideanError += sqrt(vtkMath::Distance2BetweenPoints(pointA, pointB));
  }
  
  euclideanError /= pointsCount;
  return 0;
}

//----------------------------------------------------------------------------
AppositionSurfaceFilter::PolyData AppositionSurfaceFilter::clipPlane(PolyData plane, vtkImageData* image) const
{
  vtkSmartPointer<vtkImplicitVolume> implicitVolFilter = vtkSmartPointer<vtkImplicitVolume>::New();
  implicitVolFilter->SetVolume(image);
  implicitVolFilter->SetOutValue(0);
  
  double inValue = 255.0;
  inValue = image->GetScalarRange()[1];
  
  vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
  clipper->SetClipFunction(implicitVolFilter);
  clipper->SetInput(plane);
  clipper->SetValue(inValue*CLIPPINGTHRESHOLD);
  clipper->Update();
  
  PolyData clippedPlane; // = PolyData::New();
  
  // qDebug() << "Correct Plane's visualization and cell area's computation";
  clippedPlane = triangulate(clipper->GetOutput());
  
  return clippedPlane;
}

//----------------------------------------------------------------------------
AppositionSurfaceFilter::PolyData AppositionSurfaceFilter::triangulate(PolyData plane) const
{
  vtkSmartPointer<vtkTriangleFilter> triangle_filter = vtkSmartPointer<vtkTriangleFilter>::New();
  triangle_filter->SetInput(plane);
  triangle_filter->Update();
  
  vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
  normals->SetInput(triangle_filter->GetOutput());
  normals->SplittingOff();
  normals->Update();
  
  PolyData resultPlane = PolyData::New();
  resultPlane->ShallowCopy(normals->GetOutput());
  
  return resultPlane;
}

//----------------------------------------------------------------------------
double AppositionSurfaceFilter::computeArea() const
{
  return computeArea(m_ap);
}

//----------------------------------------------------------------------------
double AppositionSurfaceFilter::computeArea(PolyData mesh) const
{
  int nc = mesh->GetNumberOfCells();
  double totalArea = nc ? 0.0 : UNDEFINED;
  for (int i = 0; i < nc; i++)
    totalArea += vtkMeshQuality::TriangleArea(mesh->GetCell(i));
  
  return totalArea;
}

//----------------------------------------------------------------------------
double AppositionSurfaceFilter::computePerimeter() const
{
  double totalPerimeter = 0.0;
  try
  {
    m_ap->BuildLinks();
    
    // std::cout << "Points: " << mesh->GetNumberOfPoints() << std::endl;
    int num_of_cells = m_ap->GetNumberOfCells();
    
    vtkSmartPointer<vtkMutableUndirectedGraph> graph =
    vtkSmartPointer<vtkMutableUndirectedGraph>::New();
    
    vtkSmartPointer<vtkIdTypeArray> pedigree = vtkSmartPointer<vtkIdTypeArray>::New();
    graph->GetVertexData()->SetPedigreeIds(pedigree);
    
    for (vtkIdType cellId = 0; cellId < num_of_cells; cellId++)
    {
      vtkSmartPointer<vtkIdList> cellPointIds = vtkSmartPointer<vtkIdList>::New();
      m_ap->GetCellPoints(cellId, cellPointIds);
      for(vtkIdType i = 0; i < cellPointIds->GetNumberOfIds(); i++)
      {
        vtkIdType p1;
        vtkIdType p2;
        
        p1 = cellPointIds->GetId(i);
        if(i+1 == cellPointIds->GetNumberOfIds())
          p2 = cellPointIds->GetId(0);
        else
          p2 = cellPointIds->GetId(i+1);
        
        if (isPerimeter(cellId,p1,p2))
        {
          /**
           * VTK BUG: in Vtk5.10.1 without
           * clearing the loopuk table, the
           * pedigree->LoockupValue(p1) fails
           *
           * TODO: Check if ClearLookup is
           * needed in other version of VTK
           */
          pedigree->ClearLookup();
          
          vtkIdType i1 = graph->AddVertex(p1);
          vtkIdType i2 = graph->AddVertex(p2);
          
          graph->AddEdge(i1, i2);
        }
      }
    }
    
    // std::cout << "Vertices: " << graph->GetNumberOfVertices() << std::endl;
    // std::cout << "Edges: " << graph->GetNumberOfEdges() << std::endl;
    // std::cout << "Pedigree: " << pedigree->GetNumberOfTuples() << std::endl;
    
    vtkSmartPointer<vtkBoostConnectedComponents> connectedComponents =
    vtkSmartPointer<vtkBoostConnectedComponents>::New();
    connectedComponents->SetInput(graph);
    connectedComponents->Update();
    vtkGraph *outputGraph = connectedComponents->GetOutput();
    vtkIntArray *components = vtkIntArray::SafeDownCast( outputGraph->GetVertexData()->GetArray("component"));
    
    double *range = components->GetRange(0);
    // std::cout << "components: " << range[0] << " to "<< range[1] << std::endl;
    
    std::vector<double> perimeters;
    perimeters.resize(range[1]+1);
    
    vtkSmartPointer<vtkEdgeListIterator> edgeListIterator = vtkSmartPointer<vtkEdgeListIterator>::New();
    graph->GetEdges(edgeListIterator);
    
    while(edgeListIterator->HasNext()) {
      vtkEdgeType edge = edgeListIterator->Next();
      
      if (components->GetValue(edge.Source) != components->GetValue(edge.Target) ) {
        throw "ERROR: Edge between 2 disconnected component";
      }
      double x[3];
      double y[3];
      m_ap->GetPoint(pedigree->GetValue(edge.Source), x);
      m_ap->GetPoint(pedigree->GetValue(edge.Target), y);
      
      // std::cout << "X: " << x[0] << " " << x[1] << " " << x[2] << " Point: " << pedigree->GetValue(edge.Source) << std::endl;
      // std::cout << "Y: " << y[0] << " " << y[1] << " " << y[2] << " Point: " << pedigree->GetValue(edge.Target) << std::endl;
      
      double edge_length = sqrt(vtkMath::Distance2BetweenPoints(x, y));
      // std::cout << edge_length << std::endl;
      perimeters[components->GetValue(edge.Source)] += edge_length;
      //perimeters[components->GetValue(edge.Source)] += 1;
    }
    
    double max_per = 0.0;
    // std::cout << "myvector contains:";
    for (unsigned int i=0;i<perimeters.size();i++)
    {
      // std::cout << " " << perimeters[i];
      max_per = std::max(perimeters[i], max_per);
    }
    totalPerimeter = max_per;
  }catch (...)
  {
    // std::cerr << "Warning: Couldn't compute " << m_seg->data().toString().toStdString() << "'s Apposition Plane perimter" << std::endl;
    totalPerimeter = UNDEFINED;
  }
  
  return totalPerimeter;
}

//----------------------------------------------------------------------------
double AppositionSurfaceFilter::computeTortuosity() const
{
  vtkSmartPointer<vtkPolyData> projected_apposition_plane = 
  vtkSmartPointer<vtkPolyData>::New();
  projectPolyDataToPlane(m_templateOrigin, m_templateNormal, m_ap, projected_apposition_plane);
  
  double curved_area = computeArea(m_ap);
  double reference_area = computeArea(projected_apposition_plane);
  
  return 1 - (reference_area / curved_area);
}

//----------------------------------------------------------------------------
bool AppositionSurfaceFilter::isPerimeter(vtkIdType cellId, vtkIdType p1, vtkIdType p2) const
{
  vtkSmartPointer<vtkIdList> neighborCellIds = vtkSmartPointer<vtkIdList>::New();
  m_ap->GetCellEdgeNeighbors(cellId, p1, p2, neighborCellIds);
  
  return (neighborCellIds->GetNumberOfIds() == 0);
}

//----------------------------------------------------------------------------
QString AppositionSurfaceFilter::getOriginSegmentation()
{
  return m_origin;
}

//----------------------------------------------------------------------------
double AppositionSurfaceFilter::getArea()
{
  if (m_ap == NULL && !fetchCachePolyDatas())
    return UNDEFINED;

  bool updateNeeded = needUpdate(0);
  if (updateNeeded)
    update(0);

  if (UNDEFINED == m_area || updateNeeded)
    m_area = computeArea();

  return m_area;
}

//----------------------------------------------------------------------------
double AppositionSurfaceFilter::getPerimeter()
{
  if (m_ap == NULL && !fetchCachePolyDatas())
    return UNDEFINED;
  
  bool updateNeeded = needUpdate(0);
  if (updateNeeded)
    update(0);
  
  if (UNDEFINED == m_perimeter || updateNeeded)
    m_perimeter = computePerimeter();
  
  return m_perimeter;
}

//----------------------------------------------------------------------------
double AppositionSurfaceFilter::getTortuosity()
{
  if (m_ap == NULL && !fetchCachePolyDatas())
    return UNDEFINED;
  
  bool updateNeeded = needUpdate(0);
  if (updateNeeded)
    update(0);
  
  if (UNDEFINED == m_tortuosity || updateNeeded)
    m_tortuosity = computeTortuosity();
  
  return m_tortuosity;
}

//----------------------------------------------------------------------------
itkVolumeType::SpacingType AppositionSurfaceFilter::getOriginSpacing()
{
  return m_input->GetSpacing();
}

//----------------------------------------------------------------------------
itkVolumeType::RegionType AppositionSurfaceFilter::getOriginRegion()
{
  return m_input->GetLargestPossibleRegion();
}

//----------------------------------------------------------------------------
bool AppositionSurfaceFilter::fetchCachePolyDatas()
{
  if (m_alreadyFetchedData)
    return true;

  bool returnValue = false;

  QString nameAS = QString().number(m_cacheId) + QString("-AS.vtp");

  if (m_cacheDir.exists(nameAS))
  {
    QString fileName = m_cacheDir.absolutePath() + QDir::separator() + nameAS;
    vtkSmartPointer<vtkGenericDataObjectReader> polyASReader = vtkSmartPointer<vtkGenericDataObjectReader>::New();
    polyASReader->SetFileName(fileName.toStdString().c_str());
    polyASReader->SetReadAllFields(true);
    polyASReader->Update();

    m_ap = PolyData(polyASReader->GetPolyDataOutput());

    returnValue = true;
  }

  QString vectorName = QString().number(m_cacheId) + QString("-Vectors.dat");

  if (m_cacheDir.exists(vectorName))
  {
    QString fileName = m_cacheDir.absolutePath() + QDir::separator() + vectorName;

    QFile fileStream(fileName);
    fileStream.open(QIODevice::ReadOnly | QIODevice::Text);
    char buffer[1024];
    while (fileStream.readLine(buffer, sizeof(buffer)) > 0)
    {
      QString line(buffer);
      QStringList fields = line.split(SEP);

      if (fields[0] == QString("Template Origin"))
      {
        m_templateOrigin[0] = fields[1].toDouble();
        m_templateOrigin[1] = fields[2].toDouble();
        m_templateOrigin[2] = fields[3].toDouble();
      }

      if (fields[0] == QString("Template Normal"))
      {
        m_templateNormal[0] = fields[1].toDouble();
        m_templateNormal[1] = fields[2].toDouble();
        m_templateNormal[2] = fields[3].toDouble();
      }
    }
    returnValue = true;
  }

  if (returnValue) //FIXME
  {
    m_alreadyFetchedData = true;
    //m_outputs[0].volume->markAsModified(true);
  }

  return returnValue;
}

//----------------------------------------------------------------------------
bool AppositionSurfaceFilter::dumpSnapshot(Snapshot &snapshot)
{
  bool returnValue = false;

  if (NULL == m_ap)
    fetchCachePolyDatas();

  if (NULL != m_ap)
  {
    vtkSmartPointer<vtkGenericDataObjectWriter> polyWriter = vtkSmartPointer<vtkGenericDataObjectWriter>::New();
    polyWriter->SetInputConnection(m_ap->GetProducerPort());
    polyWriter->SetFileTypeToBinary();
    polyWriter->SetWriteToOutputString(true);
    polyWriter->Write();

    QByteArray polyArray(polyWriter->GetOutputString(), polyWriter->GetOutputStringLength());

    snapshot << SnapshotEntry(this->id() + QString("-AS.vtp"), polyArray);

    std::ostringstream vectorData;
    vectorData << std::string("Template Origin");
    vectorData << SEP << m_templateOrigin[0];
    vectorData << SEP << m_templateOrigin[1];
    vectorData << SEP << m_templateOrigin[2] << std::endl;

    vectorData << std::string("Template Normal");
    vectorData << SEP << m_templateNormal[0];
    vectorData << SEP << m_templateNormal[1];
    vectorData << SEP << m_templateNormal[2] << std::endl;

    snapshot << SnapshotEntry(this->id() + QString("-Vectors.dat"), vectorData.str().c_str());

    returnValue = true;
  }

  return (SegmentationFilter::dumpSnapshot(snapshot) || returnValue);
}

//----------------------------------------------------------------------------
void AppositionSurfaceFilter::inputModified()
{
  run();
  //m_outputs[0].volume->markAsModified(true);
}