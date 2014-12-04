/*
 *    
 *    Copyright (C) 2014  Juan Morales del Olmo <juan.morales@upm.es>
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

///-----------------------------------------------------------------------
/// APPOSITION SURFACE EXTENSION-
///-----------------------------------------------------------------------
/// Information Provided:
/// - Sinaptic Apposition Surface Area
/// - Sinaptic Apposition Surface Perimeter
/// - Sinaptic Apposition Surface Tortuosity (aka Area Ratio)
/// - Synapse from which the Sinaptic Apposition Surface was obtained

const SegmentationExtension::Type AppositionSurfaceExtension::TYPE = "AppositionSurfaceExtensionInformation";

const SegmentationExtension::InfoTag AREA                   = "Area";
const SegmentationExtension::InfoTag PERIMETER              = "Perimeter";
const SegmentationExtension::InfoTag TORTUOSITY             = "Area Ratio";
const SegmentationExtension::InfoTag SYNAPSE                = "Synapse";
const SegmentationExtension::InfoTag MEAN_GAUSS_CURVATURE   = "Mean Gauss Curvature";
const SegmentationExtension::InfoTag STD_DEV_GAUS_CURVATURE = "Std Deviation Gauss Curvature";
const SegmentationExtension::InfoTag MEAN_MEAN_CURVATURE    = "Mean Mean Curvature";
const SegmentationExtension::InfoTag STD_DEV_MEAN_CURVATURE = "Std Deviation Mean Curvature";
const SegmentationExtension::InfoTag MEAN_MIN_CURVATURE     = "Mean Min Curvature";
const SegmentationExtension::InfoTag STD_DEV_MIN_CURVATURE  = "Std Deviation Min Curvature";
const SegmentationExtension::InfoTag MEAN_MAX_CURVATURE     = "Mean Max Curvature";
const SegmentationExtension::InfoTag STD_DEV_MAX_CURVATURE  = "Std Deviation Max Curvature";

const double UNDEFINED = -1.;

//------------------------------------------------------------------------
AppositionSurfaceExtension::AppositionSurfaceExtension(const SegmentationExtension::InfoCache &cache)
: SegmentationExtension{cache}
, m_originSegmentation {nullptr}
{
}

//------------------------------------------------------------------------
SegmentationExtension::InfoTagList AppositionSurfaceExtension::availableInformations() const
{
  SegmentationExtension::InfoTagList tags;

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

QVariant AppositionSurfaceExtension::cacheFail(const SegmentationExtension::InfoTag &tag) const
{
  if(availableInformations().contains(tag))
  {
    computeInformation();
    return information(tag);
  }
  else
    return QVariant();
}

//------------------------------------------------------------------------
bool AppositionSurfaceExtension::validCategory(const QString &classificationName) const
{
  return classificationName.contains(tr("SAS"));
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
    totalArea += vtkMeshQuality::TriangleArea(asMesh->GetCell(i));

  return totalArea;
}

//----------------------------------------------------------------------------
bool AppositionSurfaceExtension::isPerimeter(const vtkSmartPointer<vtkPolyData> &asMesh, vtkIdType cellId, vtkIdType p1, vtkIdType p2) const
{
  IdList neighborCellIds = IdList::New();
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
  auto segMesh = meshData(m_extendedItem->output());

  if (segMesh)
  {
    // vtkPolyData *asMesh = dynamic_cast<vtkPolyData *>(segMesh->mesh()->GetProducer()->GetOutputDataObject(0));
    auto asMesh = segMesh->mesh();

    vtkSmartPointer<vtkDoubleArray> gaussCurvature = vtkSmartPointer<vtkDoubleArray>::New();	 
    vtkSmartPointer<vtkDoubleArray> meanCurvature  = vtkSmartPointer<vtkDoubleArray>::New();
    vtkSmartPointer<vtkDoubleArray> maxCurvature   = vtkSmartPointer<vtkDoubleArray>::New();
    vtkSmartPointer<vtkDoubleArray> minCurvature   = vtkSmartPointer<vtkDoubleArray>::New();
    computeCurvatures(asMesh, gaussCurvature, meanCurvature, minCurvature, maxCurvature);

    auto area = computeArea(asMesh);
    updateInfoCache(AREA, area);
    updateInfoCache(PERIMETER, computePerimeter (asMesh));
    updateInfoCache(TORTUOSITY, computeTortuosity(asMesh, area));
    updateInfoCache(SYNAPSE, m_extendedItem->name());

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

    if(m_originSegmentation != nullptr)
      updateInfoCache(SYNAPSE, m_originSegmentation->data().toString());
    else
      updateInfoCache(SYNAPSE, tr("Unknown"));

    validInformation = true;
  }

  return validInformation;
}
