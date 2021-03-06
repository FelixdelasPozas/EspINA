/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include "vtkSkeletonWidgetRepresentation.h"
#include <Core/Analysis/Data/SkeletonDataUtils.h>
#include <GUI/Representations/Pipelines/SegmentationSkeletonPipelineBase.h>
#include <GUI/View/View2D.h>
#include <GUI/View/Utils.h>

// VTK
#include <vtkObjectFactory.h>
#include <vtkOpenGL.h>
#include <vtkGlyph3D.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkCellArray.h>
#include <vtkIdList.h>
#include <vtkLine.h>
#include <vtkCellData.h>
#include <vtkLookupTable.h>
#include <vtkSphereSource.h>
#include <vtkRenderWindow.h>
#include <vtkPlane.h>
#include <vtkStringArray.h>
#include <vtkTextProperty.h>
#include <vtkPointSetToLabelHierarchy.h>
#include <vtkLabelPlacementMapper.h>
#include <vtkActor2D.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkFollower.h>
#include <vtkGlyphSource2D.h>
#include <vtkGlyph3DMapper.h>
#include <vtkTransform.h>
#include <vtkFreeTypeLabelRenderStrategy.h>
#include <vtkDoubleArray.h>

// Qt
#include <QMutexLocker>
#include <QColor>

using ESPINA::GUI::Representations::SegmentationSkeletonPipelineBase;

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::GUI::View::Utils;
using namespace ESPINA::GUI::View::Widgets::Skeleton;

vtkStandardNewMacro(vtkSkeletonWidgetRepresentation);

Core::SkeletonNode *vtkSkeletonWidgetRepresentation::s_currentVertex = nullptr;
Core::SkeletonDefinition vtkSkeletonWidgetRepresentation::s_skeleton;
NmVector3 vtkSkeletonWidgetRepresentation::s_skeletonSpacing = NmVector3{1,1,1};
QMutex vtkSkeletonWidgetRepresentation::s_skeletonMutex;

//-----------------------------------------------------------------------------
vtkSkeletonWidgetRepresentation::vtkSkeletonWidgetRepresentation()
: m_orientation        {Plane::UNDEFINED}
, m_tolerance          {std::sqrt(20)}
, m_slice              {-1}
, m_shift              {0}
, m_defaultHue         {100}
, m_labelColor         {QColor::fromRgbF(1,1,1)}
, m_labelSize          {5}
, m_width              {1}
, m_currentStrokeIndex {-1}
, m_currentEdgeIndex   {-1}
, m_changeCoincidentHue{false}
, m_ignoreCursor       {false}
{
  m_colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  m_colors->SetName("Colors");
  m_colors->SetNumberOfComponents(3);

  m_points = vtkSmartPointer<vtkPoints>::New();
  m_points->SetDataTypeToDouble();

  m_pointsData = vtkSmartPointer<vtkPolyData>::New();
  m_pointsData->SetPoints(m_points);
  m_pointsData->GetPointData()->SetScalars(m_colors);

  m_glypher = vtkSmartPointer<vtkGlyph3D>::New();
  m_glypher->SetInputData(m_pointsData);
  m_glypher->SetColorModeToColorByScalar();

  m_glypher->ScalingOn();
  m_glypher->SetScaleModeToDataScalingOff();
  m_glypher->SetScaleFactor(1.0);

  // Use a sphere for nodes.
  auto sphere = vtkSmartPointer<vtkSphereSource>::New();
  sphere->SetRadius(0.5);
  m_glypher->SetSourceConnection(sphere->GetOutputPort());

  auto sphere2 = vtkSmartPointer<vtkSphereSource>::New();
  sphere2->SetRadius(0.75);

  auto polyData = vtkSmartPointer<vtkPolyData>::New();
  auto points = vtkSmartPointer<vtkPoints>::New();
  points->SetDataTypeToDouble();
  polyData->SetPoints(points);

  m_pointer = vtkSmartPointer<vtkGlyph3D>::New();
  m_pointer->SetSourceConnection(sphere2->GetOutputPort());
  m_pointer->SetInputData(polyData);
  m_pointer->ScalingOn();
  m_pointer->SetScaleModeToDataScalingOff();
  m_pointer->SetScaleFactor(1.0);

  auto pointerMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  pointerMapper->SetInputData(m_pointer->GetOutput());
  pointerMapper->SetResolveCoincidentTopologyToOff();

  m_pointerActor = vtkSmartPointer<vtkActor>::New();
  m_pointerActor->SetMapper(pointerMapper);
  m_pointerActor->GetProperty()->SetColor(1, 1, 1);

  m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_mapper->SetInputData(m_glypher->GetOutput());
  m_mapper->SetResolveCoincidentTopologyToOff();

  m_actor = vtkSmartPointer<vtkActor>::New();
  m_actor->SetMapper(m_mapper);
  m_actor->GetProperty()->SetLineWidth(m_width + 1);
  m_actor->GetProperty()->SetPointSize(1);

  m_lines = vtkSmartPointer<vtkPolyData>::New();
  m_linesMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_linesMapper->SetInputData(m_lines);
  m_linesMapper->SetResolveCoincidentTopologyToOff();

  m_linesActor = vtkSmartPointer<vtkActor>::New();
  m_linesActor->SetMapper(m_linesMapper);
  m_linesActor->GetProperty()->SetLineWidth(m_width + 1);

  m_dashedLines = vtkSmartPointer<vtkPolyData>::New();
  m_dashedLinesMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_dashedLinesMapper->SetInputData(m_dashedLines);
  m_dashedLinesMapper->SetResolveCoincidentTopologyToOff();

  m_dashedLinesActor = vtkSmartPointer<vtkActor>::New();
  m_dashedLinesActor->SetMapper(m_dashedLinesMapper);
  m_dashedLinesActor->GetProperty()->SetLineWidth(m_width + 1);
  SegmentationSkeletonPipelineBase::stippledLine(m_dashedLinesActor, 0xF0F0);

  m_truncatedPoints = vtkSmartPointer<vtkPoints>::New();
  m_truncatedPoints->SetDataTypeToDouble();

  m_truncatedData = vtkSmartPointer<vtkPolyData>::New();
  m_truncatedData->SetPoints(m_truncatedPoints);

  m_glyph2D = vtkSmartPointer<vtkGlyphSource2D>::New();
  m_glyph2D->SetGlyphTypeToSquare();
  m_glyph2D->SetFilled(false);
  m_glyph2D->SetScale(2.2);
  m_glyph2D->SetCenter(0,0,0);
  m_glyph2D->SetColor(1,0,0);
  m_glyph2D->Update();

  m_glyphMapper = vtkSmartPointer<vtkGlyph3DMapper>::New();
  m_glyphMapper->SetScalarVisibility(false);
  m_glyphMapper->SetStatic(true);
  m_glyphMapper->ScalingOn();
  m_glyphMapper->SetScaleModeToNoDataScaling();
  m_glyphMapper->SetScaleFactor(1.0);
  m_glyphMapper->SetInputData(m_truncatedData);
  m_glyphMapper->SetSourceData(m_glyph2D->GetOutput());

  m_truncatedActor = vtkSmartPointer<vtkFollower>::New();
  m_truncatedActor->SetMapper(m_glyphMapper);
  m_truncatedActor->GetProperty()->SetColor(1, 0, 0);
  m_truncatedActor->GetProperty()->Modified();

  m_showLabels = true;

  m_labelPoints = vtkSmartPointer<vtkPoints>::New();
  m_labelPoints->SetDataTypeToDouble();

  m_labels = vtkSmartPointer<vtkStringArray>::New();
  m_labels->SetName("Labels");

  m_labelData = vtkSmartPointer<vtkPolyData>::New();
  m_labelData->SetPoints(m_labelPoints);
  m_labelData->GetPointData()->AddArray(m_labels);

  m_labelProperty = vtkSmartPointer<vtkTextProperty>::New();
  m_labelProperty->SetBold(true);
  m_labelProperty->SetFontFamilyToArial();
  m_labelProperty->SetShadow(false);
  m_labelProperty->SetFontSize(m_labelSize);
  m_labelProperty->SetJustificationToCentered();
  m_labelProperty->SetVerticalJustificationToCentered();
  m_labelProperty->Modified();

  m_labelFilter = vtkSmartPointer<vtkPointSetToLabelHierarchy>::New();
  m_labelFilter->SetInputData(m_labelData);
  m_labelFilter->SetLabelArrayName("Labels");
  m_labelFilter->GetTextProperty()->SetFontSize(m_labelSize);
  m_labelFilter->GetTextProperty()->SetBold(true);

  auto strategy = vtkSmartPointer<vtkFreeTypeLabelRenderStrategy>::New();
  strategy->SetDefaultTextProperty(m_labelProperty);

  m_labelPlacer = vtkSmartPointer<vtkLabelPlacementMapper>::New();
  m_labelPlacer->SetInputConnection(m_labelFilter->GetOutputPort());
  m_labelPlacer->SetPlaceAllLabels(true);
  m_labelPlacer->SetShapeToRoundedRect();
  m_labelPlacer->SetMaximumLabelFraction(0.9);
  m_labelPlacer->SetUseDepthBuffer(false);
  m_labelPlacer->SetStyleToFilled();
  m_labelPlacer->SetRenderStrategy(strategy);
  m_labelPlacer->SetGeneratePerturbedLabelSpokes(false);
  m_labelPlacer->SetBackgroundColor(m_labelColor.redF() * 0.6, m_labelColor.greenF()*0.6, m_labelColor.blueF()*0.6);
  m_labelPlacer->SetBackgroundOpacity(0.5);
  m_labelPlacer->SetMargin(3);

  m_labelActor = vtkSmartPointer<vtkActor2D>::New();
  m_labelActor->SetMapper(m_labelPlacer);
  m_labelActor->SetDragable(false);
  m_labelActor->SetPickable(false);
  m_labelActor->SetUseBounds(false);

  m_interactionOffset[0] = 0.0;
  m_interactionOffset[1] = 0.0;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::AddNodeAtPosition(double worldPos[3])
{
  // centering coordinates in voxel center allows points to be later moved easily in other views.
  for(int i: {0,1,2})
  {
    worldPos[i] = std::round(worldPos[i]/m_spacing[i]) * m_spacing[i];
  }

  {
    auto stroke = currentStroke();
    if(stroke.isNull()) return ;

    QMutexLocker lock(&s_skeletonMutex);

    worldPos[normalCoordinateIndex(m_orientation)] = m_slice;
    auto node = new SkeletonNode{worldPos};

    s_skeleton.nodes << node;

    SkeletonEdge edge;

    if (s_currentVertex != nullptr)
    {
      switch(s_currentVertex->connections.size())
      {
        case 1:
          {
            auto otherNode = s_currentVertex->connections.keys().first();
            m_currentEdgeIndex = s_currentVertex->connections[otherNode];
            m_currentStrokeIndex = s_skeleton.edges.at(m_currentEdgeIndex).strokeIndex;

            if(s_currentVertex->flags != SkeletonNodeFlags())
            {
              node->flags = s_currentVertex->flags;
              s_currentVertex->flags = SkeletonNodeFlags();
            }
          }
          break;
        default:
          {
            unsigned int prioritary = 0;
            int prioritaryIndex = -1;
            for(auto edgeIndex: s_currentVertex->connections.values())
            {
              auto otherStroke = s_skeleton.strokes.at(s_skeleton.edges.at(edgeIndex).strokeIndex);
              if(otherStroke.useMeasure)
              {
                ++prioritary;
                prioritaryIndex = edgeIndex;
                if(prioritary > 1) break;
              }
            }

            if(prioritary == 1 && (!stroke.useMeasure || s_currentVertex->connections.size() == 2))
            {
              m_currentEdgeIndex = prioritaryIndex;
              m_currentStrokeIndex = s_skeleton.edges.at(m_currentEdgeIndex).strokeIndex;
            }
            else
            {
              s_skeleton.count[stroke]++;

              edge.strokeIndex  = s_skeleton.strokes.indexOf(stroke);
              edge.strokeNumber = s_skeleton.count[stroke];
              edge.parentEdge   = prioritaryIndex;
              s_skeleton.edges << edge;

              m_currentEdgeIndex = s_skeleton.edges.indexOf(edge);
            }
          }
          break;
      }

      s_currentVertex->connections.insert(node, m_currentEdgeIndex);
      node->connections.insert(s_currentVertex, m_currentEdgeIndex);
    }

    s_currentVertex = node;
  }

  BuildRepresentation();
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::AddNodeAtWorldPosition(double x, double y, double z)
{
  double worldPos[3]{x,y,z};
  AddNodeAtPosition(worldPos);
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::AddNodeAtDisplayPosition(int displayPos[2])
{
  double worldPos[3];
  GetWorldPositionFromDisplayPosition(displayPos, worldPos);
  AddNodeAtPosition(worldPos);
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::AddNodeAtDisplayPosition(int X, int Y)
{
  int displayPos[2]{X,Y};
  AddNodeAtDisplayPosition(displayPos);
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::ActivateNode(int displayPos[2])
{
  SkeletonNode *closestNode = nullptr;

  double worldPos[3];
  GetWorldPositionFromDisplayPosition(displayPos, worldPos);

  double distance2 = VTK_DOUBLE_MAX;
  const auto nodes = m_visiblePoints.keys();

  auto closestNodeOp = [&distance2, &worldPos, &closestNode](SkeletonNode * const &node)
  {
    auto nodeDistance = vtkMath::Distance2BetweenPoints(worldPos, node->position);
    if (nodeDistance < distance2)
    {
      distance2 = nodeDistance;
      closestNode = node;
    }
  };
  std::for_each(nodes.constBegin(), nodes.constEnd(), closestNodeOp);

  if (closestNode == nullptr || (m_tolerance < distance2))
  {
    DeactivateNode();
  }
  else
  {
    ActivateNode(closestNode);
  }

  QMutexLocker lock(&s_skeletonMutex);

  return (s_currentVertex != nullptr);
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::ActivateNode(int X, int Y)
{
  int displayPos[2]{X, Y};

  return ActivateNode(displayPos);
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::ActivateNode(SkeletonNode *node)
{
  {
    QMutexLocker lock(&s_skeletonMutex);
    s_currentVertex = node;
  }
  UpdatePointer();

  return true;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::DeactivateNode()
{
  {
    QMutexLocker lock(&s_skeletonMutex);
    s_currentVertex = nullptr;
  }
  UpdatePointer();
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::SetActiveNodeToDisplayPosition(int displayPos[2], bool updateRepresentation)
{
  double worldPos[3];
  GetWorldPositionFromDisplayPosition(displayPos, worldPos);

  return SetActiveNodeToWorldPosition(worldPos, updateRepresentation);
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::SetActiveNodeToDisplayPosition(int X, int Y, bool updateRepresentation)
{
  int displayPos[2]{X,Y};

  return SetActiveNodeToDisplayPosition(displayPos, updateRepresentation);
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::SetActiveNodeToWorldPosition(double x, double y, double z, bool updateRepresentation)
{
  double worldPos[3]{x,y,z};

  return SetActiveNodeToWorldPosition(worldPos, updateRepresentation);
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::SetActiveNodeToWorldPosition(double worldPos[3], bool updateRepresentation)
{
  {
    QMutexLocker lock(&s_skeletonMutex);

    if (s_currentVertex == nullptr) return false;

    for(int i: {0,1,2})
    {
      worldPos[i] = std::round(worldPos[i]/m_spacing[i]) * m_spacing[i];
    }

    std::memcpy(s_currentVertex->position, worldPos, 3 * sizeof(double));
  }

  if (updateRepresentation)
  {
    BuildRepresentation();
  }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::SetActiveNodeToLastNode(bool updateRepresentation)
{
  {
    QMutexLocker lock(&s_skeletonMutex);

    if(s_skeleton.nodes.isEmpty()) return false;

    s_currentVertex = s_skeleton.nodes.last();
  }

  if (updateRepresentation)
  {
    BuildRepresentation();
  }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::GetActiveNodeWorldPosition(double worldPos[3])
{
  {
    QMutexLocker lock(&s_skeletonMutex);

    if (s_currentVertex == nullptr) return false;

    std::memcpy(worldPos, s_currentVertex->position, 3 * sizeof(double));
  }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::DeleteCurrentNode()
{
  {
    QMutexLocker lock(&s_skeletonMutex);

    if (s_currentVertex == nullptr) return false;

    deleteNode(s_currentVertex);

    s_currentVertex = nullptr;
  }

  BuildRepresentation();

  return true;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::deleteNode(SkeletonNode *node)
{
  SkeletonNodes toDelete;

  if (!node) return;

  auto connections = node->connections.keys();

  if(connections.size() == 1 && node->flags != SkeletonNodeFlags())
  {
    connections.first()->flags = node->flags;
  }

  connections.removeAll(node);

  s_skeleton.nodes.removeAll(node);

  delete node;
  node = nullptr;

  std::for_each(connections.constBegin(), connections.constEnd(), [&toDelete](SkeletonNode *node) { if(node->connections.isEmpty()) toDelete << node; });
  std::for_each(toDelete.begin(), toDelete.end(), [this](SkeletonNode *node){ this->deleteNode(node); });
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::AddNodeOnContour(int X, int Y)
{
  SkeletonNode *currentVertex = nullptr;
  {
    QMutexLocker lock(&s_skeletonMutex);

    if(s_currentVertex)
    {
      currentVertex = s_currentVertex;
      for(auto connection: s_currentVertex->connections.keys())
      {
        connection->connections.remove(s_currentVertex);
      }
      s_skeleton.nodes.removeAll(s_currentVertex);
      s_currentVertex = nullptr;
    }
  }

  double worldPos[3]{0,0,0};
  int node_i = VTK_INT_MAX;
  int node_j = VTK_INT_MAX;
  auto distance = FindClosestDistanceAndNode(X, Y, worldPos, node_i, node_j);

  if(currentVertex)
  {
    QMutexLocker lock(&s_skeletonMutex);

    s_currentVertex = currentVertex;
    s_skeleton.nodes << s_currentVertex;
    for(auto connection: s_currentVertex->connections.keys())
    {
      auto value = s_currentVertex->connections[connection];
      if(!connection->connections.contains(s_currentVertex)) connection->connections.insert(s_currentVertex, value);
    }
  }

  if (distance > m_tolerance) return false;

  if (node_i != node_j)
  {
    {
      QMutexLocker lock(&s_skeletonMutex);
      if(s_currentVertex && s_skeleton.nodes[node_i]->connections[s_skeleton.nodes[node_j]] == m_currentEdgeIndex) return false;
    }

    AddNodeAtPosition(worldPos); // adds a new node and makes it current node, so it's joined to cursor, if any.
    int contourEdgeIndex = -1;
    {
      QMutexLocker lock(&s_skeletonMutex);
      contourEdgeIndex = s_skeleton.nodes[node_i]->connections[s_skeleton.nodes[node_j]];

      s_currentVertex->connections.insert(s_skeleton.nodes[node_i], contourEdgeIndex);
      s_currentVertex->connections.insert(s_skeleton.nodes[node_j], contourEdgeIndex);

      s_skeleton.nodes[node_i]->connections.remove(s_skeleton.nodes[node_j]);
      s_skeleton.nodes[node_i]->connections.insert(s_currentVertex, contourEdgeIndex);

      s_skeleton.nodes[node_j]->connections.remove(s_skeleton.nodes[node_i]);
      s_skeleton.nodes[node_j]->connections.insert(s_currentVertex, contourEdgeIndex);
    }

    if(currentVertex)
    {
      QMutexLocker lock(&s_skeletonMutex);

      for(auto connection: currentVertex->connections.keys())
      {
        if(connection == s_currentVertex) continue;
        connection->connections.insert(s_currentVertex, currentVertex->connections[connection]);
        s_currentVertex->connections.insert(connection, currentVertex->connections[connection]);
        connection->connections.remove(currentVertex);
      }
      currentVertex->connections.clear();
      currentVertex->connections.insert(s_currentVertex, m_currentEdgeIndex);
      s_currentVertex = currentVertex;

      s_skeleton.edges[m_currentEdgeIndex].parentEdge = contourEdgeIndex;
    }
    else
    {
      AddNodeAtPosition(worldPos);
    }
  }
  else
  {
    {
      // avoid loops of same stroke.
      QMutexLocker lock(&s_skeletonMutex);
      auto addedNode = s_skeleton.nodes[node_i];
      const auto nodeConn = addedNode->connections.keys();
      auto operation = [this, addedNode](SkeletonNode *node) { return addedNode->connections.value(node) == this->m_currentEdgeIndex; };
      auto count = std::count_if(nodeConn.constBegin(), nodeConn.constEnd(), operation);
      if(count > 1) return false;
    }

    SkeletonNode *addedNode = nullptr;
    int nodeEdge = -1;
    int noPrioritary = -1;
    {
      QMutexLocker lock(&s_skeletonMutex);

      addedNode = s_skeleton.nodes[node_i];
      for(auto connection: addedNode->connections.values())
      {
        auto &edge = s_skeleton.edges.at(connection);
        auto &stroke = s_skeleton.strokes.at(edge.strokeIndex);
        if(stroke.useMeasure)
        {
          nodeEdge = connection;
        }
        else
        {
          noPrioritary = connection;
        }
      }
    }

    if(currentVertex)
    {
      QMutexLocker lock(&s_skeletonMutex);

      for(auto connection: s_currentVertex->connections.keys())
      {
        if(connection == addedNode) continue;
        addedNode->connections.insert(connection, s_currentVertex->connections[connection]);
        connection->connections.insert(addedNode, s_currentVertex->connections[connection]);
        connection->connections.remove(s_currentVertex);
      }

      s_currentVertex->connections.clear();
      s_currentVertex->connections.insert(addedNode, m_currentEdgeIndex);
      addedNode->connections.insert(s_currentVertex, m_currentEdgeIndex);

      s_skeleton.edges[m_currentEdgeIndex].parentEdge = (nodeEdge != -1 ? nodeEdge : noPrioritary);
    }
    else
    {
      {
        QMutexLocker lock(&s_skeletonMutex);
        s_currentVertex = addedNode;
      }

      AddNodeAtPosition(worldPos);
    }
  }

  BuildRepresentation();

  return true;
}

//-----------------------------------------------------------------------------
unsigned int vtkSkeletonWidgetRepresentation::GetNumberOfNodes() const
{
  QMutexLocker lock(&s_skeletonMutex);

  return s_skeleton.nodes.size();
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::SetOrientation(Plane plane)
{
  if (m_orientation == Plane::UNDEFINED && plane != Plane::UNDEFINED)
  {
    m_orientation = plane;

    if(m_orientation != Plane::XY)
    {
      auto transform = vtkSmartPointer<vtkTransform>::New();
      transform->RotateWXYZ(90, (plane == Plane::YZ ? 0 : 1), (plane == Plane::XZ ? 0 : 1), 0);
      m_transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      m_transformFilter->SetTransform(transform);
      m_transformFilter->SetInputData(m_glyph2D->GetOutput());
      m_transformFilter->Update();

      m_glyphMapper->SetSourceData(m_transformFilter->GetOutput());
    }

    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::BuildRepresentation()
{
  int numNodes = 0;
  {
    QMutexLocker lock(&s_skeletonMutex);
    numNodes = s_skeleton.nodes.size();
  }

  if (numNodes == 0 || !Renderer || !Renderer->GetActiveCamera())
  {
    VisibilityOff();
    return;
  }

  auto strokes = s_skeleton.strokes;
  auto edges   = s_skeleton.edges;

  double p1[4], p2[4];
  Renderer->GetActiveCamera()->GetFocalPoint(p1);
  p1[3] = 1.0;
  Renderer->SetWorldPoint(p1);
  Renderer->WorldToView();
  Renderer->GetViewPoint(p1);

  double depth = p1[2];
  double aspect[2];
  Renderer->ComputeAspect();
  Renderer->GetAspect(aspect);

  p1[0] = -aspect[0];
  p1[1] = -aspect[1];
  Renderer->SetViewPoint(p1);
  Renderer->ViewToWorld();
  Renderer->GetWorldPoint(p1);

  p2[0] = aspect[0];
  p2[1] = aspect[1];
  p2[2] = depth;
  p2[3] = 1.0;
  Renderer->SetViewPoint(p2);
  Renderer->ViewToWorld();
  Renderer->GetWorldPoint(p2);

  double distance = sqrt(vtkMath::Distance2BetweenPoints(p1, p2));

  int *size = Renderer->GetRenderWindow()->GetSize();
  double viewport[4];
  Renderer->GetViewport(viewport);

  double x, y, scale;

  x = size[0] * (viewport[2] - viewport[0]);
  y = size[1] * (viewport[3] - viewport[1]);

  scale = sqrt(x * x + y * y);

  // to use as scale factor later.
  distance = 1000 * distance / scale;

  m_points->Reset();
  m_points->SetDataTypeToDouble();

  m_lines->Reset();
  m_dashedLines->Reset();
  m_truncatedPoints->Reset();
  m_truncatedPoints->SetDataTypeToDouble();

  m_labelPoints->Reset();
  m_labelPoints->SetDataTypeToDouble();

  m_labels->Reset();
  m_labels->SetName("Labels");

  auto cells = vtkSmartPointer<vtkCellArray>::New();
  auto dashedCells = vtkSmartPointer<vtkCellArray>::New();

  auto cellsColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  cellsColors->SetNumberOfComponents(3);
  auto dashedCellsColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  dashedCellsColors->SetNumberOfComponents(3);

  auto planeIndex = normalCoordinateIndex(m_orientation);

  unsigned char red[3]         {255,  0,  0};
  unsigned char green[3]       {  0,255,  0};
  unsigned char blue[3]        {  0,  0,255};
  unsigned char intersection[3]{255,  0,255};
  m_colors->Reset();
  m_colors->SetName("Colors");
  m_colors->SetNumberOfComponents(3);

  m_visiblePoints.clear();
  QSet<int> truncatedStrokes, visibleStrokes;
  double worldPos[3];

  s_skeletonMutex.lock();

  for(int i = 0; i < numNodes; ++i)
  {
    const auto &node = s_skeleton.nodes.at(i);

    for(auto other: node->connections.keys())
    {
      // remove self-cycles.
      if(other == node) node->connections.remove(other);
    }

    // isolated nodes to be removed.
    if(node->connections.isEmpty()) continue;

    if(node->flags.testFlag(SkeletonNodeProperty::TRUNCATED))
    {
      for(auto edgeIndex: node->connections.values())
      {
        truncatedStrokes << edgeIndex;
      }
    }

    // we only want "some" points in the screen representation and want all the representation to be visible.
    if (areEqual(node->position[planeIndex], m_slice))
    {
      if (!m_visiblePoints.contains(node))
      {
        std::memcpy(worldPos, node->position, 3 * sizeof(double));
        worldPos[planeIndex] = m_slice + m_shift;

        m_visiblePoints.insert(node, m_points->InsertNextPoint(worldPos));
        m_colors->InsertNextTypedTuple(green);

        if(node->flags.testFlag(SkeletonNodeProperty::TRUNCATED))
        {
          m_truncatedPoints->InsertNextPoint(worldPos);
        }
      }

      for (auto connectedNode : node->connections.keys())
      {
        if (!m_visiblePoints.contains(connectedNode))
        {
          std::memcpy(worldPos, connectedNode->position, 3 * sizeof(double));
          unsigned char *currentColor = areEqual(worldPos[planeIndex], m_slice) ? green : ((worldPos[planeIndex] > m_slice) ? red : blue);
          m_colors->InsertNextTypedTuple(currentColor);

          worldPos[planeIndex] = m_slice + m_shift;

          m_visiblePoints.insert(connectedNode, m_points->InsertNextPoint(worldPos));
          if(connectedNode->flags.testFlag(SkeletonNodeProperty::TRUNCATED))
          {
            m_truncatedPoints->InsertNextPoint(worldPos);
          }
        }

        auto line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, m_visiblePoints[node]);
        line->GetPointIds()->SetId(1, m_visiblePoints[connectedNode]);

        const auto edgeIndex = node->connections[connectedNode];
        const auto &edge     = edges.at(edgeIndex);
        const auto &stroke   = strokes.at(edge.strokeIndex);
        visibleStrokes << edgeIndex;

        vtkSmartPointer<vtkUnsignedCharArray> currentCellsColors = nullptr;
        if(stroke.type == 0)
        {
          cells->InsertNextCell(line);
          currentCellsColors = cellsColors;
        }
        else
        {
          dashedCells->InsertNextCell(line);
          currentCellsColors = dashedCellsColors;
        }

        const auto qcolor = computeCoincidentStrokeColor(stroke);
        unsigned char color[3]{static_cast<unsigned char>(qcolor.red()), static_cast<unsigned char>(qcolor.green()), static_cast<unsigned char>(qcolor.blue())};
        currentCellsColors->InsertNextTypedTuple(color);
      }
    }
    else
    {
      for (auto connectedNode: node->connections.keys())
      {
        if ((node->position[planeIndex] < m_slice && connectedNode->position[planeIndex] >= m_slice) ||
            (node->position[planeIndex] > m_slice && connectedNode->position[planeIndex] <= m_slice))
        {
          if (!m_visiblePoints.contains(node))
          {
            std::memcpy(worldPos, node->position, 3 * sizeof(double));
            unsigned char *currentColor = (worldPos[planeIndex] > m_slice) ? red : blue;
            m_colors->InsertNextTypedTuple(currentColor);

            worldPos[planeIndex] = m_slice + m_shift;

            m_visiblePoints.insert(node, m_points->InsertNextPoint(worldPos));
            if(node->flags.testFlag(SkeletonNodeProperty::TRUNCATED))
            {
              m_truncatedPoints->InsertNextPoint(worldPos);
            }
          }

          if (!m_visiblePoints.contains(connectedNode))
          {
            std::memcpy(worldPos, connectedNode->position, 3 * sizeof(double));
            unsigned char *currentColor = areEqual(worldPos[planeIndex], m_slice) ? green : ((worldPos[planeIndex] > m_slice) ? red : blue);
            m_colors->InsertNextTypedTuple(currentColor);

            worldPos[planeIndex] = m_slice + m_shift;

            m_visiblePoints.insert(connectedNode, m_points->InsertNextPoint(worldPos));
            if(connectedNode->flags.testFlag(SkeletonNodeProperty::TRUNCATED))
            {
              m_truncatedPoints->InsertNextPoint(worldPos);
            }
          }

          auto line = vtkSmartPointer<vtkLine>::New();
          line->GetPointIds()->SetId(0, m_visiblePoints[node]);
          line->GetPointIds()->SetId(1, m_visiblePoints[connectedNode]);

          const auto edgeIndex = node->connections[connectedNode];
          const auto &edge     = edges.at(edgeIndex);
          const auto &stroke   = strokes.at(edge.strokeIndex);
          visibleStrokes << edgeIndex;

          vtkSmartPointer<vtkUnsignedCharArray> currentCellsColors = nullptr;
          if(stroke.type == 0)
          {
            cells->InsertNextCell(line);
            currentCellsColors = cellsColors;
          }
          else
          {
            dashedCells->InsertNextCell(line);
            currentCellsColors = dashedCellsColors;
          }

          const auto qcolor = computeCoincidentStrokeColor(stroke);
          unsigned char color[3]{static_cast<unsigned char>(qcolor.red()), static_cast<unsigned char>(qcolor.green()), static_cast<unsigned char>(qcolor.blue())};
          currentCellsColors->InsertNextTypedTuple(color);

          // compute plane intersection point.
          if(!areEqual(node->position[planeIndex], m_slice) && !areEqual(connectedNode->position[planeIndex], m_slice))
          {
            double normal[3]{0,0,0};
            normal[planeIndex] = 1;
            double point[3]{0,0,0};
            point[planeIndex] = m_slice;
            double result[3]{0,0,0};
            double t = 0;
            auto intersect = vtkPlane::IntersectWithLine(node->position, connectedNode->position, normal, point, t, result);

            if(intersect != 0)
            {
              m_points->InsertNextPoint(result);
              m_colors->InsertNextTypedTuple(intersection);
            }
          }
        }
      }
    }
  }

  // 2020-05-23: fix spine name if child is truncated
  for(auto &index : truncatedStrokes)
  {
    auto edge = s_skeleton.edges.at(index);
    auto name = s_skeleton.strokes.at(edge.strokeIndex).name;
    int parentIndex = -1;
    QSet<int> visited;
    while(!name.startsWith("spine", Qt::CaseInsensitive) && edge.parentEdge != -1 && !visited.contains(edge.parentEdge))
    {
      parentIndex = edge.parentEdge;
      visited << parentIndex;
      edge = s_skeleton.edges.at(parentIndex);
      name = s_skeleton.strokes.at(edge.strokeIndex).name;
    }

    if(name.startsWith("spine", Qt::CaseInsensitive) && (parentIndex != -1) && !truncatedStrokes.contains(parentIndex))
    {
      truncatedStrokes << parentIndex;
    }
  }

  // labels
  if(m_showLabels && !m_visiblePoints.empty())
  {
    bool firstPass = true;
    while(!visibleStrokes.isEmpty())
    {
      for(auto node: m_visiblePoints.keys())
      {
        // avoid branches & current vertex if possible
        if(firstPass && (node->isBranching() || node == s_currentVertex)) continue;

        for(auto edge: node->connections.values())
        {
          if(visibleStrokes.contains(edge))
          {
            double visiblePos[3];
            m_points->GetPoint(m_visiblePoints[node], visiblePos);
            m_labelPoints->InsertNextPoint(visiblePos[0], visiblePos[1], visiblePos[2]);

            auto text = strokeName(s_skeleton.edges.at(edge), s_skeleton.strokes, s_skeleton.edges);
            visibleStrokes.remove(edge);
            if(truncatedStrokes.contains(edge)) text += " (Truncated)";
            m_labels->InsertNextValue(text.toStdString().c_str());

            break;
          }
        }
      }

      firstPass = false;
    }
  }

  s_skeletonMutex.unlock();

  if (m_visiblePoints.empty())
  {
    VisibilityOff();
    NeedToRenderOn();
    return;
  }

  cells->Modified();
  dashedCells->Modified();

  cellsColors->Modified();
  dashedCellsColors->Modified();

  m_points->Modified();
  m_pointsData->Modified();
  m_pointsData->GetPointData()->Modified();
  m_glypher->SetScaleFactor(distance * HandleSize);
  m_glypher->SetInputData(m_pointsData);
  m_glypher->UpdateWholeExtent();
  m_mapper->UpdateWholeExtent();
  m_actor->Modified();

  m_lines->SetLines(cells);
  m_lines->GetCellData()->SetScalars(cellsColors);
  m_lines->SetPoints(m_points);
  m_lines->Modified();
  m_linesMapper->SetInputData(m_lines);
  m_linesMapper->UpdateWholeExtent();
  m_linesActor->Modified();

  m_dashedLines->SetLines(dashedCells);
  m_dashedLines->SetPoints(m_points);
  // depending on the number of points the TCoords change, so we need to apply texture everytime.
  auto tcoords = vtkSmartPointer<vtkDoubleArray>::New();
  tcoords->SetNumberOfComponents(1);
  tcoords->SetNumberOfTuples(m_points->GetNumberOfPoints());
  for (int i = 0; i < m_points->GetNumberOfPoints(); ++i)
  {
    double value = static_cast<double>(i) * .5;
    tcoords->SetTypedTuple(i, &value);
  }

  m_dashedLines->GetPointData()->SetTCoords(tcoords);

  m_dashedLines->GetCellData()->SetScalars(dashedCellsColors);
  m_dashedLines->Modified();
  m_dashedLinesMapper->UpdateWholeExtent();
  m_dashedLinesActor->Modified();

  m_truncatedPoints->Modified();
  m_truncatedData->Modified();
  m_truncatedData->GetPointData()->Modified();
  m_glyphMapper->SetScaleFactor(distance * HandleSize);
  m_glyphMapper->UpdateWholeExtent();
  m_truncatedActor->SetVisibility(m_truncatedPoints->GetNumberOfPoints() != 0);
  m_truncatedActor->Modified();

  m_labelProperty->SetFontSize(m_labelSize);
  m_labelPoints->Modified();
  m_labels->Modified();
  m_labelData->Modified();
  m_labelFilter->Update();
  m_labelPlacer->RemoveAllClippingPlanes();
  m_labelPlacer->SetBackgroundColor(m_labelColor.redF() * 0.6, m_labelColor.greenF() * 0.6, m_labelColor.blueF() * 0.6);
  m_labelPlacer->SetBackgroundOpacity(0.5);
  m_labelPlacer->UpdateWholeExtent();
  m_labelActor->SetVisibility(m_showLabels);
  m_labelActor->Modified();

  m_pointer->SetScaleFactor(distance * HandleSize);
  VisibilityOn();

  UpdatePointer();
  NeedToRenderOn();
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::UpdatePointer()
{
  auto idx = normalCoordinateIndex(m_orientation);
  auto stroke = currentStroke();

  QMutexLocker lock(&s_skeletonMutex);

  if ((s_currentVertex != nullptr) && (std::abs(s_currentVertex->position[idx] - m_slice) <= std::abs(m_shift)))
  {
    double pos[3];
    std::memcpy(pos, s_currentVertex->position, 3 * sizeof(double));
    pos[idx] = m_slice + m_shift;

    auto polyData = vtkPolyData::SafeDownCast(m_pointer->GetInput());
    polyData->GetPoints()->Initialize();
    polyData->GetPoints()->SetNumberOfPoints(1);
    polyData->GetPoints()->SetPoint(0, pos);
    polyData->GetPoints()->Modified();
    polyData->Modified();
    m_pointer->Update();
    m_pointerActor->GetMapper()->Update();

    const auto color = stroke.colorHue == -1 ? m_defaultHue : stroke.colorHue;
    auto finalColor = QColor::fromHsv(color, 255,255);
    m_pointerActor->GetProperty()->SetColor(1 - finalColor.redF()/2., 1 - finalColor.greenF()/2., 1 - finalColor.blueF()/2.);
    m_pointerActor->VisibilityOn();
  }
  else
  {
    m_pointerActor->VisibilityOff();
  }

  m_pointerActor->Modified();
  NeedToRenderOn();
}

//-----------------------------------------------------------------------------
int vtkSkeletonWidgetRepresentation::ComputeInteractionState(int X, int Y, int vtkNotUsed(modified))
{
  SkeletonNode *currentNode = nullptr;

  {
    QMutexLocker lock(&s_skeletonMutex);
    if(m_ignoreCursor && s_currentVertex)
    {
      currentNode = s_currentVertex;

      s_skeleton.nodes.removeAll(s_currentVertex);
      for(auto connection: s_currentVertex->connections.keys())
      {
        connection->connections.remove(s_currentVertex);
      }

      s_currentVertex = nullptr;
    }
  }

  if (GetNumberOfNodes() != 0 && IsNearNode(X, Y))
  {
    InteractionState = vtkSkeletonWidgetRepresentation::NearPoint;
  }
  else
  {
    double worldPos[3];
    int unused1, unused2;
    if (GetNumberOfNodes() != 0 && (FindClosestDistanceAndNode(X, Y, worldPos, unused1, unused2) <= m_tolerance))
    {
      InteractionState = vtkSkeletonWidgetRepresentation::NearContour;
    }
    else
    {
      InteractionState = vtkSkeletonWidgetRepresentation::Outside;
    }
  }

  {
    QMutexLocker lock(&s_skeletonMutex);
    if(m_ignoreCursor && currentNode != nullptr)
    {
      s_currentVertex = currentNode;
      for(auto connection: s_currentVertex->connections.keys())
      {
        connection->connections.insert(s_currentVertex, s_currentVertex->connections[connection]);
      }

      s_skeleton.nodes << s_currentVertex;
    }
  }

  return InteractionState;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::ReleaseGraphicsResources(vtkWindow* win)
{
  m_actor->ReleaseGraphicsResources(win);
  m_pointerActor->ReleaseGraphicsResources(win);
  m_linesActor->ReleaseGraphicsResources(win);
  m_dashedLinesActor->ReleaseGraphicsResources(win);
  m_truncatedActor->ReleaseGraphicsResources(win);
  m_labelActor->ReleaseGraphicsResources(win);
}

//-----------------------------------------------------------------------------
int vtkSkeletonWidgetRepresentation::RenderOverlay(vtkViewport* viewport)
{
  int count = 0;

  if (m_pointerActor->GetVisibility())     count += m_pointerActor->RenderOverlay(viewport);
  if (m_linesActor->GetVisibility())       count += m_linesActor->RenderOverlay(viewport);
  if (m_dashedLinesActor->GetVisibility()) count += m_dashedLinesActor->RenderOverlay(viewport);
  if (m_actor->GetVisibility())            count += m_actor->RenderOverlay(viewport);
  if(m_truncatedActor->GetVisibility())    count += m_truncatedActor->RenderOverlay(viewport);
  if(m_labelActor->GetVisibility())        count += m_labelActor->RenderOverlay(viewport);

  return count;
}

//-----------------------------------------------------------------------------
int vtkSkeletonWidgetRepresentation::RenderOpaqueGeometry(vtkViewport* viewport)
{
  // Since we know RenderOpaqueGeometry gets called first, will do the build here
  BuildRepresentation();

  int count = 0;

  if (m_pointerActor->GetVisibility())     count += m_pointerActor->RenderOpaqueGeometry(viewport);
  if (m_linesActor->GetVisibility())       count += m_linesActor->RenderOpaqueGeometry(viewport);
  if (m_dashedLinesActor->GetVisibility()) count += m_dashedLinesActor->RenderOpaqueGeometry(viewport);
  if (m_actor->GetVisibility())            count += m_actor->RenderOpaqueGeometry(viewport);
  if (m_truncatedActor->GetVisibility())   count += m_truncatedActor->RenderOpaqueGeometry(viewport);
  if (m_labelActor->GetVisibility())       count += m_labelActor->RenderOpaqueGeometry(viewport);

  return count;
}

//-----------------------------------------------------------------------------
int vtkSkeletonWidgetRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport* viewport)
{
  int count = 0;

  if (m_pointerActor->GetVisibility())     count += m_pointerActor->RenderTranslucentPolygonalGeometry(viewport);
  if (m_linesActor->GetVisibility())       count += m_linesActor->RenderTranslucentPolygonalGeometry(viewport);
  if (m_dashedLinesActor->GetVisibility()) count += m_dashedLinesActor->RenderTranslucentPolygonalGeometry(viewport);
  if (m_actor->GetVisibility())            count += m_actor->RenderTranslucentPolygonalGeometry(viewport);
  if (m_truncatedActor->GetVisibility())   count += m_truncatedActor->RenderTranslucentPolygonalGeometry(viewport);
  if (m_labelActor->GetVisibility())       count += m_labelActor->RenderTranslucentPolygonalGeometry(viewport);

  return count;
}

//-----------------------------------------------------------------------------
int vtkSkeletonWidgetRepresentation::HasTranslucentPolygonalGeometry()
{
  int result = 0;

  if (m_pointerActor->GetVisibility())     result |= m_pointerActor->HasTranslucentPolygonalGeometry();
  if (m_linesActor->GetVisibility())       result |= m_linesActor->HasTranslucentPolygonalGeometry();
  if (m_dashedLinesActor->GetVisibility()) result |= m_dashedLinesActor->HasTranslucentPolygonalGeometry();
  if (m_actor->GetVisibility())            result |= m_actor->HasTranslucentPolygonalGeometry();
  if (m_truncatedActor->GetVisibility())   result |= m_truncatedActor->HasTranslucentPolygonalGeometry();
  if (m_labelActor->GetVisibility())       result |= m_labelActor->HasTranslucentPolygonalGeometry();

  return result;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> vtkSkeletonWidgetRepresentation::GetRepresentationPolyData()
{
  QMutexLocker lock(&s_skeletonMutex);

  Core::cleanSkeletonStrokes(s_skeleton);

  Core::removeIsolatedNodes(s_skeleton.nodes);

  Core::mergeSamePositionNodes(s_skeleton.nodes);

  performSpineSplitting();

  performPathsMerge();

  Core::adjustStrokeNumbers(s_skeleton);

  return Core::toPolyData(s_skeleton);
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::GetWorldPositionFromDisplayPosition(const int displayPos[2], double worldPos[3]) const
{
  double pos[4]{ static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]), 1.0, 1.0};

  Renderer->SetDisplayPoint(pos);
  Renderer->DisplayToWorld();
  Renderer->GetWorldPoint(pos);

  worldPos[0] = pos[0];
  worldPos[1] = pos[1];
  worldPos[2] = pos[2];
  worldPos[normalCoordinateIndex(m_orientation)] = m_slice;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::GetDisplayPositionFromWorldPosition(const double worldPos[3], int displayPos[2]) const
{
  double pos[4]{worldPos[0], worldPos[1], worldPos[2], 1.0};
  pos[normalCoordinateIndex(m_orientation)] = m_slice;

  Renderer->SetWorldPoint(pos);
  Renderer->WorldToDisplay();
  Renderer->GetDisplayPoint(pos);

  displayPos[0] = pos[0];
  displayPos[1] = pos[1];
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::FindClosestNode(const int X, const int Y, double worldPos[3], int &closestNode) const
{
  double distance = VTK_DOUBLE_MAX;
  const auto planeIndex = normalCoordinateIndex(m_orientation);
  double pos[4];
  int displayPos[2]{X,Y};

  GetWorldPositionFromDisplayPosition(displayPos, pos);

  auto closestNodeOp = [this, &distance, planeIndex, &pos, &worldPos, &closestNode](SkeletonNode * const &node)
  {
    if(!areEqual(node->position[planeIndex], m_slice)) return;

    auto nodeDistance = vtkMath::Distance2BetweenPoints(pos, node->position);
    if(distance > nodeDistance)
    {
      distance = nodeDistance;
      closestNode = s_skeleton.nodes.indexOf(node);

      std::memcpy(worldPos, node->position, 3* sizeof(double));
    }
  };

  std::for_each(s_skeleton.nodes.constBegin(), s_skeleton.nodes.constEnd(), closestNodeOp);
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::Initialize(vtkSmartPointer<vtkPolyData> pd)
{
  bool isEmpty = true;
  {
    QMutexLocker lock(&s_skeletonMutex);
    isEmpty = s_skeleton.nodes.isEmpty();
  }

  if(!isEmpty)
  {
    ClearRepresentation();
  }

  Q_ASSERT(s_skeleton.nodes.isEmpty());

  if(pd != nullptr && pd->GetNumberOfPoints() > 0)
  {
    QMutexLocker lock(&s_skeletonMutex);
    s_skeleton = Core::toSkeletonDefinition(pd);
    Core::cleanSkeletonStrokes(s_skeleton);
    Core::removeIsolatedNodes(s_skeleton.nodes);
    Core::mergeSamePositionNodes(s_skeleton.nodes);
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::PrintSelf(std::ostream &os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  Superclass::PrintSelf(os, indent);
  os << indent << "Number of points in the skeleton: " << s_skeleton.nodes.size() << std::endl;
  os << indent << "Tolerance: " << m_tolerance << std::endl;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::SetSpacing(const NmVector3& spacing)
{
  if(m_spacing != spacing)
  {
    if(s_skeletonMutex.tryLock())
    {
      if(s_skeletonSpacing != spacing)
      {
        auto changeSpacingOp = [&spacing](SkeletonNode *node)
        {
          node->position[0] = node->position[0]/s_skeletonSpacing[0] * spacing[0];
          node->position[1] = node->position[1]/s_skeletonSpacing[1] * spacing[1];
          node->position[2] = node->position[2]/s_skeletonSpacing[2] * spacing[2];
        };

        std::for_each(s_skeleton.nodes.begin(), s_skeleton.nodes.end(), changeSpacingOp);

        s_skeletonSpacing = spacing;
      }

      s_skeletonMutex.unlock();
    }

    m_spacing = spacing;

    const auto planeIdx = normalCoordinateIndex(m_orientation);
    double max = -1;
    for(auto i: {0,1,2})
    {
      if(i == planeIdx) continue;
      max = std::max(spacing[i], max);
    }

    m_tolerance = max * max;

    BuildRepresentation();
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::ClearRepresentation()
{
  QMutexLocker lock(&s_skeletonMutex);

  s_skeleton.clear();

  s_currentVertex = nullptr;
}

//-----------------------------------------------------------------------------
double vtkSkeletonWidgetRepresentation::FindClosestDistanceAndNode(const int X, const int Y, double worldPos[3], int &node_i, int &node_j) const
{
  if (!Renderer) return VTK_DOUBLE_MAX;

  std::memset(worldPos, 0, 3*sizeof(double));
  node_i = node_j = VTK_INT_MAX;

  double point_pos[3];
  int displayPos[2]{X,Y};
  GetWorldPositionFromDisplayPosition(displayPos, point_pos);

  auto result = Core::closestDistanceAndNode(point_pos, s_skeleton.nodes, node_i, node_j, worldPos);

  // NOTE: the Core:: util method returns the sqrt, that is, the real distance, but we're using the square of that in all of our
  // computations in the widget to avoid the square root operation.
  return result * result;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::SetSlice(const Nm slice)
{
  m_slice = slice;
  BuildRepresentation();
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::IsNearNode(int x, int y) const
{
  if (Renderer)
  {
    double worldPos[3];
    int nodeIndex = VTK_INT_MAX;
    if(m_ignoreCursor && s_currentVertex) s_skeleton.nodes.removeAll(s_currentVertex);
    FindClosestNode(x, y, worldPos, nodeIndex);
    if(m_ignoreCursor && s_currentVertex) s_skeleton.nodes << s_currentVertex;

    {
      QMutexLocker lock(&s_skeletonMutex);
      if ((nodeIndex == VTK_INT_MAX) || !m_visiblePoints.contains(s_skeleton.nodes[nodeIndex])) return false;
    }

    int displayPos[2]{x,y};
    double displayWorldPos[3];
    GetWorldPositionFromDisplayPosition(displayPos, displayWorldPos);

    return (m_tolerance > vtkMath::Distance2BetweenPoints(worldPos, displayWorldPos));
  }

  return false;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::setStroke(const Core::SkeletonStroke &stroke)
{
  bool updated = false;
  {
    QMutexLocker lock(&s_skeletonMutex);

    auto equalNameOp = [stroke](const SkeletonStroke &other) { return stroke.name == other.name; };
    auto it = std::find_if(s_skeleton.strokes.begin(), s_skeleton.strokes.end(), equalNameOp);
    if(it == s_skeleton.strokes.end())
    {
      s_skeleton.strokes << stroke;
      s_skeleton.count.insert(stroke, 0);
    }
    else
    {
      (*it).colorHue   = stroke.colorHue;
      (*it).type       = stroke.type;
      (*it).useMeasure = stroke.useMeasure;

      auto countKeys = s_skeleton.count.keys();
      auto cIt = std::find_if(countKeys.begin(), countKeys.end(), equalNameOp);
      Q_ASSERT(cIt != countKeys.end());
      auto count = s_skeleton.count[*cIt];
      s_skeleton.count.remove(*cIt);
      s_skeleton.count.insert(stroke, count);

      updated = true;
    }

    m_currentStrokeIndex = s_skeleton.strokes.indexOf(stroke);
    m_currentEdgeIndex   = -1;
  }

  if(updated) BuildRepresentation();
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::TryToJoin(int X, int Y)
{
  bool hasPointer = true;
  int nodeIndex = VTK_INT_MAX;

  int displayPos[2]{X,Y};
  double displayWorldPos[3], closestNodePos[3];
  GetWorldPositionFromDisplayPosition(displayPos, displayWorldPos);

  SkeletonNode *closestNode;
  {
    QMutexLocker lock(&s_skeletonMutex);
    if (s_skeleton.nodes.size() < 2) return false;

    if(s_currentVertex) s_skeleton.nodes.removeAll(s_currentVertex);
    FindClosestNode(X, Y, closestNodePos, nodeIndex);
    if(s_currentVertex) s_skeleton.nodes << s_currentVertex;

    if (nodeIndex == VTK_INT_MAX) return false;

    closestNode = s_skeleton.nodes[nodeIndex];
    hasPointer = s_currentVertex != nullptr;
    Q_ASSERT(nodeIndex < s_skeleton.nodes.size());
  }

  if(vtkMath::Distance2BetweenPoints(displayWorldPos, closestNodePos) > m_tolerance) return false;

  if(!hasPointer)
  {
    {
      QMutexLocker lock(&s_skeletonMutex);
      s_currentVertex = closestNode;
    }
    AddNodeAtPosition(displayWorldPos);

    return true;
  }

  // avoid loops of same stroke.
  if(m_currentEdgeIndex != -1)
  {
    auto closestConnections = closestNode->connections.keys();
    auto operation = [this, closestNode](SkeletonNode *node){ return (closestNode->connections.value(node) == this->m_currentEdgeIndex); };
    auto count = std::count_if(closestConnections.constBegin(), closestConnections.constEnd(), operation);
    if(count > 1) return false;
  }

  QMutexLocker lock(&s_skeletonMutex);
  for (auto connectionNode: s_currentVertex->connections.keys())
  {
    if(!connectionNode->connections.contains(closestNode)) connectionNode->connections.insert(closestNode, s_currentVertex->connections[connectionNode]);
    if(!closestNode->connections.contains(connectionNode)) closestNode->connections.insert(connectionNode, s_currentVertex->connections[connectionNode]);

    connectionNode->connections.remove(s_currentVertex);
    if(connectionNode->connections.isEmpty())
    {
      s_skeleton.nodes.removeAll(connectionNode);
      delete connectionNode;
    }
  }

  // reset flags if joining
  s_currentVertex->flags = closestNode->flags = SkeletonNodeFlags();

  auto &edge = s_skeleton.edges.at(m_currentEdgeIndex);

  int candidate = -1;
  int candidateStrokeIndex = -1;
  int nonPrioritary = -1;
  int nonPrioritaryStrokeIndex = -1;
  for (auto connection : closestNode->connections.values())
  {
    if (connection == m_currentEdgeIndex) continue; // cannot be parent of itself
    const auto &otherEdge = s_skeleton.edges.at(connection);
    const auto &stroke = s_skeleton.strokes.at(otherEdge.strokeIndex);
    if (stroke.useMeasure)
    {
      if (candidate == -1 || otherEdge.strokeIndex < candidateStrokeIndex)
      {
        candidate = connection;
        candidateStrokeIndex = otherEdge.strokeIndex;
      }
    }
    else
    {
      if (nonPrioritary == -1 || otherEdge.strokeIndex < nonPrioritaryStrokeIndex)
      {
        nonPrioritary = connection;
        nonPrioritaryStrokeIndex = otherEdge.strokeIndex;
      }
    }
  }

  const auto edgeValue = candidate != -1 ? candidate : nonPrioritary;

  if (edge.parentEdge == -1)
  {
    s_skeleton.edges[m_currentEdgeIndex].parentEdge = edgeValue;
  }
  else
  {
    if (edgeValue != -1)
    {
      const auto &candidateEdge = s_skeleton.edges.at(edgeValue);
      const auto &candidateStroke = s_skeleton.strokes.at(candidateEdge.strokeIndex);
      if (candidateStroke.name.startsWith("Shaft"))
      {
        s_skeleton.edges[m_currentEdgeIndex].parentEdge = edgeValue;
      }
    }
  }

  s_currentVertex->connections.clear();
  s_currentVertex->connections.insert(closestNode, m_currentEdgeIndex);

  return true;
}

//--------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::createConnection(const Core::SkeletonStroke &stroke)
{
  auto nodesNum = GetNumberOfNodes();

  if(nodesNum < 3) return false;

  {
    QMutexLocker lock(&s_skeletonMutex);

    // get the nodes involved in the party.
    const auto nodeB = s_skeleton.nodes.at(nodesNum-1);
    if(nodeB->connections.size() != 1) return false;
    const auto connectionNode = nodeB->connections.keys().first();
    if(connectionNode->connections.size() != 2) return false;
    const auto nodes = connectionNode->connections.keys();
    auto nodeA = nodes.first() == nodeB ? nodes.last() : nodes.first();

    // and the guest node.
    double positionNodeC[3];
    Core::closestPointToSegment(connectionNode->position, nodeA, nodeB, positionNodeC);

    // put guest node in position.
    for(int i: {0,1,2})
    {
      positionNodeC[i] = std::round(positionNodeC[i]/m_spacing[i]) * m_spacing[i];
    }
    auto node = new SkeletonNode{positionNodeC};

    // start the party.
    for(auto otherNode: connectionNode->connections.keys())
    {
      otherNode->connections.remove(connectionNode);
    }
    connectionNode->connections.clear();

    auto equalNameOp = [&stroke](const Core::SkeletonStroke &other) { return stroke.name == other.name; };
    auto it = std::find_if(s_skeleton.strokes.begin(), s_skeleton.strokes.end(), equalNameOp);
    if(it == s_skeleton.strokes.end())
    {
      s_skeleton.strokes << stroke;
      s_skeleton.count.insert(stroke, 0);
    }
    else
    {
      (*it).colorHue   = stroke.colorHue;
      (*it).useMeasure = stroke.useMeasure;
      (*it).type       = stroke.type;

      const auto countKeys = s_skeleton.count.keys();
      const auto cIt = std::find_if(countKeys.constBegin(), countKeys.constEnd(), equalNameOp);
      Q_ASSERT(cIt != countKeys.end());
      const auto count = s_skeleton.count[*cIt];
      s_skeleton.count.remove(*cIt);
      s_skeleton.count.insert(stroke, count);
    }

    s_skeleton.count[stroke]++;

    SkeletonEdge edge;
    edge.strokeIndex  = s_skeleton.strokes.indexOf(stroke);
    edge.strokeNumber = s_skeleton.count[stroke];
    edge.parentEdge   = m_currentEdgeIndex;
    s_skeleton.edges << edge;

    const auto edgeIndex = s_skeleton.edges.indexOf(edge);

    if(std::memcmp(nodeA->position, node->position, 3*sizeof(double)) == 0)
    {
      // coincident with nodeA
      delete node;

      nodeA->connections.insert(nodeB, m_currentEdgeIndex);
      nodeB->connections.insert(nodeA, m_currentEdgeIndex);

      connectionNode->connections.insert(nodeA, edgeIndex);
      nodeA->connections.insert(connectionNode, edgeIndex);
    }
    else
    {
      if(std::memcmp(nodeB->position, node->position, 3*sizeof(double)) == 0)
      {
        // coincident with nodeB
        delete node;

        nodeA->connections.insert(nodeB, m_currentEdgeIndex);
        nodeB->connections.insert(nodeA, m_currentEdgeIndex);

        connectionNode->connections.insert(nodeB, edgeIndex);
        nodeB->connections.insert(connectionNode, edgeIndex);
      }
      else
      {
        // not coincident with nodeA or nodeB
        s_skeleton.nodes << node;

        nodeA->connections.insert(node, m_currentEdgeIndex);
        nodeB->connections.insert(node, m_currentEdgeIndex);
        node->connections.insert(nodeA, m_currentEdgeIndex);
        node->connections.insert(nodeB, m_currentEdgeIndex);

        connectionNode->connections.insert(node, edgeIndex);
        node->connections.insert(connectionNode, edgeIndex);
      }
    }
  }

  return true;
}

//--------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::isStartNode(const NmVector3 &point)
{
  if(GetNumberOfNodes() == 0) return true;

  QMutexLocker lock(&s_skeletonMutex);

  int node_i, node_j;
  double unused[3];

  double pos[3];
  for(int i: {0,1,2}) pos[i] = std::round(point[i]/m_spacing[i]) * m_spacing[i];

  auto distance = Core::closestDistanceAndNode(pos, s_skeleton.nodes, node_i, node_j, unused);

  Q_ASSERT(node_i < s_skeleton.nodes.size());

  if(m_tolerance <= distance * distance) return true; // far from the skeleton.

  bool veryCloseToNode = (node_i == node_j);

  // needs correction if very close to node i
  if(!veryCloseToNode && (vtkMath::Distance2BetweenPoints(pos, s_skeleton.nodes[node_i]->position) <= m_tolerance))
  {
    node_j = node_i;
    veryCloseToNode = true;
  }

  if(veryCloseToNode)
  {
    unsigned int prioritary = 0;
    for(auto edgeIndex: s_skeleton.nodes.at(node_i)->connections)
    {
      auto stroke = s_skeleton.strokes.at(s_skeleton.edges.at(edgeIndex).strokeIndex);
      if(stroke.useMeasure) ++prioritary;
    }

    if(prioritary > 1) return true;
    return false;
  }

  if(node_i != node_j) return true; // node in middle of a segment

  return false;
}

//--------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::switchToStroke(const Core::SkeletonStroke& stroke)
{
  {
    QMutexLocker lock(&s_skeletonMutex);
    if(s_skeleton.nodes.size() < 2 || !s_currentVertex) return false;
  }

  auto oldEdgeIndex = m_currentEdgeIndex;
  setStroke(stroke);

  QMutexLocker lock(&s_skeletonMutex);
  s_skeleton.count[stroke]++;

  SkeletonEdge edge;
  edge.strokeIndex  = s_skeleton.strokes.indexOf(stroke);
  edge.strokeNumber = s_skeleton.count[stroke];
  edge.parentEdge   = m_currentEdgeIndex;
  s_skeleton.edges << edge;

  if(s_skeleton.edges.size() > oldEdgeIndex && oldEdgeIndex >= 0)
  {
    auto &oldEdge = s_skeleton.edges[oldEdgeIndex];
    if(oldEdge.parentEdge == -1) oldEdge.parentEdge = s_skeleton.edges.indexOf(edge);
  }

  double pos[3];
  for(int i: {0,1,2}) pos[i] = std::round(s_currentVertex->position[i]/m_spacing[i]) * m_spacing[i];

  auto newNode = new SkeletonNode{pos};
  s_skeleton.nodes << newNode;

  auto edgeIndex = s_skeleton.edges.indexOf(edge);

  newNode->connections.insert(s_currentVertex, edgeIndex);
  s_currentVertex->connections.insert(newNode, edgeIndex);

  m_currentEdgeIndex = edgeIndex;

  s_currentVertex = newNode;

  return false;
}

//--------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::setLabelsColor(const QColor& color)
{
  if(color != m_labelColor)
  {
    m_labelColor = color;

    m_labelPlacer->SetBackgroundColor(m_labelColor.redF() * 0.6, m_labelColor.greenF()*0.6, m_labelColor.blueF()*0.6);
    m_labelPlacer->SetBackgroundOpacity(0.5);
    m_labelPlacer->Update();
    m_labelActor->Modified();

    NeedToRenderOn();
  }
}

//--------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::setLabelsSize(unsigned int size)
{
  if(size != m_labelSize)
  {
    m_labelSize = size;

    m_labelProperty->SetFontSize(m_labelSize);
    m_labelProperty->Modified();
    m_labelFilter->GetTextProperty()->SetFontSize(m_labelSize);
    m_labelFilter->GetTextProperty()->Modified();
    m_labelFilter->Update();
    m_labelPlacer->Update();
    m_labelActor->Modified();

    NeedToRenderOn();
  }
}

//--------------------------------------------------------------------
const Core::PathList vtkSkeletonWidgetRepresentation::pathsOfNode(Core::SkeletonNode* node) const
{
  QMutexLocker lock(&s_skeletonMutex);

  PathList result;

  auto followPath = [](Path &path, bool direction) // follows the path to the end in the given direction, needs 2 nodes in seen list.
  {
    Q_ASSERT(path.seen.size() == 2);

    auto edgeIndex = path.seen.first()->connections[path.seen.last()];

    while(true)
    {
      auto node = direction ? path.seen.last() : path.seen.front();
      auto size = path.seen.size();

      for(auto connection: node->connections.keys())
      {
        if(node->connections[connection] != edgeIndex) continue;
        if(path.seen.contains(connection))             continue;
        if(direction)
        {
          path.seen.push_back(connection);
        }
        else
        {
          path.seen.push_front(connection);
        }
        break;
      }

      if(size == path.seen.size()) break; // we added nothing to the path.
    }
  };

  auto fillPathInfo = [](Path &path) // fills the rest of path information, needs the seen nodes and edge value already in.
  {
    path.begin  = path.seen.first();
    path.end    = path.seen.last();
    path.stroke = s_skeleton.edges.at(path.edge).strokeIndex;
    path.note   = strokeName(s_skeleton.edges.at(path.edge), s_skeleton.strokes, s_skeleton.edges);
    if(path.begin == path.end) path.note += QString(" (Loop)");
  };

  if(node)
  {
    switch(node->connections.size())
    {
      case 0:
        {
          Path path;
          path.seen << node;
          path.edge  = path.stroke = -1;
          path.begin = path.end    = node;
          path.note  = QString("Isolated node");

          result << path;
        }
        break;
      case 1:
        {
          Path path;
          path.seen << node << node->connections.keys().first();
          followPath(path, true);
          path.edge = node->connections.values().first();
          fillPathInfo(path);

          result << path;
        }
        break;
      default:
        {
          QMap<int, Path> paths;

          for(auto connection: node->connections.keys())
          {
            auto edge = node->connections[connection];
            if(paths.keys().contains(edge))
            {
              Path path;
              path.seen << connection << node;
              followPath(path, false);
              path.seen.takeLast();
              path.seen +=  paths[edge].seen;
              path.edge = edge;
              fillPathInfo(path);

              result << path;
              paths.remove(edge);
            }
            else
            {
              Path path;
              path.seen << node << connection;
              followPath(path, true);

              paths.insert(edge, path);
            }
          }

          for(auto edge: paths.keys())
          {
            Path path = paths[edge];
            path.edge = edge;
            fillPathInfo(path);

            result << path;
          }
        }
        break;
    }
  }

  return result;
}

//--------------------------------------------------------------------
const Core::PathList vtkSkeletonWidgetRepresentation::currentSelectedPaths(const int &x, const int &y) const
{
  SkeletonNode *node = nullptr;

  {
    QMutexLocker lock(&s_skeletonMutex);

    if(!s_currentVertex)
    {
      double worldPos[3]{0.0, 0.0, 0.0};
      int node_i = VTK_INT_MAX;
      int node_j = VTK_INT_MAX;
      auto distance = FindClosestDistanceAndNode(x, y, worldPos, node_i, node_j);

      if(distance > m_tolerance) return PathList();

      node = s_skeleton.nodes[node_i];
    }
    else
    {
      node = s_currentVertex;
    }
  }

  return pathsOfNode(node);
}

//--------------------------------------------------------------------
Core::SkeletonStroke vtkSkeletonWidgetRepresentation::currentStroke() const
{
  Core::SkeletonStroke result;

  QMutexLocker lock(&s_skeletonMutex);
  if(m_currentStrokeIndex >= 0 && !s_skeleton.strokes.isEmpty() && m_currentStrokeIndex < s_skeleton.strokes.size())
  {
    result = s_skeleton.strokes.at(m_currentStrokeIndex);
  }

  return result;
}

//--------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::performPathsMerge()
{
  auto intersect = [](const Path &one, const Path &two)
  {
    SkeletonNode *result = nullptr;

    if(!one.seen.isEmpty() && !two.seen.isEmpty())
    {
      if(one.seen.contains(two.begin))
      {
        return two.begin;
      }

      if(one.seen.contains(two.end))
      {
        return two.end;
      }
    }

    return result;
  };

  QMap<int, PathList> paths;

  // group paths by stroke (same path type).
  for(auto &path: Core::paths(s_skeleton.nodes, s_skeleton.edges, s_skeleton.strokes))
  {
    paths[s_skeleton.edges[path.edge].strokeIndex] << path;
  }

  for(auto pathList: paths.values())
  {
    PathList visited;
    for(auto &path1: pathList)
    {
      if(visited.contains(path1)) continue;

      visited << path1;

      for(auto &path2: pathList)
      {
        if(path1 == path2 || visited.contains(path2)) continue;

        auto commonNode = intersect(path1, path2);

        if(!commonNode) continue;

        const auto edges = commonNode->connections.values();

        if((path1.begin == commonNode || path1.end == commonNode) &&
           (path2.begin == commonNode || path2.end == commonNode) &&
           (edges.count(path1.edge) == 1) && (edges.count(path2.edge) == 1) &&
           (s_skeleton.edges.at(path1.edge).parentEdge != s_skeleton.edges.at(path2.edge).parentEdge))
        {
          for (int i = 0; i < path2.seen.size(); ++i)
          {
            auto node = path2.seen.at(i);

            for(auto next: node->connections.keys())
            {
              if(node->connections[next] == path2.edge)
              {
                node->connections[next] = path1.edge;
              }
            }
          }

          auto &edge1 = s_skeleton.edges[path1.edge];
          auto &edge2 = s_skeleton.edges[path2.edge];
          if (edge1.parentEdge != -1) edge1.parentEdge = edge2.parentEdge;

          for (auto &edge : s_skeleton.edges)
          {
            if (edge.parentEdge == path2.edge) edge.parentEdge = path1.edge;
          }

          if(path1.begin == commonNode) std::reverse(path1.seen.begin(), path1.seen.end());
          if(path2.end == commonNode)   std::reverse(path2.seen.begin(), path2.seen.end());
          path2.seen.takeFirst(); // to avoid duplicating commonNode in the path.

          path1.seen  = path1.seen + path2.seen;
          path1.begin = path1.seen.first();
          path1.end   = path1.seen.last();

          path2.begin = nullptr;
          path2.end   = nullptr;
          path2.seen.clear(); // to avoid merging it with anything else.
        }
      }
    }
  }
}

//--------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::performSpineSplitting()
{
  bool foundSpineStroke    = false;
  bool foundSubspineStroke = false;
  int spineIndex    = -1;
  int subspineIndex = -1;
  for(int i = 0; i < s_skeleton.strokes.size(); ++i)
  {
    auto stroke = s_skeleton.strokes.at(i);
    if (stroke.name == "Spine" && !foundSpineStroke)
    {
      spineIndex = i;
      foundSpineStroke = true;
      continue;
    }

    if (stroke.name == "Subspine" && !foundSubspineStroke)
    {
      subspineIndex = i;
      foundSubspineStroke = true;
    }

    if(foundSpineStroke && foundSubspineStroke) break;
  }

  if(!foundSpineStroke) return; // doesn't have spines.

  auto intersect = [](Path one, Path two)
  {
    SkeletonNode *result = nullptr;

    if(one.seen.contains(two.begin))
    {
      return two.begin;
    }

    if(one.seen.contains(two.end))
    {
      return two.end;
    }

    return result;
  };

  PathList spinePaths, subspinePaths, visited;
  for(auto &path: Core::paths(s_skeleton.nodes, s_skeleton.edges, s_skeleton.strokes))
  {
    if(path.stroke == spineIndex)
    {
      spinePaths << path;
      continue;
    }

    if(path.stroke == subspineIndex)
    {
      subspinePaths << path;
    }
  }

  // fix spine to spine, we won't do a thing with subspine to spine.
  for(auto &path1: spinePaths)
  {
    if(visited.contains(path1)) continue;

    auto alreadySplitted = false;
    for(auto &subSpinePath: subspinePaths)
    {
      if(intersect(path1, subSpinePath))
      {
        alreadySplitted = true;
        break;
      }
    }

    visited << path1;

    for(auto &path2: spinePaths)
    {
      if(path1 == path2 || visited.contains(path2)) continue;

      auto commonNode = intersect(path1, path2);

      if(!commonNode) continue;

      visited << path2;

      if(!foundSubspineStroke)
      {
        SkeletonStroke subspineStroke = s_skeleton.strokes.at(spineIndex);
        subspineStroke.name = "Subspine";

        s_skeleton.strokes << subspineStroke;
        s_skeleton.count.insert(subspineStroke, 0);
        subspineIndex = s_skeleton.strokes.indexOf(subspineStroke);
      }

      if(!alreadySplitted)
      {
        SkeletonEdge edge1;
        edge1.strokeIndex  = subspineIndex;
        edge1.strokeNumber = 1 + s_skeleton.count[s_skeleton.strokes.at(subspineIndex)]++;
        edge1.parentEdge   = path1.edge;
        s_skeleton.edges << edge1;

        auto edgeIndex = s_skeleton.edges.indexOf(edge1);
        for(int i = path1.seen.indexOf(commonNode); i < path1.seen.size() - 1; ++i)
        {
          auto node = path1.seen.at(i);
          auto next = path1.seen.at(i+1);
          node->connections[next] = edgeIndex;

          for(auto other: next->connections.keys())
          {
            auto otherEdge = next->connections[other];
            if(otherEdge == edgeIndex) continue;
            if(otherEdge == path1.edge)
            {
              next->connections[other] = edgeIndex;
            }
            else
            {
              if(s_skeleton.edges.at(otherEdge).parentEdge == path1.edge)
              {
                s_skeleton.edges[otherEdge].parentEdge = edgeIndex;
              }
            }
          }
        }
      }

      SkeletonEdge edge2;
      edge2.strokeIndex  = subspineIndex;
      edge2.strokeNumber = 1 + s_skeleton.count[s_skeleton.strokes.at(subspineIndex)]++;
      edge2.parentEdge   = path1.edge;
      s_skeleton.edges << edge2;

      auto edgeIndex = s_skeleton.edges.indexOf(edge2);
      // reverse seen nodes if created from connection to branching node by the user
      if(commonNode == path2.seen.last()) std::reverse(path2.seen.begin(), path2.seen.end());
      for(int i = path2.seen.indexOf(commonNode); i < path2.seen.size() - 1; ++i)
      {
        auto node = path2.seen.at(i);
        auto next = path2.seen.at(i+1);
        node->connections[next] = edgeIndex;

        for(auto other: next->connections.keys())
        {
          auto otherEdge = next->connections[other];
          if(otherEdge == edgeIndex) continue;
          if(otherEdge == path2.edge)
          {
            next->connections[other] = edgeIndex;
          }
          else
          {
            if(s_skeleton.edges.at(otherEdge).parentEdge == path2.edge)
            {
              s_skeleton.edges[otherEdge].parentEdge = edgeIndex;
            }
          }
        }
      }
    }
  }

  // fix spine to subspines
  visited.clear();
  for(auto &subPath: subspinePaths)
  {
    if(visited.contains(subPath)) continue;

    visited << subPath;

    for(auto spinePath: spinePaths)
    {
      if(spinePath.connectsTo(subPath) && !subPath.connectsTo(spinePath))
      {
        SkeletonEdge edge;
        edge.strokeIndex  = subspineIndex;
        edge.strokeNumber = 1 + s_skeleton.count[s_skeleton.strokes.at(subspineIndex)]++;
        edge.parentEdge   = subPath.edge;
        s_skeleton.edges << edge;

        auto edgeIndex = s_skeleton.edges.indexOf(edge);
        for(int i = 0; i < spinePath.seen.size(); ++i)
        {
          auto node = spinePath.seen.at(i);
          for(auto next: node->connections.keys())
          {
            auto otherEdge = node->connections[next];
            if(otherEdge == spinePath.edge)
            {
              node->connections[next] = edgeIndex;
            }
            else
            {
              if(s_skeleton.edges.at(otherEdge).parentEdge == spinePath.edge)
              {
                s_skeleton.edges[otherEdge].parentEdge = edgeIndex;
              }
            }
          }
        }
      }

      // re-join isolated subspines
      if(spinePath.connectsTo(subPath) && subPath.connectsTo(spinePath))
      {
        auto commonNode = intersect(spinePath, subPath);

        // node goes spine to subspine without other subspine
        if(commonNode->connections.size() == 2)
        {
          for(int i = 0; i < subPath.seen.size(); ++i)
          {
            auto node = subPath.seen.at(i);
            for(auto next: node->connections.keys())
            {
              auto otherEdge = node->connections[next];
              if(otherEdge == subPath.edge)
              {
                node->connections[next] = spinePath.edge;
              }
              else
              {
                auto &other = s_skeleton.edges[otherEdge];
                if(other.parentEdge == subPath.edge)
                {
                  other.parentEdge = spinePath.edge;
                }
              }
            }
          }

          if(spinePath.begin == commonNode)
          {
            if(subPath.begin == commonNode)
            {
              spinePath.begin = subPath.end;
            }
            else
            {
              spinePath.begin = subPath.begin;
            }
          }
          else
          {
            if(subPath.begin == commonNode)
            {
              spinePath.end = subPath.end;
            }
            else
            {
              spinePath.end = subPath.begin;
            }
          }

          spinePath.seen = spinePath.seen + subPath.seen;
          subPath.seen.clear();
        }
      }
    }
  }
}

//--------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::ToggleStrokeProperty(const Core::SkeletonNodeProperty property, const int &x, const int &y)
{
  SkeletonNode *node = nullptr;

  {
    QMutexLocker lock(&s_skeletonMutex);

    if(!s_currentVertex)
    {
      double worldPos[3]{0,0,0};
      int node_i = VTK_INT_MAX;
      int node_j = VTK_INT_MAX;
      auto distance = FindClosestDistanceAndNode(x, y, worldPos, node_i, node_j);

      if(distance > m_tolerance) return false;

      for(auto index: {node_i, node_j})
      {
        if(s_skeleton.nodes[index]->connections.size() > 2) continue;
        node = s_skeleton.nodes[index];
        break;
      }
    }
    else
    {
      if(s_currentVertex->connections.size() > 2) return false;

      node = s_currentVertex;
    }
  }

  if(!node) return false;

  const auto paths = pathsOfNode(node);
  if(paths.size() > 1) return false;

  {
    QMutexLocker lock(&s_skeletonMutex);

    const auto path = paths.first();
    const auto node1 = path.seen.first();
    const auto node2 = path.seen.last();

    if(!node1->isTerminal() && !node2->isTerminal()) return false;
    if(node1->isTerminal() && node2->isTerminal())   return false;

    auto operationNode = node1->isTerminal() ? node1 : node2;

    operationNode->flags ^= property;
  }

  return true;
}

//--------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::setShowLabels(const bool value)
{
  if(m_showLabels != value)
  {
    m_showLabels = value;
    BuildRepresentation();

    NeedToRenderOn();
  }
}

//--------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::setWidth(const int width)
{
  if(m_width != width)
  {
    m_width = width;

    m_actor->GetProperty()->SetLineWidth(m_width + 1);
    m_actor->Modified();
    m_linesActor->GetProperty()->SetLineWidth(m_width + 1);
    m_linesActor->Modified();
    m_dashedLinesActor->GetProperty()->SetLineWidth(m_width + 1);
    m_dashedLinesActor->Modified();

    NeedToRenderOn();
  }
}

//--------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::setChangeCoincidetHue(const bool value)
{
  if(m_changeCoincidentHue != value)
  {
    m_changeCoincidentHue = value;

    BuildRepresentation();

    NeedToRenderOn();
  }
}

//--------------------------------------------------------------------
const bool vtkSkeletonWidgetRepresentation::changeCoincidentHue() const
{
  return m_changeCoincidentHue;
}

//--------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::removeStroke(const Core::SkeletonStroke &stroke)
{
  bool updated{false};
  {
    QMutexLocker lock(&s_skeletonMutex);

    auto equalOp = [&stroke](const Core::SkeletonStroke &other) { return stroke.name == other.name; };
    auto it = std::find_if(s_skeleton.strokes.begin(), s_skeleton.strokes.end(), equalOp);

    if(it != s_skeleton.strokes.end())
    {
      auto strokeIndex = s_skeleton.strokes.indexOf(*it);
      s_skeleton.count.remove(*it);
      s_skeleton.strokes.removeAll(*it);

      std::vector<int> edgesToRemove; // QList doesn't have a reverse iterator, std::vector does.

      auto processEdges = [strokeIndex, &edgesToRemove](SkeletonEdge &edge)
      {
        if(edge.strokeIndex == strokeIndex)
        {
          edgesToRemove.push_back(s_skeleton.edges.indexOf(edge));
        }
        else
        {
          if(edge.strokeIndex > strokeIndex)
          {
            edge.strokeIndex -= 1;
          }
        }
      };

      auto processEdgesParents = [&edgesToRemove](SkeletonEdge &edge)
      {
        if(std::any_of(edgesToRemove.cbegin(), edgesToRemove.cend(), [&edge](const int edgeIndex) { return edgeIndex == edge.parentEdge; }))
        {
          edge.parentEdge = -1;
        }
      };

      std::for_each(s_skeleton.edges.begin(), s_skeleton.edges.end(), processEdges);
      std::for_each(s_skeleton.edges.begin(), s_skeleton.edges.end(), processEdgesParents);

      auto processNodes = [](const int edgeIndex)
      {
        for(auto node: s_skeleton.nodes)
        {
          SkeletonNodes toRemove;
          for(auto cNode: node->connections.keys())
          {
            if(node->connections[cNode] == edgeIndex)
            {
              toRemove << cNode;
            }
            else
            {
              if(node->connections[cNode] > edgeIndex) node->connections[cNode] -= 1;
            }
          }

          if(!toRemove.isEmpty())
          {
            for(auto cNode: toRemove) node->connections.remove(cNode);
          }
        }
      };

      std::sort(edgesToRemove.begin(), edgesToRemove.end());

      std::for_each(edgesToRemove.crbegin(), edgesToRemove.crend(), processNodes);
      std::for_each(edgesToRemove.crbegin(), edgesToRemove.crend(), [](const int edgeIndex) { s_skeleton.edges.removeAt(edgeIndex); });

      Core::removeIsolatedNodes(s_skeleton.nodes);
      updated = true;
    }
  }

  if(updated) BuildRepresentation();
}

//--------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::renameStroke(const QString &oldName, const QString &newName)
{
  bool updated{false};
  {
    QMutexLocker lock(&s_skeletonMutex);

    auto equalOp = [&oldName](const Core::SkeletonStroke &stroke) { return oldName == stroke.name; };
    auto it = std::find_if(s_skeleton.strokes.begin(), s_skeleton.strokes.end(), equalOp);

    if(it != s_skeleton.strokes.end())
    {
      const SkeletonStroke oldStroke = *it;
      const auto count = s_skeleton.count[oldStroke];
      s_skeleton.count.remove(oldStroke);
      (*it).name = newName;
      s_skeleton.count.insert(*it, count);

      updated = true;
    }
  }

  if(updated) BuildRepresentation();
}

//--------------------------------------------------------------------
const QColor vtkSkeletonWidgetRepresentation::computeCoincidentStrokeColor(const Core::SkeletonStroke &stroke)
{
  if(m_changeCoincidentHue)
  {
    return Core::alternateStrokeColor(s_skeleton.strokes, s_skeleton.strokes.indexOf(stroke), m_defaultHue);
  }

  const auto hue = stroke.colorHue == -1 ? m_defaultHue : stroke.colorHue;
  return QColor::fromHsv(hue, 255, 255);
}
