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

// Qt
#include <QMutexLocker>
#include <QColor>

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
: m_orientation       {Plane::UNDEFINED}
, m_tolerance         {std::sqrt(20)}
, m_slice             {-1}
, m_shift             {0}
, m_labelColor        {QColor::fromRgbF(1,1,1)}
, m_labelSize         {5}
, m_width             {1}
, m_currentStrokeIndex{-1}
, m_currentEdgeIndex  {-1}
, m_ignoreCursor      {false}
{
  m_colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  m_colors->SetName("Colors");
  m_colors->SetNumberOfComponents(3);

  m_points = vtkSmartPointer<vtkPoints>::New();
  m_points->SetNumberOfPoints(1);
  m_points->SetPoint(0, 0.0, 0.0, 0.0);

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
  points->SetNumberOfPoints(1);
  points->InsertNextPoint(0, 0, 0);
  polyData->SetPoints(points);

  m_pointer = vtkSmartPointer<vtkGlyph3D>::New();
  m_pointer->SetSourceConnection(sphere2->GetOutputPort());
  m_pointer->SetInputData(polyData);
  m_pointer->ScalingOn();
  m_pointer->SetScaleModeToDataScalingOff();
  m_pointer->SetScaleFactor(1.0);

  auto pointerMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  pointerMapper->SetInputData(m_pointer->GetOutput());
  pointerMapper->SetResolveCoincidentTopologyToPolygonOffset();

  m_pointerActor = vtkSmartPointer<vtkActor>::New();
  m_pointerActor->SetMapper(pointerMapper);
  m_pointerActor->GetProperty()->SetColor(1, 1, 1);

  m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_mapper->SetInputData(m_glypher->GetOutput());

  m_actor = vtkSmartPointer<vtkActor>::New();
  m_actor->SetMapper(m_mapper);
  m_actor->GetProperty()->SetLineWidth(m_width + 1);
  m_actor->GetProperty()->SetPointSize(1);

  m_lines = vtkSmartPointer<vtkPolyData>::New();
  m_linesMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_linesMapper->SetInputData(m_lines);

  m_linesActor = vtkSmartPointer<vtkActor>::New();
  m_linesActor->SetMapper(m_linesMapper);
  m_linesActor->GetProperty()->SetLineWidth(m_width + 1);

  m_dashedLines = vtkSmartPointer<vtkPolyData>::New();
  m_dashedLinesMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_dashedLinesMapper->SetInputData(m_dashedLines);

  m_dashedLinesActor = vtkSmartPointer<vtkActor>::New();
  m_dashedLinesActor->SetMapper(m_dashedLinesMapper);
  m_dashedLinesActor->GetProperty()->SetLineWidth(m_width + 1);
  m_dashedLinesActor->GetProperty()->SetLineStipplePattern(0x00F0);
  m_dashedLinesActor->GetProperty()->SetLineStippleRepeatFactor(2);

  m_truncatedPoints = vtkSmartPointer<vtkPoints>::New();

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

  m_showLabels = true;

  m_labelPoints = vtkSmartPointer<vtkPoints>::New();

  m_labels = vtkSmartPointer<vtkStringArray>::New();
  m_labels->SetName("Labels");

  m_labelData = vtkSmartPointer<vtkPolyData>::New();
  m_labelData->SetPoints(m_labelPoints);
  m_labelData->GetPointData()->AddArray(m_labels);

  m_labelProperty = vtkSmartPointer<vtkTextProperty>::New();
  m_labelProperty->SetBold(true);
  m_labelProperty->SetFontFamilyToArial();
  m_labelProperty->SetFontSize(m_labelSize);
  m_labelProperty->SetJustificationToCentered();
  m_labelProperty->SetVerticalJustificationToCentered();
  m_labelProperty->Modified();

  m_labelFilter = vtkSmartPointer<vtkPointSetToLabelHierarchy>::New();
  m_labelFilter->SetInputData(m_labelData);
  m_labelFilter->SetLabelArrayName("Labels");
  m_labelFilter->SetTextProperty(m_labelProperty);

  m_labelPlacer = vtkSmartPointer<vtkLabelPlacementMapper>::New();
  m_labelPlacer->SetInputConnection(m_labelFilter->GetOutputPort());
  m_labelPlacer->SetPlaceAllLabels(true);
  m_labelPlacer->SetBackgroundColor(m_labelColor.redF() * 0.6, m_labelColor.greenF()*0.6, m_labelColor.blueF()*0.6);
  m_labelPlacer->SetBackgroundOpacity(0.5);
  m_labelPlacer->SetShapeToRoundedRect();
  m_labelPlacer->SetMaximumLabelFraction(0.9);
  m_labelPlacer->SetUseDepthBuffer(false);
  m_labelPlacer->SetStyleToFilled();

  m_labelActor = vtkSmartPointer<vtkActor2D>::New();
  m_labelActor->SetMapper(m_labelPlacer);
  m_labelActor->SetDragable(false);
  m_labelActor->SetPickable(false);
  m_labelActor->SetUseBounds(false);

  m_interactionOffset[0] = 0.0;
  m_interactionOffset[1] = 0.0;
}

//-----------------------------------------------------------------------------
vtkSkeletonWidgetRepresentation::~vtkSkeletonWidgetRepresentation()
{
  if(s_skeletonMutex.tryLock())
  {
    for (auto node : s_skeleton.nodes)
    {
      delete node;
    }

    s_skeleton.nodes.clear();
    s_skeleton.edges.clear();
    s_skeleton.strokes.clear();
    s_skeleton.count.clear();

    s_skeletonMutex.unlock();
  }
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
    QMutexLocker lock(&s_skeletonMutex);

    auto stroke = currentStroke();

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
          }
          break;
        default:
        {
          unsigned int prioritary = 0;
          int prioritaryIndex = 0;
          for(auto edgeIndex: s_currentVertex->connections)
          {
            auto stroke = s_skeleton.strokes.at(s_skeleton.edges.at(edgeIndex).strokeIndex);
            if(stroke.useMeasure)
            {
              ++prioritary;
              prioritaryIndex = edgeIndex;
              if(prioritary > 1) break;
            }
          }

          if(prioritary == 1)
          {
            if(!stroke.useMeasure)
            {
              m_currentEdgeIndex = prioritaryIndex;
              m_currentStrokeIndex = s_skeleton.edges.at(m_currentEdgeIndex).strokeIndex;
            }
          }
          else
          {
            s_skeleton.count[stroke]++;

            edge.strokeIndex  = m_currentStrokeIndex;
            edge.strokeNumber = s_skeleton.count[stroke];
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
  for (auto node : m_visiblePoints.keys())
  {
    auto nodeDistance = vtkMath::Distance2BetweenPoints(worldPos, node->position);

    if (nodeDistance < distance2)
    {
      distance2 = nodeDistance;
      closestNode = node;
    }
  }

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

    s_skeleton.nodes.removeAll(s_currentVertex);

    for(auto connection: s_currentVertex->connections.keys())
    {
      connection->connections.remove(s_currentVertex);
      if(connection->connections.isEmpty())
      {
        s_skeleton.nodes.removeAll(connection);
        delete connection;
      }
    }

    delete s_currentVertex;
    s_currentVertex = nullptr;
  }

  BuildRepresentation();

  return true;
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
    AddNodeAtPosition(worldPos); // adds a new node and makes it current node, so it's joined to cursor, if any.

    {
      QMutexLocker lock(&s_skeletonMutex);
      auto contourEdgeIndex = s_skeleton.nodes[node_i]->connections[s_skeleton.nodes[node_j]];

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
    }
    else
    {
      AddNodeAtPosition(worldPos);
    }
  }
  else
  {
    SkeletonNode *addedNode = nullptr;
    {
      QMutexLocker lock(&s_skeletonMutex);

      addedNode = s_skeleton.nodes[node_i];
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
  m_lines->Reset();
  m_dashedLines->Reset();
  m_truncatedPoints->Reset();
  m_labelPoints->Reset();
  m_labels->Reset();

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

  m_visiblePoints.clear();
  QSet<int> truncatedStrokes, visibleStrokes;
  double worldPos[3];

  s_skeletonMutex.lock();

  for(int i = 0; i < s_skeleton.nodes.size(); ++i)
  {
    auto node = s_skeleton.nodes.at(i);

    for(auto other: node->connections.keys())
    {
      if(other == node) node->connections.remove(other);
    }

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
        m_colors->InsertNextTupleValue(green);

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
          if (areEqual(worldPos[planeIndex], m_slice))
          {
            m_colors->InsertNextTupleValue(green);
          }
          else
          {
            if (worldPos[planeIndex] > m_slice)
            {
              m_colors->InsertNextTupleValue(red);
            }
            else
            {
              m_colors->InsertNextTupleValue(blue);
            }
          }

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

        auto edgeIndex = node->connections[connectedNode];
        auto edge      = edges.at(edgeIndex);
        auto stroke    = strokes.at(edge.strokeIndex);
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

        auto qcolor = QColor::fromHsv(stroke.colorHue, 255, 255);
        unsigned char color[3]{static_cast<unsigned char>(qcolor.red()), static_cast<unsigned char>(qcolor.green()), static_cast<unsigned char>(qcolor.blue())};
        currentCellsColors->InsertNextTupleValue(color);
      }
    }
    else
    {
      for (auto connectedNode : node->connections.keys())
      {
        if ((node->position[planeIndex] < m_slice && connectedNode->position[planeIndex] >= m_slice)
            || (node->position[planeIndex] > m_slice && connectedNode->position[planeIndex] <= m_slice))
        {
          if (!m_visiblePoints.contains(node))
          {
            std::memcpy(worldPos, node->position, 3 * sizeof(double));
            if (worldPos[planeIndex] > m_slice)
            {
              m_colors->InsertNextTupleValue(red);
            }
            else
            {
              m_colors->InsertNextTupleValue(blue);
            }

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
            if (areEqual(worldPos[planeIndex], m_slice))
            {
              m_colors->InsertNextTupleValue(green);
            }
            else
            {
              if (worldPos[planeIndex] > m_slice)
              {
                m_colors->InsertNextTupleValue(red);
              }
              else
              {
                m_colors->InsertNextTupleValue(blue);
              }
            }

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

          auto edgeIndex = node->connections[connectedNode];
          auto edge      = edges.at(edgeIndex);
          auto stroke    = strokes.at(edge.strokeIndex);
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

          auto qcolor = QColor::fromHsv(stroke.colorHue, 255, 255);
          unsigned char color[3]{static_cast<unsigned char>(qcolor.red()), static_cast<unsigned char>(qcolor.green()), static_cast<unsigned char>(qcolor.blue())};
          currentCellsColors->InsertNextTupleValue(color);

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
              m_colors->InsertNextTupleValue(intersection);
            }
          }
        }
      }
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
        if(firstPass && (node->isBranching() || node == s_currentVertex)) continue;

        for(auto edge: node->connections.values())
        {
          if(visibleStrokes.contains(edge))
          {
            m_labelPoints->InsertNextPoint(node->position);

            auto text = strokeName(s_skeleton.edges.at(edge), s_skeleton.strokes);
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

  m_points->Modified();
  m_pointsData->Modified();
  m_pointsData->GetPointData()->Modified();
  m_glypher->SetInputData(m_pointsData);
  m_glypher->SetScaleFactor(distance * HandleSize);
  m_glypher->Update();
  m_mapper->Update();
  m_actor->Modified();

  m_lines->SetLines(cells);
  m_lines->GetCellData()->SetScalars(cellsColors);
  m_lines->SetPoints(m_points);
  m_lines->Modified();
  m_linesMapper->Update();
  m_linesActor->Modified();

  m_dashedLines->SetLines(dashedCells);
  m_dashedLines->GetCellData()->SetScalars(dashedCellsColors);
  m_dashedLines->SetPoints(m_points);
  m_dashedLines->Modified();
  m_dashedLinesMapper->Update();
  m_dashedLinesActor->Modified();

  m_truncatedPoints->Modified();
  m_truncatedData->Modified();
  m_truncatedData->GetPointData()->Modified();
  m_glyphMapper->SetScaleFactor(distance * HandleSize);
  m_glyphMapper->Update();
  m_truncatedActor->SetVisibility(m_truncatedPoints->GetNumberOfPoints() != 0);
  m_truncatedActor->Modified();

  m_labelProperty->SetFontSize(m_labelSize);
  m_labelPoints->Modified();
  m_labels->Modified();
  m_labelData->Modified();
  m_labelFilter->Update();
  m_labelPlacer->SetBackgroundColor(m_labelColor.redF() * 0.6, m_labelColor.greenF() * 0.6, m_labelColor.blueF() * 0.6);
  m_labelPlacer->SetUpdateExtentToWholeExtent();
  m_labelPlacer->RemoveAllClippingPlanes();
  m_labelPlacer->Update();
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

  QMutexLocker lock(&s_skeletonMutex);

  if (s_currentVertex != nullptr && (std::abs(s_currentVertex->position[idx] - m_slice) <= std::abs(m_shift)))
  {
    double pos[3];
    std::memcpy(pos, s_currentVertex->position, 3 * sizeof(double));
    pos[idx] = m_slice + m_shift;

    auto polyData = vtkPolyData::SafeDownCast(m_pointer->GetInput());
    polyData->GetPoints()->Reset();
    polyData->GetPoints()->SetNumberOfPoints(1);
    polyData->GetPoints()->SetPoint(0, pos);
    polyData->GetPoints()->Modified();
    polyData->Modified();
    m_pointer->Update();
    m_pointerActor->GetMapper()->Update();

    auto color = QColor::fromHsv(currentStroke().colorHue, 255,255);
    m_pointerActor->GetProperty()->SetColor(1 - color.redF()/2., 1 - color.greenF()/2., 1 - color.blueF()/2.);
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
    int unused1 = 0;
    int unused2 = 0;
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
  if (m_pointerActor->GetVisibility())
  {
    count += m_pointerActor->RenderOverlay(viewport);
  }

  if (m_linesActor->GetVisibility())
  {
    count += m_linesActor->RenderOverlay(viewport);
  }

  if (m_dashedLinesActor->GetVisibility())
  {
    count += m_dashedLinesActor->RenderOverlay(viewport);
  }

  if (m_actor->GetVisibility())
  {
    count += m_actor->RenderOverlay(viewport);
  }

  if(m_truncatedActor->GetVisibility())
  {
    count += m_truncatedActor->RenderOverlay(viewport);
  }

  if(m_labelActor->GetVisibility())
  {
    count += m_labelActor->RenderOverlay(viewport);
  }

  return count;
}

//-----------------------------------------------------------------------------
int vtkSkeletonWidgetRepresentation::RenderOpaqueGeometry(vtkViewport* viewport)
{
  // Since we know RenderOpaqueGeometry gets called first, will do the build here
  BuildRepresentation();

  int count = 0;
  if (m_pointerActor->GetVisibility())
  {
    count += m_pointerActor->RenderOpaqueGeometry(viewport);
  }

  if (m_linesActor->GetVisibility())
  {
    count += m_linesActor->RenderOpaqueGeometry(viewport);
  }

  if (m_dashedLinesActor->GetVisibility())
  {
    count += m_dashedLinesActor->RenderOpaqueGeometry(viewport);
  }

  if (m_actor->GetVisibility())
  {
    count += m_actor->RenderOpaqueGeometry(viewport);
  }

  if (m_truncatedActor->GetVisibility())
  {
    count += m_truncatedActor->RenderOpaqueGeometry(viewport);
  }

  if (m_labelActor->GetVisibility())
  {
    count += m_labelActor->RenderOpaqueGeometry(viewport);
  }

  return count;
}

//-----------------------------------------------------------------------------
int vtkSkeletonWidgetRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport* viewport)
{
  int count = 0;

  if (m_pointerActor->GetVisibility())
  {
    count += m_pointerActor->RenderTranslucentPolygonalGeometry(viewport);
  }

  if (m_linesActor->GetVisibility())
  {
    count += m_linesActor->RenderTranslucentPolygonalGeometry(viewport);
  }

  if (m_dashedLinesActor->GetVisibility())
  {
    count += m_dashedLinesActor->RenderTranslucentPolygonalGeometry(viewport);
  }

  if (m_actor->GetVisibility())
  {
    count += m_actor->RenderTranslucentPolygonalGeometry(viewport);
  }

  if (m_truncatedActor->GetVisibility())
  {
    count += m_truncatedActor->RenderTranslucentPolygonalGeometry(viewport);
  }

  if (m_labelActor->GetVisibility())
  {
    count += m_labelActor->RenderTranslucentPolygonalGeometry(viewport);
  }

  return count;
}

//-----------------------------------------------------------------------------
int vtkSkeletonWidgetRepresentation::HasTranslucentPolygonalGeometry()
{
  int result = 0;

  if (m_pointerActor->GetVisibility())
  {
    result |= m_pointerActor->HasTranslucentPolygonalGeometry();
  }

  if (m_linesActor->GetVisibility())
  {
    result |= m_linesActor->HasTranslucentPolygonalGeometry();
  }

  if (m_dashedLinesActor->GetVisibility())
  {
    result |= m_dashedLinesActor->HasTranslucentPolygonalGeometry();
  }

  if (m_actor->GetVisibility())
  {
    result |= m_actor->HasTranslucentPolygonalGeometry();
  }

  if (m_truncatedActor->GetVisibility())
  {
    result |= m_truncatedActor->HasTranslucentPolygonalGeometry();
  }

  if (m_labelActor->GetVisibility())
  {
    result |= m_labelActor->HasTranslucentPolygonalGeometry();
  }

  return result;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> vtkSkeletonWidgetRepresentation::GetRepresentationPolyData()
{
  QMutexLocker lock(&s_skeletonMutex);

  removeIsolatedNodes();

  performSpineSplitting();

  adjustStrokeNumbers(s_skeleton);

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
void vtkSkeletonWidgetRepresentation::FindClosestNode(int X, int Y, double worldPos[3], int &closestNode) const
{
  double distance = VTK_DOUBLE_MAX;
  auto planeIndex = normalCoordinateIndex(m_orientation);
  double pos[4];
  int displayPos[2]{X,Y};

  GetWorldPositionFromDisplayPosition(displayPos, pos);

  for(auto i = 0; i < s_skeleton.nodes.size(); ++i)
  {
    if(!areEqual(s_skeleton.nodes[i]->position[planeIndex], m_slice)) continue;

    auto nodeDistance = vtkMath::Distance2BetweenPoints(pos, s_skeleton.nodes[i]->position);
    if(distance > nodeDistance)
    {
      distance = nodeDistance;
      closestNode = i;

      std::memcpy(worldPos, s_skeleton.nodes[i]->position, 3* sizeof(double));
    }
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::Initialize(vtkSmartPointer<vtkPolyData> pd)
{
  {
    QMutexLocker lock(&s_skeletonMutex);
    Q_ASSERT(s_skeleton.nodes.isEmpty());
  }

  auto nPoints = pd->GetNumberOfPoints();
  if (nPoints <= 0) return; // Yeah right.. build from nothing!

  {
    QMutexLocker lock(&s_skeletonMutex);
    s_skeleton = Core::toSkeletonDefinition(pd);
  }

  BuildRepresentation();
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
        for(auto node: s_skeleton.nodes)
        {
          node->position[0] = node->position[0]/s_skeletonSpacing[0] * spacing[0];
          node->position[1] = node->position[1]/s_skeletonSpacing[1] * spacing[1];
          node->position[2] = node->position[2]/s_skeletonSpacing[2] * spacing[2];
        }

        s_skeletonSpacing = spacing;
      }

      s_skeletonMutex.unlock();
    }

    m_spacing = spacing;

    auto planeIdx = normalCoordinateIndex(m_orientation);
    double max = -1;
    for(int i = 0; i < 3; ++i)
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

  for(auto node: s_skeleton.nodes)
  {
    node->connections.clear();
    delete node;
  }

  s_skeleton.nodes.clear();
  s_skeleton.count.clear();

  s_currentVertex = nullptr;
}

//-----------------------------------------------------------------------------
double vtkSkeletonWidgetRepresentation::FindClosestDistanceAndNode(int X, int Y, double worldPos[3], int &node_i, int &node_j) const
{
  if (!Renderer) return VTK_DOUBLE_MAX;

  std::memset(worldPos, 0, 3*sizeof(double));
  node_i = node_j = VTK_INT_MAX;

  double point_pos[3];
  int displayPos[2]{X,Y};
  GetWorldPositionFromDisplayPosition(displayPos, point_pos);

  auto result = Core::closestDistanceAndNode(point_pos, s_skeleton.nodes, node_i, node_j, worldPos);

  // NOTE: the Core:: util method returns the sqrt, that is, the real distance, but we're using the square of that in all of our computations in the widget.
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
  if(!s_skeleton.strokes.contains(stroke))
  {
    s_skeleton.strokes << stroke;
    s_skeleton.count.insert(stroke, 0);
  }

  m_currentStrokeIndex = s_skeleton.strokes.indexOf(stroke);
  m_currentEdgeIndex   = -1;
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
    if (s_skeleton.nodes.size() < 3) return false;

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

  QMutexLocker lock(&s_skeletonMutex);
  for (auto connectionNode : s_currentVertex->connections.keys())
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
  s_currentVertex->connections.clear();
  s_currentVertex->connections.insert(closestNode, m_currentStrokeIndex);

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
    auto nodeA = s_skeleton.nodes.at(nodesNum-3);
    auto connectionNode = s_skeleton.nodes.at(nodesNum-2);
    auto nodeB = s_skeleton.nodes.at(nodesNum-1);

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

    s_skeleton.nodes << node;

    // get the stroke values.
    if(!s_skeleton.strokes.contains(stroke))
    {
      s_skeleton.strokes << stroke;
      s_skeleton.count.insert(stroke, 0);
    }

    s_skeleton.count[stroke]++;

    SkeletonEdge edge;
    edge.strokeIndex  = s_skeleton.strokes.indexOf(stroke);
    edge.strokeNumber = s_skeleton.count[stroke];
    s_skeleton.edges << edge;

    auto edgeIndex = s_skeleton.edges.indexOf(edge);

    nodeA->connections.insert(node, m_currentEdgeIndex);
    nodeB->connections.insert(node, m_currentEdgeIndex);
    node->connections.insert(nodeA, m_currentEdgeIndex);
    node->connections.insert(nodeB, m_currentEdgeIndex);

    connectionNode->connections.insert(node, edgeIndex);
    node->connections.insert(connectionNode, edgeIndex);
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
  for(auto i: {0,1,2})
  {
    if(i == normalCoordinateIndex(m_orientation)) continue;
    veryCloseToNode &= distance <= 2*m_spacing[i];
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
  if(GetNumberOfNodes() < 2 || !s_currentVertex) return false;

  QMutexLocker lock(&s_skeletonMutex);

  setStroke(stroke);

  s_skeleton.count[stroke]++;

  SkeletonEdge edge;
  edge.strokeIndex  = s_skeleton.strokes.indexOf(stroke);
  edge.strokeNumber = s_skeleton.count[stroke];
  s_skeleton.edges << edge;

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
void vtkSkeletonWidgetRepresentation::removeIsolatedNodes()
{
  SkeletonNodes toRemove;
  for(auto node: s_skeleton.nodes)
  {
    if(node->connections.isEmpty()) toRemove << node;
  }

  for(auto node: toRemove)
  {
    s_skeleton.nodes.removeAll(node);
    delete node;
  }
}

//--------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::setLabelsColor(const QColor& color)
{
  if(color != m_labelColor)
  {
    m_labelColor = color;

    m_labelPlacer->SetBackgroundColor(m_labelColor.redF() * 0.6, m_labelColor.greenF()*0.6, m_labelColor.blueF()*0.6);
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

    m_labelProperty->SetLineOffset(m_labelSize);
    m_labelProperty->SetFontSize(m_labelSize);
    m_labelProperty->Modified();
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
    path.note   = strokeName(s_skeleton.edges.at(path.edge), s_skeleton.strokes);
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
const Core::PathList vtkSkeletonWidgetRepresentation::currentSelectedPaths() const
{
  SkeletonNode *node = nullptr;

  {
    QMutexLocker lock(&s_skeletonMutex);

    if(!s_currentVertex) return PathList();
    node = s_currentVertex;
  }

  return pathsOfNode(node);
}

//--------------------------------------------------------------------
Core::SkeletonStroke vtkSkeletonWidgetRepresentation::currentStroke() const
{
  Q_ASSERT(m_currentStrokeIndex >= 0);

  return s_skeleton.strokes.at(m_currentStrokeIndex);
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
    if (stroke.name == "Spine")
    {
      spineIndex = i;
      foundSpineStroke = true;
    }

    if (stroke.name == "Subspine")
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
    for(auto node: two.seen)
    {
      if(one.seen.contains(node))
      {
        result = node;
        break;
      }
    }

    return result;
  };

  PathList spinePaths, subspinePaths, visited;
  for(auto path: Core::paths(s_skeleton.nodes, s_skeleton.edges, s_skeleton.strokes))
  {
    if(path.stroke == spineIndex) spinePaths << path;
    if(path.stroke == subspineIndex) subspinePaths << path;
  }

  // fix spine to spine
  for(auto path1: spinePaths)
  {
    if(visited.contains(path1)) continue;

    for(auto path2: spinePaths)
    {
      if(path1 == path2 || visited.contains(path2)) continue;

      auto commonNode = intersect(path1, path2);

      if(!commonNode) continue;

      visited << path1 << path2;

      SkeletonEdge edge1, edge2;

      if(!foundSubspineStroke)
      {
        SkeletonStroke subspineStroke = s_skeleton.strokes.at(spineIndex);
        subspineStroke.name = "Subspine";

        s_skeleton.strokes << subspineStroke;
        s_skeleton.count.insert(subspineStroke, 0);
        subspineIndex = s_skeleton.strokes.indexOf(subspineStroke);
      }

      edge1.strokeIndex = subspineIndex;
      edge1.strokeNumber = 1 + s_skeleton.count[s_skeleton.strokes.at(subspineIndex)]++;

      edge2.strokeIndex = subspineIndex;
      edge2.strokeNumber = 1 + s_skeleton.count[s_skeleton.strokes.at(subspineIndex)]++;

      s_skeleton.edges << edge1 << edge2;

      auto edgeIndex = s_skeleton.edges.indexOf(edge1);
      for(int i = path1.seen.indexOf(commonNode); i < path1.seen.size(); ++i)
      {
        if(i+1 == path1.seen.size()) break;

        path1.seen.at(i)->connections[path1.seen.at(i+1)] = edgeIndex;
        path1.seen.at(i+1)->connections[path1.seen.at(i)] = edgeIndex;
      }

      edgeIndex = s_skeleton.edges.indexOf(edge2);
      for(int i = path2.seen.indexOf(commonNode); i < path2.seen.size(); ++i)
      {
        if(i+1 == path2.seen.size()) break;

        path2.seen.at(i)->connections[path2.seen.at(i+1)] = edgeIndex;
        path2.seen.at(i+1)->connections[path2.seen.at(i)] = edgeIndex;
      }
    }

    // fix subspine to spine
    for(auto path2: subspinePaths)
    {
      auto commonNode = intersect(path1, path2);

      if(!commonNode) continue;

      if(commonNode != path1.end && commonNode != path1.begin)
      {
        SkeletonEdge edge;
        edge.strokeIndex = subspineIndex;
        edge.strokeNumber = 1 + s_skeleton.count[s_skeleton.strokes.at(subspineIndex)]++;

        s_skeleton.edges << edge;
        auto edgeIndex = s_skeleton.edges.indexOf(edge);

        for(int i = path1.seen.indexOf(commonNode); i < path1.seen.size(); ++i)
        {
          if(i+1 == path1.seen.size()) break;

          path1.seen.at(i)->connections[path1.seen.at(i+1)] = edgeIndex;
          path1.seen.at(i+1)->connections[path1.seen.at(i)] = edgeIndex;
        }
      }
    }
  }
}

//--------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::ToggleStrokeProperty(const Core::SkeletonNodeProperty property)
{
  {
    QMutexLocker lock(&s_skeletonMutex);

    if(!s_currentVertex || s_currentVertex->connections.size() > 2) return false;
  }

  auto paths = pathsOfNode(s_currentVertex);
  if(paths.size() > 1) return false;

  {
    QMutexLocker lock(&s_skeletonMutex);

    auto node1 = paths.first().seen.first();
    auto node2 = paths.first().seen.last();

    if(!node1->isTerminal() && !node2->isTerminal()) return false;
    if(node1->isTerminal() && node2->isTerminal())   return false;

    auto node = node1->isTerminal() ? node1 : node2;

    node->flags ^= property;
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
