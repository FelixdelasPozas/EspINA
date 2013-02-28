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

#include <Core/Model/Channel.h>
#include "vtkCountingFrameSliceWidget.h"
#include <Core/Extensions/EdgeDistances/AdaptiveEdges.h>
#include "Extensions/CountingFrameExtension.h"

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include "vtkCountingFrame3DWidget.h"
#include <GUI/QtWidget/SliceView.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
const QString AdaptiveCountingFrame::ID       = "AdaptiveCountingFrame";
const QString AdaptiveCountingFrame::ID_1_2_5 = "AdaptiveBoundingRegion";

//-----------------------------------------------------------------------------
AdaptiveCountingFrame::AdaptiveCountingFrame(Id id,
                                             CountingFrameExtension *channelExt,
                                             Nm inclusion[3],
                                             Nm exclusion[3],
                                             ViewManager *vm)
: CountingFrame(id, channelExt, inclusion, exclusion, vm)
, m_channel(channelExt->channel())
{
  updateCountingFrame();
}

// TODO: Memory leak
//-----------------------------------------------------------------------------
AdaptiveCountingFrame::~AdaptiveCountingFrame()
{
  m_channelExt->deleteCountingFrame(this);
  foreach(vtkAbstractWidget *w, m_widgets2D)
  {
    w->EnabledOn();
    w->Delete();
  }
  foreach(vtkAbstractWidget *w, m_widgets3D)
  {
    w->EnabledOn();
    w->Delete();
  }
  m_widgets2D.clear();
  m_widgets3D.clear();
}

//-----------------------------------------------------------------------------
QVariant AdaptiveCountingFrame::data(int role) const
{
  if (role == Qt::DisplayRole)
    return tr("%1 - CF %2: Adaptive")
             .arg(m_channelExt->channel()->data().toString())
             .arg(m_id);

  return CountingFrame::data(role);
}

//-----------------------------------------------------------------------------
QString AdaptiveCountingFrame::serialize() const
{
  return QString("%1=%2,%3,%4,%5,%6,%7")
         .arg(ID)
         .arg(left(),0,'f',2).arg(top(),0,'f',2).arg(upper(),0,'f',2)
         .arg(right(),0,'f',2).arg(bottom(),0,'f',2).arg(lower(),0,'f',2);
}

//-----------------------------------------------------------------------------
vtkAbstractWidget* AdaptiveCountingFrame::create3DWidget(VolumeView* view)
{
  CountingFrame3DWidgetAdapter *wa = new CountingFrame3DWidgetAdapter();
  Q_ASSERT(wa);
  wa->SetCountingFrame(m_boundingRegion, m_inclusion, m_exclusion);

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
SliceWidget* AdaptiveCountingFrame::createSliceWidget(SliceView *view)
{
  Channel *channel = m_channelExt->channel();
  double spacing[3];
  channel->volume()->spacing(spacing);

  CountingFrame2DWidgetAdapter *wa = new CountingFrame2DWidgetAdapter();
  Q_ASSERT(wa);
  wa->AddObserver(vtkCommand::EndInteractionEvent, this);
  wa->SetPlane(view->plane());
  wa->SetSlicingStep(spacing);
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
  double spacing[3];
  m_channel->volume()->spacing(spacing);
  int extent[6];
  m_channel->volume()->extent(extent);

  m_inclusionVolume = 0;

  AdaptiveEdgesPtr edgesExtension = NULL;
  Channel::ExtensionPtr extension = m_channel->extension(AdaptiveEdgesID);
  if (extension)
  {
    edgesExtension = dynamic_cast<AdaptiveEdgesPtr>(extension);
  }
  else
  {
    edgesExtension = new AdaptiveEdges(true);
    m_channel->addExtension(edgesExtension);
  }
  Q_ASSERT(edgesExtension);
  m_totalVolume = edgesExtension->computedVolume();

  vtkSmartPointer<vtkPolyData> margins = edgesExtension->channelEdges();
  Q_ASSERT(margins.GetPointer());

  int inSliceOffset = upperOffset() / spacing[2];
  int exSliceOffset = lowerOffset() / spacing[2];

  int upperSlice = extent[4] + inSliceOffset;
  upperSlice = std::max(upperSlice, extent[4]);
  upperSlice = std::min(upperSlice, extent[5]);

  int lowerSlice = extent[5] + exSliceOffset;
  lowerSlice = std::max(lowerSlice, extent[4]);
  lowerSlice = std::min(lowerSlice, extent[5]);

  // upper and lower refer to Espina's orientation
  Q_ASSERT(upperSlice <= lowerSlice);

  m_boundingRegion = vtkSmartPointer<vtkPolyData>::New();
  m_representation = margins;

  vtkSmartPointer<vtkPoints> regionVertex = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> faces = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkIntArray> faceData = vtkSmartPointer<vtkIntArray>::New();

  for (int slice = upperSlice; slice <= lowerSlice; slice++)
  {
    vtkIdType cell[4];
    vtkIdType lastCell[4];

    double LB[3], LT[3], RT[3], RB[3];

    margins->GetPoint(4*slice+0, LB);
    applyOffset(LB[0], leftOffset());
    applyOffset(LB[1], bottomOffset());
    applyOffset(LB[2], 0);
    cell[0] = regionVertex->InsertNextPoint(LB);

    margins->GetPoint(4*slice+1, LT);
    applyOffset(LT[0], leftOffset());
    applyOffset(LT[1], topOffset());
    applyOffset(LT[2], 0);
    cell[1] = regionVertex->InsertNextPoint(LT);

    margins->GetPoint(4*slice+2, RT);
    applyOffset(RT[0], rightOffset());
    applyOffset(RT[1], topOffset());
    applyOffset(RT[2], 0);
    cell[2] = regionVertex->InsertNextPoint(RT);

    margins->GetPoint(4*slice+3, RB);
    applyOffset(RB[0], rightOffset());
    applyOffset(RB[1], bottomOffset());
    applyOffset(RB[2], 0);
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
    if (slice != lowerSlice)
    {
      m_inclusionVolume += (RT[0] - LT[0])*(LB[1] - LT[1])*spacing[2];
    }
  }

  m_boundingRegion->SetPoints(regionVertex);
  m_boundingRegion->SetPolys(faces);
  vtkCellData *data = m_boundingRegion->GetCellData();
  data->SetScalars(faceData);
  data->GetScalars()->SetName("Type");
}
