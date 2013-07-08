/*
 * AppositionSurfaceFilter.cpp
 *
 *  Created on: Jan 18, 2013
 *      Author: F�lix de las Pozas �lvarez
 */

// plugin
#include "AppositionSurfaceFilter.h"
#include <Core/ASMeshProxy.h>
#include <Core/ASVolumeProxy.h>
#include <Core/Model/Segmentation.h>
#include <Core/OutputRepresentations/MeshType.h>
#include <Core/OutputRepresentations/RawMesh.h>
#include <Core/OutputRepresentations/RasterizedVolume.h>
#include <GUI/Representations/SliceRepresentation.h>
#include <GUI/Representations/SimpleMeshRepresentation.h>

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
#include <vtkGenericDataObjectReader.h>
#include <vtkPlane.h>
#include <vtkDoubleArray.h>


const double UNDEFINED = -1.;
const char SEP = ',';

using namespace EspINA;

const QString AppositionSurfaceFilter::INPUTLINK = "Input";
const QString AppositionSurfaceFilter::SAS = "SAS";

const char * AppositionSurfaceFilter::MESH_NORMAL = "Normal";
const char * AppositionSurfaceFilter::MESH_ORIGIN = "Origin";

const ModelItem::ArgumentId AppositionSurfaceFilter::ORIGIN = "Origin Segmentation";

//----------------------------------------------------------------------------
AppositionSurfaceFilter::AppositionSurfaceFilter(NamedInputs inputs, Arguments args, FilterType type)
: BasicSegmentationFilter(inputs, args, type)
, m_resolution(50)
, m_iterations(10)
, m_converge(true)
, m_ap(NULL)
, m_originSegmentation(NULL)
, m_origin(args[ORIGIN])
, m_alreadyFetchedData(false)
{
  if (m_origin == QString())
    m_origin = QString("Unspecified origin");
}

//----------------------------------------------------------------------------
AppositionSurfaceFilter::~AppositionSurfaceFilter()
{
  if (m_ap != NULL)
  {
    disconnect(m_originSegmentation, SIGNAL(outputModified()),
               this, SLOT(inputModified()));
    m_ap->Delete();
  }
}

//----------------------------------------------------------------------------
SegmentationRepresentationSPtr AppositionSurfaceFilter::createRepresentationProxy(FilterOutputId id, const FilterOutput::OutputRepresentationName &type)
{
  SegmentationRepresentationSPtr proxy;

  Q_ASSERT(m_outputs.contains(id));
  Q_ASSERT( NULL == m_outputs[id]->representation(type));

  if (SegmentationVolume::TYPE == type)
    proxy = VolumeProxySPtr(new ASVolumeProxy());
  else if (MeshRepresentation::TYPE == type)
    proxy = MeshProxySPtr(new ASMeshProxy());
  else
    Q_ASSERT(false);

  m_outputs[id]->setRepresentation(type, proxy);

  return proxy;
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

    SegmentationVolumeSPtr inputVolume = segmentationVolume(m_inputs[0] );
    MeshRepresentationSPtr outputMesh  = meshRepresentation(m_outputs[0]);

    update = outputMesh->timeStamp() < inputVolume->timeStamp();
  }

  return update;
}

QString AppositionSurfaceFilter::getOriginSegmentation()
{
  return m_originSegmentation->data().toString();
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

    ModelItemSList items = segFilter->relatedItems(EspINA::RELATION_OUT, Filter::CREATELINK);
    while(!m_originSegmentation && i < items.size())
    {
      SegmentationPtr segmentation = segmentationPtr(items[i].get());
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

  m_input = segmentationVolume(m_originSegmentation->output())->toITK();

  ModelItemSList items = relatedItems(EspINA::RELATION_OUT, Filter::CREATELINK);
  if (items.size() == 1)
  {
    SegmentationPtr segmentation = segmentationPtr(items.first().get());
    segmentation->setInputSegmentationDependent(true);
    items.first()->notifyModification();
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
  qDebug() << "Compute AS";
  Q_ASSERT(0 == oId);

  const QString        namedInput = m_args[Filter::INPUTS];
  QStringList          list       = namedInput.split(QChar('_'));
  const FilterOutputId outputId   = list[1].toInt();

  FilterSPtr segFilter = m_namedInputs[AppositionSurfaceFilter::INPUTLINK];

  m_input = segmentationVolume(segFilter->output(outputId))->toITK();

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

  vtkSmartPointer<vtkDoubleArray> originArray = vtkSmartPointer<vtkDoubleArray>::New();
  vtkSmartPointer<vtkDoubleArray> normalArray = vtkSmartPointer<vtkDoubleArray>::New();

  originArray->SetName(MESH_ORIGIN);
  originArray->SetNumberOfValues(3);
  normalArray->SetName(MESH_NORMAL);
  normalArray->SetNumberOfValues(3);

  double v[3], displacement[3];
  for (int i = 0; i < 3; i++) {
    v[i] = avgMaxDistPoint[i] - obbCorners->GetPoint(0)[i];
    originArray->SetValue(i, obbCorners->GetPoint(0)[i]);
    normalArray->SetValue(i, normal[i]);
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
  m_ap->GetPointData()->AddArray(originArray);
  m_ap->GetPointData()->AddArray(normalArray);
  m_ap->Modified();

  RawMeshSPtr meshRepresentation(new RawMesh(m_ap, m_input->GetSpacing()));

  SegmentationRepresentationSList repList;
  repList << meshRepresentation;
  repList << RasterizedVolumeSPtr(new RasterizedVolume(meshRepresentation));

  addOutputRepresentations(0, repList);

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
void AppositionSurfaceFilter::inputModified()
{
  run();
  //m_outputs[0].volume->markAsModified(true);
}