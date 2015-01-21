/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <GUI/View/Widgets/Contour/vtkContourToPolygonFilter.h>
#include <GUI/View/Widgets/Contour/vtkPlaneContourRepresentation.h>

// C++
#include <cmath>

// VTK
#include <vtkBox.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkContourLineInterpolator.h>
#include <vtkCoordinate.h>
#include <vtkHandleRepresentation.h>
#include <vtkIncrementalOctreePointLocator.h>
#include <vtkIntArray.h>
#include <vtkInteractorObserver.h>
#include <vtkLine.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPointPlacer.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkWindow.h>

// Qt
#include <QtGlobal>

using namespace ESPINA;

vtkCxxSetObjectMacro(vtkPlaneContourRepresentation, PointPlacer, vtkPointPlacer);
vtkCxxSetObjectMacro(vtkPlaneContourRepresentation, LineInterpolator, vtkContourLineInterpolator);

//----------------------------------------------------------------------------
vtkPlaneContourRepresentation::vtkPlaneContourRepresentation()
{
  this->Internal = new vtkPlaneContourRepresentationInternals;

  this->PixelTolerance = 15;
  this->WorldTolerance = 0.004;
  this->PointPlacer = nullptr;
  this->LineInterpolator = nullptr;
  this->Locator = nullptr;
  this->RebuildLocator = false;
  this->ActiveNode = -1;
  this->NeedToRender = 0;
  this->ClosedLoop = 0;
  this->ShowSelectedNodes = 0;
  this->CurrentOperation = vtkPlaneContourRepresentation::Inactive;
  this->Orientation = Plane::XY;
  this->PlaneShift = 0;
  this->Slice = 0;

  this->ResetLocator();
}

//----------------------------------------------------------------------------
vtkPlaneContourRepresentation::~vtkPlaneContourRepresentation()
{
  this->SetPointPlacer(nullptr);
  this->SetLineInterpolator(nullptr);
  this->Internal->ClearNodes();

  delete this->Internal;

  if (this->Locator)
  {
    this->Locator->Delete();
  }
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentation::ResetLocator()
{
  if (this->Locator)
  {
    this->Locator->Delete();
  }

  this->Locator = vtkIncrementalOctreePointLocator::New();
  this->Locator->SetBuildCubicOctree(1);
  this->RebuildLocator = true;
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentation::ClearAllNodes()
{
  this->ResetLocator();
  this->Internal->ClearNodes();

  this->BuildLines();
  this->BuildLocator();
  this->NeedToRender = 1;
  this->Modified();
}

//----------------------------------------------------------------------------
// NOTE: modified to avoid inserting duplicated nodes
void vtkPlaneContourRepresentation::AddNodeAtPositionInternal(double worldPos[3], double worldOrient[9], double displayPos[2])
{
  // Add a new point at this position
  vtkPlaneContourRepresentationNode *node = new vtkPlaneContourRepresentationNode;
  memcpy(node->WorldPosition, worldPos, 3*sizeof(double));

  node->WorldPosition[normalCoordinateIndex(this->Orientation)] = this->Slice + this->PlaneShift;

  node->Selected = 0;
  node->NormalizedDisplayPosition[0] = displayPos[0];
  node->NormalizedDisplayPosition[1] = displayPos[1];
  node->Points.clear();
  this->Renderer->DisplayToNormalizedDisplay(node->NormalizedDisplayPosition[0], node->NormalizedDisplayPosition[1]);

  memcpy(node->WorldOrientation, worldOrient, 9 * sizeof(double));

  // we have to check that node-2 and node-1 are not duplicated. node-1 and node are allowed to be duplicated because
  // the last one is the cursor node and it's not (yet) part of the contour
  if (this->CheckNodesForDuplicates(this->GetNumberOfNodes()-3, this->GetNumberOfNodes()-2))
  {
    this->DeleteNthNode(this->GetNumberOfNodes()-2);
    this->UpdateLines(this->GetNumberOfNodes()-2);
  }

  this->Internal->Nodes.push_back(node);

  if (this->LineInterpolator && (this->GetNumberOfNodes() > 1))
  {
    // Give the line interpolator a chance to update the node.
    int didNodeChange = this->LineInterpolator->UpdateNode(this->Renderer, this, node->WorldPosition, this->GetNumberOfNodes() - 1);

    // Give the point placer a chance to validate the updated node. If its not
    // valid, discard the LineInterpolator's change.
    if (didNodeChange && !this->PointPlacer->ValidateWorldPosition(node->WorldPosition, worldOrient))
    {
      node->WorldPosition[0] = worldPos[0];
      node->WorldPosition[1] = worldPos[1];
      node->WorldPosition[2] = worldPos[2];

      node->WorldPosition[normalCoordinateIndex(this->Orientation)] = this->Slice + this->PlaneShift;
    }
  }

  this->UpdateLines(static_cast<int>(this->Internal->Nodes.size()) - 1);
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentation::GetNodePolyData(vtkPolyData* poly)
{
  poly->Initialize();
  int count = this->GetNumberOfNodes();

  if (count == 0) return;

  vtkPoints *points = vtkPoints::New();
  vtkCellArray *lines = vtkCellArray::New();

  points->SetNumberOfPoints(count);
  vtkIdType numLines = count;

  if (this->ClosedLoop)
  {
    numLines++;
  }

  vtkIdType *lineIndices = new vtkIdType[numLines];

  int i;
  vtkIdType index = 0;
  double pos[3];

  for (i = 0; i < this->GetNumberOfNodes(); ++i)
  {
    // Add the node
    this->GetNthNodeWorldPosition(i, pos);
    points->InsertPoint(index, pos);
    lineIndices[index] = index;
    index++;
  }

  if (this->ClosedLoop)
  {
    lineIndices[index] = 0;
  }

  lines->InsertNextCell(numLines, lineIndices);
  delete[] lineIndices;

  poly->SetPoints(points);
  poly->SetLines(lines);

  points->Delete();
  lines->Delete();
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentation::AddNodeAtPositionInternal(double worldPos[3], double worldOrient[9], int displayPos[2])
{
  double dispPos[2];
  dispPos[0] = static_cast<double>(displayPos[0]);
  dispPos[1] = static_cast<double>(displayPos[1]);
  this->AddNodeAtPositionInternal(worldPos, worldOrient, dispPos);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::AddNodeAtWorldPosition(double worldPos[3], double worldOrient[9])
{
  // Check if this is a valid location
  if (!this->PointPlacer->ValidateWorldPosition(worldPos, worldOrient)) return 0;

  double displayPos[2];
  this->GetRendererComputedDisplayPositionFromWorldPosition(worldPos, worldOrient, displayPos);
  this->AddNodeAtPositionInternal(worldPos, worldOrient, displayPos);

  return 1;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::AddNodeAtWorldPosition(double x, double y, double z)
{
  double worldPos[3] = { x, y, z };
  return this->AddNodeAtWorldPosition(worldPos);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::AddNodeAtWorldPosition(double worldPos[3])
{
  // Check if this is a valid location
  if (!this->PointPlacer->ValidateWorldPosition(worldPos)) return 0;

  double worldOrient[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };

  double displayPos[2];
  this->GetRendererComputedDisplayPositionFromWorldPosition(worldPos, worldOrient, displayPos);
  this->AddNodeAtPositionInternal(worldPos, worldOrient, displayPos);

  return 1;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::AddNodeAtDisplayPosition(double displayPos[2])
{
  double worldPos[3];
  double worldOrient[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };

  // Compute the world position from the display position based on the concrete representation's constraints
  // If this is not a valid display location return 0
  if (!this->PointPlacer->ComputeWorldPosition(this->Renderer, displayPos, worldPos, worldOrient)) return 0;

  this->AddNodeAtPositionInternal(worldPos, worldOrient, displayPos);
  return 1;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::AddNodeAtDisplayPosition(int displayPos[2])
{
  double doubleDisplayPos[2];
  doubleDisplayPos[0] = displayPos[0];
  doubleDisplayPos[1] = displayPos[1];

  return this->AddNodeAtDisplayPosition(doubleDisplayPos);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::AddNodeAtDisplayPosition(int X, int Y)
{
  double displayPos[2] = { static_cast<double>(X), static_cast<double>(Y) };
  return this->AddNodeAtDisplayPosition(displayPos);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::ActivateNode(double displayPos[2])
{
  this->BuildLocator();

  // Find closest node to this display pos that is within PixelTolerance
  double dPos[3] = { displayPos[0], displayPos[1], 0 };
  double closestDistance2 = VTK_DOUBLE_MAX;
  int closestNode = this->Locator->FindClosestPointWithinRadius(this->PixelTolerance, dPos, closestDistance2);

  if (closestNode != this->ActiveNode)
  {
    this->ActiveNode = closestNode;
    this->NeedToRender = 1;
  }

  return (this->ActiveNode >= 0);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::ActivateNode(int displayPos[2])
{
  double doubleDisplayPos[2];

  doubleDisplayPos[0] = displayPos[0];
  doubleDisplayPos[1] = displayPos[1];
  return this->ActivateNode(doubleDisplayPos);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::ActivateNode(int X, int Y)
{
  double doubleDisplayPos[2];

  doubleDisplayPos[0] = X;
  doubleDisplayPos[1] = Y;
  return this->ActivateNode(doubleDisplayPos);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::SetActiveNodeToWorldPosition(double worldPos[3], double worldOrient[9])
{
  if ((this->ActiveNode < 0) || (static_cast<unsigned int>(this->ActiveNode) >= this->Internal->Nodes.size())) return 0;

  // Check if this is a valid location
  if (!this->PointPlacer->ValidateWorldPosition(worldPos, worldOrient)) return 0;

  this->SetNthNodeWorldPositionInternal(this->ActiveNode, worldPos, worldOrient);
  return 1;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::SetActiveNodeToWorldPosition(double worldPos[3])
{
  if ((this->ActiveNode < 0) || (static_cast<unsigned int>(this->ActiveNode) >= this->Internal->Nodes.size())) return 0;

  // Check if this is a valid location
  if (!this->PointPlacer->ValidateWorldPosition(worldPos)) return 0;

  double worldOrient[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };

  this->SetNthNodeWorldPositionInternal(this->ActiveNode, worldPos, worldOrient);
  return 1;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::SetActiveNodeToDisplayPosition(double displayPos[2])
{
  if ((this->ActiveNode < 0) || static_cast<unsigned int>(this->ActiveNode) >= this->Internal->Nodes.size()) return 0;

  double worldPos[3];
  double worldOrient[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };

  // Compute the world position from the display position
  // based on the concrete representation's constraints
  // If this is not a valid display location return 0
  if (!this->PointPlacer->ComputeWorldPosition(this->Renderer, displayPos, worldPos, worldOrient)) return 0;

  this->SetNthNodeWorldPositionInternal(this->ActiveNode, worldPos, worldOrient);
  return 1;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::SetActiveNodeToDisplayPosition(int displayPos[2])
{
  double doubleDisplayPos[2];
  doubleDisplayPos[0] = displayPos[0];
  doubleDisplayPos[1] = displayPos[1];
  return this->SetActiveNodeToDisplayPosition(doubleDisplayPos);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::SetActiveNodeToDisplayPosition(int X, int Y)
{
  double doubleDisplayPos[2];
  doubleDisplayPos[0] = X;
  doubleDisplayPos[1] = Y;

  return this->SetActiveNodeToDisplayPosition(doubleDisplayPos);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::ToggleActiveNodeSelected()
{
  if ((this->ActiveNode < 0) || (static_cast<unsigned int>(this->ActiveNode) >= this->Internal->Nodes.size()))
  {
    // Failed to toggle the value
    return 0;
  }

  this->Internal->Nodes[this->ActiveNode]->Selected = this->Internal->Nodes[this->ActiveNode]->Selected ? 0 : 1;
  this->NeedToRender = 1;
  this->Modified();
  return 1;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::GetNthNodeSelected(int n)
{
  if ((n < 0) || (static_cast<unsigned int>(n) >= this->Internal->Nodes.size()))
  {
    // This case is considered not Selected.
    return 0;
  }

  return this->Internal->Nodes[n]->Selected;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::SetNthNodeSelected(int n)
{
  if ((n < 0) || (static_cast<unsigned int>(n) >= this->Internal->Nodes.size()))
  {
    // Failed.
    return 0;
  }

  int val = n > 0 ? 1 : 0;

  if (this->Internal->Nodes[n]->Selected != val)
  {
    this->Internal->Nodes[n]->Selected = val;
    this->NeedToRender = 1;
    this->Modified();
  }

  return 1;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::GetActiveNodeSelected()
{
  return this->GetNthNodeSelected(this->ActiveNode);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::GetActiveNodeWorldPosition(double pos[3])
{
  return this->GetNthNodeWorldPosition(this->ActiveNode, pos);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::GetActiveNodeWorldOrientation(double orient[9])
{
  return this->GetNthNodeWorldOrientation(this->ActiveNode, orient);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::GetActiveNodeDisplayPosition(double pos[2])
{
  return this->GetNthNodeDisplayPosition(this->ActiveNode, pos);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::GetNumberOfNodes()
{
  return static_cast<int>(this->Internal->Nodes.size());
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::GetNumberOfIntermediatePoints(int n)
{
  if ((n < 0) || (static_cast<unsigned int>(n) >= this->Internal->Nodes.size()))
    return 0;

  return static_cast<int>(this->Internal->Nodes[n]->Points.size());
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::GetIntermediatePointWorldPosition(int n, int idx, double point[3])
{
  if ((n < 0) || (static_cast<unsigned int>(n) >= this->Internal->Nodes.size())) return 0;

  if ((idx < 0) || (static_cast<unsigned int>(idx) >= this->Internal->Nodes[n]->Points.size()))
    return 0;

  point[0] = this->Internal->Nodes[n]->Points[idx]->WorldPosition[0];
  point[1] = this->Internal->Nodes[n]->Points[idx]->WorldPosition[1];
  point[2] = this->Internal->Nodes[n]->Points[idx]->WorldPosition[2];

  return 1;
}

//----------------------------------------------------------------------------
// The display position for a given world position must be re-computed
// from the world positions... It should not be queried from the renderer
// whose camera position may have changed
int vtkPlaneContourRepresentation::GetNthNodeDisplayPosition(int n, double displayPos[2])
{
  if ((n < 0) || (static_cast<unsigned int>(n) >= this->Internal->Nodes.size())) return 0;

  double pos[4];
  pos[0] = this->Internal->Nodes[n]->WorldPosition[0];
  pos[1] = this->Internal->Nodes[n]->WorldPosition[1];
  pos[2] = this->Internal->Nodes[n]->WorldPosition[2];
  pos[3] = 1.0;

  this->Renderer->SetWorldPoint(pos);
  this->Renderer->WorldToDisplay();
  this->Renderer->GetDisplayPoint(pos);

  displayPos[0] = pos[0];
  displayPos[1] = pos[1];
  return 1;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::GetNthNodeWorldPosition(int n, double worldPos[3])
{
  if ((n < 0) || (static_cast<unsigned int>(n) >= this->Internal->Nodes.size())) return 0;

  worldPos[0] = this->Internal->Nodes[n]->WorldPosition[0];
  worldPos[1] = this->Internal->Nodes[n]->WorldPosition[1];
  worldPos[2] = this->Internal->Nodes[n]->WorldPosition[2];

  worldPos[normalCoordinateIndex(this->Orientation)] = this->Slice + this->PlaneShift;

  return 1;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::GetNthNodeWorldOrientation(int n, double worldOrient[9])
{
  if ((n < 0) || (static_cast<unsigned int>(n) >= this->Internal->Nodes.size())) return 0;

  memcpy(worldOrient, this->Internal->Nodes[n]->WorldOrientation, 9 * sizeof(double));
  return 1;
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentation::SetNthNodeWorldPositionInternal(int n, double worldPos[3], double worldOrient[9])
{
  this->Internal->Nodes[n]->WorldPosition[0] = worldPos[0];
  this->Internal->Nodes[n]->WorldPosition[1] = worldPos[1];
  this->Internal->Nodes[n]->WorldPosition[2] = worldPos[2];

  this->Internal->Nodes[n]->WorldPosition[normalCoordinateIndex(this->Orientation)] = this->Slice + this->PlaneShift;

  this->GetRendererComputedDisplayPositionFromWorldPosition(worldPos, worldOrient, this->Internal->Nodes[n]->NormalizedDisplayPosition);
  this->Renderer->DisplayToNormalizedDisplay(this->Internal->Nodes[n]->NormalizedDisplayPosition[0], this->Internal->Nodes[n]->NormalizedDisplayPosition[1]);

  memcpy(this->Internal->Nodes[n]->WorldOrientation, worldOrient, 9 * sizeof(double));

  this->UpdateLines(n);
  this->NeedToRender = 1;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::SetNthNodeWorldPosition(int n, double worldPos[3], double worldOrient[9])
{
  if ((n < 0) || (static_cast<unsigned int>(n) >= this->Internal->Nodes.size())) return 0;

  // Check if this is a valid location
  if (!this->PointPlacer->ValidateWorldPosition(worldPos, worldOrient)) return 0;

  this->SetNthNodeWorldPositionInternal(n, worldPos, worldOrient);
  return 1;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::SetNthNodeWorldPosition(int n, double worldPos[3])
{
  if ((n < 0) || (static_cast<unsigned int>(n) >= this->Internal->Nodes.size())) return 0;

  // Check if this is a valid location
  if (!this->PointPlacer->ValidateWorldPosition(worldPos)) return 0;

  double worldOrient[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };

  this->SetNthNodeWorldPositionInternal(n, worldPos, worldOrient);
  return 1;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::SetNthNodeDisplayPosition(int n, double displayPos[2])
{
  double worldPos[3];
  double worldOrient[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };

  // Compute the world position from the display position
  // based on the concrete representation's constraints
  // If this is not a valid display location return 0
  if (!this->PointPlacer->ComputeWorldPosition(this->Renderer, displayPos, worldPos, worldOrient)) return 0;

  return this->SetNthNodeWorldPosition(n, worldPos, worldOrient);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::SetNthNodeDisplayPosition(int n, int displayPos[2])
{
  double doubleDisplayPos[2];
  doubleDisplayPos[0] = displayPos[0];
  doubleDisplayPos[1] = displayPos[1];

  return this->SetNthNodeDisplayPosition(n, doubleDisplayPos);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::SetNthNodeDisplayPosition(int n, int X, int Y)
{
  double doubleDisplayPos[2];
  doubleDisplayPos[0] = X;
  doubleDisplayPos[1] = Y;

  return this->SetNthNodeDisplayPosition(n, doubleDisplayPos);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::FindClosestPointOnContour(int X, int Y, double closestWorldPos[3], int *idx)
{
  // Make a line out of this viewing ray
  double p1[4], p2[4], *p3 = NULL, *p4 = NULL;

  double tmp1[4], tmp2[4];
  tmp1[0] = X;
  tmp1[1] = Y;
  tmp1[2] = 0.0;

  this->Renderer->SetDisplayPoint(tmp1);
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(p1);

  tmp1[2] = 1.0;
  this->Renderer->SetDisplayPoint(tmp1);
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(p2);

  double closestDistance2 = VTK_DOUBLE_MAX;
  int closestNode = 0;

  // compute a world tolerance based on pixel tolerance on the focal plane
  double fp[4];
  this->Renderer->GetActiveCamera()->GetFocalPoint(fp);
  fp[3] = 1.0;
  this->Renderer->SetWorldPoint(fp);
  this->Renderer->WorldToDisplay();
  this->Renderer->GetDisplayPoint(tmp1);

  tmp1[0] = 0;
  tmp1[1] = 0;
  this->Renderer->SetDisplayPoint(tmp1);
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(tmp2);

  tmp1[0] = this->PixelTolerance;
  this->Renderer->SetDisplayPoint(tmp1);
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(tmp1);

  double wt2 = vtkMath::Distance2BetweenPoints(tmp1, tmp2);

  // Now loop through all lines and look for closest one within tolerance
  for (unsigned int i = 0; i < this->Internal->Nodes.size(); i++)
  {
    for (unsigned int j = 0; j <= this->Internal->Nodes[i]->Points.size(); j++)
    {
      if (j == 0)
      {
        p3 = this->Internal->Nodes[i]->WorldPosition;
        if (this->Internal->Nodes[i]->Points.size())
          p4 = this->Internal->Nodes[i]->Points[j]->WorldPosition;
        else
        {
          if (i < this->Internal->Nodes.size() - 1)
            p4 = this->Internal->Nodes[i + 1]->WorldPosition;
          else
          {
            if (this->ClosedLoop)
              p4 = this->Internal->Nodes[0]->WorldPosition;
          }
        }
      }
      else
        if (j == this->Internal->Nodes[i]->Points.size())
        {
          p3 = this->Internal->Nodes[i]->Points[j - 1]->WorldPosition;
          if (i < this->Internal->Nodes.size() - 1)
            p4 = this->Internal->Nodes[i + 1]->WorldPosition;
          else
            if (this->ClosedLoop)
              p4 = this->Internal->Nodes[0]->WorldPosition;
            else
            {
              // Shouldn't be able to get here (only if we don't have
              // a closed loop but we do have intermediate points after
              // the last node - contradictary conditions)
              continue;
            }
        }
        else
        {
          p3 = this->Internal->Nodes[i]->Points[j - 1]->WorldPosition;
          p4 = this->Internal->Nodes[i]->Points[j]->WorldPosition;
        }

      // Now we have the four points - check closest intersection
      double u, v;

      if (vtkLine::Intersection(p1, p2, p3, p4, u, v))
      {
        double p5[3], p6[3];
        p5[0] = p1[0] + u * (p2[0] - p1[0]);
        p5[1] = p1[1] + u * (p2[1] - p1[1]);
        p5[2] = p1[2] + u * (p2[2] - p1[2]);

        p6[0] = p3[0] + v * (p4[0] - p3[0]);
        p6[1] = p3[1] + v * (p4[1] - p3[1]);
        p6[2] = p3[2] + v * (p4[2] - p3[2]);

        double d = vtkMath::Distance2BetweenPoints(p5, p6);

        if ((d < wt2) && (d < closestDistance2))
        {
          closestWorldPos[0] = p6[0];
          closestWorldPos[1] = p6[1];
          closestWorldPos[2] = p6[2];
          closestDistance2 = d;
          closestNode = static_cast<int>(i);
        }
      }
      else
      {
        double d = vtkLine::DistanceToLine(p3, p1, p2);
        if ((d < wt2) && (d < closestDistance2))
        {
          closestWorldPos[0] = p3[0];
          closestWorldPos[1] = p3[1];
          closestWorldPos[2] = p3[2];
          closestDistance2 = d;
          closestNode = static_cast<int>(i);
        }

        d = vtkLine::DistanceToLine(p4, p1, p2);
        if (d < wt2 && d < closestDistance2)
        {
          closestWorldPos[0] = p4[0];
          closestWorldPos[1] = p4[1];
          closestWorldPos[2] = p4[2];
          closestDistance2 = d;
          closestNode = static_cast<int>(i);
        }
      }
    }
  }

  if (closestDistance2 < VTK_DOUBLE_MAX)
  {
    if (closestNode < this->GetNumberOfNodes() - 1)
    {
      *idx = closestNode + 1;
      return 1;
    }
    else if (this->ClosedLoop)
    {
      *idx = 0;
      return 1;
    }
  }
  return 0;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::AddNodeOnContour(int X, int Y)
{
  int idx;

  double worldPos[3];
  double worldOrient[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };

  // Compute the world position from the display position based on the concrete representation's constraints
  // If this is not a valid display location return 0
  double displayPos[2];
  displayPos[0] = X;
  displayPos[1] = Y;
  if (!this->PointPlacer->ComputeWorldPosition(this->Renderer, displayPos, worldPos, worldOrient))
    return 0;

  double pos[3];
  if (!this->FindClosestPointOnContour(X, Y, pos, &idx))
    return 0;

  if (!this->PointPlacer->ComputeWorldPosition(this->Renderer, displayPos, pos, worldPos, worldOrient))
    return 0;

  // Add a new point at this position
  vtkPlaneContourRepresentationNode *node = new vtkPlaneContourRepresentationNode;
  node->WorldPosition[0] = worldPos[0];
  node->WorldPosition[1] = worldPos[1];
  node->WorldPosition[2] = worldPos[2];

  node->WorldPosition[normalCoordinateIndex(this->Orientation)] = this->Slice + this->PlaneShift;

  node->Selected = 0;

  this->GetRendererComputedDisplayPositionFromWorldPosition(worldPos, worldOrient, node->NormalizedDisplayPosition);
  this->Renderer->DisplayToNormalizedDisplay(node->NormalizedDisplayPosition[0], node->NormalizedDisplayPosition[1]);

  memcpy(node->WorldOrientation, worldOrient, 9 * sizeof(double));

  this->Internal->Nodes.insert(this->Internal->Nodes.begin() + idx, node);
  this->UpdateLines(idx);
  this->NeedToRender = 1;

  return 1;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::DeleteNthNode(int n)
{
  if ((n < 0) || (static_cast<unsigned int>(n) >= this->Internal->Nodes.size())) return 0;

  for (unsigned int j = 0; j < this->Internal->Nodes[n]->Points.size(); j++)
  {
    delete this->Internal->Nodes[n]->Points[j];
  }

  this->Internal->Nodes[n]->Points.clear();
  delete this->Internal->Nodes[n];
  this->Internal->Nodes.erase(this->Internal->Nodes.begin() + n);

  if (n)
  {
    this->UpdateLines(n - 1);
  }
  else
  {
    this->UpdateLines(this->GetNumberOfNodes() - 1);
  }

  this->NeedToRender = 1;
  return 1;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::DeleteActiveNode()
{
  return this->DeleteNthNode(this->ActiveNode);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::DeleteLastNode()
{
  return this->DeleteNthNode(static_cast<int>(this->Internal->Nodes.size()) - 1);
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentation::SetClosedLoop(int val)
{
  if (this->ClosedLoop != val)
  {
    this->ClosedLoop = val;
    this->UpdateLines(this->GetNumberOfNodes() - 1);
    this->NeedToRender = 1;
    this->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentation::UpdateLines(int index)
{
  int indices[2];

  if (this->LineInterpolator)
  {
    vtkIntArray *arr = vtkIntArray::New();
    this->LineInterpolator->GetSpan(index, arr, this);

    int nNodes = arr->GetNumberOfTuples();
    for (int i = 0; i < nNodes; i++)
    {
      arr->GetTupleValue(i, indices);
      this->UpdateLine(indices[0], indices[1]);
    }
    arr->Delete();
  }

  // A check to make sure that we have no line segments in
  // the last node if the loop is not closed
  if (!this->ClosedLoop && this->GetNumberOfNodes() > 0)
  {
    int idx = static_cast<int>(this->Internal->Nodes.size()) - 1;
    for (unsigned int j = 0; j < this->Internal->Nodes[idx]->Points.size(); j++)
    {
      delete this->Internal->Nodes[idx]->Points[j];
    }

    this->Internal->Nodes[idx]->Points.clear();
  }

  this->BuildLines();
  this->RebuildLocator = true;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::AddIntermediatePointWorldPosition(int n, double pos[3])
{
  if ((n < 0) || (static_cast<unsigned int>(n) >= this->Internal->Nodes.size())) return 0;

  vtkPlaneContourRepresentationPoint *point = new vtkPlaneContourRepresentationPoint;
  point->WorldPosition[0] = pos[0];
  point->WorldPosition[1] = pos[1];
  point->WorldPosition[2] = pos[2];

  point->WorldPosition[normalCoordinateIndex(this->Orientation)] = this->Slice + this->PlaneShift;

  double worldOrient[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };

  this->GetRendererComputedDisplayPositionFromWorldPosition(pos, worldOrient, point->NormalizedDisplayPosition);
  this->Renderer->DisplayToNormalizedDisplay(point->NormalizedDisplayPosition[0], point->NormalizedDisplayPosition[1]);

  this->Internal->Nodes[n]->Points.push_back(point);
  return 1;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::GetNthNodeSlope(int n, double slope[3])
{
  if ((n < 0) || (static_cast<unsigned int>(n) >= this->Internal->Nodes.size()))
    return 0;

  int idx1, idx2;

  if ((n == 0) && !this->ClosedLoop)
  {
    idx1 = 0;
    idx2 = 1;
  }
  else
    if ((n == this->GetNumberOfNodes() - 1) && !this->ClosedLoop)
    {
      idx1 = this->GetNumberOfNodes() - 2;
      idx2 = idx1 + 1;
    }
    else
    {
      idx1 = n - 1;
      idx2 = n + 1;

      if (idx1 < 0)
      {
        idx1 += this->GetNumberOfNodes();
      }

      if (idx2 >= this->GetNumberOfNodes())
      {
        idx2 -= this->GetNumberOfNodes();
      }
    }

  slope[0] = this->Internal->Nodes[idx2]->WorldPosition[0] - this->Internal->Nodes[idx1]->WorldPosition[0];
  slope[1] = this->Internal->Nodes[idx2]->WorldPosition[1] - this->Internal->Nodes[idx1]->WorldPosition[1];
  slope[2] = this->Internal->Nodes[idx2]->WorldPosition[2] - this->Internal->Nodes[idx1]->WorldPosition[2];

  vtkMath::Normalize(slope);
  return 1;
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentation::UpdateLine(int idx1, int idx2)
{
  if (!this->LineInterpolator) return;

  // Clear all the points at idx1
  for (unsigned int j = 0; j < this->Internal->Nodes[idx1]->Points.size(); j++)
  {
    delete this->Internal->Nodes[idx1]->Points[j];
  }

  this->Internal->Nodes[idx1]->Points.clear();
  this->LineInterpolator->InterpolateLine(this->Renderer, this, idx1, idx2);
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::ComputeInteractionState(int vtkNotUsed(X), int vtkNotUsed(Y), int vtkNotUsed(modified))
{
  return this->InteractionState;
}

//----------------------------------------------------------------------------
int vtkPlaneContourRepresentation::UpdateContour()
{
  this->PointPlacer->UpdateInternalState();

  //even if just the camera has moved we need to mark the locator as needing to be rebuilt
  if (this->Locator->GetMTime() < this->Renderer->GetActiveCamera()->GetMTime())
  {
    this->RebuildLocator = true;
  }

  if (this->ContourBuildTime > this->PointPlacer->GetMTime()) return 0;   // Contour does not need to be rebuilt

  unsigned int i;
  for (i = 0; i < this->Internal->Nodes.size(); i++)
  {
    this->PointPlacer->UpdateWorldPosition(this->Renderer, this->Internal->Nodes[i]->WorldPosition, this->Internal->Nodes[i]->WorldOrientation);
  }

  for (i = 0; (i + 1) < this->Internal->Nodes.size(); i++)
  {
    this->UpdateLine(i, i + 1);
  }

  if (this->ClosedLoop)
  {
    this->UpdateLine(static_cast<int>(this->Internal->Nodes.size()) - 1, 0);
  }

  this->BuildLines();
  this->RebuildLocator = true;

  this->ContourBuildTime.Modified();

  return 1;
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentation::GetRendererComputedDisplayPositionFromWorldPosition(double worldPos[3], double worldOrient[9], int displayPos[2])
{
  double dispPos[2];
  dispPos[0] = static_cast<double>(displayPos[0]);
  dispPos[1] = static_cast<double>(displayPos[1]);
  this->GetRendererComputedDisplayPositionFromWorldPosition(worldPos, worldOrient, dispPos);
  displayPos[0] = static_cast<int>(dispPos[0]);
  displayPos[1] = static_cast<int>(dispPos[1]);
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentation::GetRendererComputedDisplayPositionFromWorldPosition(double worldPos[3], double * vtkNotUsed(worldOrient[9]), double displayPos[2])
{
  double pos[4];
  pos[0] = worldPos[0];
  pos[1] = worldPos[1];
  pos[2] = worldPos[2];
  pos[3] = 1.0;

  this->Renderer->SetWorldPoint(pos);
  this->Renderer->WorldToDisplay();
  this->Renderer->GetDisplayPoint(pos);

  displayPos[0] = pos[0];
  displayPos[1] = pos[1];
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentation::Initialize(vtkPolyData * pd)
{
  vtkPoints *points = pd->GetPoints();
  vtkIdType nPoints = points->GetNumberOfPoints();
  if (nPoints <= 0) return; // Yeah right.. build from nothing!

  // Clear all existing nodes.
  for (unsigned int i = 0; i < this->Internal->Nodes.size(); i++)
  {
    for (unsigned int j = 0; j < this->Internal->Nodes[i]->Points.size(); j++)
      delete this->Internal->Nodes[i]->Points[j];

    this->Internal->Nodes[i]->Points.clear();
    delete this->Internal->Nodes[i];
  }
  this->Internal->Nodes.clear();

  vtkPolyData *tmpPoints = vtkPolyData::New();
  tmpPoints->DeepCopy(pd);
  this->Locator->SetDataSet(tmpPoints);
  tmpPoints->Delete();

  //reserver space in memory to speed up vector push_back
  this->Internal->Nodes.reserve(nPoints);

  vtkIdList *pointIds = pd->GetCell(0)->GetPointIds();

  // Get the worldOrient from the point placer
  double ref[3], displayPos[2], worldPos[3];
  double worldOrient[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
  ref[0] = 0.0;
  ref[1] = 0.0;
  ref[2] = 0.0;
  displayPos[0] = 0.0;
  displayPos[1] = 0.0;
  if (!this->PointPlacer->ComputeWorldPosition(this->Renderer, displayPos, ref, worldPos, worldOrient))
    return;

  // Add nodes without calling rebuild lines to improve performance dramatically(~15x) on large datasets
  double *pos;
  for (vtkIdType i = 0; i < nPoints; i++)
  {
    pos = points->GetPoint(i);
    this->GetRendererComputedDisplayPositionFromWorldPosition(pos, worldOrient, displayPos);

    // Add a new point at this position
    vtkPlaneContourRepresentationNode *node = new vtkPlaneContourRepresentationNode;
    node->WorldPosition[0] = pos[0];
    node->WorldPosition[1] = pos[1];
    node->WorldPosition[2] = pos[2];
    node->Selected = 0;

    node->WorldPosition[normalCoordinateIndex(this->Orientation)] = this->Slice + this->PlaneShift;

    node->NormalizedDisplayPosition[0] = displayPos[0];
    node->NormalizedDisplayPosition[1] = displayPos[1];

    this->Renderer->DisplayToNormalizedDisplay(node->NormalizedDisplayPosition[0], node->NormalizedDisplayPosition[1]);

    memcpy(node->WorldOrientation, worldOrient, 9 * sizeof(double));

    this->Internal->Nodes.push_back(node);

    if (this->LineInterpolator && this->GetNumberOfNodes() > 1)
    {
      // Give the line interpolator a chance to update the node.
      int didNodeChange = this->LineInterpolator->UpdateNode(this->Renderer, this, node->WorldPosition, this->GetNumberOfNodes() - 1);

      // Give the point placer a chance to validate the updated node. If its not
      // valid, discard the LineInterpolator's change.
      if (didNodeChange && !this->PointPlacer->ValidateWorldPosition(node->WorldPosition, worldOrient))
      {
        node->WorldPosition[0] = worldPos[0];
        node->WorldPosition[1] = worldPos[1];
        node->WorldPosition[2] = worldPos[2];

        node->WorldPosition[normalCoordinateIndex(this->Orientation)] = this->Slice + this->PlaneShift;
      }
    }
  }

  if (pointIds->GetNumberOfIds() > nPoints)
  {
    this->ClosedLoopOn();
  }

  // Update the contour representation from the nodes using the line interpolator
  for (vtkIdType i = 1; i <= nPoints; ++i)
  {
    this->UpdateLines(i);
  }

  this->BuildRepresentation();

  // Show the contour.
  this->VisibilityOn();
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentation::BuildLocator()
{
  if (!this->RebuildLocator && !this->NeedToRender) return; // rebuild if rebuildLocator or needtorender are true

  vtkIdType size = (vtkIdType) this->Internal->Nodes.size();
  vtkPoints *points = vtkPoints::New();
  points->SetNumberOfPoints(size);

  //setup up the matrixes needed to transform
  //world to display. We are going to do this manually
  // as calling the renderer will create a new matrix for each call
  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
  matrix->DeepCopy(this->Renderer->GetActiveCamera()->GetCompositeProjectionTransformMatrix(this->Renderer->GetTiledAspectRatio(), 0, 1));

  //viewport info
  double viewPortRatio[2];
  int sizex, sizey;

  /* get physical window dimensions */
  if (this->Renderer->GetVTKWindow())
  {
    double *viewPort = this->Renderer->GetViewport();
    sizex = this->Renderer->GetVTKWindow()->GetSize()[0];
    sizey = this->Renderer->GetVTKWindow()->GetSize()[1];
    viewPortRatio[0] = (sizex * (viewPort[2] - viewPort[0])) / 2.0 + sizex * viewPort[0];
    viewPortRatio[1] = (sizey * (viewPort[3] - viewPort[1])) / 2.0 + sizey * viewPort[1];
  }
  else
  {
    return; //can't compute the locator without a vtk window
  }

  double view[4];
  double pos[3] = { 0, 0, 0 };
  double *wp;
  for (vtkIdType i = 0; i < size; ++i)
  {
    wp = this->Internal->Nodes[i]->WorldPosition;
    pos[0] = this->Internal->Nodes[i]->WorldPosition[0];
    pos[1] = this->Internal->Nodes[i]->WorldPosition[1];
    pos[2] = this->Internal->Nodes[i]->WorldPosition[2];

    //convert from world to view
    view[0] = wp[0] * matrix->Element[0][0] + wp[1] * matrix->Element[0][1] + wp[2] * matrix->Element[0][2] + matrix->Element[0][3];
    view[1] = wp[0] * matrix->Element[1][0] + wp[1] * matrix->Element[1][1] + wp[2] * matrix->Element[1][2] + matrix->Element[1][3];
    view[2] = wp[0] * matrix->Element[2][0] + wp[1] * matrix->Element[2][1] + wp[2] * matrix->Element[2][2] + matrix->Element[2][3];
    view[3] = wp[0] * matrix->Element[3][0] + wp[1] * matrix->Element[3][1] + wp[2] * matrix->Element[3][2] + matrix->Element[3][3];
    if (view[3] != 0.0)
    {
      pos[0] = view[0] / view[3];
      pos[1] = view[1] / view[3];
    }

    //now from view to display
    pos[0] = (pos[0] + 1.0) * viewPortRatio[0];
    pos[1] = (pos[1] + 1.0) * viewPortRatio[1];
    pos[2] = 0;

    points->InsertPoint(i, pos);
  }

  matrix->Delete();
  vtkPolyData *tmp = vtkPolyData::New();
  tmp->SetPoints(points);
  this->Locator->SetDataSet(tmp);
  tmp->FastDelete();
  points->FastDelete();

  //we fully updated the display locator
  this->RebuildLocator = false;
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentation::SetShowSelectedNodes(int flag)
{
  if (this->ShowSelectedNodes != flag)
  {
    this->ShowSelectedNodes = flag;
    this->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Pixel Tolerance: " << this->PixelTolerance << "\n";
  os << indent << "World Tolerance: " << this->WorldTolerance << "\n";

  os << indent << "Closed Loop: " << (this->ClosedLoop ? "On\n" : "Off\n");
  os << indent << "ShowSelectedNodes: " << this->ShowSelectedNodes << endl;
  os << indent << "Rebuild Locator: " << (this->RebuildLocator ? "On" : "Off") << endl;

  os << indent << "Current Operation: ";
  if (this->CurrentOperation == vtkPlaneContourRepresentation::Inactive)
  {
    os << "Inactive\n";
  }
  else
  {
    os << "Translate\n";
  }

  os << indent << "Line Interpolator: " << this->LineInterpolator << "\n";
  os << indent << "Point Placer: " << this->PointPlacer << "\n";
}

//----------------------------------------------------------------------------
bool vtkPlaneContourRepresentation::CheckNodesForDuplicates(int node1, int node2)
{
  int numNodes = this->GetNumberOfNodes();

  if ((numNodes < 2) || (node1 > numNodes - 1) || (node2 > numNodes - 1))
    return false;

  double p1[3];
  double p2[3];

  this->GetNthNodeWorldPosition(node1, p1);
  this->GetNthNodeWorldPosition(node2, p2);

  if ((p1[0] == p2[0]) && (p1[1] == p2[1]) && (p1[2] == p2[2]))
  {
    return true;
  }

  return false;
}

//----------------------------------------------------------------------------
bool vtkPlaneContourRepresentation::CheckAndCutContourIntersectionInFinalPoint(void)
{
  double intersection[3];
  int node;
  bool previousNode = false;

  // get a unique list of points
  RemoveDuplicatedNodes();

  int lastNode = this->GetNumberOfNodes()-1;

  // segment (lastNode-1, lastNode)
  if (this->LineIntersection(lastNode - 1, intersection, &node, &previousNode))
  {
    // delete useless nodes
    for (int j = 0; j <= node; j++)
    {
      this->DeleteNthNode(0);
    }

    if (previousNode)
    {
      this->DeleteLastNode();
      previousNode = false;
    }

    // repeat the process to detect spiral-like contours (contours with multiple intersections of the (n-2,n-1) segment)
    lastNode = this->GetNumberOfNodes()-1;
    while (this->LineIntersection(lastNode - 1, intersection, &node, &previousNode))
    {
      for (int j = 0; j <= node; j++)
      {
        this->DeleteNthNode(0);
      }

      if (previousNode)
      {
        this->DeleteLastNode();
      }

      lastNode = this->GetNumberOfNodes()-1;
    }
    this->DeleteLastNode();
    this->AddNodeAtWorldPosition(intersection);

    this->UpdateContour();
    this->NeedToRender = 1;
    RemoveDuplicatedNodes(); // maybe the intersection point is a node.
    return true;
  }

  // segment (lastNode,0)
  previousNode = false;
  if (this->LineIntersection(lastNode, intersection, &node, &previousNode))
  {
    for (int j = this->GetNumberOfNodes() - 1; j > node; j--)
    {
      this->DeleteLastNode();
    }

    if (previousNode)
    {
      this->DeleteLastNode();
      previousNode = false;
    }

    // repeat the process to detect spiral-like contours (contours with multiple intersections of the (n-1,0) segment)
    lastNode = this->GetNumberOfNodes()-1;
    while (this->LineIntersection(lastNode, intersection, &node, &previousNode))
    {
      for (int j = this->GetNumberOfNodes() - 1; j > node; j--)
      {
        this->DeleteLastNode();
      }

      if (previousNode)
      {
        this->DeleteLastNode();
      }

      lastNode = this->GetNumberOfNodes()-1;
    }
    this->AddNodeAtWorldPosition(intersection);

    this->UpdateContour();
    this->NeedToRender = 1;
    RemoveDuplicatedNodes(); // maybe the intersection point is a node.
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
bool vtkPlaneContourRepresentation::CheckAndCutContourIntersection(void)
{
  // remove duplicated nodes except the last one (the cursor node)
  for (int i = this->GetNumberOfNodes() - 3; i >= 0; i--)
  {
    double p1[3], p2[3];
    this->GetNthNodeWorldPosition(i, p1);
    this->GetNthNodeWorldPosition(i+1, p2);

    if ((p1[0] == p2[0]) && (p1[1] == p2[1]) && (p1[2] == p2[2]))
    {
      this->DeleteNthNode(i+1);
    }
  }

  if (this->GetNumberOfNodes() < 4) return false;

  double intersection[3];
  int node;

  int lastNode = (this->CheckNodesForDuplicates(this->GetNumberOfNodes()-1, this->GetNumberOfNodes()-2) ? this->GetNumberOfNodes()-2 : this->GetNumberOfNodes()-1);

  // segment (n-1,0)
  bool previousNode = false;
  if (this->LineIntersection(lastNode-1, intersection, &node, &previousNode))
  {
    // delete useless nodes
    for (int j = 0; j <= node; j++)
    {
      this->DeleteNthNode(0);
    }

    if (previousNode)
    {
      this->DeleteLastNode();
      previousNode = false;
    }

    // repeat the process to detect spiral-like contours (contours with multiple intersections of the (n-2,n-1) segment)
    lastNode = (this->CheckNodesForDuplicates(this->GetNumberOfNodes()-1, this->GetNumberOfNodes()-2) ? this->GetNumberOfNodes()-2 : this->GetNumberOfNodes()-1);
    while (LineIntersection(lastNode-1, intersection, &node, &previousNode))
    {
      for (int j = 0; j <= node; j++)
      {
        this->DeleteNthNode(0);
      }

      if (previousNode)
      {
        this->DeleteLastNode();
      }

      lastNode = (this->CheckNodesForDuplicates(this->GetNumberOfNodes()-1, this->GetNumberOfNodes()-2) ? this->GetNumberOfNodes()-2 : this->GetNumberOfNodes()-1);
    }

    // we have an intersection so delete the cursor nodeif duplicated and the last one
    if (this->CheckNodesForDuplicates(this->GetNumberOfNodes() - 1, this->GetNumberOfNodes() - 2))
    {
      this->DeleteLastNode();
    }

    this->DeleteLastNode();

    this->AddNodeAtWorldPosition(intersection);
    this->UpdateContour();
    this->NeedToRender = 1;
    return true;
  }

  return false;
}

//----------------------------------------------------------------------------
bool vtkPlaneContourRepresentation::CheckContourIntersection(int nodeA)
{
  // must check the intersection of (nodeA-1, nodeA) and (nodeA, nodeA+1)
  int nodeB = (nodeA == 0) ? (this->GetNumberOfNodes() - 1) : (nodeA - 1);

  for (int i = 0; i < this->GetNumberOfNodes(); i++)
  {
    if (this->NodesIntersection(nodeA, i))
    {
      return true;
    }

    if (this->NodesIntersection(nodeB, i))
    {
      return true;
    }
  }

  return false;
}

//----------------------------------------------------------------------------
bool vtkPlaneContourRepresentation::LineIntersection(int n, double *intersection, int *intersectionNode, bool *previous)
{
  int node = n;
  int numNodes = this->GetNumberOfNodes() - 1;

  if ((node < 0) || (node > numNodes) || (numNodes < 3)) return false;

  // avoid using the cursor node when (n-1) is the same node as (n)
  if (this->CheckNodesForDuplicates(numNodes, numNodes - 1))
  {
    if (node == numNodes)
    {
      node--;
    }

    numNodes--;
  }

  double u, v;
  double p1[3], p2[3], p3[3], p4[3];
  int previousNode = (node == 0) ? numNodes : node - 1;
  int nextNode = (node == numNodes) ? 0 : node + 1;

  this->GetNthNodeWorldPosition(node, p1);
  this->GetNthNodeWorldPosition(nextNode, p2);

  // segments (0,1)-(1,2)- ... -(numNodes-1, numNodes);
  for (int i = 0; i < numNodes; i++)
  {
    // we already know we're colliding with those nodes
    if ((node == i) || (previousNode == i))
    {
      continue;
    }

    if (NodesIntersection(previousNode, i) && (i != previousNode-1))
    {
      this->GetNthNodeWorldPosition(previousNode, p1);
      this->GetNthNodeWorldPosition(node, p2);

      int j = i + 1 ;
      this->GetNthNodeWorldPosition(i, p3);
      this->GetNthNodeWorldPosition(j % numNodes, p4);
      while ((p3[0] == p4[0]) && (p3[1] == p4[1]) && (p3[2] == p4[2]))
      {
        j++;
        this->GetNthNodeWorldPosition(j  % numNodes, p4);
      }
      vtkLine::Intersection(p1, p2, p3, p4, u, v);
      *intersectionNode = i;
      intersection[0] = p1[0] + u * (p2[0] - p1[0]);
      intersection[1] = p1[1] + u * (p2[1] - p1[1]);
      intersection[2] = p1[2] + u * (p2[2] - p1[2]);
      *previous = true;
      return true;
    }

    if (NodesIntersection(node,i))
    {
      int j = i + 1 ;
      this->GetNthNodeWorldPosition(i, p3);
      this->GetNthNodeWorldPosition(j % numNodes, p4);
      while ((p3[0] == p4[0]) && (p3[1] == p4[1]) && (p3[2] == p4[2]))
      {
        j++;
        this->GetNthNodeWorldPosition(j  % numNodes, p4);
      }

      vtkLine::Intersection(p1, p2, p3, p4, u, v);
      *intersectionNode = i;
      intersection[0] = p1[0] + u * (p2[0] - p1[0]);
      intersection[1] = p1[1] + u * (p2[1] - p1[1]);
      intersection[2] = p1[2] + u * (p2[2] - p1[2]);
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------------------
// we orient the point where the cursor is attending to the plane is in
// (axial, coronal, sagittal), and do the same with the contour points.
// that's because we're going to compute the shooting algorithm in an
// axial coordinated plane.
bool vtkPlaneContourRepresentation::ShootingAlgorithm(int X, int Y)
{
  // is the polygon closed?
  if (!this->ClosedLoop) return false;

  double displayPos[3] = { static_cast<double>(X), static_cast<double>(Y), 0.0 };
  double point[3];
  this->Renderer->SetDisplayPoint(displayPos);
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(point);

  // from now on only coords [0] and [1] as we're working in a plane
  switch(this->Orientation)
  {
    case Plane::XY:
      break;
    case Plane::XZ:
      point[1] = point[2];
      break;
    case Plane::YZ:
      point[0] = point[1];
      point[1] = point[2];
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  double p1[3], p2[3];

  int right = 0;
  int left = 0;

  for (int i = 0; i < this->GetNumberOfNodes(); i++)
  {
    this->GetNthNodeWorldPosition(i, p1);
    int j = (i + 1) % this->GetNumberOfNodes();
    this->GetNthNodeWorldPosition(j, p2);

    switch(this->Orientation)
    {
      case Plane::XY:
        break;
      case Plane::XZ:
        p1[1] = p1[2];
        p2[1] = p2[2];
        break;
      case Plane::YZ:
        p1[0] = p1[1];
        p1[1] = p1[2];
        p2[0] = p2[1];
        p2[1] = p2[2];
        break;
      default:
        Q_ASSERT(false);
        break;
    }

    // check if the point is a vertex
    if ((point[0] == p1[0]) && (point[1] == p1[1]))
    {
      return false;
    }

    if (((p1[1] - point[1]) > 0) != ((p2[1] - point[1]) > 0))
    {
      double x = ((p1[0] - point[0]) * (double) (p2[1] - point[1]) - (p2[0] - point[0]) * (double) (p1[1] - point[1])) / (double) ((p2[1] - point[1]) - (p1[1] - point[1]));

      if (x > 0)
      {
        right++;
      }
    }

    if (((p1[1] - point[1]) < 0) != ((p2[1] - point[1]) < 0))
    {
      double x = ((p1[0] - point[0]) * (double) (p2[1] - point[1])
          - (p2[0] - point[0]) * (double) (p1[1] - point[1]))
          / (double) ((p2[1] - point[1]) - (p1[1] - point[1]));

      if (x < 0)
      {
        left++;
      }
    }
  }

  // check if the point belongs to the frontier
  if ((right % 2) != (left % 2))
  {
    return false;
  }

  // if there is an even number of intersections then the point is inside
  if ((right % 2) == 1)
  {
    return true;
  }

  return false;
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentation::RemoveDuplicatedNodes()
{
  int i = this->GetNumberOfNodes() - 2;

  while (i > 0)
  {
    double pos1[3], pos2[3];
    this->GetNthNodeWorldPosition(i, pos1);
    this->GetNthNodeWorldPosition(i + 1, pos2);

    if ((pos1[0] == pos2[0]) && (pos1[1] == pos2[1]) && (pos1[2] == pos2[2]))
    {
      this->DeleteNthNode(i + 1);
    }

    i--;
  }
}

//----------------------------------------------------------------------------
// checks intersection between segments [A,B], [C,D]
bool vtkPlaneContourRepresentation::NodesIntersection(int nodeA, int nodeC)
{
  double a[3], b[3], c[3], d[3];

  // don't want to check the obvious case, return false instead of true
  if (nodeA == nodeC) return false;

  int nodeB = (nodeA + 1) % this->GetNumberOfNodes();
  int nodeD = (nodeC + 1) % this->GetNumberOfNodes();

  this->GetNthNodeWorldPosition(nodeA, a);
  this->GetNthNodeWorldPosition(nodeB, b);
  this->GetNthNodeWorldPosition(nodeC, c);
  this->GetNthNodeWorldPosition(nodeD, d);

  // NOTE: from now on we'll only use [0] and [1] (we're working in a plane)
  switch(this->Orientation)
  {
    case Plane::XY:
      // p[0] & p[1] are the correct values
      break;
    case Plane::XZ:
      // p[0] & p[2] are the correct values
      a[1] = a[2];
      b[1] = b[2];
      c[1] = c[2];
      d[1] = d[2];
      break;
    case Plane::YZ:
      // p[1] & p[2] are the correct values
      a[0] = a[1];
      a[1] = a[2];
      b[0] = b[1];
      b[1] = b[2];
      c[0] = c[1];
      c[1] = c[2];
      d[0] = d[1];
      d[1] = d[2];
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  // determinant of the matrix whose elements are the coefficients of the parametric equations of the lines A and B
  double det = ((b[0] - a[0]) * (d[1] - c[1])) - ((d[0] - c[0]) * (b[1] - a[1]));

  if (nodeC == nodeB) // check only node A with segment [C,D]
  {
    if (0 != det)
    {
      return false;
    }

    if ((a[0] == b[0]) && (c[0] == d[0])) // special case: [A,B] and [C,D] are vertical line segments.
    {
      if (a[0] == c[0]) // true if [A,B] and [C,D] are in the same vertical line.
      {
        if (c[1] < d[1]) // check if A is in the bounds of [C,D] (y coord).
        {
          return ((c[1] <= a[1]) && (a[1] <= d[1]));
        }
        else
        {
          return ((d[1] <= a[1]) && (a[1] <= c[1]));
        }
      }
      else
      {
        return false;
      }
    }

    // for parallel lines to overlap, they need the same y-intercept. integer relations to y-intercepts of [A,B] and [C,D] are as follows.
    double ab_offset = ((b[0] - a[0]) * a[1] - (b[1] - a[1]) * a[0]) * (d[0] - c[0]);
    double cd_offset = ((d[0] - c[0]) * c[1] - (d[1] - c[1]) * c[0]) * (b[0] - a[0]);

    if (cd_offset == ab_offset) // true only when [A,B] y_intercept == [C,D] y_intercept.
    {
      if (c[0] < d[0]) // check if A is in the bounds of [C,D] (x coord)
      {
        return ((c[0] <= a[0]) && (a[0] <= d[0]));
      }
      else
      {
        return ((d[0] <= a[0]) && (a[0] <= c[0]));
      }
    }
    else
    {
      return false; // different y_intercepts; no intersection.
    }
  }

  if (nodeA == nodeD) // check only node B with segment [C,D]
  {
    if (0 != det)
    {
      return false;
    }

    if ((a[0] == b[0]) && (c[0] == d[0])) // special case: [A,B] and [C,D] are vertical line segments.
    {
      if (a[0] == c[0]) // true if [A,B] and [C,D] are in the same vertical line.
      {
        if (c[1] < d[1]) // check if B is in the bounds of [C,D] (y coord).
        {
          return ((c[1] <= b[1]) && (b[1] <= d[1]));
        }
        else
        {
          return ((d[1] <= b[1]) && (b[1] <= c[1]));
        }
      }
      else
      {
        return false;
      }
    }

    // for parallel lines to overlap, they need the same y-intercept. integer relations to y-intercepts of A and B are as follows.
    double ab_offset = ((b[0] - a[0]) * a[1] - (b[1] - a[1]) * a[0]) * (d[0] - c[0]);
    double cd_offset = ((d[0] - c[0]) * c[1] - (d[1] - c[1]) * c[0]) * (b[0] - a[0]);

    if (cd_offset == ab_offset) // true only when [A,B] y_intercept == [C,D] y_intercept.
    {
      if (c[0] < d[0]) // check B is in the bounds of [C,D] (x coord)
      {
        return ((c[0] <= b[0]) && (b[0] <= d[0]));
      }
      else
      {
        return ((d[0] <= b[0]) && (b[0] <= c[0]));
      }
    }
    else
    {
      return false; // different y_intercepts; no intersection.
    }
  }

  // [A,B] and [C,D] are disjoint segments
  if (det == 0) // [A,B] and [C,D] are parallel if det == 0
  {
    if ((a[0] == b[0]) && (c[0] == d[0]))
    {
      if (a[0] == c[0]) // true if [A,B] and [C,D] are in the same vertical line.
      {
        if (c[1] < d[1]) // check if bounds of [A,B] are in the bounds of [C,D]
        {
          return ((c[1] <= a[1]) && (a[1] <= d[1])) || ((c[1] <= b[1]) && (b[1] <= d[1]));
        }
        else
        {
          return ((d[1] <= a[1]) && (a[1] <= c[1])) || ((d[1] <= b[1]) && (b[1] <= c[1]));
        }
      }
      else
      {
        return false; // different vertical lines, no intersection.
      }
    }

    // for parallel lines to overlap, they need the same y-intercept. relations to y-intercepts of [A,B] and [C,D] are as follows.
    double a_offset = ((b[0] - a[0]) * a[1] - (b[1] - a[1]) * a[0]) * (d[0] - c[0]);
    double b_offset = ((d[0] - c[0]) * c[1] - (d[1] - c[1]) * c[0]) * (b[0] - a[0]);

    if (b_offset == a_offset) // true only when [A,B] y_intercept == [C,D] y_intercept.
    {
      if (c[0] < d[0]) // true when some bounds of [A,B] are in the bounds of [C,D].
      {
        return (c[0] <= a[0] && a[0] <= d[0]) || (c[0] <= b[0] && b[0] <= d[0]);
      }
      else
      {
        return (d[0] <= a[0] && a[0] <= c[0]) || (d[0] <= b[0] && b[0] <= c[0]);
      }
    }
    else
    {
      return false; // different y intercepts; no intersection.
    }
  }

  // nMitc[0] = numerator_of_M_inverse_times_c0
  // nMitc[1] = numerator_of_M_inverse_times_c1
  double nMitc[2] = { ((c[0] - a[0]) * (d[1] - c[1])) + ((c[1] - a[1]) * (c[0] - d[0])), ((c[0] - a[0]) * (a[1] - b[1])) + ((c[1] - a[1]) * (b[0] - a[0])) };

  // true if an intersection between two non-parallel lines occurs between the given segment double *s.
  return (((0 <= nMitc[0]) && (nMitc[0] <= det)) && ((0 >= nMitc[1]) && (nMitc[1] >= -det))) || (((0 >= nMitc[0]) && (nMitc[0] >= det)) && ((0 <= nMitc[1]) && (nMitc[1] <= -det)));
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentation::TranslatePoints(double *vector)
{
  double worldPos[3];
  double worldOrient[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
  for (int i = 0; i < this->GetNumberOfNodes(); i++)
  {
    this->GetNthNodeWorldPosition(i, worldPos);
    worldPos[0] += vector[0];
    worldPos[1] += vector[1];
    this->SetNthNodeWorldPosition(i, worldPos, worldOrient);
  }
}

//----------------------------------------------------------------------------
double vtkPlaneContourRepresentation::FindClosestDistanceToContour(int x, int y)
{
  double displayPos_i[3], displayPos_j[3];
  double result = 10000.0; // just use a big enough constant to boot operations
  double tempN, tempD;

  for (int i = 0; i < this->GetNumberOfNodes(); i++)
  {
    int j = (i + 1) % this->GetNumberOfNodes();

    this->GetNthNodeDisplayPosition(i, displayPos_i);
    this->GetNthNodeDisplayPosition(j, displayPos_j);

    //            (y1-y2)x + (x2-x1)y + (x1y2-x2y1)
    //dist(P,L) = ---------------------------------
    //              sqrt( (x2-x1)^2 + (y2-y1)^2 )

    tempN = ((displayPos_i[1] - displayPos_j[1]) * static_cast<double>(x))
        + ((displayPos_j[0] - displayPos_i[0]) * static_cast<double>(y))
        + ((displayPos_i[0] * displayPos_j[1]) - (displayPos_j[0] * displayPos_i[1]));

    tempD = sqrt(pow(displayPos_j[0] - displayPos_i[0], 2)
        + pow(displayPos_j[1] - displayPos_i[1], 2));

    if (tempD == 0.0)
    {
      continue;
    }

    double distance = fabs(tempN) / tempD;

    // if the distance is to a point outside the segment i,i+1, then the real distance to the segment is
    // the distance to one of the nodes
    double r = ((static_cast<double>(x) - displayPos_i[0]) * (displayPos_j[0] - displayPos_i[0])
        + (static_cast<double>(y) - displayPos_i[1]) * (displayPos_j[1] - displayPos_i[1]))
        / (tempD * tempD);

    if ((r < 0.0) || (r > 1.0))
    {

      double dist1 = fabs(sqrt(pow(displayPos_i[0] - static_cast<double>(x), 2) + pow(displayPos_i[1] - static_cast<double>(y), 2)));
      double dist2 = fabs(sqrt(pow(displayPos_j[0] - static_cast<double>(x), 2) + pow(displayPos_j[1] - static_cast<double>(y), 2)));

      if (dist1 <= dist2)
      {
        distance = dist1;
      }
      else
      {
        distance = dist2;
      }
    }

    if (distance < result)
    {
      result = distance;
    }
  }

  return result;
}

//----------------------------------------------------------------------------
void vtkPlaneContourRepresentation::SetOrientation(Plane orientation)
{
  this->Orientation = orientation;
}

//----------------------------------------------------------------------------
Plane vtkPlaneContourRepresentation::GetOrientation()
{
  return this->Orientation;
}
