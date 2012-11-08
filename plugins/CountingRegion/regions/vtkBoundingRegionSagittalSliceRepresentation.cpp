/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
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


#include "vtkBoundingRegionSagittalSliceRepresentation.h"

#include <vtkObjectFactory.h>
#include <vtkCellArray.h>
#include <vtkActor.h>

vtkStandardNewMacro(vtkBoundingRegionSagittalSliceRepresentation);

//----------------------------------------------------------------------------
void vtkBoundingRegionSagittalSliceRepresentation::SetSlice(Nm pos)
{
  Slice = pos;

  double firstSliceBounds[6];
  double lastSliceBounds[6];

  regionBounds(0, firstSliceBounds);
  int firstSlice = sliceNumber(firstSliceBounds[4] + InclusionOffset[2]);

  regionBounds(NumSlices-1, lastSliceBounds);
  int lastSlice = sliceNumber(lastSliceBounds[5] - ExclusionOffset[2]);

  bool visible = false;
  int  slice   = firstSlice;
  // cheack all visible slices
  while (!visible && slice <= lastSlice)
  {
    double sliceBounds[6];
    regionBounds(slice, sliceBounds);
    visible = sliceBounds[0] + InclusionOffset[0] <= Slice
           && Slice <= sliceBounds[1] - ExclusionOffset[0];
    slice++;
  }

  if (visible)
  {
    for(EDGE i = LEFT; i <= TOP; i = EDGE(i+1))
      this->EdgeActor[i]->SetProperty(InclusionEdgeProperty);
    for(EDGE i = RIGHT; i <= BOTTOM; i = EDGE(i+1))
      this->EdgeActor[i]->SetProperty(ExclusionEdgeProperty);

    CreateRegion();
  } else
  {
    for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
      this->EdgeActor[i]->SetProperty(InvisibleProperty);
  }
}

//----------------------------------------------------------------------------
void vtkBoundingRegionSagittalSliceRepresentation::CreateRegion()
{
//   std::cout << "Created YZ FACE" << std::endl;
  double LB[3], RB[3];
  this->Region->GetPoint(0, LB);
  this->Region->GetPoint(NumPoints-1, RB);

//   std::cout << "LB: " << LB[2] << std::endl;
//   std::cout << "RB: " << RB[2] << std::endl;
//   std::cout << "LB+Shift: " << LB[2] + InclusionOffset[hCoord]  << std::endl;
//   std::cout << "RB+Shift: " << RB[2]+ ExclusionOffset[hCoord] << std::endl;
  int UpperSlice = sliceNumber(LB[2] + InclusionOffset[hCoord]);
  int LowerSlice = sliceNumber(RB[2] - ExclusionOffset[hCoord]);
  if (UpperSlice == LowerSlice)
    UpperSlice--;
//   std::cout << "Upper Slice: " << UpperSlice << std::endl;
//   std::cout << "Lower Slice: " << LowerSlice << std::endl;

  int numRepSlices = LowerSlice - UpperSlice + 1;
  if (numRepSlices == 0)
    return;

  unsigned int numIntervals = numRepSlices - 1;
  unsigned int numVertex    = numRepSlices * 2;

  // Set of point pairs. Each pair belong to the same slice .
  // First pair belongs to the left edge
  // Last pair belongs to the right edge
  // Odd indexed points belong to the top edge
  // Even indexed points belong to the bottom edge
  /*
   *   1\ /5-7\ /11
   *   | 3     9 |
   *   |         |
   *   | 2-4     |
   *   0/   \6-8-10
   */
  this->Vertex->SetNumberOfPoints(numVertex);

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgePolyData[i]->GetLines()->Reset();

  this->EdgePolyData[LEFT]->GetLines()->Allocate(
    this->EdgePolyData[LEFT]->GetLines()->EstimateSize(1,2));
  this->EdgePolyData[LEFT]->GetLines()->InsertNextCell(2);
  this->EdgePolyData[LEFT]->GetLines()->InsertCellPoint(0);
  this->EdgePolyData[LEFT]->GetLines()->InsertCellPoint(1);

  this->EdgePolyData[TOP]->GetLines()->Allocate(
    this->EdgePolyData[TOP]->GetLines()->EstimateSize(numIntervals,2));
  for(unsigned int interval=0; interval < numIntervals; interval++)
  {
    this->EdgePolyData[TOP]->GetLines()->InsertNextCell(2);
    this->EdgePolyData[TOP]->GetLines()->InsertCellPoint(2*interval+1);
    this->EdgePolyData[TOP]->GetLines()->InsertCellPoint(2*interval+3);
  }

  this->EdgePolyData[RIGHT]->GetLines()->Allocate(
    this->EdgePolyData[RIGHT]->GetLines()->EstimateSize(1,2));
  this->EdgePolyData[RIGHT]->GetLines()->InsertNextCell(2);
  this->EdgePolyData[RIGHT]->GetLines()->InsertCellPoint(numVertex-2);
  this->EdgePolyData[RIGHT]->GetLines()->InsertCellPoint(numVertex-1);

  this->EdgePolyData[BOTTOM]->GetLines()->Allocate(
    this->EdgePolyData[BOTTOM]->GetLines()->EstimateSize(numIntervals,2));
  for(unsigned int interval=0; interval < numIntervals; interval++)
  {
    this->EdgePolyData[BOTTOM]->GetLines()->InsertNextCell(2);
    this->EdgePolyData[BOTTOM]->GetLines()->InsertCellPoint(2*interval);
    this->EdgePolyData[BOTTOM]->GetLines()->InsertCellPoint(2*interval+2);
  }

  double point[3];
  /// Loop over slices and create Top/Bottom Edges
  for (int slice=UpperSlice; slice <= LowerSlice; slice++)
  {
    int interval = slice - UpperSlice;
    // Bottom
    Region->GetPoint(slice*4+0,point);
    point[0] = 0.1;
    point[1] -= ExclusionOffset[vCoord];
    if (slice == 0)
      point[2] += InclusionOffset[hCoord];
    else if (slice == NumSlices - 1)
      point[2] -= ExclusionOffset[hCoord];
    this->Vertex->SetPoint(2*interval, point);
    // Top
    Region->GetPoint(slice*4+1,point);
    point[0] = 0.1;
    point[1] += InclusionOffset[vCoord];
    if (slice == 0)
      point[2] += InclusionOffset[hCoord];
    else if (slice == NumSlices - 1)
      point[2] -= ExclusionOffset[hCoord];
    this->Vertex->SetPoint(2*interval+1, point);
  }

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgePolyData[i]->Modified();
}

//----------------------------------------------------------------------------
void vtkBoundingRegionSagittalSliceRepresentation::MoveLeftEdge(double* p1, double* p2)
{
  double shift = p2[hCoord] - p1[hCoord];

  Nm offset   = InclusionOffset[hCoord] + shift;

  if (offset < 0)
    offset = 0;
  else
  {
    Nm nextLeftEdge = realLeftEdge() + offset;
    Nm rightEdgeLimit  = rightEdge() - SlicingStep[hCoord];

    if (nextLeftEdge > rightEdgeLimit)
      offset = rightEdgeLimit - realLeftEdge();
  }

  InclusionOffset[hCoord] = offset;
  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkBoundingRegionSagittalSliceRepresentation::MoveRightEdge(double* p1, double* p2)
{
  double shift = p2[hCoord] - p1[hCoord];

  Nm offset = ExclusionOffset[hCoord] - shift;

  if (offset < 0)
    offset = 0;
  else
  {
    Nm nextRightEdge = realRightEdge() - offset;
    Nm leftEdgeLimit = leftEdge() + SlicingStep[hCoord];

    if (leftEdgeLimit > nextRightEdge)
      offset = realRightEdge() - leftEdgeLimit;
  }

  ExclusionOffset[hCoord] = offset;
  CreateRegion();

}

//----------------------------------------------------------------------------
void vtkBoundingRegionSagittalSliceRepresentation::MoveTopEdge(double* p1, double* p2)
{

  double shift = p2[vCoord] - p1[vCoord];
  Nm offset = InclusionOffset[vCoord] + shift;

  if (offset < 0)
    offset = 0;
  else
  {
    double firstSliceBounds[6];
    double lastSliceBounds[6];

    regionBounds(0, firstSliceBounds);
    int firstSlice = sliceNumber(firstSliceBounds[4] + InclusionOffset[2]);

    regionBounds(NumSlices-1, lastSliceBounds);
    int lastSlice = sliceNumber(lastSliceBounds[5] - ExclusionOffset[2]);

    bool collision = false;
    int  slice   = firstSlice;
    // cheack all visible slices
    while (!collision && slice <= lastSlice)
    {
      double sliceBounds[6];
      regionBounds(slice, sliceBounds);
      collision = sliceBounds[2] + offset >= sliceBounds[3] - ExclusionOffset[1] - SlicingStep[1];
      slice++;
    }

    if (collision)
      offset -= shift;
  }


  InclusionOffset[vCoord] = offset;
  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkBoundingRegionSagittalSliceRepresentation::MoveBottomEdge(double* p1, double* p2)
{
  double shift = p2[vCoord] - p1[vCoord];

  Nm offset = ExclusionOffset[vCoord] - shift;

  if (offset < 0)
    offset = 0;
  else
  {
  // Invalid Collision Detection
    Nm nextBottomEdge = realBottomEdge() - offset;
    Nm topEdgeLimit = topEdge() + SlicingStep[vCoord];

    if (topEdgeLimit > nextBottomEdge)
      offset = realBottomEdge() - topEdgeLimit;
  }

  ExclusionOffset[vCoord] = offset;
  CreateRegion();

}
