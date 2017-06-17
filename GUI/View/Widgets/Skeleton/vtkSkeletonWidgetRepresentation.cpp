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
#include <GUI/View/View2D.h>

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

// Qt
#include <QMutexLocker>

using namespace ESPINA;
using namespace ESPINA::GUI::View::Widgets::Skeleton;

vtkStandardNewMacro(vtkSkeletonWidgetRepresentation);

vtkSkeletonWidgetRepresentation::SkeletonNode *vtkSkeletonWidgetRepresentation::s_currentVertex = nullptr;
QList<vtkSkeletonWidgetRepresentation::SkeletonNode *> vtkSkeletonWidgetRepresentation::s_skeleton;
NmVector3 vtkSkeletonWidgetRepresentation::s_skeletonSpacing = NmVector3{1,1,1};
QMutex vtkSkeletonWidgetRepresentation::s_skeletonMutex;

//-----------------------------------------------------------------------------
vtkSkeletonWidgetRepresentation::vtkSkeletonWidgetRepresentation()
: m_orientation {Plane::UNDEFINED}
, m_tolerance   {std::sqrt(20)}
, m_slice       {-1}
, m_shift       {-1}
, m_color       {QColor{254,254,254}}
, m_ignoreCursor{false}
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
  m_actor->GetProperty()->SetLineWidth(3);
  m_actor->GetProperty()->SetPointSize(1);
  m_actor->GetProperty()->SetColor(m_color.redF(), m_color.greenF(), m_color.blueF());

  m_lines = vtkSmartPointer<vtkPolyData>::New();
  m_linesMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_linesMapper->SetInputData(m_lines);

  m_linesActor = vtkSmartPointer<vtkActor>::New();
  m_linesActor->SetMapper(m_linesMapper);
  m_linesActor->GetProperty()->SetLineWidth(2.5);
  m_linesActor->GetProperty()->SetColor(m_color.redF(), m_color.greenF(), m_color.blueF());

  m_interactionOffset[0] = 0.0;
  m_interactionOffset[1] = 0.0;
}

//-----------------------------------------------------------------------------
vtkSkeletonWidgetRepresentation::~vtkSkeletonWidgetRepresentation()
{
  if(s_skeletonMutex.tryLock())
  {
    for (auto node : s_skeleton)
    {
      delete node;
    }

    s_skeleton.clear();
    s_skeletonSpacing = NmVector3{1,1,1};

    s_skeletonMutex.unlock();
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::AddNodeAtPosition(double worldPos[3])
{
  {
    QMutexLocker lock(&s_skeletonMutex);

    worldPos[normalCoordinateIndex(m_orientation)] = m_slice;
    auto node = new SkeletonNode{worldPos};

    s_skeleton.push_back(node);

    if (s_currentVertex != nullptr)
    {
      s_currentVertex->connections << node;
      node->connections << s_currentVertex;
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
    auto nodeDistance = vtkMath::Distance2BetweenPoints(worldPos, node->worldPosition);

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

    std::memcpy(s_currentVertex->worldPosition, worldPos, 3 * sizeof(double));
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

    std::memcpy(worldPos, s_currentVertex->worldPosition, 3 * sizeof(double));
  }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::DeleteCurrentNode()
{
  {
    QMutexLocker lock(&s_skeletonMutex);

    if (s_currentVertex == nullptr) return false;

    s_skeleton.removeAll(s_currentVertex);
    for(auto connection: s_currentVertex->connections)
    {
      connection->connections.removeAll(s_currentVertex);
    }
    s_currentVertex->connections.clear();
    delete s_currentVertex;
    s_currentVertex = nullptr;
  }

  BuildRepresentation();

  return true;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::ClearAllNodes()
{
  {
    QMutexLocker lock(&s_skeletonMutex);

    for (auto node : s_skeleton)
    {
      delete node;
    }

    s_skeleton.clear();
    s_currentVertex = nullptr;
  }

  BuildRepresentation();
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::AddNodeOnContour(int X, int Y)
{
  SkeletonNode *currentVertex = nullptr;
  {
    QMutexLocker lock(&s_skeletonMutex);

    currentVertex = s_currentVertex;
    if(currentVertex) s_skeleton.removeAll(s_currentVertex);
  }

  double worldPos[3];
  int node_i = 0;
  int node_j = 0;
  auto distance = FindClosestDistanceAndNode(X, Y, worldPos, node_i, node_j);

  if (distance > m_tolerance) return false;

  SkeletonNode *addedNode = nullptr;

  if (node_i != node_j)
  {
    DeactivateNode();
    AddNodeAtPosition(worldPos); // adds a new node and makes it current node.

    QMutexLocker lock(&s_skeletonMutex);
    s_currentVertex->connections << s_skeleton[node_i] << s_skeleton[node_j];

    s_skeleton[node_i]->connections.removeAll(s_skeleton[node_j]);
    s_skeleton[node_i]->connections << s_currentVertex;

    s_skeleton[node_j]->connections.removeAll(s_skeleton[node_i]);
    s_skeleton[node_j]->connections << s_currentVertex;

    addedNode = s_currentVertex;
  }
  else
  {
    QMutexLocker lock(&s_skeletonMutex);

    addedNode = s_skeleton[node_i];
  }

  if(currentVertex == nullptr)
  {
    AddNodeAtDisplayPosition(X, Y);
  }
  else
  {
    QMutexLocker lock(&s_skeletonMutex);

    s_currentVertex = currentVertex;

    for(auto connection: s_currentVertex->connections)
    {
      if(!addedNode->connections.contains(connection)) addedNode->connections << connection;
    }

    s_currentVertex->connections.clear();

    s_skeleton << s_currentVertex;
  }

  {
    QMutexLocker lock(&s_skeletonMutex);

    s_currentVertex->connections << addedNode;
  }

  BuildRepresentation();

  return true;
}

//-----------------------------------------------------------------------------
unsigned int vtkSkeletonWidgetRepresentation::GetNumberOfNodes() const
{
  QMutexLocker lock(&s_skeletonMutex);

  return s_skeleton.size();
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::SetOrientation(Plane plane)
{
  if (m_orientation == Plane::UNDEFINED)
  {
    m_orientation = plane;
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
    numNodes = s_skeleton.size();
  }

  if (numNodes == 0 || !Renderer || !Renderer->GetActiveCamera())
  {
    VisibilityOff();
    return;
  }

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
  auto cells = vtkSmartPointer<vtkCellArray>::New();

  auto planeIndex = normalCoordinateIndex(m_orientation);

  unsigned char red[3]{255,0,0};
  unsigned char green[3]{0,255,0};
  unsigned char blue[3]{0,0,255};
  m_colors->Reset();

  m_visiblePoints.clear();
  double worldPos[3];

  s_skeletonMutex.lock();
  for (auto node : s_skeleton)
  {
    // we only want "some" points in the screen representation and want all the representation to be visible.
    if (areEqual(node->worldPosition[planeIndex], m_slice))
    {
      if (!m_visiblePoints.contains(node))
      {
        std::memcpy(worldPos, node->worldPosition, 3 * sizeof(double));
        worldPos[planeIndex] = m_slice + m_shift;

        m_visiblePoints.insert(node, m_points->InsertNextPoint(worldPos));
        m_colors->InsertNextTupleValue(green);
      }

      for (auto connectedNode : node->connections)
      {
        if (!m_visiblePoints.contains(connectedNode))
        {
          std::memcpy(worldPos, connectedNode->worldPosition, 3 * sizeof(double));
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
        }

        auto line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, m_visiblePoints[node]);
        line->GetPointIds()->SetId(1, m_visiblePoints[connectedNode]);
        cells->InsertNextCell(line);
      }
    }
    else
    {
      for (auto connectedNode : node->connections)
        if ((node->worldPosition[planeIndex] < m_slice && connectedNode->worldPosition[planeIndex] >= m_slice)
            || (node->worldPosition[planeIndex] > m_slice && connectedNode->worldPosition[planeIndex] <= m_slice))
        {
          if (!m_visiblePoints.contains(node))
          {
            std::memcpy(worldPos, node->worldPosition, 3 * sizeof(double));
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
          }

          if (!m_visiblePoints.contains(connectedNode))
          {
            std::memcpy(worldPos, connectedNode->worldPosition, 3 * sizeof(double));
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
          }

          auto line = vtkSmartPointer<vtkLine>::New();
          line->GetPointIds()->SetId(0, m_visiblePoints[node]);
          line->GetPointIds()->SetId(1, m_visiblePoints[connectedNode]);
          cells->InsertNextCell(line);
        }
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
  m_lines->SetPoints(m_points);
  m_lines->Modified();
  m_linesMapper->Update();
  m_linesActor->Modified();

  m_pointer->SetScaleFactor(distance * HandleSize);
  UpdatePointer();

  VisibilityOn();
  NeedToRenderOn();
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::UpdatePointer()
{
  QMutexLocker lock(&s_skeletonMutex);

  if (s_currentVertex != nullptr)
  {
    double pos[3];
    std::memcpy(pos, s_currentVertex->worldPosition, 3 * sizeof(double));
    pos[normalCoordinateIndex(m_orientation)] = m_slice;

    auto polyData = vtkPolyData::SafeDownCast(m_pointer->GetInput());
    polyData->GetPoints()->Reset();
    polyData->GetPoints()->SetNumberOfPoints(1);
    polyData->GetPoints()->SetPoint(0, pos);
    polyData->GetPoints()->Modified();
    polyData->Modified();
    m_pointer->Update();
    m_pointerActor->GetMapper()->Update();
    double color[3];
    m_linesActor->GetProperty()->GetColor(color);
    m_pointerActor->GetProperty()->SetColor(1 - color[0]/2., 1 - color[1]/2., 1 - color[2]/2.);
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
  {
    QMutexLocker lock(&s_skeletonMutex);
    if(m_ignoreCursor && s_currentVertex) s_skeleton.removeAll(s_currentVertex);
  }

  if (IsNearNode(X, Y))
  {
    InteractionState = vtkSkeletonWidgetRepresentation::NearPoint;
  }
  else
  {
    double worldPos[3];
    int unused1 = 0;
    int unused2 = 0;
    if (FindClosestDistanceAndNode(X, Y, worldPos, unused1, unused2) <= m_tolerance)
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
    if(m_ignoreCursor && s_currentVertex) s_skeleton << s_currentVertex;
  }

  return InteractionState;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::ReleaseGraphicsResources(vtkWindow* win)
{
  m_actor->ReleaseGraphicsResources(win);
  m_pointerActor->ReleaseGraphicsResources(win);
  m_linesActor->ReleaseGraphicsResources(win);
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

  if (m_actor->GetVisibility())
  {
    count += m_actor->RenderOverlay(viewport);
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

  if (m_actor->GetVisibility())
  {
    count += m_actor->RenderOpaqueGeometry(viewport);
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

  if (m_actor->GetVisibility())
  {
    count += m_actor->RenderTranslucentPolygonalGeometry(viewport);
  }

  return count;
}

//-----------------------------------------------------------------------------
int vtkSkeletonWidgetRepresentation::HasTranslucentPolygonalGeometry()
{
  int result = 0;

  if (m_pointerActor->GetVisibility())
  {
    result += m_pointerActor->HasTranslucentPolygonalGeometry();
  }

  if (m_linesActor->GetVisibility())
  {
    result |= m_linesActor->HasTranslucentPolygonalGeometry();
  }
  if (m_actor->GetVisibility())
  {
    result |= m_actor->HasTranslucentPolygonalGeometry();
  }

  return result;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> vtkSkeletonWidgetRepresentation::GetRepresentationPolyData() const
{
  QMap<SkeletonNode *, vtkIdType> locator;
  QMap<vtkIdType, QList<vtkIdType>> relationsLocator;

  auto points = vtkSmartPointer<vtkPoints>::New();
  auto lines = vtkSmartPointer<vtkCellArray>::New();

  {
    QMutexLocker lock(&s_skeletonMutex);
    points->SetNumberOfPoints(s_skeleton.size());

    for (auto node : s_skeleton)
    {
      locator.insert(node, points->InsertNextPoint(node->worldPosition));
    }

    for (auto node : s_skeleton)
    {
      relationsLocator.insert(locator[node], QList<vtkIdType>());

      for (auto connectedNode : node->connections)
      {
        relationsLocator[locator[node]] << locator[connectedNode];
      }
    }
  }

  for (auto nodeId : relationsLocator.keys())
  {
    for (auto connectionId : relationsLocator[nodeId])
    {
      auto line = vtkSmartPointer<vtkLine>::New();
      line->GetPointIds()->SetId(0, nodeId);
      line->GetPointIds()->SetId(1, connectionId);
      lines->InsertNextCell(line);

      // remove duplicated lines
      relationsLocator[connectionId].removeAll(nodeId);
    }
  }

  auto polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);
  polyData->SetLines(lines);

  return polyData;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::GetWorldPositionFromDisplayPosition(int displayPos[2], double worldPos[3]) const
{
  double pos[4];
  pos[0] = displayPos[0];
  pos[1] = displayPos[1];
  pos[2] = 1.0;
  pos[3] = 1.0;

  Renderer->SetDisplayPoint(pos);
  Renderer->DisplayToWorld();
  Renderer->GetWorldPoint(pos);

  worldPos[0] = pos[0];
  worldPos[1] = pos[1];
  worldPos[2] = pos[2];
  worldPos[normalCoordinateIndex(m_orientation)] = m_slice;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::FindClosestNode(int X, int Y, double worldPos[3], int &closestNode) const
{
  double distance = VTK_DOUBLE_MAX;

  double pos[4];
  int displayPos[2]{X,Y};
  GetWorldPositionFromDisplayPosition(displayPos, pos);
  pos[3] = 0;

  auto planeIndex = normalCoordinateIndex(m_orientation);
  for(auto i = 0; i < s_skeleton.size(); ++i)
  {
    if(!areEqual(s_skeleton[i]->worldPosition[planeIndex], m_slice)) continue;

    auto nodeDistance = vtkMath::Distance2BetweenPoints(pos, s_skeleton[i]->worldPosition);
    if(distance > nodeDistance)
    {
      distance = nodeDistance;
      closestNode = i;
      worldPos[0] = s_skeleton[i]->worldPosition[0];
      worldPos[1] = s_skeleton[i]->worldPosition[1];
      worldPos[2] = s_skeleton[i]->worldPosition[2];
    }
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::Initialize(vtkSmartPointer<vtkPolyData> pd)
{
  auto points = pd->GetPoints();
  auto lines = pd->GetLines();

  vtkIdType nPoints = points->GetNumberOfPoints();
  if (nPoints <= 0) return; // Yeah right.. build from nothing!

  // Clear all existing nodes.
  ClearAllNodes();

  // Points in our skeleton will be added while traversing lines.
  lines->InitTraversal();
  vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();
  {
    QMutexLocker lock(&s_skeletonMutex);
    while (lines->GetNextCell(idList))
    {
      if (idList->GetNumberOfIds() != 2) continue;

      SkeletonNode *pointA = nullptr;
      SkeletonNode *pointB = nullptr;
      vtkIdType data[2];
      data[0] = idList->GetId(0);
      data[1] = idList->GetId(1);
      double dataCoords[2][3];
      points->GetPoint(data[0], dataCoords[0]);
      points->GetPoint(data[1], dataCoords[1]);

      // Find the points.
      for (auto i = 0; i < s_skeleton.size(); ++i)
      {
        if (memcmp(s_skeleton[i]->worldPosition, dataCoords[0], 3 * sizeof(double)) == 0)
          pointA = s_skeleton[i];

        if (memcmp(s_skeleton[i]->worldPosition, dataCoords[1], 3 * sizeof(double)) == 0)
          pointB = s_skeleton[i];
      }

      if (pointA == nullptr)
      {
        pointA = new SkeletonNode
        { dataCoords[0] };
        s_skeleton << pointA;
      }

      if (pointB == nullptr)
      {
        pointB = new SkeletonNode
        { dataCoords[1] };
        s_skeleton << pointB;
      }

      if (!pointA->connections.contains(pointB))
      {
        pointA->connections << pointB;
      }

      if (!pointB->connections.contains(pointA))
      {
        pointB->connections << pointA;
      }
    }
  }

  BuildRepresentation();
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::PrintSelf(std::ostream &os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  Superclass::PrintSelf(os, indent);
  os << indent << "Number of points in the skeleton: " << s_skeleton.size() << std::endl;
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
        for(auto node: s_skeleton)
        {
          node->worldPosition[0] = node->worldPosition[0]/s_skeletonSpacing[0] * spacing[0];
          node->worldPosition[1] = node->worldPosition[1]/s_skeletonSpacing[1] * spacing[1];
          node->worldPosition[2] = node->worldPosition[2]/s_skeletonSpacing[2] * spacing[2];
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

    m_tolerance = 2 * max;

    BuildRepresentation();
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::cleanup()
{
  QMutexLocker lock(&s_skeletonMutex);

  for(auto node: s_skeleton)
  {
    delete node;
  }

  s_skeleton.clear();
  s_currentVertex = nullptr;
}

//-----------------------------------------------------------------------------
double vtkSkeletonWidgetRepresentation::FindClosestDistanceAndNode(int X, int Y, double worldPos[3], int &node_i, int &node_j) const
{
  if (!Renderer) return VTK_DOUBLE_MAX;

  std::memset(worldPos, 0, 3*sizeof(double));
  node_i = node_j = VTK_INT_MAX;

  double *pos_i, *pos_j;
  double projection[3];
  double result = VTK_DOUBLE_MAX;
  unsigned int segmentNode1Index = VTK_INT_MAX;
  unsigned int segmentNode2Index = VTK_INT_MAX;

  double point_pos[3];
  int displayPos[2]{X,Y};
  GetWorldPositionFromDisplayPosition(displayPos, point_pos);

  // build temporary map to accelerate access to lines
  QMap<SkeletonNode *, unsigned int> locator;
  for(unsigned int i = 0; i < GetNumberOfNodes(); ++i)
  {
    locator[s_skeleton[i]] = i;
  }

  for (int i = 0; i < s_skeleton.size(); i++)
  {
    pos_i = s_skeleton[i]->worldPosition;

    auto connections = s_skeleton[i]->connections;
    for(int j = 0; j < connections.size(); ++j)
    {
      if(connections[j] == s_currentVertex) continue;

      pos_j = s_skeleton[locator[connections[j]]]->worldPosition;

      double v[3]{pos_j[0]-pos_i[0], pos_j[1]-pos_i[1], pos_j[2]-pos_i[2]};
      double w[3]{point_pos[0]-pos_i[0], point_pos[1]-pos_i[1], point_pos[2]-pos_i[2]};

      double dotwv = 0;
      double dotvv = 0;
      for(auto ii: {0,1,2})
      {
        dotwv += w[ii]*v[ii];
        dotvv += v[ii]*v[ii];
      }

      double r = dotwv / dotvv;

      if(r <= 0)
      {
        std::memcpy(projection, pos_i, 3*sizeof(double));
        segmentNode1Index = segmentNode2Index = i;
      }
      else
      {
        if(r >= 1)
        {
          std::memcpy(projection, pos_j, 3*sizeof(double));
          segmentNode1Index = segmentNode2Index = locator[connections[j]];
        }
        else
        {
          projection[0] = pos_i[0] + r*(pos_j[0] - pos_i[0]);
          projection[1] = pos_i[1] + r*(pos_j[1] - pos_i[1]);
          projection[2] = pos_i[2] + r*(pos_j[2] - pos_i[2]);
          segmentNode1Index = i;
          segmentNode2Index = locator[connections[j]];
        }
      }

      double pointDistance = std::pow(projection[0] - point_pos[0], 2) + std::pow(projection[1] - point_pos[1], 2) + std::pow(projection[2] - point_pos[2], 2);

      if(result > pointDistance)
      {
        node_i = segmentNode1Index;
        node_j = segmentNode2Index;
        std::memcpy(worldPos, projection, 3*sizeof(double));
        result = pointDistance;
      }
    }
  }

  return result;
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
    FindClosestNode(x, y, worldPos, nodeIndex);

    {
      QMutexLocker lock(&s_skeletonMutex);
      if ((nodeIndex == VTK_INT_MAX) || !m_visiblePoints.contains(s_skeleton[nodeIndex])) return false;
    }

    int displayPos[2]{x,y};
    double displayWorldPos[3];
    GetWorldPositionFromDisplayPosition(displayPos, displayWorldPos);

    return (m_tolerance > vtkMath::Distance2BetweenPoints(worldPos, displayWorldPos));
  }

  return false;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidgetRepresentation::SetColor(const QColor &color)
{
  if (color != m_color)
  {
    m_color = color;
    m_linesActor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
    m_linesActor->Modified();
    NeedToRenderOn();
  }
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidgetRepresentation::TryToJoin(int X, int Y)
{
  bool hasPointer = true;
  SkeletonNode *closestNode;
  {
    QMutexLocker lock(&s_skeletonMutex);
    if (s_skeleton.size() < 3) return false;

    double worldPos[3];
    int nodeIndex = VTK_INT_MAX;
    if(s_currentVertex) s_skeleton.removeAll(s_currentVertex);
    FindClosestNode(X, Y, worldPos, nodeIndex);
    if(s_currentVertex) s_skeleton << s_currentVertex;

    if (nodeIndex == VTK_INT_MAX) return false;

    closestNode = s_skeleton[nodeIndex];
    hasPointer = s_currentVertex != nullptr;
  }

  if(!hasPointer) AddNodeAtDisplayPosition(X, Y);

  QMutexLocker lock(&s_skeletonMutex);
  auto isClose = (m_tolerance > vtkMath::Distance2BetweenPoints(s_currentVertex->worldPosition, closestNode->worldPosition));
  if (isClose)
  {
    for (auto connectionNode : s_currentVertex->connections)
    {
      if(!connectionNode->connections.contains(closestNode)) connectionNode->connections << closestNode;
      if(!closestNode->connections.contains(connectionNode)) closestNode->connections << connectionNode;

      connectionNode->connections.removeAll(s_currentVertex);
    }
    s_currentVertex->connections.clear();
    s_currentVertex->connections << closestNode;

    return true;
  }

  return false;
}
