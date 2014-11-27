/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "vtkCountingFrameRepresentationXY.h"
#include <GUI/View/View2D.h>

#include <vtkObjectFactory.h>
#include <vtkCellArray.h>
#include <vtkActor.h>
#include <Core/Utils/Spatial.h>

vtkStandardNewMacro(vtkCountingFrameRepresentationXY);

//----------------------------------------------------------------------------
void vtkCountingFrameRepresentationXY::SetSlice(ESPINA::Nm pos)
{
  Slice = pos;

  double firstSliceBounds[6];
  double lastSliceBounds[6];

  regionBounds(0, firstSliceBounds);
  regionBounds(NumSlices-1, lastSliceBounds); // there is one more extra for the cover

  double fs = frontSlice();
  double bs = backSlice();

  if ((Slice < fs && !ESPINA::areEqual(Slice, fs, SlicingStep[2]))
    ||(bs < Slice && !ESPINA::areEqual(Slice, bs, SlicingStep[2])))
  {
    for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    {
      this->EdgeActor[i]->SetProperty(InvisibleProperty);
    }
  } else
  {
    for(EDGE i = LEFT; i <= TOP; i = EDGE(i+1))
    {
      // Check if it is back slice
      if (ESPINA::areEqual(Slice, bs, SlicingStep[2]))
      {
        this->EdgeActor[i]->SetProperty(ExclusionEdgeProperty);
      } else
      {
        this->EdgeActor[i]->SetProperty(InclusionEdgeProperty);
      }
    }
    for(EDGE i = RIGHT; i <= BOTTOM; i = EDGE(i+1))
    {
      this->EdgeActor[i]->SetProperty(ExclusionEdgeProperty);
    }

    CreateRegion();
  }
}

//----------------------------------------------------------------------------
void vtkCountingFrameRepresentationXY::CreateRegion()
{
  if (Region.GetPointer() == nullptr)
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

  int slice = sliceNumber(Slice);
  // Get original Region Points
  Region->GetPoint(slice*4+0, LB);
  Region->GetPoint(slice*4+1, LT);
  Region->GetPoint(slice*4+2, RT);
  Region->GetPoint(slice*4+3, RB);

  // Change its depth to be always on top of the XY plane
  // according to Espina's Camera
  LB[2] = LT[2] = RT[2] = RB[2] = -ESPINA::View2D::WIDGET_SHIFT; // TODO: make this constant private and use view->widgetDepth() instead.

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
void vtkCountingFrameRepresentationXY::MoveLeftEdge(double* p1, double* p2)
{
  double shift = p2[hCoord] - p1[hCoord];

  ESPINA::Nm offset   = InclusionOffset[hCoord] + shift;

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
      collision = sliceBounds[0] + offset >= sliceBounds[1] - ExclusionOffset[0] - SlicingStep[0];
      slice++;
    }

    if (collision)
      offset -= shift;
  }

  InclusionOffset[hCoord] = offset;
  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkCountingFrameRepresentationXY::MoveRightEdge(double* p1, double* p2)
{
  double shift = p2[hCoord] - p1[hCoord];

  ESPINA::Nm offset = ExclusionOffset[hCoord] - shift;

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
      collision = sliceBounds[0] + InclusionOffset[0] >= sliceBounds[1] - offset - SlicingStep[0];
      slice++;
    }

    if (collision)
      offset += shift;
  }

  ExclusionOffset[hCoord] = offset;
  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkCountingFrameRepresentationXY::MoveTopEdge(double* p1, double* p2)
{
  double shift = p2[vCoord] - p1[vCoord];
  ESPINA::Nm offset = InclusionOffset[vCoord] + shift;

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
void vtkCountingFrameRepresentationXY::MoveBottomEdge(double* p1, double* p2)
{
  double shift = p2[vCoord] - p1[vCoord];

  ESPINA::Nm offset = ExclusionOffset[vCoord] - shift;

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
      collision = sliceBounds[2] + InclusionOffset[1] >= sliceBounds[3] - offset - SlicingStep[1];
      slice++;
    }

    if (collision)
      offset += shift;
  }

  ExclusionOffset[vCoord] = offset;
  CreateRegion();
}


//----------------------------------------------------------------------------
ESPINA::Nm vtkCountingFrameRepresentationXY::frontSlice() const
{
  double frontBounds[6];

  regionBounds(0, frontBounds);

  double shift = int(InclusionOffset[2]/SlicingStep[2])+0.5;

  return frontBounds[4] + shift*SlicingStep[2];
}

//----------------------------------------------------------------------------
ESPINA::Nm vtkCountingFrameRepresentationXY::backSlice() const
{
  double backBounds[6];

  regionBounds(NumSlices - 1, backBounds);

  double shift = int(ExclusionOffset[2]/SlicingStep[2])+0.5;

  return backBounds[5] - shift*SlicingStep[2];
}
