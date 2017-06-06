/*
 *    
o *    Copyright (C) 2014  Juan Morales del Olmo <juan.morales@upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// Plugin
#include "AppositionSurfaceExtension.h"
#include <Filter/AppositionSurfaceFilter.h>

// ESPINA
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Utils/EspinaException.h>

// VTK
#include <vtkIdList.h>
#include <vtkMeshQuality.h>
#include <vtkPolyDataNormals.h>
#include <vtkMutableUndirectedGraph.h>
#include <vtkDataSetAttributes.h>
#include <vtkEdgeListIterator.h>
#include <vtkPointData.h>
#include <vtkIdTypeArray.h>
#include <vtkMath.h>
#include <vtkPlane.h>
#include <vtkDoubleArray.h>
#include <vtkCurvatures.h>
#include <vtkBoostConnectedComponents.h>

using IdList = vtkSmartPointer<vtkIdList>;
using IdTypeArray = vtkSmartPointer<vtkIdTypeArray>;
using MutableUndirectedGraph = vtkSmartPointer<vtkMutableUndirectedGraph>;
using BoostConnectedComponents = vtkSmartPointer<vtkBoostConnectedComponents>;
using EdgeListIterator = vtkSmartPointer<vtkEdgeListIterator>;
using PolyDataNormals = vtkSmartPointer<vtkPolyDataNormals>;

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;

///-----------------------------------------------------------------------
/// APPOSITION SURFACE EXTENSION-
///-----------------------------------------------------------------------
/// Information Provided:
/// - Synaptic Apposition Surface Area
/// - Synaptic Apposition Surface Perimeter
/// - Synaptic Apposition Surface Tortuosity (aka Area Ratio)
/// - Synapse from which the Synaptic Apposition Surface was obtained

const QString AppositionSurfaceExtension::SAS_PREFIX = QObject::tr("SAS ");

const SegmentationExtension::Type AppositionSurfaceExtension::TYPE = "AppositionSurface";

// NOTE: some old files have this signature, right now there is a fix in ESPINA::Core to convert old signatures to new
const SegmentationExtension::Type AppositionSurfaceExtension::OLD_TYPE = "AppositionSurfaceExtensionInformation";

const SegmentationExtension::Key AREA                   = "Area";
const SegmentationExtension::Key PERIMETER              = "Perimeter";
const SegmentationExtension::Key TORTUOSITY             = "Area Ratio";
const SegmentationExtension::Key SYNAPSE                = "Synapse";
const SegmentationExtension::Key MEAN_GAUSS_CURVATURE   = "Mean Gauss Curvature";
const SegmentationExtension::Key STD_DEV_GAUS_CURVATURE = "Std Deviation Gauss Curvature";
const SegmentationExtension::Key MEAN_MEAN_CURVATURE    = "Mean Mean Curvature";
const SegmentationExtension::Key STD_DEV_MEAN_CURVATURE = "Std Deviation Mean Curvature";
const SegmentationExtension::Key MEAN_MIN_CURVATURE     = "Mean Min Curvature";
const SegmentationExtension::Key STD_DEV_MIN_CURVATURE  = "Std Deviation Min Curvature";
const SegmentationExtension::Key MEAN_MAX_CURVATURE     = "Mean Max Curvature";
const SegmentationExtension::Key STD_DEV_MAX_CURVATURE  = "Std Deviation Max Curvature";

const double UNDEFINED = -1.;

//------------------------------------------------------------------------
AppositionSurfaceExtension::AppositionSurfaceExtension(const SegmentationExtension::InfoCache &cache)
: SegmentationExtension{cache}
{
}

//------------------------------------------------------------------------
SegmentationExtension::InformationKeyList AppositionSurfaceExtension::availableInformation() const
{
  InformationKeyList keys;

  keys << createKey(AREA)
       << createKey(PERIMETER)
       << createKey(TORTUOSITY)
       << createKey(SYNAPSE)
       << createKey(MEAN_GAUSS_CURVATURE)
       << createKey(STD_DEV_GAUS_CURVATURE)
       << createKey(MEAN_MEAN_CURVATURE)
       << createKey(STD_DEV_MEAN_CURVATURE)
       << createKey(MEAN_MIN_CURVATURE)
       << createKey(STD_DEV_MIN_CURVATURE)
       << createKey(MEAN_MAX_CURVATURE)
       << createKey(STD_DEV_MAX_CURVATURE);

  return keys;
}

//------------------------------------------------------------------------
QVariant AppositionSurfaceExtension::cacheFail(const InformationKey &key) const
{
  QVariant result;

  if(availableInformation().contains(key))
  {
    computeInformation();

    result = information(key);
  }

  return result;
}

//------------------------------------------------------------------------
bool AppositionSurfaceExtension::validCategory(const QString &classificationName) const
{
  return classificationName.contains(tr("SAS"));
}

//------------------------------------------------------------------------
SegmentationExtension::Key AppositionSurfaceExtension::addSASPrefix(const Key& value)
{
  return SAS_PREFIX + value;
}

//------------------------------------------------------------------------
SegmentationExtension::Key AppositionSurfaceExtension::removeSASPrefix(const Key& value)
{
  auto key = value;

  if(key.startsWith(SAS_PREFIX))
  {
    key.remove(0,SAS_PREFIX.size());
  }

  return key;
}

//------------------------------------------------------------------------
void AppositionSurfaceExtension::onExtendedItemSet(Segmentation *item)
{
  connect(item, SIGNAL(outputModified()),
          this, SLOT(invalidate()));
}

//------------------------------------------------------------------------
Nm AppositionSurfaceExtension::computeArea(const vtkSmartPointer<vtkPolyData> &asMesh) const
{
  int nc = asMesh->GetNumberOfCells();
  Nm totalArea = nc ? 0.0 : UNDEFINED;

  for (int i = 0; i < nc; i++)
  {
    totalArea += vtkMeshQuality::TriangleArea(asMesh->GetCell(i));
  }

  return totalArea;
}

//----------------------------------------------------------------------------
bool AppositionSurfaceExtension::isPerimeter(const vtkSmartPointer<vtkPolyData> &asMesh, vtkIdType cellId, vtkIdType p1, vtkIdType p2) const
{
  auto neighborCellIds = IdList::New();
  asMesh->GetCellEdgeNeighbors(cellId, p1, p2, neighborCellIds);

  return (neighborCellIds->GetNumberOfIds() == 0);
}

//------------------------------------------------------------------------
Nm AppositionSurfaceExtension::computePerimeter(const vtkSmartPointer<vtkPolyData> &asMesh) const
{
  Nm totalPerimeter = 0.0;

  try
  {
    asMesh->BuildLinks();

    // std::cout << "Points: " << mesh->GetNumberOfPoints() << std::endl;
    auto numCells = asMesh->GetNumberOfCells();
    auto graph    = MutableUndirectedGraph::New();
    auto pedigree = IdTypeArray::New();

    graph->GetVertexData()->SetPedigreeIds(pedigree);

    for (vtkIdType cellId = 0; cellId < numCells; cellId++)
    {
      auto cellPointIds = IdList::New();
      asMesh->GetCellPoints(cellId, cellPointIds);
      auto numIds = cellPointIds->GetNumberOfIds();

      for(vtkIdType idx = 0; idx < numIds; idx++)
      {
        vtkIdType id1;
        vtkIdType id2;

        id1 = cellPointIds->GetId(idx);
        id2 = cellPointIds->GetId((idx+1) % numIds);

        if (isPerimeter(asMesh, cellId,id1, id2))
        {
          /**
           * VTK BUG: in Vtk5.10.1 without
           * clearing the loopuk table, the
           * pedigree->LoockupValue(p1) fails
           *
           * NOTE: Check if ClearLookup is
           * needed in other version of VTK
           */
          pedigree->ClearLookup();

          vtkIdType i1 = graph->AddVertex(id1);
          vtkIdType i2 = graph->AddVertex(id2);

          graph->AddEdge(i1, i2);
        }
      }
    }

    // std::cout << "Vertices: " << graph->GetNumberOfVertices() << std::endl;
    // std::cout << "Edges: " << graph->GetNumberOfEdges() << std::endl;
    // std::cout << "Pedigree: " << pedigree->GetNumberOfTuples() << std::endl;

    BoostConnectedComponents connectedComponents = BoostConnectedComponents::New();
    connectedComponents->SetInputData(graph);
    connectedComponents->Update();
    vtkGraph *outputGraph = connectedComponents->GetOutput();
    vtkIntArray *components = vtkIntArray::SafeDownCast( outputGraph->GetVertexData()->GetArray("component"));

    double *range = components->GetRange(0);
    // std::cout << "components: " << range[0] << " to "<< range[1] << std::endl;

    std::vector<double> perimeters;
    perimeters.resize(range[1]+1);

    EdgeListIterator edgeListIterator = EdgeListIterator::New();
    graph->GetEdges(edgeListIterator);

    while(edgeListIterator->HasNext())
    {
      vtkEdgeType edge = edgeListIterator->Next();

      if (components->GetValue(edge.Source) != components->GetValue(edge.Target) )
      {
        auto what = QObject::tr("Edge between two disconnected component.");
        auto details = QObject::tr("AppositionSurfaceExtension::computePerimeter() -> Edge between two disconnected component.");

        throw EspinaException(what, details);
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
  }
  catch (...)
  {
    // std::cerr << "Warning: Couldn't compute " << m_seg->data().toString().toStdString() << "'s Apposition Plane perimter" << std::endl;
    totalPerimeter = UNDEFINED;
  }

  return totalPerimeter;
}

//------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> AppositionSurfaceExtension::projectPolyDataToPlane(const vtkSmartPointer<vtkPolyData> &mesh) const
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
  normals->SetInputData(auxMesh);
  normals->SplittingOff();
  normals->Update();

  vtkSmartPointer<vtkPolyData> projection = vtkSmartPointer<vtkPolyData>::New();
  projection->ShallowCopy(normals->GetOutput());

  return projection;
}

//------------------------------------------------------------------------
double AppositionSurfaceExtension::computeTortuosity(const vtkSmartPointer<vtkPolyData> &asMesh, Nm asArea) const
{
  vtkSmartPointer<vtkPolyData> projectedAS = projectPolyDataToPlane(asMesh);

  double referenceArea = computeArea(projectedAS);

  return 1 - (referenceArea / asArea);
}

//------------------------------------------------------------------------
void AppositionSurfaceExtension::computeCurvatures(const vtkSmartPointer<vtkPolyData> &asMesh,
						                                       vtkSmartPointer<vtkDoubleArray> gaussCurvature,
						                                       vtkSmartPointer<vtkDoubleArray> meanCurvature,
						                                       vtkSmartPointer<vtkDoubleArray> minCurvature,
						                                       vtkSmartPointer<vtkDoubleArray> maxCurvature) const
{
	vtkSmartPointer<vtkCurvatures> curvatures_fliter = vtkSmartPointer<vtkCurvatures>::New();
	curvatures_fliter->SetInputData(asMesh);

	vtkPolyData * output = curvatures_fliter->GetOutput();

	curvatures_fliter->SetCurvatureTypeToGaussian();
	curvatures_fliter->Update();
	gaussCurvature->DeepCopy(output->GetPointData()->GetArray("Gauss_Curvature"));

	curvatures_fliter->SetCurvatureTypeToMean();
	curvatures_fliter->Update();
	meanCurvature->DeepCopy(output->GetPointData()->GetArray("Mean_Curvature"));

	curvatures_fliter->SetCurvatureTypeToMinimum();
	curvatures_fliter->Update();
	minCurvature->DeepCopy(output->GetPointData()->GetArray("Minimum_Curvature"));

	curvatures_fliter->SetCurvatureTypeToMaximum();
	curvatures_fliter->Update();
	maxCurvature->DeepCopy(output->GetPointData()->GetArray("Maximum_Curvature"));
}


//------------------------------------------------------------------------
// Aux function to compute the mean
double mean(const vtkSmartPointer<vtkDoubleArray> dataArray)
{
  double mean = 0;
  int num_pts = dataArray->GetNumberOfTuples();

  for (int i = 0; i < num_pts; i++)
    mean += dataArray->GetValue(i);

  mean = mean / double(num_pts);

  return mean;
}

//------------------------------------------------------------------------
// Aux function to compute the Standard Deviation
double stdDev(const vtkSmartPointer<vtkDoubleArray> dataArray, const double mean)
{
  double std_dev = 0;
  double acum = 0;
  int num_pts = dataArray->GetNumberOfTuples();

  for (int i = 0; i < num_pts; i++)
    acum += pow(dataArray->GetValue(i) - mean, 2);

  std_dev = sqrt(acum / double(num_pts));

  return std_dev;
}

//------------------------------------------------------------------------
bool AppositionSurfaceExtension::computeInformation() const
{
  bool validInformation = false;
  auto segMesh = writeLockMesh(m_extendedItem->output());

  if (segMesh->isValid())
  {
    auto asMesh = segMesh->mesh();

    auto gaussCurvature = vtkSmartPointer<vtkDoubleArray>::New();
    auto meanCurvature  = vtkSmartPointer<vtkDoubleArray>::New();
    auto maxCurvature   = vtkSmartPointer<vtkDoubleArray>::New();
    auto minCurvature   = vtkSmartPointer<vtkDoubleArray>::New();

    computeCurvatures(asMesh, gaussCurvature, meanCurvature, minCurvature, maxCurvature);

    auto area = computeArea(asMesh);
    updateInfoCache(AREA, area);
    updateInfoCache(PERIMETER, computePerimeter (asMesh));
    updateInfoCache(TORTUOSITY, computeTortuosity(asMesh, area));

    auto meanGaussCurvature = mean(gaussCurvature);
    updateInfoCache(MEAN_GAUSS_CURVATURE, meanGaussCurvature);
    updateInfoCache(STD_DEV_GAUS_CURVATURE, stdDev(gaussCurvature,meanGaussCurvature));

    auto meanMeanCurvature = mean(meanCurvature);
    updateInfoCache(MEAN_MEAN_CURVATURE, meanMeanCurvature);
    updateInfoCache(STD_DEV_MEAN_CURVATURE, stdDev(meanCurvature, meanMeanCurvature));

    auto meanMinCurvature = mean(minCurvature);
    updateInfoCache(MEAN_MIN_CURVATURE, meanMinCurvature);
    updateInfoCache(STD_DEV_MIN_CURVATURE, stdDev(minCurvature, meanMinCurvature));

    auto meanMaxCurvature = mean(maxCurvature);
    updateInfoCache(MEAN_MAX_CURVATURE, meanMaxCurvature);
    updateInfoCache(STD_DEV_MAX_CURVATURE, stdDev(maxCurvature, meanMaxCurvature));

    auto origin = cachedInfo(createKey(SYNAPSE)).toString();

    if(origin.isEmpty())
    {
      bool found = false;
      auto ancestors = m_extendedItem->analysis()->relationships()->ancestors(m_extendedItem, tr("SAS"));

      if(!ancestors.isEmpty())
      {
        auto segmentation = std::dynamic_pointer_cast<Segmentation>(ancestors.first());
        if(segmentation)
        {
          auto name = segmentation->name().isEmpty() ? segmentation->alias() : segmentation->name();

          if(!name.isEmpty())
          {
            updateInfoCache(SYNAPSE, name);
            found = true;
          }
        }
      }

      if(!found)
      {
        updateInfoCache(SYNAPSE, tr("Unknown"));
      }
    }

    validInformation = true;
  }

  return validInformation;
}

//------------------------------------------------------------------------
void AppositionSurfaceExtension::setOriginSegmentation(SegmentationAdapterSPtr segmentation)
{
  if(segmentation != nullptr)
  {
    updateInfoCache(SYNAPSE, segmentation->data().toString());
  }
  else
  {
    updateInfoCache(SYNAPSE, tr("Unknown"));
  }
}

//------------------------------------------------------------------------
bool AppositionSurfaceExtension::validData(const OutputSPtr output) const
{
  return hasVolumetricData(output);
}
