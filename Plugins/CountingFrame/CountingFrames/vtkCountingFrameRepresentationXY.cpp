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

// Plugin
#include "vtkCountingFrameRepresentationXY.h"

// ESPINA
#include <GUI/View/View2D.h>
#include <Core/Utils/Spatial.h>

// VTK
#include <vtkObjectFactory.h>
#include <vtkCellArray.h>
#include <vtkActor.h>
#include <vtkMath.h>

vtkStandardNewMacro(vtkCountingFrameRepresentationXY);

//----------------------------------------------------------------------------
void vtkCountingFrameRepresentationXY::SetSlice(ESPINA::Nm pos)
{
  Slice = pos;

  double fs = frontSlice();
  double bs = backSlice();

  if ((Slice < fs && !ESPINA::areEqual(Slice, fs, SlicingStep[2])) ||
      (bs < Slice && !ESPINA::areEqual(Slice, bs, SlicingStep[2])))
  {
    for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    {
      this->EdgeActor[i]->SetProperty(InvisibleProperty);
    }
  }
  else
  {
    for(EDGE i = LEFT; i <= TOP; i = EDGE(i+1))
    {
      // Check if it is back slice
      if (ESPINA::areEqual(Slice, bs, SlicingStep[2]))
      {
        this->EdgeActor[i]->SetProperty(ExclusionEdgeProperty);
      }
      else
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
  if (Region.GetPointer() == nullptr) return;

  // Corners of the rectangular region
  this->Vertex->SetNumberOfPoints(8);

  for(EDGE i=LEFT; i <= BOTTOM; i=EDGE(i+1))
  {
    this->EdgePolyData[i]->GetLines()->Reset();
    this->EdgePolyData[i]->GetLines()->Allocate(this->EdgePolyData[i]->GetLines()->EstimateSize(1,2));
    this->EdgePolyData[i]->GetLines()->InsertNextCell(2);
    this->EdgePolyData[i]->GetLines()->InsertCellPoint(2*i);
    this->EdgePolyData[i]->GetLines()->InsertCellPoint(2*i+1);
  }

  double LB[3], LT[3], RT[3], RB[3];

  int slice = sliceNumber(Slice);

  // LEFT
  Region->GetPoint(slice*4+0, LB);
  Region->GetPoint(slice*4+1, LT);
  LB[0] += InclusionOffset[0];
  LB[1] -= ExclusionOffset[1];
  LT[0] += InclusionOffset[0];
  LT[1] += InclusionOffset[1];
  LB[2] = LT[2] = Depth;
  this->Vertex->SetPoint(0, LB);
  this->Vertex->SetPoint(1, LT);

  // TOP
  Region->GetPoint(slice*4+1, LT);
  Region->GetPoint(slice*4+2, RT);
  LT[0] += InclusionOffset[0];
  LT[1] += InclusionOffset[1];
  RT[0] -= ExclusionOffset[0];
  RT[1] += InclusionOffset[1];
  RT[2] = LT[2] = Depth;
  this->Vertex->SetPoint(2, LT);
  this->Vertex->SetPoint(3, RT);

  // RIGHT
  Region->GetPoint(slice*4+2, RT);
  Region->GetPoint(slice*4+3, RB);
  RT[0] -= ExclusionOffset[0];
  RB[0] -= ExclusionOffset[0];
  RB[1] -= ExclusionOffset[1];
  RT[2] = RB[2] = Depth;
  this->Vertex->SetPoint(4, RT);
  this->Vertex->SetPoint(5, RB);

  // BOTTOM
  Region->GetPoint(slice*4+3, RB);
  Region->GetPoint(slice*4+0, LB);
  RB[0] -= ExclusionOffset[0];
  RB[1] -= ExclusionOffset[1];
  LB[1] -= ExclusionOffset[1];
  RB[2] = LB[2] = Depth;
  this->Vertex->SetPoint(6, RB);
  this->Vertex->SetPoint(7, LB);

  this->Vertex->Modified();

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
  {
    this->EdgePolyData[i]->GetLines()->Modified();
    this->EdgePolyData[i]->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkCountingFrameRepresentationXY::MoveLeftEdge(double* p1, double* p2)
{
  auto shift  = p2[0] - p1[0];
  auto offset = InclusionOffset[0] + shift;

  if (offset < 0)
  {
    offset = 0;
  }
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
    // check all visible slices
    while (!collision && slice <= lastSlice)
    {
      double sliceBounds[6];
      regionBounds(slice, sliceBounds);
      collision = sliceBounds[0] + offset >= sliceBounds[1] - ExclusionOffset[0] - SlicingStep[0];
      slice++;
    }

    if (collision)
    {
      offset -= shift;
    }
  }

  InclusionOffset[0] = offset;
  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkCountingFrameRepresentationXY::MoveRightEdge(double* p1, double* p2)
{
  auto shift  = p2[0] - p1[0];
  auto offset = ExclusionOffset[0] - shift;

  if (offset < 0)
  {
    offset = 0;
  }
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
    // check all visible slices
    while (!collision && slice <= lastSlice)
    {
      double sliceBounds[6];
      regionBounds(slice, sliceBounds);
      collision = sliceBounds[0] + InclusionOffset[0] >= sliceBounds[1] - offset - SlicingStep[0];
      slice++;
    }

    if (collision)
    {
      offset += shift;
    }
  }

  ExclusionOffset[0] = offset;
  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkCountingFrameRepresentationXY::MoveTopEdge(double* p1, double* p2)
{
  auto shift  = p2[1] - p1[1];
  auto offset = InclusionOffset[1] + shift;

  if (offset < 0)
  {
    offset = 0;
  }
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
    // check all visible slices
    while (!collision && slice <= lastSlice)
    {
      double sliceBounds[6];
      regionBounds(slice, sliceBounds);
      collision = sliceBounds[2] + offset >= sliceBounds[3] - ExclusionOffset[1] - SlicingStep[1];
      slice++;
    }

    if (collision)
    {
      offset -= shift;
    }
  }

  InclusionOffset[1] = offset;
  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkCountingFrameRepresentationXY::MoveBottomEdge(double* p1, double* p2)
{
  auto shift  = p2[1] - p1[1];
  auto offset = ExclusionOffset[1] - shift;

  if (offset < 0)
  {
    offset = 0;
  }
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
    // check all visible slices
    while (!collision && slice <= lastSlice)
    {
      double sliceBounds[6];
      regionBounds(slice, sliceBounds);
      collision = sliceBounds[2] + InclusionOffset[1] >= sliceBounds[3] - offset - SlicingStep[1];
      slice++;
    }

    if (collision)
    {
      offset += shift;
    }
  }

  ExclusionOffset[1] = offset;
  CreateRegion();
}


//----------------------------------------------------------------------------
ESPINA::Nm vtkCountingFrameRepresentationXY::frontSlice() const
{
  double frontBounds[6];

  regionBounds(0, frontBounds);

  auto shift = vtkMath::Floor(InclusionOffset[2]/SlicingStep[2])+0.5;

  return frontBounds[4] + shift*SlicingStep[2];
}

//----------------------------------------------------------------------------
ESPINA::Nm vtkCountingFrameRepresentationXY::backSlice() const
{
  double backBounds[6];

  regionBounds(NumSlices - 1, backBounds);

  auto shift = vtkMath::Floor(ExclusionOffset[2]/SlicingStep[2])+0.5;

  return backBounds[5] - shift*SlicingStep[2];
}
