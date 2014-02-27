/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "AdaptiveCountingFrame.h"

#include "vtkCountingFrameSliceWidget.h"
#include "Extensions/CountingFrameExtension.h"

#include <Core/Analysis/Channel.h>

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include "vtkCountingFrame3DWidget.h"
#include <GUI/View/View2D.h>
#include <Extensions/EdgeDistances/ChannelEdges.h>
#include <Extensions/ExtensionUtils.h>

using namespace EspINA;
using namespace EspINA::CF;

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
//   foreach(vtkAbstractWidget *w, m_widgets2D)
//   {
//     w->EnabledOn();
//     w->Delete();
//   }
  foreach(vtkAbstractWidget *w, m_widgets3D)
  {
    w->EnabledOn();
    w->Delete();
  }
  m_widgets2D.clear();
  m_widgets3D.clear();
}

//-----------------------------------------------------------------------------
vtkAbstractWidget* AdaptiveCountingFrame::createWidget(View3D *view)
{
  CountingFrame3DWidgetAdapter *wa = new CountingFrame3DWidgetAdapter();
  Q_ASSERT(wa);
  wa->SetCountingFrame(m_countingFrame, m_inclusion, m_exclusion);

  m_widgets3D << wa;

  return wa;
}

// //-----------------------------------------------------------------------------
// void AdaptiveCountingFrame::deleteWidget(vtkAbstractWidget* widget)
// {
//   widget->Off();
//   widget->RemoveAllObservers();
// 
//   CountingFrame3DWidgetAdapter *brwa3D = dynamic_cast<CountingFrame3DWidgetAdapter *>(widget);
//   if (brwa3D)
//     m_widgets3D.removeAll(brwa3D);
//   else
//   {
//     CountingFrame2DWidgetAdapter *brwa2D = dynamic_cast<CountingFrame2DWidgetAdapter *>(widget);
//     if (brwa2D)
//       m_widgets2D.removeAll(brwa2D);
//     else
//       Q_ASSERT(false);
//   }
// 
//   widget->Delete();
// }

//-----------------------------------------------------------------------------
SliceWidget* AdaptiveCountingFrame::createSliceWidget(View2D *view)
{
  auto wa = new CountingFrame2DWidgetAdapter();
  Q_ASSERT(wa);
  wa->AddObserver(vtkCommand::EndInteractionEvent, this);
  wa->SetPlane(view->plane());
  wa->SetSlicingStep(m_channel->output()->spacing());
  wa->SetCountingFrame(m_representation, m_inclusion, m_exclusion);

  m_widgets2D << wa;

  return new CountingFrameSliceWidget(wa);
}

//-----------------------------------------------------------------------------
bool AdaptiveCountingFrame::processEvent(vtkRenderWindowInteractor* iren,
                                          long unsigned int event)
{
  foreach(CountingFrame2DWidgetAdapter *wa, m_widgets2D)
  {
    if (wa->GetInteractor() == iren)
      return wa->ProcessEventsHandler(event);
  }
  foreach(CountingFrame3DWidgetAdapter *wa, m_widgets3D)
  {
    if (wa->GetInteractor() == iren)
      return wa->ProcessEventsHandler(event);
  }

  return false;
}


//-----------------------------------------------------------------------------
void AdaptiveCountingFrame::setEnabled(bool enable)
{
  Q_ASSERT(false);
}

#include <QDebug>

//-----------------------------------------------------------------------------
void AdaptiveCountingFrame::updateCountingFrameImplementation()
{
  auto volume  = volumetricData(m_channel->output());
  auto origin  = volume->origin();
  auto spacing = volume->spacing();

  m_inclusionVolume = 0;

  auto edgesExtension = retrieveOrCreateExtension<ChannelEdges>(m_channel);

  vtkSmartPointer<vtkPolyData> margins = edgesExtension->channelEdges();
  Q_ASSERT(margins.GetPointer());

  m_totalVolume = 0;

  int inSliceOffset = frontOffset() / spacing[2];
  int exSliceOffset = -backOffset() / spacing[2];

  int    extent[6];
  double bounds[6];
  margins->GetBounds(bounds);
  extent[4] = bounds[4] / spacing[2];
  extent[5] = bounds[5] / spacing[2];

  int upperSlice = extent[4] + inSliceOffset;
  upperSlice = std::max(upperSlice, extent[4]);
  upperSlice = std::min(upperSlice, extent[5]);

  int lowerSlice = extent[5] + exSliceOffset;
  lowerSlice = std::max(lowerSlice, extent[4]);
  lowerSlice = std::min(lowerSlice, extent[5]);

  // upper and lower refer to Espina's orientation
  Q_ASSERT(upperSlice <= lowerSlice);

  m_countingFrame = vtkSmartPointer<vtkPolyData>::New();
  m_representation = margins;

  vtkSmartPointer<vtkPoints>    regionVertex = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> faces        = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkIntArray>  faceData     = vtkSmartPointer<vtkIntArray>::New();

  for (int slice = extent[4]; slice < extent[5]; slice++)
  {
    double LB[3], LT[3], RT[3], RB[3];
    margins->GetPoint(4*slice+0, LB);
    margins->GetPoint(4*slice+1, LT);
    margins->GetPoint(4*slice+2, RT);
    margins->GetPoint(4*slice+3, RB);

    Bounds sliceBounds{LT[0], RT[0], LT[1], LB[1], 0, 0};
    m_totalVolume += equivalentVolume(sliceBounds);
  }

  for (int slice = upperSlice; slice <= lowerSlice; slice++)
  {
    vtkIdType cell[4];
    vtkIdType lastCell[4];

    double LB[3], LT[3], RT[3], RB[3];

    margins->GetPoint(4*slice+0, LB);
    LB[0] += leftOffset();
    LB[1] -= bottomOffset();
    cell[0] = regionVertex->InsertNextPoint(LB);

    margins->GetPoint(4*slice+1, LT);
    LT[0] += leftOffset();
    LT[1] += topOffset();
    cell[1] = regionVertex->InsertNextPoint(LT);

    margins->GetPoint(4*slice+2, RT);
    RT[0] -= rightOffset();
    RT[1] += topOffset();
    cell[2] = regionVertex->InsertNextPoint(RT);

    margins->GetPoint(4*slice+3, RB);
    RB[0] -= rightOffset();
    RB[1] -= bottomOffset();
    cell[3] = regionVertex->InsertNextPoint(RB);
    if (slice == upperSlice)
    {
      // Upper Inclusion Face
      faces->InsertNextCell(4, cell);
      faceData->InsertNextValue(INCLUSION_FACE);
    } else if (slice == lowerSlice)
    {
      // Lower Inclusion Face
      faces->InsertNextCell(4, cell);
      faceData->InsertNextValue(EXCLUSION_FACE);
    } else
    {
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
    if (slice < lowerSlice - 1)
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
}
