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


#include "vtkCountingFrameAxialSliceRepresentation.h"

#include <vtkObjectFactory.h>
#include <vtkCellArray.h>
#include <vtkActor.h>

vtkStandardNewMacro(vtkCountingFrameAxialSliceRepresentation);

//----------------------------------------------------------------------------
void vtkCountingFrameAxialSliceRepresentation::SetSlice(EspINA::Nm pos)
{
  Slice = pos;

  double firstSliceBounds[6];
  double lastSliceBounds[6];

  regionBounds(0, firstSliceBounds);
  regionBounds(NumSlices-1, lastSliceBounds);

  if (Slice < firstSliceBounds[4] + InclusionOffset[2]
   || lastSliceBounds[5] - ExclusionOffset[2] < Slice)
  {
    for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
      this->EdgeActor[i]->SetProperty(InvisibleProperty);
    return;
  } else
  {
    for(EDGE i = LEFT; i <= TOP; i = EDGE(i+1))
      this->EdgeActor[i]->SetProperty(InclusionEdgeProperty);
    for(EDGE i = RIGHT; i <= BOTTOM; i = EDGE(i+1))
      this->EdgeActor[i]->SetProperty(ExclusionEdgeProperty);

    CreateRegion();
  }
}

//----------------------------------------------------------------------------
void vtkCountingFrameAxialSliceRepresentation::CreateRegion()
{
  if (Region.GetPointer() == NULL)
    return;

  // Corners of the rectangular region
  this->Vertex->SetNumberOfPoints(4);

  for(EDGE i=LEFT; i <= BOTTOM; i=EDGE(i+1))
  {
    this->EdgePolyData[i]->GetLines()->Reset();
    this->EdgePolyData[i]->GetLines()->Allocate(
      this->EdgePolyData[i]->GetLines()->EstimateSize(1,2));
    this->EdgePolyData[i]->GetLines()->InsertNextCell(2);
    this->EdgePolyData[i]->GetLines()->InsertCellPoint(i);
    this->EdgePolyData[i]->GetLines()->InsertCellPoint((i+1)%4);
  }

  double LB[3], LT[3], RT[3], RB[3];

  EspINA::Nm slice = sliceNumber(Slice);
  // Get original Region Points
  Region->GetPoint(slice*4+0, LB);
  Region->GetPoint(slice*4+1, LT);
  Region->GetPoint(slice*4+2, RT);
  Region->GetPoint(slice*4+3, RB);

  // Change its depth to be always on top of the XY plane
  // according to Espina's Camera
  LB[2] = LT[2] = RT[2] = RB[2] = -0.1;

  // Shift edges' points
  LB[0] += InclusionOffset[hCoord];
  LT[0] += InclusionOffset[hCoord];

  LT[1] += InclusionOffset[vCoord];
  RT[1] += InclusionOffset[vCoord];

  RB[0] -= ExclusionOffset[hCoord];
  RT[0] -= ExclusionOffset[hCoord];

  RB[1] -= ExclusionOffset[vCoord];
  LB[1] -= ExclusionOffset[vCoord];

  this->Vertex->SetPoint(0, LB);
  this->Vertex->SetPoint(1, LT);
  this->Vertex->SetPoint(2, RT);
  this->Vertex->SetPoint(3, RB);

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgePolyData[i]->Modified();
}

//----------------------------------------------------------------------------
void vtkCountingFrameAxialSliceRepresentation::MoveLeftEdge(double* p1, double* p2)
{
  double shift = p2[hCoord] - p1[hCoord];

  EspINA::Nm offset   = InclusionOffset[hCoord] + shift;

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
      collision = sliceBounds[0] + offset >= sliceBounds[1] - ExclusionOffset[0] - Resolution[0];
      slice++;
    }

    if (collision)
      offset -= shift;
  }

  InclusionOffset[hCoord] = offset;
  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkCountingFrameAxialSliceRepresentation::MoveRightEdge(double* p1, double* p2)
{
  double shift = p2[hCoord] - p1[hCoord];

  EspINA::Nm offset = ExclusionOffset[hCoord] - shift;

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
      collision = sliceBounds[0] + InclusionOffset[0] >= sliceBounds[1] - offset - Resolution[0];
      slice++;
    }

    if (collision)
      offset += shift;
  }

  ExclusionOffset[hCoord] = offset;
  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkCountingFrameAxialSliceRepresentation::MoveTopEdge(double* p1, double* p2)
{
  double shift = p2[vCoord] - p1[vCoord];
  EspINA::Nm offset = InclusionOffset[vCoord] + shift;

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
      collision = sliceBounds[2] + offset >= sliceBounds[3] - ExclusionOffset[1] - Resolution[1];
      slice++;
    }

    if (collision)
      offset -= shift;
  }

  InclusionOffset[vCoord] = offset;
  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkCountingFrameAxialSliceRepresentation::MoveBottomEdge(double* p1, double* p2)
{
  double shift = p2[vCoord] - p1[vCoord];

  EspINA::Nm offset = ExclusionOffset[vCoord] - shift;

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
      collision = sliceBounds[2] + InclusionOffset[1] >= sliceBounds[3] - offset - Resolution[1];
      slice++;
    }

    if (collision)
      offset += shift;
  }

  ExclusionOffset[vCoord] = offset;
  CreateRegion();
}