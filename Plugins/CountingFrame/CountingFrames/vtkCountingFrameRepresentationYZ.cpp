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
#include "vtkCountingFrameRepresentationYZ.h"

// ESPINA
#include <GUI/View/View2D.h>
#include <Core/Utils/Spatial.h>

// VTK
#include <vtkObjectFactory.h>
#include <vtkCellArray.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

vtkStandardNewMacro(vtkCountingFrameRepresentationYZ);

//----------------------------------------------------------------------------
void vtkCountingFrameRepresentationYZ::SetSlice(ESPINA::Nm pos)
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
  double lastSliceX = -VTK_DOUBLE_MAX;
  while (slice <= lastSlice)
  {
    double sliceBounds[6];
    regionBounds(slice, sliceBounds);
    auto sliceX = sliceBounds[1] - ExclusionOffset[0];

    if(!visible)
    {
      visible = (sliceBounds[0] + InclusionOffset[0] <= Slice) && (Slice <= sliceX);
    }

    if(sliceX > lastSliceX)
    {
      lastSliceX = sliceX;
    }

    slice++;
  }

  if (visible)
  {
    for(EDGE i = LEFT; i <= TOP; i = EDGE(i+1))
    {
      // Check if it is back slice
      if (ESPINA::areEqual(lastSliceX, pos, SlicingStep[0]))
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
  else
  {
    for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    {
      this->EdgeActor[i]->SetProperty(InvisibleProperty);
      this->EdgeActor[i]->GetProperty()->Modified();
      this->EdgeActor[i]->Modified();
    }
  }
}

//----------------------------------------------------------------------------
void vtkCountingFrameRepresentationYZ::CreateRegion()
{
  if(Region.GetPointer() == nullptr) return;

  double LB[3], RB[3];
  this->Region->GetPoint(0, LB);
  this->Region->GetPoint(NumPoints-1, RB);

  int UpperSlice = sliceNumber(LB[2] + InclusionOffset[2]);
  int LowerSlice = sliceNumber(RB[2] - ExclusionOffset[2]);
  if (UpperSlice == LowerSlice)
  {
    UpperSlice--;
  }

  int numRepSlices = LowerSlice - UpperSlice + 1;

  if (numRepSlices == 0) return;

  unsigned int numIntervals = numRepSlices - 1;
  unsigned int numVertex    = (numRepSlices * 2) + 2;

  // Set of point pairs. Each pair belong to the same slice .
  // First pair belongs to the left edge
  // Last pair belongs to the right edge
  // Odd indexed points belong to the top edge
  // Even indexed points belong to the bottom edge
  /*
   *                  TOP   11'
   *                         |
   *              1\ /5-7\ /11
   *               | 3     9 |
   *       LEFT    |         |    RIGHT
   *               | 2-4     |
   *        0'-----0/   \6-8-10
   *
   *                 BOTTOM
   *
   */
  this->Vertex->Reset();
  this->Vertex->SetNumberOfPoints(numVertex);
  double zeroPoint[3]{0.,0.,0.};
  for(vtkIdType i = 0; i < this->Vertex->GetNumberOfPoints(); ++i) this->Vertex->SetPoint(i, zeroPoint);

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
  {
    this->EdgePolyData[i]->GetLines()->Reset();
  }

  // LEFT
  double LT[3];
  Region->GetPoint(UpperSlice*4+0, LB);
  Region->GetPoint(UpperSlice*4+1, LT);
  LB[1] -= ExclusionOffset[1];
  LB[2] += (UpperSlice == 0) ? InclusionOffset[2] : SlicingStep[2]/2;
  LT[1] += InclusionOffset[1];
  LT[2] += (UpperSlice == 0) ? InclusionOffset[2] : SlicingStep[2]/2;
  LB[0] = LT[0] = Slice + Depth;

  this->EdgePolyData[LEFT]->GetLines()->Allocate(this->EdgePolyData[LEFT]->GetLines()->EstimateSize(1,2));
  this->EdgePolyData[LEFT]->GetLines()->InsertNextCell(2);
  this->EdgePolyData[LEFT]->GetLines()->InsertCellPoint(this->Vertex->InsertNextPoint(LB));
  this->EdgePolyData[LEFT]->GetLines()->InsertCellPoint(this->Vertex->InsertNextPoint(LT));

  // RIGHT
  Region->GetPoint(LowerSlice*4+0, LB);
  Region->GetPoint(LowerSlice*4+1, LT);
  LB[1] -= ExclusionOffset[1];
  LB[2] += (LowerSlice == NumSlices - 1) ? -ExclusionOffset[2] : SlicingStep[2]/2;
  LT[2] += (LowerSlice == NumSlices - 1) ? -ExclusionOffset[2] : SlicingStep[2]/2;
  LB[0] = LT[0] = Slice + Depth;

  this->EdgePolyData[RIGHT]->GetLines()->Allocate(this->EdgePolyData[RIGHT]->GetLines()->EstimateSize(1,2));
  this->EdgePolyData[RIGHT]->GetLines()->InsertNextCell(2);
  this->EdgePolyData[RIGHT]->GetLines()->InsertCellPoint(this->Vertex->InsertNextPoint(LB));
  this->EdgePolyData[RIGHT]->GetLines()->InsertCellPoint(this->Vertex->InsertNextPoint(LT));

  // BOTTOM
  this->EdgePolyData[BOTTOM]->GetLines()->Allocate(this->EdgePolyData[BOTTOM]->GetLines()->EstimateSize(numIntervals + 1,2));
  Region->GetPoint(             0, LB);
  Region->GetPoint(UpperSlice*4+0, LT);
  LT[1] -= ExclusionOffset[1];
  LB[1] = LT[1];
  LT[2] += (UpperSlice == 0) ? InclusionOffset[2] : SlicingStep[2]/2;
  LB[0] = LT[0] = Slice + Depth;

  this->EdgePolyData[BOTTOM]->GetLines()->InsertNextCell(2);
  this->EdgePolyData[BOTTOM]->GetLines()->InsertCellPoint(this->Vertex->InsertNextPoint(LB));
  this->EdgePolyData[BOTTOM]->GetLines()->InsertCellPoint(this->Vertex->InsertNextPoint(LT));

  vtkIdType previous_id = 0;
  for(int slice = UpperSlice; slice <= LowerSlice; ++slice)
  {
    double point[3];

    Region->GetPoint(slice*4+0,point);
    point[0] = Slice + Depth;
    point[1] -= ExclusionOffset[1];
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
    if(slice != UpperSlice)
    {
      this->EdgePolyData[BOTTOM]->GetLines()->InsertNextCell(2);
      this->EdgePolyData[BOTTOM]->GetLines()->InsertCellPoint(previous_id);
      this->EdgePolyData[BOTTOM]->GetLines()->InsertCellPoint(id);
    }

    previous_id = id;
  }

  // TOP
  previous_id = 0;
  this->EdgePolyData[TOP]->GetLines()->Allocate(this->EdgePolyData[TOP]->GetLines()->EstimateSize(numIntervals,2));
  for(int slice = UpperSlice; slice <= LowerSlice; ++slice)
  {
    double point[3];

    Region->GetPoint(slice*4+1,point);
    point[0]  = Slice + Depth;
    point[1] += InclusionOffset[1];
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
    if(slice != UpperSlice)
    {
      this->EdgePolyData[TOP]->GetLines()->InsertNextCell(2);
      this->EdgePolyData[TOP]->GetLines()->InsertCellPoint(previous_id);
      this->EdgePolyData[TOP]->GetLines()->InsertCellPoint(id);
    }

    previous_id = id;
  }

  this->Vertex->Modified();
  this->Vertex->ComputeBounds();

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
  {
    this->EdgePolyData[i]->GetLines()->Modified();
    this->EdgePolyData[i]->BuildCells();
    this->EdgePolyData[i]->Modified();
    this->EdgeMapper[i]->Update();
    this->EdgeActor[i]->GetProperty()->Modified();
    this->EdgeActor[i]->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkCountingFrameRepresentationYZ::MoveLeftEdge(double* p1, double* p2)
{
  auto shift  = p2[2] - p1[2];
  auto offset = InclusionOffset[2] + shift;

  if (offset < 0)
  {
    offset = 0;
  }
  else
  {
    ESPINA::Nm nextLeftEdge = realLeftEdge() + offset;
    ESPINA::Nm rightEdgeLimit  = rightEdge() - SlicingStep[2];

    if (nextLeftEdge > rightEdgeLimit)
    {
      offset = rightEdgeLimit - realLeftEdge();
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
        collision = sliceBounds[2] + InclusionOffset[1] >= sliceBounds[3] - ExclusionOffset[1] - SlicingStep[1];
        slice++;
      }

      if (collision)
      {
        InclusionOffset[1] = ExclusionOffset[1] = 0;
      }
    }
  }

  InclusionOffset[2] = offset;
  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkCountingFrameRepresentationYZ::MoveRightEdge(double* p1, double* p2)
{
  auto shift  = p2[2] - p1[2];
  auto offset = ExclusionOffset[2] - shift;

  if (offset < 0)
  {
    offset = 0;
  }
  else
  {
    ESPINA::Nm nextRightEdge = realRightEdge() - offset;
    ESPINA::Nm leftEdgeLimit = leftEdge() + SlicingStep[3];

    if (leftEdgeLimit > nextRightEdge)
    {
      offset = realRightEdge() - leftEdgeLimit;
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
        collision = sliceBounds[2] + InclusionOffset[1] >= sliceBounds[3] - ExclusionOffset[1] - SlicingStep[1];
        slice++;
      }

      if (collision)
      {
        InclusionOffset[1] = ExclusionOffset[1] = 0;
      }
    }
  }

  ExclusionOffset[2] = offset;
  CreateRegion();

}

//----------------------------------------------------------------------------
void vtkCountingFrameRepresentationYZ::MoveTopEdge(double* p1, double* p2)
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
void vtkCountingFrameRepresentationYZ::MoveBottomEdge(double* p1, double* p2)
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
