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
#include "vtkCountingFrameRepresentationXZ.h"

// ESPINA
#include <GUI/View/View2D.h>
#include <Core/Utils/Spatial.h>

// VTK
#include <vtkObjectFactory.h>
#include <vtkCellArray.h>
#include <vtkActor.h>

vtkStandardNewMacro(vtkCountingFrameRepresentationXZ);

//----------------------------------------------------------------------------
void vtkCountingFrameRepresentationXZ::SetSlice(ESPINA::Nm pos)
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
  // check all slices
  double lastSliceY = -VTK_DOUBLE_MAX;
  while (slice <= lastSlice)
  {
    double sliceBounds[6];
    regionBounds(slice, sliceBounds);
    auto sliceY = sliceBounds[3] - ExclusionOffset[1];

    if(!visible)
    {
      visible = (sliceBounds[2] + InclusionOffset[1] <= Slice) && (Slice <= sliceY);
    }

    if(sliceY > lastSliceY)
    {
      lastSliceY = sliceY;
    }

    slice++;
  }

  if (visible)
  {
    // check for back slice
    if (ESPINA::areEqual(lastSliceY, pos, SlicingStep[1]))
    {
      for(EDGE i = LEFT; i <= TOP; i = EDGE(i+1))
      {
        this->EdgeActor[i]->SetProperty(ExclusionEdgeProperty);
      }
    }
    else
    {
      for(EDGE i = LEFT; i <= TOP; i = EDGE(i+1))
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
  else
  {
    for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    {
      this->EdgeActor[i]->SetProperty(InvisibleProperty);
    }
  }
}

//----------------------------------------------------------------------------
void vtkCountingFrameRepresentationXZ::CreateRegion()
{
  if(Region.GetPointer() == nullptr) return;

  double LT[3], LB[3];
  this->Region->GetPoint(0, LT);
  this->Region->GetPoint(NumPoints-4, LB);

  auto FrontSlice = sliceNumber(LT[2] + InclusionOffset[2]);
  auto BackSlice  = sliceNumber(LB[2] - ExclusionOffset[2]);
  if (FrontSlice == BackSlice)
  {
    FrontSlice--;
  }

  int numRepSlices = BackSlice - FrontSlice + 1;

  if (numRepSlices == 0) return;

  unsigned int numIntervals = numRepSlices - 1;
  unsigned int numVertex    = numRepSlices * 2;

  // Set of point pairs. Each pair belong to the same slice .
  // First pair belongs to the top edge
  // Last pair belongs to the bottom edge
  // Even indexed points belong to the left edge
  // Odd indexed points belong to the right edge
  /*
   *                   TOP   1'
   *                         |
   *               0---------1
   *                \       /
   *                 2     3
   *      LEFT      /       \     RIGHT
   *               4         5
   *               |         |
   *          6'---6---------7
   *
   *                  BOTTOM
   */
  this->Vertex->SetNumberOfPoints(numVertex + 2);

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
  {
    this->EdgePolyData[i]->GetLines()->Reset();
  }

  // TOP
  double RT[3];
  Region->GetPoint(FrontSlice*4+0,LT);
  Region->GetPoint(FrontSlice*4+3,RT);
  LT[0] += InclusionOffset[0];
  LT[2] += FrontSlice == 0 ? InclusionOffset[2] : SlicingStep[2]/2;
  RT[0] -= ExclusionOffset[0];
  RT[2] += FrontSlice == 0 ? InclusionOffset[2] : SlicingStep[2]/2;
  LT[1] = RT[1] = Slice + Depth;

  this->EdgePolyData[TOP]->GetLines()->Allocate(this->EdgePolyData[TOP]->GetLines()->EstimateSize(1,2));
  this->EdgePolyData[TOP]->GetLines()->InsertNextCell(2);
  this->EdgePolyData[TOP]->GetLines()->InsertCellPoint(this->Vertex->InsertNextPoint(LT));
  this->EdgePolyData[TOP]->GetLines()->InsertCellPoint(this->Vertex->InsertNextPoint(RT));

  // BOTTOM
  Region->GetPoint(BackSlice*4+0,LT);
  Region->GetPoint(BackSlice*4+3,RT);
  LT[2] += (BackSlice == NumSlices -1) ? -ExclusionOffset[2] : SlicingStep[2]/2;
  RT[0] -= ExclusionOffset[0];
  RT[2] += (BackSlice == NumSlices -1) ? -ExclusionOffset[2] : SlicingStep[2]/2;
  LT[1] = RT[1] = Slice + Depth;

  this->EdgePolyData[BOTTOM]->GetLines()->Allocate(this->EdgePolyData[BOTTOM]->GetLines()->EstimateSize(1,2));
  this->EdgePolyData[BOTTOM]->GetLines()->InsertNextCell(2);
  this->EdgePolyData[BOTTOM]->GetLines()->InsertCellPoint(this->Vertex->InsertNextPoint(LT));
  this->EdgePolyData[BOTTOM]->GetLines()->InsertCellPoint(this->Vertex->InsertNextPoint(RT));

  // LEFT
  this->EdgePolyData[LEFT]->GetLines()->Allocate(this->EdgePolyData[LEFT]->GetLines()->EstimateSize(numIntervals,2));
  for(int slice = FrontSlice; slice <= BackSlice; ++slice)
  {
    double point[3];
    vtkIdType previous_id;

    Region->GetPoint(slice*4+0, point);
    point[0] += InclusionOffset[0];
    point[1] = Slice + Depth;

    if (slice == 0)
    {
      point[2] += InclusionOffset[2];
    }
    else
    {
      if (slice == NumSlices - 1)
      {
        point[2] -= ExclusionOffset[2];
      }
      else
      {
        point[2] += SlicingStep[2] / 2;
      }
    }

    auto id = this->Vertex->InsertNextPoint(point);
    if(slice != FrontSlice)
    {
      this->EdgePolyData[LEFT]->GetLines()->InsertNextCell(2);
      this->EdgePolyData[LEFT]->GetLines()->InsertCellPoint(previous_id);
      this->EdgePolyData[LEFT]->GetLines()->InsertCellPoint(id);
    }

    previous_id = id;
  }

  // RIGHT
  this->EdgePolyData[RIGHT]->GetLines()->Allocate(this->EdgePolyData[RIGHT]->GetLines()->EstimateSize(numIntervals+1,2));
  double p1[3], p2[3];
  Region->GetPoint(             3, p1);
  Region->GetPoint(FrontSlice*4+3, p2);
  p2[0] -= ExclusionOffset[0];
  p2[2] += (FrontSlice == 0) ? InclusionOffset[2] : SlicingStep[2]/2;
  p1[0] = p2[0];
  p2[1] = p1[1] = Slice + Depth;

  this->EdgePolyData[RIGHT]->GetLines()->InsertNextCell(2);
  this->EdgePolyData[RIGHT]->GetLines()->InsertCellPoint(this->Vertex->InsertNextPoint(p1));
  this->EdgePolyData[RIGHT]->GetLines()->InsertCellPoint(this->Vertex->InsertNextPoint(p2));

  for(int slice = FrontSlice; slice <= BackSlice; ++slice)
  {
    double point[3];
    vtkIdType previous_id;

    Region->GetPoint(slice*4+3, point);
    point[0] -= ExclusionOffset[0];
    point[1] = Slice + Depth;

    if (slice == 0)
    {
      point[2] += InclusionOffset[2];
    }
    else
    {
      if (slice == NumSlices - 1)
      {
        point[2] -= ExclusionOffset[2];
      }
      else
      {
        point[2] += SlicingStep[2] / 2;
      }
    }

    auto id = this->Vertex->InsertNextPoint(point);
    if(slice != FrontSlice)
    {
      this->EdgePolyData[RIGHT]->GetLines()->InsertNextCell(2);
      this->EdgePolyData[RIGHT]->GetLines()->InsertCellPoint(previous_id);
      this->EdgePolyData[RIGHT]->GetLines()->InsertCellPoint(id);
    }

    previous_id = id;
  }

  this->Vertex->Modified();

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
  {
    this->EdgePolyData[i]->GetLines()->Modified();
    this->EdgePolyData[i]->Modified();
  }
}


//----------------------------------------------------------------------------
void vtkCountingFrameRepresentationXZ::MoveLeftEdge(double* p1, double* p2)
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
void vtkCountingFrameRepresentationXZ::MoveRightEdge(double* p1, double* p2)
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
void vtkCountingFrameRepresentationXZ::MoveTopEdge(double* p1, double* p2)
{
  auto shift  = p2[2] - p1[2];
  auto offset = InclusionOffset[2] + shift;

  if (offset < 0)
  {
    offset = 0;
  }
  else
  {
    ESPINA::Nm nextTopEdge = realTopEdge() + offset;
    ESPINA::Nm bottomEdgeLimit  = bottomEdge() - SlicingStep[2];

    if (nextTopEdge > bottomEdgeLimit)
    {
      offset = bottomEdgeLimit - realTopEdge();
    }
    else
    {
      double firstSliceBounds[6];
      double lastSliceBounds[6];

      regionBounds(0, firstSliceBounds);
      int firstSlice = sliceNumber(firstSliceBounds[4] + offset);

      regionBounds(NumSlices-1, lastSliceBounds);
      int lastSlice = sliceNumber(lastSliceBounds[5] - ExclusionOffset[2]);

      bool collision = false;
      int  slice = firstSlice;
      // check all visible slices
      while (!collision && slice < lastSlice)
      {
        double sliceBounds[6];
        regionBounds(slice, sliceBounds);
        collision = sliceBounds[0] + InclusionOffset[0] >= sliceBounds[1] - ExclusionOffset[0] - SlicingStep[0];
        slice++;
      }

      if (collision)
      {
        InclusionOffset[0] = ExclusionOffset[0] = 0;
      }
    }
  }

  InclusionOffset[2] = offset;
  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkCountingFrameRepresentationXZ::MoveBottomEdge(double* p1, double* p2)
{
  auto shift  = p2[2] - p1[2];
  auto offset = ExclusionOffset[2] - shift;

  if (offset < 0)
  {
    offset = 0;
  }
  else
  {
    ESPINA::Nm nextBottomEdge = realBottomEdge() - offset;
    ESPINA::Nm topEdgeLimit = topEdge() + SlicingStep[2];

    if (topEdgeLimit > nextBottomEdge)
    {
      offset = realBottomEdge() - topEdgeLimit;
    }
    else
    {
      double firstSliceBounds[6];
      double lastSliceBounds[6];

      regionBounds(0, firstSliceBounds);
      int firstSlice = sliceNumber(firstSliceBounds[4] + InclusionOffset[2]);

      regionBounds(NumSlices-1, lastSliceBounds);
      int lastSlice = sliceNumber(lastSliceBounds[5] - offset);

      bool collision = false;
      int  slice = firstSlice;
      // check all visible slices
      while (!collision && slice < lastSlice)
      {
        double sliceBounds[6];
        regionBounds(slice, sliceBounds);
        collision = sliceBounds[0] + InclusionOffset[0] >= sliceBounds[1] - ExclusionOffset[0] - SlicingStep[0];
        slice++;
      }

      if (collision)
      {
        InclusionOffset[0] = ExclusionOffset[0] = 0;
      }
    }
  }

  ExclusionOffset[2] = offset;
  CreateRegion();
}
