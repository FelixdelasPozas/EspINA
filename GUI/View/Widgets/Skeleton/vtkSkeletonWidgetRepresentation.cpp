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

namespace ESPINA
{
  vtkStandardNewMacro(vtkSkeletonWidgetRepresentation);

  const double vtkSkeletonWidgetRepresentation::s_sliceWindow = 40;
  vtkSkeletonWidgetRepresentation::SkeletonNode *vtkSkeletonWidgetRepresentation::s_currentVertex = nullptr;
  QList<vtkSkeletonWidgetRepresentation::SkeletonNode *> vtkSkeletonWidgetRepresentation::s_skeleton;

  //-----------------------------------------------------------------------------
  vtkSkeletonWidgetRepresentation::vtkSkeletonWidgetRepresentation()
  : m_orientation     {Plane::UNDEFINED}
  , m_tolerance       {40}
  , m_slice           {-1}
  , m_shift           {-1}
  , m_color           {QColor{254,254,254}}
  {
    this->m_colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    this->m_colors->SetName("Colors");
    this->m_colors->SetNumberOfComponents(3);

    this->m_points = vtkSmartPointer<vtkPoints>::New();
    this->m_points->SetNumberOfPoints(1);
    this->m_points->SetPoint(0, 0.0, 0.0, 0.0);

    this->m_pointsData = vtkSmartPointer<vtkPolyData>::New();
    this->m_pointsData->SetPoints(this->m_points);
    this->m_pointsData->GetPointData()->SetScalars(m_colors);

    this->m_glypher = vtkSmartPointer<vtkGlyph3D>::New();
    this->m_glypher->SetInputData(m_pointsData);
    this->m_glypher->SetColorModeToColorByScalar();

    this->m_glypher->ScalingOn();
    this->m_glypher->SetScaleModeToDataScalingOff();
    this->m_glypher->SetScaleFactor(1.0);

    // Use a sphere for nodes.
    auto sphere = vtkSmartPointer<vtkSphereSource>::New();
    sphere->SetRadius(0.5);
    this->m_glypher->SetSourceConnection(sphere->GetOutputPort());

    auto sphere2 = vtkSmartPointer<vtkSphereSource>::New();
    sphere2->SetRadius(0.75);

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(1);
    points->InsertNextPoint(0,0,0);
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
    m_pointerActor->GetProperty()->SetColor(1,1,1);

    this->m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    this->m_mapper->SetInputData(m_glypher->GetOutput());

    this->m_actor = vtkSmartPointer<vtkActor>::New();
    this->m_actor->SetMapper(this->m_mapper);
    this->m_actor->GetProperty()->SetLineWidth(3);
    this->m_actor->GetProperty()->SetPointSize(1);
    this->m_actor->GetProperty()->SetColor(m_color.redF(),m_color.greenF(),m_color.blueF());

    this->m_lines = vtkSmartPointer<vtkPolyData>::New();
    this->m_linesMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    this->m_linesMapper->SetInputData(this->m_lines);

    this->m_linesActor = vtkSmartPointer<vtkActor>::New();
    this->m_linesActor->SetMapper(this->m_linesMapper);
    this->m_linesActor->GetProperty()->SetLineWidth(2.5);
    this->m_linesActor->GetProperty()->SetColor(m_color.redF(),m_color.greenF(),m_color.blueF());

    this->m_interactionOffset[0] = 0.0;
    this->m_interactionOffset[1] = 0.0;
  }

  //-----------------------------------------------------------------------------
  vtkSkeletonWidgetRepresentation::~vtkSkeletonWidgetRepresentation()
  {
    for(auto node: s_skeleton)
    {
      delete node;
    }

    s_skeleton.clear();
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidgetRepresentation::AddNodeAtPosition(double worldPos[3])
  {
    for(auto i: {0,1,2})
    {
      worldPos[i] = std::round(worldPos[i]/m_spacing[i])*m_spacing[i];
    }

    worldPos[normalCoordinateIndex(this->m_orientation)] = this->m_slice;
    auto node = new SkeletonNode{worldPos};

    this->s_skeleton.push_back(node);

    if(this->s_currentVertex != nullptr)
    {
      this->s_currentVertex->connections << node;
      node->connections << s_currentVertex;
    }

    this->s_currentVertex = node;
    this->BuildRepresentation();
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidgetRepresentation::AddNodeAtWorldPosition(double x, double y, double z)
  {
    double worldPos[3]{x,y,z};
    this->AddNodeAtPosition(worldPos);
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidgetRepresentation::AddNodeAtDisplayPosition(int displayPos[2])
  {
    double worldPos[3];
    this->GetWorldPositionFromDisplayPosition(displayPos, worldPos);
    this->AddNodeAtPosition(worldPos);
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidgetRepresentation::AddNodeAtDisplayPosition(int X, int Y)
  {
    int displayPos[2] = {X,Y};
    this->AddNodeAtDisplayPosition(displayPos);
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::ActivateNode(int displayPos[2])
  {
    SkeletonNode *closestNode = nullptr;

    double worldPos[3];
    this->GetWorldPositionFromDisplayPosition(displayPos, worldPos);

    double distance2 = VTK_DOUBLE_MAX; // we'll use distance^2 to avoid doing square root operation.
    for(auto node: m_visiblePoints.keys())
    {
      auto nodeDistance =  vtkMath::Distance2BetweenPoints(worldPos, node->worldPosition);

      if(nodeDistance < distance2)
      {
        distance2 = nodeDistance;
        closestNode = node;
      }
    }

    if((closestNode == nullptr) || ((m_tolerance * m_tolerance) < vtkMath::Distance2BetweenPoints(worldPos, closestNode->worldPosition)))
    {
      this->s_currentVertex = nullptr;
    }
    else
    {
      this->s_currentVertex = closestNode;
    }
    this->UpdatePointer();

    return (s_currentVertex != nullptr);
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::ActivateNode(int X, int Y)
  {
    int displayPos[2]{X,Y};

    return this->ActivateNode(displayPos);
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::ActivateNode(SkeletonNode *node)
  {
    s_currentVertex = node;
    this->UpdatePointer();

    return true;
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidgetRepresentation::DeactivateNode()
  {
    s_currentVertex = nullptr;
    this->UpdatePointer();
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::SetActiveNodeToDisplayPosition(int displayPos[2], bool updateRepresentation)
  {
    double worldPos[3];
    this->GetWorldPositionFromDisplayPosition(displayPos, worldPos);

    return this->SetActiveNodeToWorldPosition(worldPos, updateRepresentation);
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::SetActiveNodeToDisplayPosition(int X, int Y, bool updateRepresentation)
  {
    int displayPos[2]{X,Y};

    return this->SetActiveNodeToDisplayPosition(displayPos, updateRepresentation);
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::SetActiveNodeToWorldPosition(double x, double y, double z, bool updateRepresentation)
  {
    double worldPos[3]{x,y,z};

    return this->SetActiveNodeToWorldPosition(worldPos, updateRepresentation);
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::SetActiveNodeToWorldPosition(double worldPos[3], bool updateRepresentation)
  {
    if (this->s_currentVertex == nullptr) return false;

    std::memcpy(this->s_currentVertex->worldPosition, worldPos, 3*sizeof(double));

    if(updateRepresentation)
    {
      this->BuildRepresentation();
    }

    return true;
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::GetActiveNodeWorldPosition(double worldPos[3])
  {
    if (this->s_currentVertex == nullptr) return false;

    std::memcpy(worldPos, this->s_currentVertex->worldPosition, 3*sizeof(double));

    return true;
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::DeleteCurrentNode()
  {
    if(this->s_currentVertex == nullptr) return false;

    s_skeleton.removeAll(s_currentVertex);
    delete this->s_currentVertex;
    this->s_currentVertex = nullptr;

    this->BuildRepresentation();

    return true;
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidgetRepresentation::ClearAllNodes()
  {
    DeleteCurrentNode();
    for(auto node: this->s_skeleton)
    {
      delete node;
    }

    this->s_skeleton.clear();

    this->BuildRepresentation();
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::AddNodeOnContour(int X, int Y)
  {
    double worldPos[3];
    int node_i = 0;
    int node_j = 0;
    auto distance = this->FindClosestDistanceAndNode(X,Y,worldPos, node_i, node_j);

    if(distance > m_tolerance) return false;

    if(node_i != node_j)
    {
      s_currentVertex = nullptr;
      AddNodeAtPosition(worldPos); // adds a new node and makes it current node.

      s_currentVertex->connections << s_skeleton[node_i] << s_skeleton[node_j];

      s_skeleton[node_i]->connections.removeAll(s_skeleton[node_j]);
      s_skeleton[node_i]->connections << s_currentVertex;

      s_skeleton[node_j]->connections.removeAll(s_skeleton[node_i]);
      s_skeleton[node_j]->connections << s_currentVertex;
    }
    else
    {
      s_currentVertex = s_skeleton[node_i];
      this->AddNodeAtDisplayPosition(X,Y);
    }

    this->BuildRepresentation();

    return true;
  }

  //-----------------------------------------------------------------------------
  unsigned int vtkSkeletonWidgetRepresentation::GetNumberOfNodes() const
  {
    return s_skeleton.size();
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::SetOrientation(Plane plane)
  {
    if(m_orientation == Plane::UNDEFINED)
    {
      m_orientation = plane;
      return true;
    }

    return false;
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidgetRepresentation::BuildRepresentation()
  {
    if(this->s_skeleton.size() == 0 || !this->Renderer || !this->Renderer->GetActiveCamera())
    {
      VisibilityOff();
      return;
    }

    double p1[4], p2[4];
    this->Renderer->GetActiveCamera()->GetFocalPoint(p1);
    p1[3] = 1.0;
    this->Renderer->SetWorldPoint(p1);
    this->Renderer->WorldToView();
    this->Renderer->GetViewPoint(p1);

    double depth = p1[2];
    double aspect[2];
    this->Renderer->ComputeAspect();
    this->Renderer->GetAspect(aspect);

    p1[0] = -aspect[0];
    p1[1] = -aspect[1];
    this->Renderer->SetViewPoint(p1);
    this->Renderer->ViewToWorld();
    this->Renderer->GetWorldPoint(p1);

    p2[0] = aspect[0];
    p2[1] = aspect[1];
    p2[2] = depth;
    p2[3] = 1.0;
    this->Renderer->SetViewPoint(p2);
    this->Renderer->ViewToWorld();
    this->Renderer->GetWorldPoint(p2);

    double distance = sqrt(vtkMath::Distance2BetweenPoints(p1, p2));

    int *size = this->Renderer->GetRenderWindow()->GetSize();
    double viewport[4];
    this->Renderer->GetViewport(viewport);

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

    unsigned char red[3]   = {255, 0, 0};
    unsigned char green[3] = {0, 255, 0};
    unsigned char blue[3]  = {0, 0, 255};
    m_colors->Reset();

    m_visiblePoints.clear();
    double worldPos[3];
    for(auto node: s_skeleton)
    {
      // we only want "some" points in the screen representation and want all the representation to be visible.
      if(areEqual(node->worldPosition[planeIndex], m_slice))
      {
        if(!m_visiblePoints.contains(node))
        {
          std::memcpy(worldPos, node->worldPosition, 3 * sizeof(double));
          worldPos[planeIndex] = m_slice + m_shift;

          m_visiblePoints.insert(node, m_points->InsertNextPoint(worldPos));
          m_colors->InsertNextTupleValue(green);
        }

        for(auto connectedNode: node->connections)
        {
          if(!m_visiblePoints.contains(connectedNode))
          {
            std::memcpy(worldPos, connectedNode->worldPosition, 3 * sizeof(double));
            if(areEqual(worldPos[planeIndex], m_slice))
            {
                m_colors->InsertNextTupleValue(green);
            }
            else
            {
              if(worldPos[planeIndex] > m_slice)
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
        for(auto connectedNode: node->connections)
          if((node->worldPosition[planeIndex] < m_slice && connectedNode->worldPosition[planeIndex] >= m_slice) ||
             (node->worldPosition[planeIndex] > m_slice && connectedNode->worldPosition[planeIndex] <= m_slice))
          {
            if(!m_visiblePoints.contains(node))
            {
              std::memcpy(worldPos, node->worldPosition, 3 * sizeof(double));
              if(worldPos[planeIndex] > m_slice)
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

            if(!m_visiblePoints.contains(connectedNode))
            {
              std::memcpy(worldPos, connectedNode->worldPosition, 3 * sizeof(double));
              if(areEqual(worldPos[planeIndex], m_slice))
              {
                  m_colors->InsertNextTupleValue(green);
              }
              else
              {
                if(worldPos[planeIndex] > m_slice)
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

    if(m_visiblePoints.empty())
    {
      VisibilityOff();
      NeedToRenderOn();
      return;
    }

    m_points->Modified();
    m_pointsData->Modified();
    m_pointsData->GetPointData()->Modified();
    m_glypher->SetInputData(m_pointsData);
    m_glypher->SetScaleFactor(distance * this->HandleSize);
    m_glypher->Update();
    m_mapper->Update();
    m_actor->Modified();

    m_lines->SetLines(cells);
    m_lines->SetPoints(m_points);
    m_lines->Modified();
    m_linesMapper->Update();
    m_linesActor->Modified();

    m_pointer->SetScaleFactor(distance * this->HandleSize);
    this->UpdatePointer();

    this->VisibilityOn();
    this->NeedToRenderOn();
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidgetRepresentation::UpdatePointer()
  {
    if(s_currentVertex != nullptr)
    {
      double pos[3];
      std::memcpy(pos, s_currentVertex->worldPosition, 3*sizeof(double));
      pos[normalCoordinateIndex(m_orientation)] = m_slice;

      auto polyData = vtkPolyData::SafeDownCast(m_pointer->GetInput());
      polyData->GetPoints()->Reset();
      polyData->GetPoints()->SetNumberOfPoints(1);
      polyData->GetPoints()->SetPoint(0,s_currentVertex->worldPosition);
      polyData->GetPoints()->Modified();
      polyData->Modified();
      m_pointer->Update();
      m_pointerActor->GetMapper()->Update();
      double color[3];
      m_linesActor->GetProperty()->GetColor(color);
      m_pointerActor->GetProperty()->SetColor(1-color[0], 1-color[1], 1-color[2]);
      m_pointerActor->VisibilityOn();
    }
    else
    {
      m_pointerActor->VisibilityOff();
    }

    m_pointerActor->Modified();
    this->NeedToRenderOn();
  }

  //-----------------------------------------------------------------------------
  int vtkSkeletonWidgetRepresentation::ComputeInteractionState(int X, int Y, int vtkNotUsed(modified))
  {
    if (IsNearNode(X,Y))
    {
      this->InteractionState = vtkSkeletonWidgetRepresentation::NearPoint;
    }
    else
    {
      double worldPos[3];
      int unused1 = 0;
      int unused2 = 0;
      if (this->FindClosestDistanceAndNode(X, Y, worldPos, unused1, unused2) <= this->m_tolerance)
      {
        this->InteractionState = vtkSkeletonWidgetRepresentation::NearContour;
      }
      else
      {
        this->InteractionState = vtkSkeletonWidgetRepresentation::Outside;
      }
    }

    return this->InteractionState;
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidgetRepresentation::ReleaseGraphicsResources(vtkWindow* win)
  {
    this->m_actor->ReleaseGraphicsResources(win);
    this->m_pointerActor->ReleaseGraphicsResources(win);
    this->m_linesActor->ReleaseGraphicsResources(win);
  }

  //-----------------------------------------------------------------------------
  int vtkSkeletonWidgetRepresentation::RenderOverlay(vtkViewport* viewport)
  {
    int count = 0;
    if (this->m_pointerActor->GetVisibility())
    {
      count += this->m_pointerActor->RenderOverlay(viewport);
    }

    if (this->m_linesActor->GetVisibility())
    {
      count += this->m_linesActor->RenderOverlay(viewport);
    }

    if (this->m_actor->GetVisibility())
    {
      count += this->m_actor->RenderOverlay(viewport);
    }

    return count;
  }

  //-----------------------------------------------------------------------------
  int vtkSkeletonWidgetRepresentation::RenderOpaqueGeometry(vtkViewport* viewport)
  {
    // Since we know RenderOpaqueGeometry gets called first, will do the build here
    this->BuildRepresentation();

    int count = 0;
    if (this->m_pointerActor->GetVisibility())
    {
      count += this->m_pointerActor->RenderOpaqueGeometry(viewport);
    }

    if (this->m_linesActor->GetVisibility())
    {
      count += this->m_linesActor->RenderOpaqueGeometry(viewport);
    }

    if (this->m_actor->GetVisibility())
    {
      count += this->m_actor->RenderOpaqueGeometry(viewport);
    }

    return count;
  }

  //-----------------------------------------------------------------------------
  int vtkSkeletonWidgetRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport* viewport)
  {
    int count = 0;

    if (this->m_pointerActor->GetVisibility())
    {
      count += this->m_pointerActor->RenderTranslucentPolygonalGeometry(viewport);
    }

    if (this->m_linesActor->GetVisibility())
    {
      count += this->m_linesActor->RenderTranslucentPolygonalGeometry(viewport);
    }

    if (this->m_actor->GetVisibility())
    {
      count += this->m_actor->RenderTranslucentPolygonalGeometry(viewport);
    }

    return count;
  }

  //-----------------------------------------------------------------------------
  int vtkSkeletonWidgetRepresentation::HasTranslucentPolygonalGeometry()
  {
    int result = 0;

    if (this->m_pointerActor->GetVisibility())
    {
      result += this->m_pointerActor->HasTranslucentPolygonalGeometry();
    }

    if (this->m_linesActor->GetVisibility())
    {
      result |= this->m_linesActor->HasTranslucentPolygonalGeometry();
    }
    if (this->m_actor->GetVisibility())
    {
      result |= this->m_actor->HasTranslucentPolygonalGeometry();
    }

    return result;
  }

  //-----------------------------------------------------------------------------
  vtkSmartPointer<vtkPolyData> vtkSkeletonWidgetRepresentation::GetRepresentationPolyData() const
  {
    QMap<SkeletonNode *, vtkIdType> locator;
    QMap<vtkIdType, QList<vtkIdType>> relationsLocator;

    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(s_skeleton.size());
    auto lines = vtkSmartPointer<vtkCellArray>::New();

    for(auto node: s_skeleton)
    {
      locator.insert(node, points->InsertNextPoint(node->worldPosition));
    }

    for(auto node: s_skeleton)
    {
      relationsLocator.insert(locator[node], QList<vtkIdType>());

      for(auto connectedNode: node->connections)
      {
        relationsLocator[locator[node]] << locator[connectedNode];
      }
    }

    for(auto nodeId: relationsLocator.keys())
    {
      for(auto connectionId: relationsLocator[nodeId])
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

    this->Renderer->SetDisplayPoint(pos);
    this->Renderer->DisplayToWorld();
    this->Renderer->GetWorldPoint(pos);

    worldPos[0] = pos[0];
    worldPos[1] = pos[1];
    worldPos[2] = pos[2];
    worldPos[normalCoordinateIndex(this->m_orientation)] = m_slice;
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidgetRepresentation::FindClosestNode(int X, int Y, double worldPos[3], int &closestNode) const
  {
    double distance = VTK_DOUBLE_MAX;

    double pos[4];
    int displayPos[2]{X,Y};
    this->GetWorldPositionFromDisplayPosition(displayPos, pos);
    pos[3] = 0;

    auto planeIndex = normalCoordinateIndex(m_orientation);
    for(auto i = 0; i < s_skeleton.size(); ++i)
    {
      if(!areEqual(this->s_skeleton[i]->worldPosition[planeIndex], m_slice)) continue;

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
    this->ClearAllNodes();

    // Points in our skeleton will be added while traversing lines.
    lines->InitTraversal();
    vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();
    while(lines->GetNextCell(idList))
    {
      if(idList->GetNumberOfIds() != 2) continue;

      SkeletonNode *pointA = nullptr;
      SkeletonNode *pointB = nullptr;
      vtkIdType data[2];
      data[0] = idList->GetId(0);
      data[1] = idList->GetId(1);
      double dataCoords[2][3];
      points->GetPoint(data[0], dataCoords[0]);
      points->GetPoint(data[1], dataCoords[1]);

      // Find the points.
      for(auto i = 0; i < s_skeleton.size(); ++i)
      {
        if(memcmp(s_skeleton[i]->worldPosition, dataCoords[0], 3*sizeof(double)) == 0)
          pointA = s_skeleton[i];

        if(memcmp(s_skeleton[i]->worldPosition, dataCoords[1], 3*sizeof(double)) == 0)
          pointB = s_skeleton[i];
      }

      if(pointA == nullptr)
      {
        pointA = new SkeletonNode{dataCoords[0]};
        s_skeleton << pointA;
      }

      if(pointB == nullptr)
      {
        pointB = new SkeletonNode{dataCoords[1]};
        s_skeleton << pointB;
      }

      if(!pointA->connections.contains(pointB))
      {
        pointA->connections << pointB;
      }

      if(!pointB->connections.contains(pointA))
      {
        pointB->connections << pointA;
      }
    }

    this->BuildRepresentation();
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidgetRepresentation::PrintSelf(std::ostream &os, vtkIndent indent)
  {
    //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
    this->Superclass::PrintSelf(os, indent);
    os << indent << "Number of points in the skeleton: " << this->s_skeleton.size() << std::endl;
    os << indent << "Pixel Tolerance: " << this->m_tolerance << std::endl;
  }

  //-----------------------------------------------------------------------------
  double vtkSkeletonWidgetRepresentation::FindClosestDistanceAndNode(int X, int Y, double worldPos[3], int &node_i, int &node_j) const
  {
    if(!this->Renderer) return VTK_DOUBLE_MAX;

    std::memset(worldPos, 0, 3*sizeof(double));
    node_i = node_j = VTK_INT_MAX;

    double *pos_i, *pos_j;
    double projection[3];
    double result = VTK_DOUBLE_MAX;
    unsigned int segmentNode1Index = VTK_INT_MAX;
    unsigned int segmentNode2Index = VTK_INT_MAX;

    double point_pos[3];
    int displayPos[2]{X,Y};
    this->GetWorldPositionFromDisplayPosition(displayPos, point_pos);

    // build temporary map to accelerate access to lines
    QMap<SkeletonNode *, unsigned int> locator;
    for(unsigned int i = 0; i < this->GetNumberOfNodes(); ++i)
    {
      locator[this->s_skeleton[i]] = i;
    }

    for (int i = 0; i < this->s_skeleton.size(); i++)
    {
      pos_i = this->s_skeleton[i]->worldPosition;

      auto connections = this->s_skeleton[i]->connections;
      for(int j = 0; j < connections.size(); ++j)
      {
        pos_j = this->s_skeleton[locator[connections[j]]]->worldPosition;

        double v[3]{ pos_j[0]-pos_i[0],
                     pos_j[1]-pos_i[1],
                     pos_j[2]-pos_i[2]};

        double w[3]{ point_pos[0]-pos_i[0],
                     point_pos[1]-pos_i[1],
                     point_pos[2]-pos_i[2]};

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
    this->m_slice = slice;
    this->BuildRepresentation();
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::IsNearNode(int x, int y) const
  {
    if(!this->Renderer) return false;

    double worldPos[3];
    int nodeIndex = VTK_INT_MAX;
    this->FindClosestNode(x,y, worldPos, nodeIndex);

    if((nodeIndex == VTK_INT_MAX) || !m_visiblePoints.contains(s_skeleton[nodeIndex])) return false;

    int displayPos[2]{x,y};
    double displayWorldPos[3];
    this->GetWorldPositionFromDisplayPosition(displayPos, displayWorldPos);

    return (m_tolerance * m_tolerance > vtkMath::Distance2BetweenPoints(worldPos, displayWorldPos));
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::IsPointTooClose(int x, int y) const
  {
    if(this->s_currentVertex == nullptr) return false;

    double worldPos[3];
    int displayPos[2]{x,y};
    this->GetWorldPositionFromDisplayPosition(displayPos, worldPos);

    return ((m_tolerance * m_tolerance) > vtkMath::Distance2BetweenPoints(worldPos, s_currentVertex->worldPosition));
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidgetRepresentation::SetColor(const QColor &color)
  {
    if(color == m_color)
      return;

    m_color = color;
    this->m_linesActor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
    this->m_linesActor->Modified();
    this->NeedToRenderOn();
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::TryToJoin(int X, int Y)
  {
    if(s_skeleton.size() < 3) return false;

    double worldPos[3];
    int nodeIndex = VTK_INT_MAX;
    s_skeleton.removeAll(s_currentVertex);
    this->FindClosestNode(X,Y, worldPos, nodeIndex);
    s_skeleton << s_currentVertex;

    if(nodeIndex == VTK_INT_MAX) return false;

    auto closestNode = s_skeleton[nodeIndex];

    // remove current vertex if the distance is too close
    auto isClose = ((m_tolerance * m_tolerance) > vtkMath::Distance2BetweenPoints(s_currentVertex->worldPosition, closestNode->worldPosition));
    if(!s_currentVertex->connections.contains(closestNode) && isClose)
    {
      for(auto connectionNode: s_currentVertex->connections)
      {
        connectionNode->connections.removeAll(s_currentVertex);
        connectionNode->connections << closestNode;
        closestNode->connections << connectionNode;
      }
      s_currentVertex->connections.clear();
      s_skeleton.removeAll(s_currentVertex);
      delete s_currentVertex;

      s_currentVertex = closestNode;
      return true;
    }

    return false;
  }

} // namespace EspINA
