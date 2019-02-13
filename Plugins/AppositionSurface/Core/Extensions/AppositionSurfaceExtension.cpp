/*
 *    
 *    Copyright (C) 2014  Juan Morales del Olmo <juan.morales@upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
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
#include <vtkLine.h>

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
const SegmentationExtension::Key SHAPE                  = "Shape";
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
, m_hasErrors          {false}
, m_synapse            {nullptr}
{
}

//------------------------------------------------------------------------
const SegmentationExtension::InformationKeyList AppositionSurfaceExtension::availableInformation() const
{
  InformationKeyList keys;

  keys << createKey(AREA)
       << createKey(PERIMETER)
       << createKey(TORTUOSITY)
       << createKey(SYNAPSE)
       << createKey(SHAPE)
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

  if(key.value() == SYNAPSE)
  {
    if(!m_synapse)
    {
      obtainOriginSynapse();

      if(!m_synapse) result = tr("Failed to identify");
    }
    else
    {
      result = m_synapse->alias().isEmpty() ? m_synapse->name() : m_synapse->alias();
    }
  }
  else
  {
    if(availableInformation().contains(key))
    {
      if(!computeInformation())
      {
        auto name = m_extendedItem->alias().isEmpty() ? m_extendedItem->name() : m_extendedItem->alias();
        qWarning() << "AppositionSurfaceExtension -> Invalid mesh in" << name;
      }

      result = information(key);
    }
  }

  return result;
}

//------------------------------------------------------------------------
bool AppositionSurfaceExtension::validCategory(const QString &classificationName) const
{
  return classificationName.startsWith(tr("SAS"));
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
void AppositionSurfaceExtension::obtainOriginSynapse() const
{
  auto ancestors = m_extendedItem->analysis()->relationships()->ancestors(m_extendedItem, tr("SAS"));
  while (!ancestors.isEmpty())
  {
    auto candidate = ancestors.takeFirst();
    auto segmentation = std::dynamic_pointer_cast<Segmentation>(candidate);
    if (segmentation)
    {
      m_synapse = segmentation;
      break;
    }
  }
}

//------------------------------------------------------------------------
void AppositionSurfaceExtension::onExtendedItemSet(Segmentation *item)
{
  connect(item, SIGNAL(outputModified()),
          this, SLOT(invalidate()));

  // NOTE: Synapse key is generated on request in cacheFail() method, never saved to cache. That means it's "udpated"
  // when the user changes the origin segmentation name or category.  Need to clear it because previously it was
  // saved to cache.
  if(m_infoCache.keys().contains(SYNAPSE))
  {
    m_infoCache.remove(SYNAPSE);
  }
}

//------------------------------------------------------------------------
Nm AppositionSurfaceExtension::computeArea(const vtkSmartPointer<vtkPolyData> asMesh) const
{
  Nm totalArea = 0.0;

  if(asMesh)
  {
    auto nc   = asMesh->GetNumberOfCells();
    totalArea = nc ? 0.0 : UNDEFINED;

    for (int i = 0; i < nc; i++)
    {
      totalArea += vtkMeshQuality::TriangleArea(asMesh->GetCell(i));
    }
  }
  else
  {
    m_hasErrors = true;
  }

  return totalArea;
}

//----------------------------------------------------------------------------
bool AppositionSurfaceExtension::isPerimeter(const vtkSmartPointer<vtkPolyData> asMesh, vtkIdType cellId, vtkIdType p1, vtkIdType p2) const
{
  if(asMesh)
  {
    auto neighborCellIds = IdList::New();
    asMesh->GetCellEdgeNeighbors(cellId, p1, p2, neighborCellIds);

    return (neighborCellIds->GetNumberOfIds() == 0);
  }

  return false;
}

//------------------------------------------------------------------------
std::pair<Nm, AppositionSurfaceExtension::Shape> AppositionSurfaceExtension::computePerimeterAndShape(const vtkSmartPointer<vtkPolyData> asMesh) const
{
  Nm totalPerimeter = 0.0;
  Shape shape = Shape::UNKNOWN;

  if(!asMesh)
  {
    m_hasErrors = true;
    return std::make_pair(-1, Shape::UNKNOWN);
  }

  try
  {
    asMesh->BuildLinks();

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

    auto connectedComponents = BoostConnectedComponents::New();
    connectedComponents->SetInputData(graph);
    connectedComponents->Update();
    auto outputGraph = connectedComponents->GetOutput();
    auto components = vtkIntArray::SafeDownCast(outputGraph->GetVertexData()->GetArray("component"));

    double *range = components->GetRange(0);
    // std::cout << "components: " << range[0] << " to "<< range[1] << std::endl;

    /** \struct PerimeterData
     * \brief Contains all data required to compute inclusion and the length of the perimeter. We store the
     *        points in raw form instead of using vtkLine or similar to try to reduce memory footprint and be faster.
     */
    struct PerimeterData
    {
        Bounds              bounds;  /** bounds of the perimeter in the projected plane.       */
        std::vector<double> points;  /** projected points on the plane.                        */
        double              length;  /** length of the non-projeted perimeter.                 */
        bool                added;   /** true if added to final perimeter and false otherwise. */

        /** \brief PerimeterData struct constructor.
         *
         */
        PerimeterData(): bounds{Bounds()}, points{std::vector<double>()}, length{0.}, added{false} {};
    };

    std::vector<PerimeterData> perimeters;
    perimeters.resize(range[1]+1);

    // helper method to include a point in a Bounds structure.
    // param[inout] bounds Bounds object to be expanded to include point p
    // param[in] p Point coordinates.
    auto includePointInBounds = [](Bounds &bounds, double p[3])
    {
      if(bounds.areValid())
      {
        bounds[0] = std::min(bounds[0], p[0]);
        bounds[1] = std::max(bounds[1], p[0]);
        bounds[2] = std::min(bounds[2], p[1]);
        bounds[3] = std::max(bounds[3], p[1]);
        bounds[4] = std::min(bounds[4], p[2]);
        bounds[5] = std::max(bounds[5], p[2]);
      }
      else
      {
        bounds = Bounds{p[0], p[0], p[1], p[1], p[2], p[2]};
      }
    };

    // helper method that returs true if the second Bounds are included in the first Bounds given and false otherwise.
    // param[in] container Bounds object to test if contains the second Bounds object.
    // param[in] bounds Bounds object to test for inclusion.
    auto isIncluded = [](const Bounds &container, const Bounds &bounds)
    {
      return (bounds[0] >= container[0] && bounds[1] <= container[1] &&
              bounds[2] >= container[2] && bounds[3] <= container[3] &&
              bounds[4] >= container[4] && bounds[5] <= container[5]);
    };

    auto originArray = vtkDoubleArray::SafeDownCast(asMesh->GetPointData()->GetArray(AppositionSurfaceFilter::MESH_ORIGIN));
    auto normalArray = vtkDoubleArray::SafeDownCast(asMesh->GetPointData()->GetArray(AppositionSurfaceFilter::MESH_NORMAL));

    if (!originArray || !normalArray)
    {
      m_hasErrors = true;
      return std::make_pair(UNDEFINED, Shape::UNKNOWN);
    }

    double origin[3], normal[3];
    for (int i = 0; i < 3; ++i)
    {
      origin[i] = originArray->GetValue(i);
      normal[i] = normalArray->GetValue(i);
    }

    auto plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(origin);
    plane->SetNormal(normal);

    auto edgeListIterator = EdgeListIterator::New();
    graph->GetEdges(edgeListIterator);
    while(edgeListIterator->HasNext())
    {
      auto edge = edgeListIterator->Next();

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

      auto edgeLength = std::sqrt(vtkMath::Distance2BetweenPoints(x, y));

      auto index = components->GetValue(edge.Source);
      perimeters[index].length += edgeLength;

      double projected[3];
      plane->ProjectPoint(x,projected);
      includePointInBounds(perimeters[index].bounds, projected);
      perimeters[index].points.push_back(projected[0]);
      perimeters[index].points.push_back(projected[1]);
      perimeters[index].points.push_back(projected[2]);

      plane->ProjectPoint(y,projected);
      includePointInBounds(perimeters[index].bounds, projected);
      perimeters[index].points.push_back(projected[0]);
      perimeters[index].points.push_back(projected[1]);
      perimeters[index].points.push_back(projected[2]);
    }

// @felix Modified 27/09/2018
// Asked solution: Return only the sum of external perimeters, without adding the perimeters of the perforations.
// Bounding box method works for the majority of the real-life cases, but not all. Correct implementation is:
// A) Test intersection with bounding boxes to obtain candidates. If no inclusion detected add the length to the total.
// B) If inclusion detected with bounding boxes the use the shooting algorithm in the plane containing both contours to
//    check for real contour nesting.
//
    for(unsigned int i = 0; i < perimeters.size(); ++i)
    {
      bool isInside = false;
      auto iBounds = perimeters.at(i).bounds;
      for(unsigned int j = 0; j < perimeters.size() && !isInside; ++j)
      {
        if(i == j) continue;
        auto jBounds = perimeters.at(j).bounds;

        if(isIncluded(jBounds, iBounds))
        {
          // test with ray-casting algorithm (shooting algorithm)
          double max[3]{2*jBounds[1], 2*jBounds[3], 2*jBounds[5]};
          double pMax[3];
          plane->ProjectPoint(max, pMax);
          double jx[3], jy[3];
          double p[3]{perimeters.at(i).points[0], perimeters.at(i).points[1], perimeters.at(i).points[2]};
          auto &points = perimeters.at(j).points;
          double u,v;

          int count = 0;
          for(unsigned int k = 0; k < points.size(); k += 6)
          {
            std::memcpy(jx, &points[k+0], 3*sizeof(double));
            std::memcpy(jy, &points[k+3], 3*sizeof(double));

            vtkLine::Intersection(pMax, p, jx, jy, u, v);

            if((0.0 <= u) && (u <= 1.0) && (0.0 <= v) && (v <= 1.0)) ++count;
          }

          if(count % 2 != 0)
          {
            isInside = true;
          }
        }
      }

      if(!isInside)
      {
        perimeters[i].added = true;
        totalPerimeter += perimeters[i].length;
      }
    }

    unsigned int added = 0;
    unsigned int discarded = 0;
    for(unsigned int i = 0; i < perimeters.size(); ++i)
    {
      auto perimeter = perimeters.at(i);
      if(perimeter.added) ++added;

      // discard very small ones to simplify classification
      if(perimeter.length < totalPerimeter*0.1 && perimeter.points.size()/2 < 50)
      {
        ++discarded;

        if(perimeter.added) --added;

      }
    }

    auto validPerimeters = perimeters.size() - discarded;
    switch(added)
    {
      case 0:
        shape = Shape::UNKNOWN;
        break;
      case 1:
        if(validPerimeters == 1)
        {
          shape = Shape::MACULAR;
        }
        else
        {
          shape = Shape::PERFORATED;
        }
        break;
      default:
        if(added == validPerimeters)
        {
          shape = Shape::FRAGMENTED;
        }
        else
        {
          shape = Shape::FRAGMENTEDANDPERFORATED;
        }
        break;
    }
  }
  catch (...)
  {
    // std::cerr << "Warning: Couldn't compute " << m_seg->data().toString().toStdString() << "'s Apposition Plane perimter" << std::endl;
    totalPerimeter = UNDEFINED;
  }

  return std::make_pair(totalPerimeter, shape);
}

//------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> AppositionSurfaceExtension::projectPolyDataToPlane(const vtkSmartPointer<vtkPolyData> mesh) const
{
  double origin[3];
  double normal[3]; // Normal's magnitude is 1

  if(!mesh || !mesh->GetPointData() || !mesh->GetPoints())
  {
    m_hasErrors = true;
    return nullptr;
  }

  try
  {
    auto originArray = vtkDoubleArray::SafeDownCast(mesh->GetPointData()->GetArray(AppositionSurfaceFilter::MESH_ORIGIN));
    auto normalArray = vtkDoubleArray::SafeDownCast(mesh->GetPointData()->GetArray(AppositionSurfaceFilter::MESH_NORMAL));

    if(!originArray || !normalArray)
    {
      m_hasErrors = true;
      return nullptr;
    }

    for (int i = 0; i < 3; ++i)
    {
      origin[i] = originArray->GetValue(i);
      normal[i] = normalArray->GetValue(i);
    }

    auto plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(origin);
    plane->SetNormal(normal);

    int pointsCount = 0;
    double projected[3], p[3];

    auto pointsIn  = mesh->GetPoints();
    auto pointsOut = vtkSmartPointer<vtkPoints>::New();
    pointsCount = pointsIn->GetNumberOfPoints();
    pointsOut->SetNumberOfPoints(pointsCount);
    for (int i = 0; i < pointsCount; i++)
    {
      pointsIn->GetPoint(i, p);
      plane->ProjectPoint(p, projected);
      pointsOut->SetPoint(i, projected);
    }

    auto auxMesh = vtkSmartPointer<vtkPolyData>::New();
    auxMesh->DeepCopy(mesh);
    auxMesh->SetPoints(pointsOut);

    auto normals = PolyDataNormals::New();
    normals->SetInputData(auxMesh);
    normals->SplittingOff();
    normals->Update();

    auto projection = vtkSmartPointer<vtkPolyData>::New();
    projection->ShallowCopy(normals->GetOutput());

    return projection;
  }
  catch(...)
  {
    m_hasErrors = true;
    return nullptr;
  }

  return nullptr;
}

//------------------------------------------------------------------------
double AppositionSurfaceExtension::computeTortuosity(const vtkSmartPointer<vtkPolyData> asMesh, Nm asArea) const
{
  auto projectedAS = projectPolyDataToPlane(asMesh);

  if(projectedAS)
  {
    auto referenceArea = computeArea(projectedAS);

    if(referenceArea != 0)
    {
      return 1 - (referenceArea / asArea);
    }
  }

  m_hasErrors = true;

  return -1;
}

//------------------------------------------------------------------------
void AppositionSurfaceExtension::computeCurvatures(const vtkSmartPointer<vtkPolyData> asMesh,
						                                       vtkSmartPointer<vtkDoubleArray> gaussCurvature,
						                                       vtkSmartPointer<vtkDoubleArray> meanCurvature,
						                                       vtkSmartPointer<vtkDoubleArray> minCurvature,
						                                       vtkSmartPointer<vtkDoubleArray> maxCurvature) const
{
  if(asMesh)
  {
    auto curvatures_fliter = vtkSmartPointer<vtkCurvatures>::New();
    curvatures_fliter->SetInputData(asMesh);

    auto output = curvatures_fliter->GetOutput();

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
  else
  {
    m_hasErrors = true;
  }
}


//------------------------------------------------------------------------
// Aux function to compute the mean
double mean(const vtkSmartPointer<vtkDoubleArray> dataArray)
{
  double mean = 0;

  if(dataArray)
  {
    auto num_pts = dataArray->GetNumberOfTuples();

    for (int i = 0; i < num_pts; i++)
    {
      mean += dataArray->GetValue(i);
    }

    mean = mean / double(num_pts);
  }

  return mean;
}

//------------------------------------------------------------------------
// Aux function to compute the Standard Deviation
double stdDev(const vtkSmartPointer<vtkDoubleArray> dataArray, const double mean)
{
  double std_dev = 0;

  if(dataArray)
  {
    double acum = 0;
    auto num_pts = dataArray->GetNumberOfTuples();

    for (int i = 0; i < num_pts; i++)
    {
      acum += pow(dataArray->GetValue(i) - mean, 2);
    }

    std_dev = sqrt(acum / double(num_pts));
  }

  return std_dev;
}

//------------------------------------------------------------------------
bool AppositionSurfaceExtension::computeInformation() const
{
  if(!m_extendedItem) return false;

  m_hasErrors = false;
  bool validInformation = false;

  const auto mesh = writeLockMesh(m_extendedItem->output())->mesh();

  if (mesh)
  {
    auto gaussCurvature = vtkSmartPointer<vtkDoubleArray>::New();
    auto meanCurvature  = vtkSmartPointer<vtkDoubleArray>::New();
    auto maxCurvature   = vtkSmartPointer<vtkDoubleArray>::New();
    auto minCurvature   = vtkSmartPointer<vtkDoubleArray>::New();

    computeCurvatures(mesh, gaussCurvature, meanCurvature, minCurvature, maxCurvature);

    auto area = computeArea(mesh);
    updateInfoCache(AREA, area);
    auto value = computePerimeterAndShape(mesh);
    updateInfoCache(PERIMETER, value.first);

    switch(value.second)
    {
      case Shape::MACULAR:
        updateInfoCache(SHAPE, "Macular");
        break;
      case Shape::FRAGMENTED:
        updateInfoCache(SHAPE, "Fragmented");
        break;
      case Shape::FRAGMENTEDANDPERFORATED:
        updateInfoCache(SHAPE, "Fragmented and perforated");
        break;
      case Shape::PERFORATED:
        updateInfoCache(SHAPE, "Perforated");
        break;
      default:
        updateInfoCache(SHAPE, "Unknown");
        break;
    }

    updateInfoCache(TORTUOSITY, computeTortuosity(mesh, area));

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

    validInformation = true;
  }

  if(m_hasErrors || !validInformation)
  {
    auto message = QString("Failed to compute");

    for(auto key: {AREA, PERIMETER, TORTUOSITY, MEAN_GAUSS_CURVATURE, STD_DEV_GAUS_CURVATURE, MEAN_MEAN_CURVATURE, STD_DEV_MEAN_CURVATURE,
                   MEAN_MIN_CURVATURE, STD_DEV_MIN_CURVATURE, MEAN_MAX_CURVATURE, STD_DEV_MAX_CURVATURE})
    {
      updateInfoCache(key, message);
    }
  }

  return !m_hasErrors && validInformation;
}

//------------------------------------------------------------------------
bool AppositionSurfaceExtension::validData(const OutputSPtr output) const
{
  return hasVolumetricData(output);
}
