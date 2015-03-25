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


#include "AdaptiveCountingFrame.h"

#include "vtkCountingFrameSliceWidget.h"
#include "Extensions/CountingFrameExtension.h"

#include <Core/Analysis/Channel.h>

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkRenderWindow.h>
#include "vtkCountingFrame3DWidget.h"
#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>
#include <Extensions/EdgeDistances/ChannelEdges.h>
#include <Extensions/ExtensionUtils.h>

using namespace ESPINA;
using namespace ESPINA::CF;

//-----------------------------------------------------------------------------
AdaptiveCountingFrame::AdaptiveCountingFrame(CountingFrameExtension *channelExt,
                                             const Bounds &bounds,
                                             Nm inclusion[3],
                                             Nm exclusion[3],
                                             SchedulerSPtr scheduler)
: CountingFrame(channelExt, inclusion, exclusion, scheduler)
, m_channel(channelExt->extendedItem())
{
  updateCountingFrame();
}

//-----------------------------------------------------------------------------
AdaptiveCountingFrame::~AdaptiveCountingFrame()
{
}

//-----------------------------------------------------------------------------
void AdaptiveCountingFrame::updateCountingFrameImplementation()
{
  //qDebug() << "Updating CountingFrame Implementation";
  auto volume  = volumetricData(m_channel->output());
  auto origin  = volume->origin();
  auto spacing = volume->spacing();

  m_inclusionVolume = 0;

  auto edgesExtension = retrieveOrCreateExtension<ChannelEdges>(m_channel);

  vtkSmartPointer<vtkPolyData> channelEdges = edgesExtension->channelEdges();

  m_totalVolume = 0;

  int frontSliceOffset = frontOffset() / spacing[2];
  int backSliceOffset  = backOffset()  / spacing[2];

  double bounds[6];
  channelEdges->GetBounds(bounds);

  int channelFrontSlice = (bounds[4] + spacing[2]/2) / spacing[2];
  int channelBackSlice  = (bounds[5] + spacing[2]/2) / spacing[2];

  int frontSlice = channelFrontSlice + frontSliceOffset;
  frontSlice = std::max(frontSlice, channelFrontSlice);
  frontSlice = std::min(frontSlice, channelBackSlice);

  int backSlice = channelBackSlice - backSliceOffset;
  backSlice = std::max(backSlice, channelFrontSlice);
  backSlice = std::min(backSlice, channelBackSlice);

  // upper and lower refer to Espina's orientation
  Q_ASSERT(frontSlice <= backSlice);

  m_countingFrame = vtkSmartPointer<vtkPolyData>::New();
  m_channelEdges = channelEdges;

  vtkSmartPointer<vtkPoints>    regionVertex = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> faces        = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkIntArray>  faceData     = vtkSmartPointer<vtkIntArray>::New();

  for (int slice = channelFrontSlice; slice < channelBackSlice; slice++)
  {
    double LB[3], LT[3], RT[3], RB[3];
    channelEdges->GetPoint(4*slice+0, LB);
    channelEdges->GetPoint(4*slice+1, LT);
    channelEdges->GetPoint(4*slice+2, RT);
    channelEdges->GetPoint(4*slice+3, RB);

    Bounds sliceBounds{LT[0], RT[0], LT[1], LB[1], 0, 0};
    m_totalVolume += equivalentVolume(sliceBounds);
  }

  vtkIdType lastCell[4] = {-1, -1, -1, -1};
  for (int slice = frontSlice; slice <= backSlice; slice++)
  {
    vtkIdType cell[4];

    double LB[3], LT[3], RT[3], RB[3];
    double zOffset = 0;

    if (slice == frontSlice)
    {
      zOffset = frontOffset();
      if (frontSliceOffset > 0)
      {
        zOffset -= frontSliceOffset*spacing[2];
      }
    } else if (slice == backSlice)
    {
      zOffset = -backOffset();
      if (backSliceOffset > 0)
      {
        zOffset += backSliceOffset*spacing[2];
      }
    }


    channelEdges->GetPoint(4*slice+0, LB);
    LB[0] += leftOffset();
    LB[1] -= bottomOffset();
    LB[2] += zOffset;
    cell[0] = regionVertex->InsertNextPoint(LB);

    channelEdges->GetPoint(4*slice+1, LT);
    LT[0] += leftOffset();
    LT[1] += topOffset();
    LT[2] += zOffset;
    cell[1] = regionVertex->InsertNextPoint(LT);

    channelEdges->GetPoint(4*slice+2, RT);
    RT[0] -= rightOffset();
    RT[1] += topOffset();
    RT[2] += zOffset;
    cell[2] = regionVertex->InsertNextPoint(RT);

    channelEdges->GetPoint(4*slice+3, RB);
    RB[0] -= rightOffset();
    RB[1] -= bottomOffset();
    RB[2] += zOffset;
    cell[3] = regionVertex->InsertNextPoint(RB);
    if (slice == frontSlice)
    {
      // Upper Inclusion Face
      faces->InsertNextCell(4, cell);
      faceData->InsertNextValue(INCLUSION_FACE);
    } else if (slice == backSlice)
    {
      // Lower Inclusion Face
      faces->InsertNextCell(4, cell);
      faceData->InsertNextValue(EXCLUSION_FACE);
    } else
    {
      Q_ASSERT(lastCell[0] != -1);
      Q_ASSERT(lastCell[1] != -1);
      Q_ASSERT(lastCell[2] != -1);
      Q_ASSERT(lastCell[3] != -1);
      // Create lateral faces

      // Left Inclusion Face
      vtkIdType left[4];
      left[0] = lastCell[0];
      left[1] = lastCell[1];
      left[2] = cell[1];
      left[3] = cell[0];
      faces->InsertNextCell(4, left);
      faceData->InsertNextValue(INCLUSION_FACE);

      // Right Exclusion Face
      vtkIdType right[4];
      right[0] = lastCell[2];
      right[1] = lastCell[3];
      right[2] = cell[3];
      right[3] = cell[2];
      faces->InsertNextCell(4, right);
      faceData->InsertNextValue(EXCLUSION_FACE);

      // Top Inclusion Face
      vtkIdType top[4];
      top[0] = lastCell[1];
      top[1] = lastCell[2];
      top[2] = cell[2];
      top[3] = cell[1];
      faces->InsertNextCell(4, top);
      faceData->InsertNextValue(INCLUSION_FACE);

      // Bottom Exclusion Face
      vtkIdType bottom[4];
      bottom[0] = lastCell[3];
      bottom[1] = lastCell[0];
      bottom[2] = cell[0];
      bottom[3] = cell[3];
      faces->InsertNextCell(4, bottom);
      faceData->InsertNextValue(EXCLUSION_FACE);
    }
    memcpy(lastCell,cell,4*sizeof(vtkIdType));

    // Update Volumes
    if (slice < backSlice - 1)
    {
      // We don't care about the actual Z values
      Bounds sliceBounds{LT[0], RT[0]-spacing[0],
                         LT[1], LB[1]-spacing[1],
                         origin[2]-spacing[2]/2, origin[2]+spacing[2]/2};
      m_inclusionVolume += equivalentVolume(sliceBounds);
    }
  }

  m_countingFrame->SetPoints(regionVertex);
  m_countingFrame->SetPolys(faces);

  vtkCellData *data = m_countingFrame->GetCellData();
  data->SetScalars(faceData);
  data->GetScalars()->SetName("Type");

  //qDebug() << "CountingFrame Implementation Updated";
}