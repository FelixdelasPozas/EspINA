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
#include <vtkSphereSource.h>
#include <vtkRenderWindow.h>

// Qt
#include <QStack>
#include <QMap>
#include <QDebug>

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
    this->m_points = vtkSmartPointer<vtkPoints>::New();
    this->m_points->SetNumberOfPoints(1);
    this->m_points->SetPoint(0, 0.0, 0.0, 0.0);

    this->m_pointsData = vtkSmartPointer<vtkPolyData>::New();
    this->m_pointsData->SetPoints(this->m_points);

    this->m_glypher = vtkSmartPointer<vtkGlyph3D>::New();
    this->m_glypher->SetInputData(m_pointsData);
    this->m_glypher->ScalingOn();
    this->m_glypher->SetScaleModeToDataScalingOff();
    this->m_glypher->SetScaleFactor(1.0);

    // Use a sphere for nodes.
    auto sphere = vtkSmartPointer<vtkSphereSource>::New();
    sphere->SetRadius(0.5);
    this->m_glypher->SetSourceConnection(sphere->GetOutputPort());

    this->m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    this->m_mapper->SetInputData(m_glypher->GetOutput());
    this->m_mapper->SetResolveCoincidentTopologyToPolygonOffset();
    this->m_mapper->ImmediateModeRenderingOn();

    this->m_actor = vtkSmartPointer<vtkActor>::New();
    this->m_actor->SetMapper(this->m_mapper);
    this->m_actor->GetProperty()->SetLineWidth(3);
    this->m_actor->GetProperty()->SetPointSize(1);
    this->m_actor->GetProperty()->SetColor(m_color.redF(),m_color.greenF(),m_color.blueF());

    this->m_lines = vtkSmartPointer<vtkPolyData>::New();
    this->m_linesMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    this->m_linesMapper->SetInputData(this->m_lines);
    this->m_linesMapper->SetResolveCoincidentTopologyToPolygonOffset();
    this->m_linesMapper->ImmediateModeRenderingOn();

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
      delete node;

    s_skeleton.clear();
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidgetRepresentation::AddNodeAtPosition(double worldPos[3])
  {
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
    for(auto node: s_skeleton)
    {
      // only activate nodes in the slice.
      if(!areEqual(node->worldPosition[normalCoordinateIndex(this->m_orientation)], m_slice, 0.1))
        continue;

      auto nodeDistance =  vtkMath::Distance2BetweenPoints(worldPos, node->worldPosition);

      if(nodeDistance < distance2)
      {
        distance2 = nodeDistance;
        closestNode = node;
      }
    }

    if(closestNode == nullptr)
      return false;

    this->s_currentVertex = closestNode;
    this->BuildRepresentation();

    return true;
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
    this->BuildRepresentation();

    return true;
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::SetActiveNodeToDisplayPosition(int displayPos[2])
  {
    double worldPos[3];
    this->GetWorldPositionFromDisplayPosition(displayPos, worldPos);
    return this->SetActiveNodeToWorldPosition(worldPos);
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::SetActiveNodeToDisplayPosition(int X, int Y)
  {
    int displayPos[2]{X,Y};
    return this->SetActiveNodeToDisplayPosition(displayPos);
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::SetActiveNodeToWorldPosition(double x, double y, double z)
  {
    double worldPos[3]{x,y,z};
    return this->SetActiveNodeToWorldPosition(worldPos);
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::SetActiveNodeToWorldPosition(double worldPos[3])
  {
    if (this->s_currentVertex == nullptr)
      return false;

    std::memcpy(this->s_currentVertex->worldPosition, worldPos, 3*sizeof(double));
    this->s_currentVertex->worldPosition[normalCoordinateIndex(m_orientation)] = m_slice;

    this->BuildRepresentation();

    return true;
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::GetActiveNodeWorldPosition(double worldPos[3]) const
  {
    if(this->s_currentVertex == nullptr)
      return false;

    std::memcpy(worldPos, this->s_currentVertex->worldPosition, 3*sizeof(double));
    return true;
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::DeleteCurrentNode()
  {
    if(this->s_currentVertex == nullptr)
      return false;

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
      delete node;

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

    if(distance > m_tolerance)
      return false;

    if(node_i != node_j)
    {
      auto newNode = new SkeletonNode{worldPos};
      s_skeleton << newNode;

      newNode->connections << s_skeleton[node_i] << s_skeleton[node_j];

      s_skeleton[node_i]->connections.removeAll(s_skeleton[node_j]);
      s_skeleton[node_i]->connections << newNode;

      s_skeleton[node_j]->connections.removeAll(s_skeleton[node_i]);
      s_skeleton[node_j]->connections << newNode;

      s_currentVertex = newNode;
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
    if(this->s_skeleton.size() == 0)
      return;

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

    QMap<SkeletonNode *, vtkIdType> visiblePoints;
    unsigned int numLines = 0;
    double worldPos[3];
    for(auto node: s_skeleton)
    {
      // we only want "some" points in the screen representation and want all the representation to be visible.
      if((node->worldPosition[planeIndex] > (m_slice - s_sliceWindow)) && (node->worldPosition[planeIndex] < (m_slice + s_sliceWindow)))
      {
        std::memcpy(worldPos, node->worldPosition, 3 * sizeof(double));
        worldPos[planeIndex] = m_slice + m_shift;

        visiblePoints.insert(node, m_points->InsertNextPoint(worldPos));

        for(auto connectedNode: node->connections)
        {
          std::memcpy(worldPos, connectedNode->worldPosition, 3 * sizeof(double));
          worldPos[planeIndex] = m_slice + m_shift;

          visiblePoints.insert(connectedNode, m_points->InsertNextPoint(worldPos));

          auto line = vtkSmartPointer<vtkLine>::New();
          line->GetPointIds()->SetId(0, visiblePoints[node]);
          line->GetPointIds()->SetId(1, visiblePoints[connectedNode]);
          cells->InsertNextCell(line);
          ++numLines;
        }
      }
    }

    if(visiblePoints.empty())
    {
      VisibilityOff();
      NeedToRenderOn();
      return;
    }

    m_points->Modified();
    m_pointsData->Modified();
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

    this->VisibilityOn();
    this->NeedToRenderOn();
  }

  //-----------------------------------------------------------------------------
  int vtkSkeletonWidgetRepresentation::ComputeInteractionState(int X, int Y, int vtkNotUsed(modified))
  {
    static int i = 0;
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
    this->m_linesActor->ReleaseGraphicsResources(win);
  }

  //-----------------------------------------------------------------------------
  int vtkSkeletonWidgetRepresentation::RenderOverlay(vtkViewport* viewport)
  {
    int count = 0;
    if (this->m_linesActor->GetVisibility())
      count += this->m_linesActor->RenderOverlay(viewport);
    if (this->m_actor->GetVisibility())
      count += this->m_actor->RenderOverlay(viewport);
    return count;
  }

  //-----------------------------------------------------------------------------
  int vtkSkeletonWidgetRepresentation::RenderOpaqueGeometry(vtkViewport* viewport)
  {
    // Since we know RenderOpaqueGeometry gets called first, will do the build here
    this->BuildRepresentation();

    int count = 0;
    if (this->m_linesActor->GetVisibility())
      count += this->m_linesActor->RenderOpaqueGeometry(viewport);
    if (this->m_actor->GetVisibility())
      count += this->m_actor->RenderOpaqueGeometry(viewport);

    return count;
  }

  //-----------------------------------------------------------------------------
  int vtkSkeletonWidgetRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport* viewport)
  {
    int count = 0;

    if (this->m_linesActor->GetVisibility())
      count += this->m_linesActor->RenderTranslucentPolygonalGeometry(viewport);
    if (this->m_actor->GetVisibility())
      count += this->m_actor->RenderTranslucentPolygonalGeometry(viewport);

    return count;
  }

  //-----------------------------------------------------------------------------
  int vtkSkeletonWidgetRepresentation::HasTranslucentPolygonalGeometry()
  {
    int result = 0;

    if (this->m_linesActor->GetVisibility())
      result |= this->m_linesActor->HasTranslucentPolygonalGeometry();
    if (this->m_actor->GetVisibility())
      result |= this->m_actor->HasTranslucentPolygonalGeometry();

    return result;
  }

  //-----------------------------------------------------------------------------
  vtkSmartPointer<vtkPolyData> vtkSkeletonWidgetRepresentation::GetRepresentationPolyData() const
  {
    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->DeepCopy(this->m_lines);

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
  void vtkSkeletonWidgetRepresentation::FindClosestNode(int X, int Y, double worldPos[3], SkeletonNode* closestNode) const
  {
    double distance = VTK_DOUBLE_MAX;

    double pos[4];
    int displayPos[2]{X,Y};
    this->GetWorldPositionFromDisplayPosition(displayPos, pos);
    pos[3] = 0;

    for(auto node: s_skeleton)
    {
      auto nodeDistance = vtkMath::Distance2BetweenPoints(pos, node->worldPosition);
      if(distance > nodeDistance)
      {
        distance = nodeDistance;
        closestNode = node;
      }
    }

    if(closestNode != nullptr)
    {
      worldPos[0] = closestNode->worldPosition[0];
      worldPos[1] = closestNode->worldPosition[1];
      worldPos[2] = closestNode->worldPosition[2];
    }
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidgetRepresentation::Initialize(vtkSmartPointer<vtkPolyData> pd)
  {
    auto points = pd->GetPoints();
    auto linesData = pd->GetLines()->GetData();

    vtkIdType nPoints = points->GetNumberOfPoints();
    if (nPoints <= 0) // Yeah right.. build from nothing!
      return;

    // Clear all existing nodes.
    this->ClearAllNodes();
    for(vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i)
    {
      double worldPos[3];
      points->GetPoint(i, worldPos);

      auto node = new SkeletonNode{worldPos};
      s_skeleton << node;
    }

    for(vtkIdType i = 0; i < linesData->GetNumberOfTuples(); ++i)
    {
      double data[2];
      linesData->GetTuple(i, data);

      s_skeleton[data[0]]->connections << s_skeleton[data[1]];
      s_skeleton[data[1]]->connections << s_skeleton[data[0]];
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
    std::memset(worldPos, 0, 3*sizeof(double));
    node_i = node_j = VTK_INT_MAX;

    double *pos_i, *pos_j;
    double projection[3];
    double result = VTK_DOUBLE_MAX;
    unsigned int segmentNode1Index = VTK_INT_MAX;
    unsigned int segmentNode2Index = VTK_INT_MAX;
    double r, buenr;

    double point_pos[3];
    int displayPos[2]{X,Y};
    this->GetWorldPositionFromDisplayPosition(displayPos, point_pos);

    // build temporary map to accelerate access to lines
    QMap<SkeletonNode *, unsigned int> locator;
    for(unsigned int i = 0; i < this->GetNumberOfNodes(); ++i)
      locator[this->s_skeleton[i]] = i;

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

        r = dotwv / dotvv;

        if(r <= 0)
        {
          std::memcpy(projection, pos_i, 3*sizeof(double));
          segmentNode1Index = segmentNode2Index = i;
        }
        else
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

        double pointDistance = std::pow(projection[0] - point_pos[0], 2) + std::pow(projection[1] - point_pos[1], 2) + std::pow(projection[2] - point_pos[2], 2);

        if(result > pointDistance)
        {
          node_i = segmentNode1Index;
          node_j = segmentNode2Index;
          std::memcpy(worldPos, projection, 3*sizeof(double));
          buenr = r;
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
    double worldPos[3];
    SkeletonNode *closestNode = nullptr;

    this->FindClosestNode(x,y, worldPos, closestNode);

    int displayPos[2]{x,y};
    double displayWorldPos[3];
    this->GetWorldPositionFromDisplayPosition(displayPos, displayWorldPos);

    return ((m_tolerance * m_tolerance) > vtkMath::Distance2BetweenPoints(worldPos, displayWorldPos));
  }

  //-----------------------------------------------------------------------------
  bool vtkSkeletonWidgetRepresentation::IsPointTooClose(int x, int y) const
  {
    if(this->s_currentVertex == nullptr)
      return false;

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
    this->m_actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
    this->m_actor->Modified();
    this->m_linesActor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
    this->m_linesActor->Modified();
    this->NeedToRenderOn();
  }

} // namespace EspINA
