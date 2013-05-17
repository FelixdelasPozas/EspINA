/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "AppositionSurfaceExtension.h"
#include <Filter/AppositionSurfaceFilter.h>

// EspINA
#include <Core/Model/Segmentation.h>
#include <Core/Model/EspinaModel.h>
#include <Core/EspinaSettings.h>
#include <Core/Outputs/MeshType.h>
#include <vtkMeshQuality.h>
#include <vtkPolyDataNormals.h>
#include <vtkMutableUndirectedGraph.h>
#include <vtkDataSetAttributes.h>
#include <vtkEdgeListIterator.h>
#include <vtkPointData.h>
#include <vtkIdTypeArray.h>
#include <vtkIdList.h>
#include <vtkMath.h>
#include <vtkPlane.h>
#include <vtkDoubleArray.h>
#include <vtkBoostConnectedComponents.h>

// boost
#include <boost/concept_check.hpp>

using namespace EspINA;

///-----------------------------------------------------------------------
/// APPOSITION SURFACE EXTENSION-
///-----------------------------------------------------------------------
/// Information Provided:
/// - Sinaptic Apposition Surface Area
/// - Sinaptic Apposition Surface Perimeter
/// - Sinaptic Apposition Surface Tortuosity
/// - Synapse from which the Sinaptic Apposition Surface was obtained

const ModelItem::ExtId   AppositionSurfaceExtension::ID = "AppositionSurfaceExtension";

const Segmentation::InfoTag AppositionSurfaceExtension::AREA                   = "Area";
const Segmentation::InfoTag AppositionSurfaceExtension::PERIMETER              = "Perimeter";
const Segmentation::InfoTag AppositionSurfaceExtension::TORTUOSITY             = "Tortuosity";
const Segmentation::InfoTag AppositionSurfaceExtension::SYNAPSE                = "Synapse";
const Segmentation::InfoTag AppositionSurfaceExtension::COMPUTATION_TIME       = "Computation Time";
const Segmentation::InfoTag AppositionSurfaceExtension::MEAN_GAUSS_CURVATURE   = "Mean Gauss Curvature";
const Segmentation::InfoTag AppositionSurfaceExtension::STD_DEV_GAUS_CURVATURE = "Std Deviation Gauss Curvature";
const Segmentation::InfoTag AppositionSurfaceExtension::MEAN_MEAN_CURVATURE    = "Mean Mean Curvature";
const Segmentation::InfoTag AppositionSurfaceExtension::STD_DEV_MEAN_CURVATURE = "Std Deviation Mean Curvature";
const Segmentation::InfoTag AppositionSurfaceExtension::MEAN_MIN_CURVATURE     = "Mean Min Curvature";
const Segmentation::InfoTag AppositionSurfaceExtension::STD_DEV_MIN_CURVATURE  = "Std Deviation Mix Curvature";
const Segmentation::InfoTag AppositionSurfaceExtension::MEAN_MAX_CURVATURE     = "Mean Max Curvature";
const Segmentation::InfoTag AppositionSurfaceExtension::STD_DEV_MAX_CURVATURE  = "Std Deviation Max Curvature";

const QString AppositionSurfaceExtension::EXTENSION_FILE = "AppositionSurfaceExtension/AppositionSurfaceExtension.csv";

const int CURRENT_EXTENSION_VERSION = 2;
const std::string OLD_EXTENSION_VERSION = AppositionSurfaceExtension::ID.toStdString() + " 1.0\n";
const char SEP = ',';

AppositionSurfaceExtension::ExtensionCache AppositionSurfaceExtension::s_cache;

const double UNDEFINED = -1.;

//------------------------------------------------------------------------
AppositionSurfaceExtension::ExtensionData::ExtensionData()
: Area(UNDEFINED)
, Perimeter(UNDEFINED)
, Tortuosity(UNDEFINED)
, SynapticSource(QString())
, MeanGaussCurvature(UNDEFINED)
, StdDevGaussCurvature(UNDEFINED)
, MeanMeanCurvature(UNDEFINED)
, StdDevMeanCurvature(UNDEFINED)
, MeanMinCurvature(UNDEFINED)
, StdDevMinCurvature(UNDEFINED)
, MeanMaxCurvature(UNDEFINED)
, StdDevMaxCurvature(UNDEFINED)
{
}

//------------------------------------------------------------------------
AppositionSurfaceExtension::AppositionSurfaceExtension()
{
}

//------------------------------------------------------------------------
AppositionSurfaceExtension::~AppositionSurfaceExtension()
{
  invalidate();
}

//------------------------------------------------------------------------
ModelItem::ExtId AppositionSurfaceExtension::id()
{
  return ID;
}

//------------------------------------------------------------------------
Segmentation::InfoTagList AppositionSurfaceExtension::availableInformations() const
{
  Segmentation::InfoTagList tags;

  tags << AREA
       << PERIMETER
       << TORTUOSITY
       << SYNAPSE
       << MEAN_GAUSS_CURVATURE
       << STD_DEV_GAUS_CURVATURE
       << MEAN_MEAN_CURVATURE
       << STD_DEV_MEAN_CURVATURE
       << MEAN_MIN_CURVATURE
       << STD_DEV_MIN_CURVATURE
       << MEAN_MAX_CURVATURE
       << STD_DEV_MAX_CURVATURE;

  return tags;
}

//------------------------------------------------------------------------
bool AppositionSurfaceExtension::validTaxonomy(const QString &qualifiedName) const
{
  return qualifiedName.contains(tr("SAS"));
}

//------------------------------------------------------------------------
void AppositionSurfaceExtension::setSegmentation(SegmentationPtr seg)
{
  EspINA::Segmentation::Information::setSegmentation(seg);

  connect(m_segmentation, SIGNAL(outputModified()),
          this, SLOT(invalidate()));

  if (m_segmentation->outputIsModified())
    invalidate();
  else
    initialize();
}

//------------------------------------------------------------------------
QVariant AppositionSurfaceExtension::information(const Segmentation::InfoTag &tag)
{
  QString fullTaxonomy = m_segmentation->taxonomy()->qualifiedName();
  if (!fullTaxonomy.startsWith(SAS+"/") && (fullTaxonomy.compare(SAS) != 0))
    return QVariant();

  bool cached = s_cache.isCached(m_segmentation);

  if (!cached)
  {
    computeInformation();
  }

  ExtensionData &data = s_cache[m_segmentation].Data;

  if (AREA == tag)
    return data.Area;
  if (PERIMETER == tag)
    return data.Perimeter;
  if (TORTUOSITY == tag)
    return data.Tortuosity;
  if (SYNAPSE == tag)
    return data.SynapticSource;

  qWarning() << ID << ":"  << tag << " is not provided";
  Q_ASSERT(false);
  return QVariant();
}

//------------------------------------------------------------------------
Segmentation::InformationExtension AppositionSurfaceExtension::clone()
{
  return new AppositionSurfaceExtension();
}

//------------------------------------------------------------------------
bool AppositionSurfaceExtension::isCacheFile(const QString &file) const
{
  return EXTENSION_FILE == file;
}

//------------------------------------------------------------------------
void AppositionSurfaceExtension::initialize()
{

}

//------------------------------------------------------------------------
void AppositionSurfaceExtension::invalidate(SegmentationPtr segmentation)
{
  s_cache.remove(m_segmentation);
}

//------------------------------------------------------------------------
void AppositionSurfaceExtension::loadCache(QuaZipFile &file, const QDir &tmpDir, IEspinaModel *model)
{
  QString header(file.readLine());

  int cachedVersion = header.toStdString() == OLD_EXTENSION_VERSION ? 1 : header.toInt();
  Q_ASSERT(cachedVersion <= CURRENT_EXTENSION_VERSION);

  char buffer[1024];
  while (file.readLine(buffer, sizeof(buffer)) > 0)
  {
    QString line(buffer);
    QStringList fields = line.remove('\n').split(SEP);

    Q_ASSERT(fields.size() > 5);

    SegmentationPtr extensionSegmentation = NULL;
    int i = 0;
    while (!extensionSegmentation && i < model->segmentations().size())
    {
      SegmentationSPtr segmentation = model->segmentations()[i];
      if ( segmentation->filter()->id()       == fields[0]
        && segmentation->outputId()           == fields[1].toInt()
        && segmentation->filter()->cacheDir() == tmpDir)
      {
        extensionSegmentation = segmentation.data();
      }
      i++;
    }
    // NOTE: This assert means someone's removing an extension from the model
    //       without invalidating its extensions
    Q_ASSERT(extensionSegmentation);

    ExtensionData &data = s_cache[extensionSegmentation].Data;

    data.Area           = fields[2].toDouble();
    data.Perimeter      = fields[3].toDouble();
    data.Tortuosity     = fields[4].toDouble();
    data.SynapticSource = fields[5];

    if (CURRENT_EXTENSION_VERSION == cachedVersion)
    {
      data.MeanGaussCurvature   = fields[ 6].toDouble();
      data.StdDevGaussCurvature = fields[ 7].toDouble();
      data.MeanMeanCurvature    = fields[ 8].toDouble();
      data.StdDevMeanCurvature  = fields[ 9].toDouble();
      data.MeanMinCurvature     = fields[10].toDouble();
      data.StdDevMinCurvature   = fields[11].toDouble();
      data.MeanMaxCurvature     = fields[12].toDouble();
      data.StdDevMaxCurvature   = fields[13].toDouble();
    }
  }
}

//------------------------------------------------------------------------
// It's declared static to avoid collisions with other functions with same
// signature in different compilation units
static bool invalidData(SegmentationPtr seg)
{
  return !seg->hasInformationExtension(AppositionSurfaceExtension::ID)
       && seg->outputIsModified();
}
//------------------------------------------------------------------------
bool AppositionSurfaceExtension::saveCache(Snapshot &cacheList)
{
  s_cache.purge(invalidData);

  if (s_cache.isEmpty())
    return false;

  std::ostringstream cache;
  cache << CURRENT_EXTENSION_VERSION << std::endl;

  foreach(SegmentationPtr segmentation, s_cache.keys())
  {
    ExtensionData &data = s_cache[segmentation].Data;

    cache << segmentation->filter()->id().toStdString();
    cache << SEP << segmentation->outputId();

    cache << SEP << data.Area;
    cache << SEP << data.Perimeter;
    cache << SEP << data.Tortuosity;
    cache << SEP << data.SynapticSource.toStdString();
    cache << SEP << data.MeanGaussCurvature;
    cache << SEP << data.StdDevGaussCurvature;
    cache << SEP << data.MeanMeanCurvature;
    cache << SEP << data.StdDevMeanCurvature;
    cache << SEP << data.MeanMinCurvature;
    cache << SEP << data.StdDevMinCurvature;
    cache << SEP << data.MeanMaxCurvature;
    cache << SEP << data.StdDevMaxCurvature;

    cache << std::endl;
  }

  cacheList << QPair<QString, QByteArray>(EXTENSION_FILE, cache.str().c_str());

  return true;
}

//------------------------------------------------------------------------
Nm AppositionSurfaceExtension::computeArea(vtkPolyData *asMesh) const
{
  int nc = asMesh->GetNumberOfCells();
  Nm totalArea = nc ? 0.0 : UNDEFINED;

  for (int i = 0; i < nc; i++)
    totalArea += vtkMeshQuality::TriangleArea(asMesh->GetCell(i));

  return totalArea;
}

typedef vtkSmartPointer<vtkIdList>                   IdList;
typedef vtkSmartPointer<vtkIdTypeArray>              IdTypeArray;
typedef vtkSmartPointer<vtkMutableUndirectedGraph>   MutableUndirectedGraph;
typedef vtkSmartPointer<vtkBoostConnectedComponents> BoostConnectedComponents;
typedef vtkSmartPointer<vtkEdgeListIterator>         EdgeListIterator;

//----------------------------------------------------------------------------
bool AppositionSurfaceExtension::isPerimeter(vtkPolyData *asMesh, vtkIdType cellId, vtkIdType p1, vtkIdType p2) const
{
  IdList neighborCellIds = IdList::New();
  asMesh->GetCellEdgeNeighbors(cellId, p1, p2, neighborCellIds);

  return (neighborCellIds->GetNumberOfIds() == 0);
}

//------------------------------------------------------------------------
Nm AppositionSurfaceExtension::computePerimeter(vtkPolyData *asMesh) const
{
  Nm totalPerimeter = 0.0;
  try
  {
    asMesh->BuildLinks();

    // std::cout << "Points: " << mesh->GetNumberOfPoints() << std::endl;
    int num_of_cells = asMesh->GetNumberOfCells();

    MutableUndirectedGraph graph = MutableUndirectedGraph::New();

    IdTypeArray pedigree = IdTypeArray::New();
    graph->GetVertexData()->SetPedigreeIds(pedigree);

    for (vtkIdType cellId = 0; cellId < num_of_cells; cellId++)
    {
      IdList cellPointIds = IdList::New();
      asMesh->GetCellPoints(cellId, cellPointIds);
      for(vtkIdType i = 0; i < cellPointIds->GetNumberOfIds(); i++)
      {
        vtkIdType p1;
        vtkIdType p2;

        p1 = cellPointIds->GetId(i);
        if(i+1 == cellPointIds->GetNumberOfIds())
          p2 = cellPointIds->GetId(0);
        else
          p2 = cellPointIds->GetId(i+1);

        if (isPerimeter(asMesh, cellId,p1,p2))
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

    BoostConnectedComponents connectedComponents =BoostConnectedComponents::New();
    connectedComponents->SetInput(graph);
    connectedComponents->Update();
    vtkGraph *outputGraph = connectedComponents->GetOutput();
    vtkIntArray *components = vtkIntArray::SafeDownCast( outputGraph->GetVertexData()->GetArray("component"));

    double *range = components->GetRange(0);
    // std::cout << "components: " << range[0] << " to "<< range[1] << std::endl;

    std::vector<double> perimeters;
    perimeters.resize(range[1]+1);

    EdgeListIterator edgeListIterator = EdgeListIterator::New();
    graph->GetEdges(edgeListIterator);

    while(edgeListIterator->HasNext()) {
      vtkEdgeType edge = edgeListIterator->Next();

      if (components->GetValue(edge.Source) != components->GetValue(edge.Target) ) {
        throw "ERROR: Edge between 2 disconnected component";
      }
      double x[3];
      double y[3];
      asMesh->GetPoint(pedigree->GetValue(edge.Source), x);
      asMesh->GetPoint(pedigree->GetValue(edge.Target), y);

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

typedef vtkSmartPointer<vtkPolyDataNormals> PolyDataNormals;

//------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> AppositionSurfaceExtension::projectPolyDataToPlane(vtkPolyData *mesh) const
{
  double origin[3];
  double normal[3]; // Normal's magnitude is 1

  vtkDoubleArray *originArray = dynamic_cast<vtkDoubleArray *>(mesh->GetPointData()->GetArray(AppositionSurfaceFilter::MESH_ORIGIN));
  vtkDoubleArray *normalArray = dynamic_cast<vtkDoubleArray *>(mesh->GetPointData()->GetArray(AppositionSurfaceFilter::MESH_NORMAL));

  for (int i = 0; i < 3; ++i)
  {
    origin[i] = originArray->GetValue(i);
    normal[i] = normalArray->GetValue(i);
  }

  vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
  plane->SetOrigin(origin);
  plane->SetNormal(normal);

  int pointsCount = 0;
  double projected[3], p[3];

  vtkSmartPointer<vtkPoints> pointsIn, pointsOut;
  pointsIn  = mesh->GetPoints();
  pointsOut = vtkSmartPointer<vtkPoints>::New();

  pointsCount = pointsIn->GetNumberOfPoints();
  pointsOut->SetNumberOfPoints(pointsCount);
  for (int i = 0; i < pointsCount; i++) {
    pointsIn->GetPoint(i, p);
    plane->ProjectPoint(p, projected);
    pointsOut->SetPoint(i, projected);
  }
  vtkSmartPointer<vtkPolyData> auxMesh = vtkSmartPointer<vtkPolyData>::New();
  auxMesh->DeepCopy(mesh);
  auxMesh->SetPoints(pointsOut);

  PolyDataNormals normals = PolyDataNormals::New();
  normals->SetInput(auxMesh);
  normals->SplittingOff();
  normals->Update();

  vtkSmartPointer<vtkPolyData> projection = vtkSmartPointer<vtkPolyData>::New();
  projection->ShallowCopy(normals->GetOutput());

  return projection;
}

//------------------------------------------------------------------------
double AppositionSurfaceExtension::computeTortuosity(vtkPolyData *asMesh, Nm asArea) const
{
  vtkSmartPointer<vtkPolyData> projectedAS = projectPolyDataToPlane(asMesh);

  double referenceArea = computeArea(projectedAS);

  return 1 - (referenceArea / asArea);
}

//------------------------------------------------------------------------
bool AppositionSurfaceExtension::computeInformation()
{
  bool validInformation = false;

  qDebug() << "Computing AS Information:" << m_segmentation->data().toString() << ID;

  MeshRepresentationSPtr segMesh = meshRepresentation(m_segmentation->output());

  if (segMesh)
  {
    ExtensionData &data = s_cache[m_segmentation].Data;

    vtkPolyData *asMesh = dynamic_cast<vtkPolyData *>(segMesh->mesh()->GetProducer()->GetOutputDataObject(0));

    data.Area       = computeArea      (asMesh);
    data.Perimeter  = computePerimeter (asMesh);
    data.Tortuosity = computeTortuosity(asMesh, data.Area);

    s_cache.markAsClean(m_segmentation);
    validInformation = true;
  }

  return validInformation;
}
