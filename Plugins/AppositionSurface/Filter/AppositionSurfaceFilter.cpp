/*

    Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Plugin
#include "AppositionSurfaceFilter.h"

// ESPINA
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Data/Mesh/RawMesh.h>
#include <Core/Analysis/Data/Volumetric/RasterizedVolume.hxx>

// Qt
#include <QtGlobal>
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

using namespace ESPINA;

const QString SAS = "SAS";

const char * AppositionSurfaceFilter::MESH_NORMAL = "Normal";
const char * AppositionSurfaceFilter::MESH_ORIGIN = "Origin";

//----------------------------------------------------------------------------
AppositionSurfaceFilter::AppositionSurfaceFilter(InputSList inputs, Type type, SchedulerSPtr scheduler)
: Filter              {inputs, type, scheduler}
, m_resolution        {50}
, m_iterations        {10}
, m_converge          {true}
, m_ap                {nullptr}
, m_alreadyFetchedData{false}
, m_lastModifiedMesh  {0}
{
  setDescription(tr("Compute SAS"));
}

//----------------------------------------------------------------------------
AppositionSurfaceFilter::~AppositionSurfaceFilter()
{
  if (m_ap != nullptr)
  {
    disconnect(m_inputs[0]->output().get(), SIGNAL(modified()),
               this, SLOT(inputModified()));
  }
}

//----------------------------------------------------------------------------
bool AppositionSurfaceFilter::needUpdate() const
{
  return needUpdate(0);
}

//----------------------------------------------------------------------------
bool AppositionSurfaceFilter::needUpdate(Output::Id oId) const
{
  Q_ASSERT(oId == 0);

  bool update = true;

  if (!m_inputs.isEmpty() && m_alreadyFetchedData)
  {
    Q_ASSERT(m_inputs.size() == 1 && hasVolumetricData(m_inputs[0]->output()));

    auto inputVolume = readLockVolume(m_inputs[0]->output());

    if(inputVolume->isValid())
    {
      update = (m_lastModifiedMesh < inputVolume->lastModified());
    }
  }

  return update;
}

//----------------------------------------------------------------------------
void AppositionSurfaceFilter::execute(Output::Id oId)
{
  Q_ASSERT(oId == 0);
  execute();
}

//----------------------------------------------------------------------------
void AppositionSurfaceFilter::execute()
{
  reportProgress(0);

  if (!canExecute()) return;

  auto input = readLockVolume(m_inputs[0]->output())->itkImage();
  input->SetBufferedRegion(input->GetLargestPossibleRegion());

  itkVolumeType::SizeType padding;
  padding.Fill(1);
  PadFilterType::Pointer padder = PadFilterType::New();
  padder->SetInput(input);
  padder->SetPadLowerBound(padding);
  padder->SetPadUpperBound(padding);
  padder->SetConstant(0); // extend with black pixels
  padder->Update();

  auto paddedImage = padder->GetOutput();

  reportProgress(20);

  if (!canExecute()) return;

  auto region = paddedImage->GetLargestPossibleRegion();
  region.SetIndex(region.GetIndex() + padding);
  paddedImage->SetRegions(region);

  ItkToVtkFilterType::Pointer itk2vtk_filter = ItkToVtkFilterType::New();
  itk2vtk_filter->SetInput(paddedImage);
  itk2vtk_filter->Update();
  vtkSmartPointer<vtkImageData> vtk_padImage = itk2vtk_filter->GetOutput();

  double spacing[3];
  vtk_padImage->GetSpacing(spacing);

  //qDebug() << "Computing Distance Map";
  Points points = segmentationPoints(paddedImage);
  //qDebug() << points->GetNumberOfPoints() << " segmentation points");

  double corner[3], max[3], mid[3], min[3], size[3];
  OBBTreeType obbTree = OBBTreeType::New();
  obbTree->ComputeOBB(points, corner, max, mid, min, size);
  Points obbCorners = corners(corner, max, mid, min);
  DistanceMapType::Pointer distanceMap = computeDistanceMap(paddedImage, DISTANCESMOOTHSIGMAFACTOR);

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

  reportProgress(40);
  if (!canExecute()) return;

  vtkSmartPointer<vtkDoubleArray> originArray = vtkSmartPointer<vtkDoubleArray>::New();
  vtkSmartPointer<vtkDoubleArray> normalArray = vtkSmartPointer<vtkDoubleArray>::New();

  originArray->SetName(MESH_ORIGIN);
  originArray->SetNumberOfValues(3);
  normalArray->SetName(MESH_NORMAL);
  normalArray->SetNumberOfValues(3);

  double v[3], displacement[3];
  for(auto i: {0,1,2})
  {
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

  reportProgress(60);
  if (!canExecute()) return;

  vtkSmartPointer<vtkImageData> gradientVectorGrid = vtkSmartPointer<vtkImageData>::New();
  vectorImageToVTKImage(gradientFilter->GetOutput(), gradientVectorGrid);
  //gradientVectorGrid->Print(std::cout);

  projectVectors(gradientVectorGrid, normal);

  GridTransform grid_transform = GridTransform::New();
  grid_transform->SetDisplacementGridData(gradientVectorGrid);
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
  for (int i =0; i <= numIterations; i++)
  {
    transformer->SetInputData(auxPlane);
    transformer->Modified();
    transformer->Update();

    auxPlane->DeepCopy(transformer->GetOutput());
    if (m_converge)
    {
      if (hasConverged(auxPlane->GetPoints(), pointsList, thresholdError))
      {
        //   qDebug() << "Total iterations: " << i << std::endl;
        break;
      }
      else
      {
        pointsList.push_front(auxPlane->GetPoints());
        if (pointsList.size() > MAXSAVEDSTATUSES)
          pointsList.pop_back();
      }
    }
  }
  pointsList.clear();

  reportProgress(80);
  if (!canExecute()) return;

  PolyData clippedPlane = clipPlane(transformer->GetOutput(), vtk_padImage);
  //ESPINA_DEBUG(clippedPlane->GetNumberOfCells() << " cells after clip");

  /**
   * Traslate
   */
  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  transform->Translate (-spacing[0]*padding[0], -spacing[1]*padding[0], -spacing[2]*padding[0]);
  transformFilter->SetTransform(transform);
  transformFilter->SetInputData(clippedPlane);
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

  auto outpuSpacing = ToNmVector3<itkVolumeType>(input->GetSpacing());
  auto meshOutput   = std::make_shared<RawMesh>(m_ap, outpuSpacing);

  m_lastModifiedMesh = meshOutput->lastModified();

  double meshVTKBounds[6];
  m_ap->GetBounds(meshVTKBounds);

  Bounds meshBounds{meshVTKBounds};

  if(!m_outputs.contains(0))
  {
    m_outputs[0] = std::make_shared<Output>(this, 0, outpuSpacing);
  }

  m_outputs[0]->setData(meshOutput);
  auto volume = std::make_shared<RasterizedVolume<itkVolumeType>>(m_outputs[0].get(), meshBounds, outpuSpacing);
  volume->rasterize();
  m_outputs[0]->setData(volume);
  m_outputs[0]->setSpacing(outpuSpacing);

  reportProgress(100);
}

//----------------------------------------------------------------------------
AppositionSurfaceFilter::Points AppositionSurfaceFilter::segmentationPoints(const itkVolumeType::Pointer &seg) const
{
  itkVolumeType::PointType   origin  = seg->GetOrigin();
  itkVolumeType::SpacingType spacing = seg->GetSpacing();

  Points points = Points::New();
  points->SetDataTypeToDouble();

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
  points->Squeeze();
  points->Modified();

  return points;
}

//----------------------------------------------------------------------------
AppositionSurfaceFilter::Points AppositionSurfaceFilter::corners(const double corner[3], const double max[3], const double mid[3], const double min[3]) const
{
  Points points = Points::New();
  points->SetDataTypeToDouble();

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

  points->Squeeze();
  points->Modified();

  return points;
}

//----------------------------------------------------------------------------
AppositionSurfaceFilter::DistanceMapType::Pointer AppositionSurfaceFilter::computeDistanceMap(const itkVolumeType::Pointer &volume, const float sigma) const
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

  auto smoothingRecursiveGaussianImageFilter = SmoothingFilterType::New();
  smoothingRecursiveGaussianImageFilter->SetSigma(sigma * max_distance);
  smoothingRecursiveGaussianImageFilter->SetInput(smdm_filter->GetOutput());

  auto regionSize = smdm_filter->GetOutput()->GetLargestPossibleRegion().GetSize();
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
void AppositionSurfaceFilter::maxDistancePoint(const DistanceMapType::Pointer &map,
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
void AppositionSurfaceFilter::computeResolution(const double *max, const double *mid, const double *spacing, int & xResolution, int & yResolution) const
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
void AppositionSurfaceFilter::project(const double *A, const double *B, double *projection) const
{
  double scale = vtkMath::Dot(A,B)/pow(vtkMath::Norm(B), 2);
  for(unsigned int i = 0; i < 3; i++)
    projection[i] = scale * B[i];
}

//----------------------------------------------------------------------------
void AppositionSurfaceFilter::vectorImageToVTKImage(const CovariantVectorImageType::Pointer vectorImage,
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
void AppositionSurfaceFilter::projectVectors(vtkImageData* vectors_image, double *unitary) const
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
void AppositionSurfaceFilter::computeIterationLimits(const double *min, const double *spacing, int & iterations, double & thresholdError) const
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
bool AppositionSurfaceFilter::hasConverged(vtkPoints * lastPlanePoints, PointsListType & pointsList, double threshold) const
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
AppositionSurfaceFilter::PolyData AppositionSurfaceFilter::clipPlane(vtkPolyData *plane, vtkImageData* image) const
{
  vtkSmartPointer<vtkImplicitVolume> implicitVolFilter = vtkSmartPointer<vtkImplicitVolume>::New();
  implicitVolFilter->SetVolume(image);
  implicitVolFilter->SetOutValue(0);

  double inValue = image->GetScalarRange()[1];

  vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
  clipper->SetClipFunction(implicitVolFilter);
  clipper->SetInputData(plane);
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
  triangle_filter->SetInputData(plane);
  triangle_filter->Update();

  vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
  normals->SetInputData(triangle_filter->GetOutput());
  normals->SplittingOff();
  normals->Update();

  PolyData resultPlane = PolyData::New();
  resultPlane->DeepCopy(normals->GetOutput());

  return resultPlane;
}

//----------------------------------------------------------------------------
void AppositionSurfaceFilter::inputModified()
{
  run();
}
