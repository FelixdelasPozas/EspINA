#include "AppositionPlaneExtension.h"

// EspINA
#include <common/model/Segmentation.h>

// VTK
#include <vtkMath.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkTriangleFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkClipPolyData.h>
#include <vtkMeshQuality.h>
#include <vtkImplicitVolume.h>
#include <vtkPointData.h>
#include <vtkMutableUndirectedGraph.h>
#include <vtkEdgeListIterator.h>
#include <vtkBoostConnectedComponents.h>

// Qt
#include <QDebug>
#include <QApplication>



///-----------------------------------------------------------------------
/// APPOSITION PLANE EXTENSION-
///-----------------------------------------------------------------------
/// Information Provided:
/// - Apposition Plane Area
/// - Apposition Plane Perimeter

const ModelItemExtension::ExtId   AppositionPlaneExtension::ID
= "AppositionPlaneExtension";

const ModelItemExtension::InfoTag AppositionPlaneExtension::AREA      
= "AP Area";
const ModelItemExtension::InfoTag AppositionPlaneExtension::PERIMETER 
= "AP Perimeter";

const double UNDEFINED = -1.;

//------------------------------------------------------------------------
AppositionPlaneExtension::AppositionPlaneExtension()
: m_resolution(50)
, m_iterations(10)
, m_converge  (true)
, m_area      (UNDEFINED)
, m_perimeter (UNDEFINED)
{
  m_ap = PolyData::New();
  //m_availableRepresentations << AppositionPlaneRepresentation::ID;
  m_availableInformations << AREA << PERIMETER;
}

//------------------------------------------------------------------------
AppositionPlaneExtension::~AppositionPlaneExtension()
{
}

//------------------------------------------------------------------------
ModelItemExtension::ExtId AppositionPlaneExtension::id()
{
  return ID;
}

//------------------------------------------------------------------------
QVariant AppositionPlaneExtension::information(ModelItemExtension::InfoTag tag) const
{
  if (!m_init)
    return QVariant();

  if (updateAppositionPlane() || UNDEFINED == m_area || UNDEFINED == m_perimeter)
  {
    qDebug() << "Update Apposition Plane";
    m_area = computeArea();
    m_perimeter = computePerimeter();
  }

  if (AREA == tag)
    return m_area;
  if (PERIMETER == tag)
    return m_perimeter;

  qWarning() << ID << ":"  << tag << " is not provided";
  Q_ASSERT(false);
  return QVariant();
}

//------------------------------------------------------------------------
SegmentationRepresentation* AppositionPlaneExtension::representation(QString rep)
{
  //   if (rep == AppositionPlaneRepresentation::ID)
  //     return m_planeRep;
  //
  qWarning() << ID << ":" << rep << " is not provided";
  Q_ASSERT(false);
  return NULL;
}

//------------------------------------------------------------------------
void AppositionPlaneExtension::initialize(ModelItem::Arguments args)
{
  m_init = true;
}

//------------------------------------------------------------------------
SegmentationExtension* AppositionPlaneExtension::clone()
{
  return new AppositionPlaneExtension();
}

//------------------------------------------------------------------------
bool AppositionPlaneExtension::updateAppositionPlane() const
{
  if (m_seg->itkVolume()->GetTimeStamp() <= m_lastUpdate)
    return false;

  QApplication::setOverrideCursor(Qt::WaitCursor);
  //qDebug() << "Updating Apposition Plane:" << m_seg->data().toString();
  //qDebug() << "Padding Image";
  EspinaVolume::SizeType bounds;
  bounds[0] = bounds[1] = bounds[2] = 1;
  PadFilterType::Pointer padder = PadFilterType::New();
  padder->SetInput(m_seg->itkVolume());
  padder->SetPadLowerBound(bounds);
  padder->SetPadUpperBound(bounds);
  padder->SetConstant(0); // extend with black pixels
  padder->Update();
  EspinaVolume::Pointer padImage = padder->GetOutput();

  EspinaVolume::RegionType  region = padImage->GetLargestPossibleRegion();
  EspinaVolume::SizeType imageSize = region.GetSize();
  region.SetIndex(region.GetIndex() + bounds);
  padImage->SetRegions(region);

  ItkToVtkFilterType::Pointer itk2vtk_filter = ItkToVtkFilterType::New();
  itk2vtk_filter->SetInput(padImage);
  itk2vtk_filter->Update();
  vtkSmartPointer<vtkImageData> vtk_padImage = itk2vtk_filter->GetOutput();

  //qDebug() << "Computing Distance Map";
  Points points = segmentationPoints(padImage);
  //qDebug() << points->GetNumberOfPoints() << " segmentation points");

  double corner[3], max[3], mid[3], min[3], size[3];
  OBBTreeType obbTree = OBBTreeType::New();
  obbTree->ComputeOBB(points, corner, max, mid, min, size);
  Points obbCorners = corners(corner, max, mid, min);
  DistanceMapType::Pointer distanceMap = computeDistanceMap(padImage);

  //   qDebug() << "Build and move the plane to Avg Max Distance";
  Points maxPoints = Points::New();
  double avgMaxDistPoint[3];
  maxDistancePoint(distanceMap, maxPoints, avgMaxDistPoint);

  PlaneSourceType planeSource = PlaneSourceType::New();
  planeSource->SetOrigin(obbCorners->GetPoint(0));
  planeSource->SetPoint1(obbCorners->GetPoint(1));
  planeSource->SetPoint2(obbCorners->GetPoint(2));
  planeSource->SetResolution(m_resolution, m_resolution);
  planeSource->Update();

  //   qDebug() << "Create Path with point + min and update min\n"
  //               "Fill vtkthinPlatesplineTransform";

  double *normal = planeSource->GetNormal();
  vtkMath::Normalize(normal);

  double v[3], displacement[3];
  for (int i = 0; i < 3; i++) {
    v[i] = avgMaxDistPoint[i] - obbCorners->GetPoint(0)[i];
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
  gradientFilter->Update();

  vtkSmartPointer<vtkImageData> gradientVectorGrid =
  vtkSmartPointer<vtkImageData>::New();
  vectorImageToVTKImage(gradientFilter->GetOutput(), gradientVectorGrid);
  //gradientVectorGrid->Print(std::cout);

  projectVectors(gradientVectorGrid, normal);

  GridTransform grid_transform = GridTransform::New();
  grid_transform->SetDisplacementGrid(gradientVectorGrid);
  grid_transform->SetInterpolationModeToCubic();

  TransformPolyDataFilter transformer = TransformPolyDataFilter::New();
  PolyData auxPlane = sourcePlane;

  int numIterations = m_iterations;
  if (m_converge)
  {
    double *spacing = vtk_padImage->GetSpacing();
    double min_in_pixels[3] = {0,0,0};
    for (unsigned int i=0; i < 3; i++) {
      min_in_pixels[i] = min[i] / spacing[i];
    }
    numIterations = std::max( 1, int(floor(sqrt(vtkMath::Norm(min_in_pixels)))));
  }

  //   qDebug() << "Number of iterations:" << m_iterations;

  transformer->SetTransform(grid_transform);
  for (int i =0; i <= numIterations; i++) {
    transformer->SetInput(auxPlane);
    transformer->Modified();
    transformer->Update();

    auxPlane->DeepCopy(transformer->GetOutput());
  }

  PolyData clippedPlane = clipPlane(transformer->GetOutput(), vtk_padImage);
  //ESPINA_DEBUG(clippedPlane->GetNumberOfCells() << " cells after clip");

  //   qDebug() << "Correct Plane's visualization and cell area's computation";
  vtkSmartPointer<vtkTriangleFilter> triangle_filter =
  vtkSmartPointer<vtkTriangleFilter>::New();
  triangle_filter->SetInput(clippedPlane);
  triangle_filter->Update();

  vtkSmartPointer<vtkPolyDataNormals> normals =
  vtkSmartPointer<vtkPolyDataNormals>::New();
  normals->SetInput(triangle_filter->GetOutput());
  normals->SplittingOff();
  normals->Update();

  vtkSmartPointer<vtkPolyData> appositionPlane = normals->GetOutput();
  //ESPINA_DEBUG(appositionPlane->GetNumberOfCells() << " cells in apppositionPlane");

  //   qDebug() << "Create Mesh";
  //m_ap->DeepCopy(appositionPlane);
  m_ap->Initialize();
  m_ap->SetPoints(appositionPlane->GetPoints());
  m_ap->SetPolys(appositionPlane->GetPolys());
  m_ap->SetLines(appositionPlane->GetLines());
  m_ap->Modified();

  m_lastUpdate = m_seg->itkVolume()->GetTimeStamp();
  QApplication::restoreOverrideCursor();

  return true;
}

//------------------------------------------------------------------------
AppositionPlaneExtension::PolyData AppositionPlaneExtension::clipPlane(AppositionPlaneExtension::PolyData plane, vtkImageData* image) const
{
  vtkSmartPointer<vtkImplicitVolume> implicitVolFilter =
  vtkSmartPointer<vtkImplicitVolume>::New();
  implicitVolFilter->SetVolume(image);
  implicitVolFilter->SetOutValue(0);

  vtkSmartPointer<vtkClipPolyData> clipper =
  vtkSmartPointer<vtkClipPolyData>::New();
  clipper->SetClipFunction(implicitVolFilter);
  clipper->SetInput(plane);
  clipper->SetValue(0);
  clipper->Update();

  PolyData clippedPlane = PolyData::New();
  clippedPlane->DeepCopy(clipper->GetOutput());

  return clippedPlane;
}

//------------------------------------------------------------------------
AppositionPlaneExtension::DistanceMapType::Pointer AppositionPlaneExtension::computeDistanceMap(EspinaVolume::Pointer volume) const
{
  SDDistanceMapFilterType::Pointer sddm_filter = SDDistanceMapFilterType::New();
  sddm_filter->InsideIsPositiveOn();
  sddm_filter->UseImageSpacingOn();
  sddm_filter->SquaredDistanceOn();
  sddm_filter->SetInput(volume);
  sddm_filter->Update();

  return sddm_filter->GetOutput();
}

//------------------------------------------------------------------------
AppositionPlaneExtension::Points AppositionPlaneExtension::corners(double corner[3], double max[3], double mid[3], double min[3]) const
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

//------------------------------------------------------------------------
void AppositionPlaneExtension::maxDistancePoint(itk::Image< AppositionPlaneExtension::DistanceType, 3 >::Pointer map, AppositionPlaneExtension::Points points, double avgMaxDistPoint[3]) const
{
  DistanceType maxDist = 0;
  DistanceMapType::PointType origin = map->GetOrigin();
  DistanceMapType::SpacingType spacing = map->GetSpacing();

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

//------------------------------------------------------------------------
void AppositionPlaneExtension::project(const double* A, const double* B, double* Projection) const
{
  double scale = vtkMath::Dot(A,B)/pow(vtkMath::Norm(B), 2);
  for(unsigned int i = 0; i < 3; i++)
    Projection[i] = scale * B[i];
}

//------------------------------------------------------------------------
void AppositionPlaneExtension::projectVectors(vtkImageData* vectors_image, double* unitary) const
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
AppositionPlaneExtension::Points AppositionPlaneExtension::segmentationPoints(EspinaVolume::Pointer seg) const
{
  EspinaVolume::PointType   origin  = seg->GetOrigin();
  EspinaVolume::SpacingType spacing = seg->GetSpacing();

  Points points = Points::New();

  VoxelIterator it(seg, seg->GetLargestPossibleRegion());
  while (!it.IsAtEnd())
  {
    EspinaVolume::PixelType val = it.Get();
    EspinaVolume::IndexType index = it.GetIndex();
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

//------------------------------------------------------------------------
void AppositionPlaneExtension::vectorImageToVTKImage(itk::Image< AppositionPlaneExtension::CovariantVectorType, 3 >::Pointer vectorImage, vtkImageData* image) const
{

  CovariantVectorImageType::PointType origin = vectorImage->GetOrigin();
  // ESPINA_DEBUG("CovariantVectorMap Origin " << origin[0] << " " << origin[1] << " " << origin[2]);
  CovariantVectorImageType::RegionType region = vectorImage->GetLargestPossibleRegion();
  //   region.Print(std::cout);
  CovariantVectorImageType::SpacingType spacing = vectorImage->GetSpacing();
  CovariantVectorImageType::SizeType imageSize = region.GetSize();
  CovariantVectorImageType::IndexType originIndex = region.GetIndex();

  image->SetOrigin(origin[0], origin[1], origin[2]);
  image->SetExtent(originIndex[0], originIndex[0] + imageSize[0] - 1,
                   originIndex[1], originIndex[1] + imageSize[1] - 1,
                   originIndex[2], originIndex[2] + imageSize[2] - 1);
  image->SetSpacing(spacing[0], spacing[1], spacing[2]);
  //   image->SetSpacing(vectorImage->GetSpacing()[0], vectorImage->GetSpacing()[1], vectorImage->GetSpacing()[2]);

  //   image->Print(std::cout);
  vtkSmartPointer<vtkFloatArray> vectors = vtkSmartPointer<vtkFloatArray>::New();
  vectors->SetNumberOfComponents(3);
  vectors->SetNumberOfTuples(imageSize[0] * imageSize[1] * imageSize[2]);
  vectors->SetName("GradientVectors");

  // #ifdef DEBUG_AP_FILES
  //   std::ofstream covarianceFile;
  //   covarianceFile.open("decCovarianceFile");
  // #endif

  int counter = 0;
  for(unsigned int k = originIndex[2]; k < originIndex[2] + imageSize[2]; k++)
    for(unsigned int j = originIndex[1]; j < originIndex[1] + imageSize[1]; j++)
      for(unsigned int i = originIndex[0]; i < originIndex[0] + imageSize[0]; i++)
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
        // 	covarianceFile << val[0] << " " << val[1] << " " << val[2] << std::endl;
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

//------------------------------------------------------------------------
double AppositionPlaneExtension::computeArea() const
{
  int nc = m_ap->GetNumberOfCells();
  double totalArea = nc?0.0:UNDEFINED;
  for (int i = 0; i < nc; i++)
    totalArea += vtkMeshQuality::TriangleArea(m_ap->GetCell(i));

  return totalArea;
}

//------------------------------------------------------------------------
double AppositionPlaneExtension::computePerimeter() const
{
  double totalPerimeter = 0.0;
  try
  {
    m_ap->BuildLinks();

    //  std::cout << "Points: " << mesh->GetNumberOfPoints() << std::endl;
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
          vtkIdType i1 = graph->AddVertex(p1);
          vtkIdType i2 = graph->AddVertex(p2);

          graph->AddEdge(i1, i2);
        }
      }
    }

    //  std::cout << "Vertices: " << graph->GetNumberOfVertices() << std::endl;
    //  std::cout << "Edges: " << graph->GetNumberOfEdges() << std::endl;
    //  std::cout << "Pedigree: " << pedigree->GetNumberOfTuples() << std::endl;

    vtkSmartPointer<vtkBoostConnectedComponents> connectedComponents =
    vtkSmartPointer<vtkBoostConnectedComponents>::New();
    connectedComponents->SetInput(graph);
    connectedComponents->Update();
    vtkGraph *outputGraph = connectedComponents->GetOutput();
    vtkIntArray *components = vtkIntArray::SafeDownCast( outputGraph->GetVertexData()->GetArray("component"));

    double *range = components->GetRange(0);
    //  std::cout << "components: " << range[0] << " to "<< range[1] << std::endl;

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

      //      std::cout << "X: " << x[0] << " " << x[1] << " " << x[2] << " Point: " << pedigree->GetValue(edge.Source) << std::endl;
      //      std::cout << "Y: " << y[0] << " " << y[1] << " " << y[2] << " Point: " << pedigree->GetValue(edge.Target) << std::endl;

      double edge_length = sqrt(vtkMath::Distance2BetweenPoints(x, y));
      //      std::cout << edge_length << std::endl;
      perimeters[components->GetValue(edge.Source)] += edge_length;
      //perimeters[components->GetValue(edge.Source)] += 1;
    }

    double sum_per = 0.0;
    double max_per = 0.0;
    //  std::cout << "myvector contains:";
    for (unsigned int i=0;i<perimeters.size();i++)
    {
      //      std::cout << " " << perimeters[i];
      sum_per += perimeters[i];
      max_per = std::max(perimeters[i], max_per);
    }
    totalPerimeter = max_per;
  }catch (...)
  {
    std::cerr << "Couldn't compute perimter" << std::endl;
    totalPerimeter = UNDEFINED;
  }

  return totalPerimeter;
}

//------------------------------------------------------------------------
bool AppositionPlaneExtension::isPerimeter(vtkIdType cellId, vtkIdType p1, vtkIdType p2) const
{
  vtkSmartPointer<vtkIdList> neighborCellIds = vtkSmartPointer<vtkIdList>::New();
  m_ap->GetCellEdgeNeighbors(cellId, p1, p2, neighborCellIds);

  return (neighborCellIds->GetNumberOfIds() == 0);
}