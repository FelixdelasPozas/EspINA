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


#include "CountingFrames/RectangularCountingFrame.h"

#include "Extensions/CountingFrameChannelExtension.h"
#include "CountingFrames/vtkCountingFrameSliceWidget.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include "vtkCountingFrame3DWidget.h"
#include <Core/Model/Channel.h>


const QString RectangularCountingFrame::ID       = "RectangularCountingFrame";
const QString RectangularCountingFrame::ID_1_2_5 = "RectangularBoundingRegion";

//-----------------------------------------------------------------------------
RectangularCountingFrame::RectangularCountingFrame(Id id,
                                                     CountingFrameChannelExtension *channelExt,
                                                     Nm borders[6],
                                                     Nm inclusion[3],
                                                     Nm exclusion[3],
                                                     ViewManager *vm)
: CountingFrame(id, channelExt, inclusion, exclusion, vm)
{
  memcpy(m_borders, borders, 6*sizeof(Nm));

  updateCountingFrameImplementation();
}


//-----------------------------------------------------------------------------
RectangularCountingFrame::~RectangularCountingFrame()
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
QVariant RectangularCountingFrame::data(int role) const
{
  if (role == Qt::DisplayRole)
    return tr("%1 - Region %2: Rectangular")
             .arg(m_channelExt->channel()->data().toString())
             .arg(m_id);

  return CountingFrame::data(role);
}

//-----------------------------------------------------------------------------
QString RectangularCountingFrame::serialize() const
{
  return QString("%1=%2,%3,%4,%5,%6,%7")
         .arg(ID)
         .arg(left(),0,'f',2).arg(top(),0,'f',2).arg(upper(),0,'f',2)
         .arg(right(),0,'f',2).arg(bottom(),0,'f',2).arg(lower(),0,'f',2);
}



//-----------------------------------------------------------------------------
vtkAbstractWidget *RectangularCountingFrame::createWidget()
{
  CountingFrame3DWidgetAdapter *wa = new CountingFrame3DWidgetAdapter();
  Q_ASSERT(wa);
  wa->SetCountingFrame(m_boundingRegion, m_inclusion, m_exclusion);

  m_widgets3D << wa;

  return wa;
}

//-----------------------------------------------------------------------------
void RectangularCountingFrame::deleteWidget(vtkAbstractWidget* widget)
{
  widget->Off();
  widget->RemoveAllObservers();

  CountingFrame3DWidgetAdapter *brwa3D = dynamic_cast<CountingFrame3DWidgetAdapter *>(widget);
  if (brwa3D)
    m_widgets3D.removeAll(brwa3D);
  else
  {
    CountingFrame2DWidgetAdapter *brwa2D = dynamic_cast<CountingFrame2DWidgetAdapter *>(widget);
    if (brwa2D)
      m_widgets2D.removeAll(brwa2D);
    else
      Q_ASSERT(false);
  }

  widget->Delete();
}

//-----------------------------------------------------------------------------
SliceWidget* RectangularCountingFrame::createSliceWidget(PlaneType plane)
{
  Channel *channel = m_channelExt->channel();
  double spacing[3];
  channel->volume()->spacing(spacing);

  CountingFrame2DWidgetAdapter *wa = new CountingFrame2DWidgetAdapter();
  Q_ASSERT(wa);
  wa->AddObserver(vtkCommand::EndInteractionEvent, this);
  wa->SetPlane(plane);
  wa->SetSlicingStep(spacing);
  wa->SetCountingFrame(m_representation, m_inclusion, m_exclusion);

  m_widgets2D << wa;

  return new CountingFrameSliceWidget(wa);
}

//-----------------------------------------------------------------------------
bool RectangularCountingFrame::processEvent(vtkRenderWindowInteractor* iren,
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
void RectangularCountingFrame::setEnabled(bool enable)
{
  Q_ASSERT(false);
}


#include <QDebug>

//-----------------------------------------------------------------------------
void RectangularCountingFrame::updateCountingFrameImplementation()
{

  Nm Left   = m_borders[0] + m_inclusion[0];
  Nm Top    = m_borders[2] + m_inclusion[1];
  Nm Upper  = m_borders[4] + m_inclusion[2];
  Nm Right  = m_borders[1] - m_exclusion[0];
  Nm Bottom = m_borders[3] - m_exclusion[1];
  Nm Lower  = m_borders[5] - m_exclusion[2];

  m_boundingRegion = createRectangularRegion(Left, Top, Upper,
                                             Right, Bottom, Lower);

  m_representation = createRectangularRegion(m_borders[0], m_borders[2], m_borders[4],
                                             m_borders[1], m_borders[3], m_borders[5]);

  m_totalVolume = (m_borders[1]-m_borders[0]+1)*
                  (m_borders[3]-m_borders[2]+1)*
                  (m_borders[5]-m_borders[4]+1);
  m_inclusionVolume = (Right-Left)*(Top-Bottom)*(Upper-Lower);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> RectangularCountingFrame::createRectangularRegion(Nm left,
                                                                                Nm top,
                                                                                Nm upper,
                                                                                Nm right,
                                                                                Nm bottom,
                                                                                Nm lower)
{
  vtkSmartPointer<vtkPolyData> region = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> vertex = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> faces = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkIntArray> faceData = vtkSmartPointer<vtkIntArray>::New();

  vtkIdType upperFace[4], leftFace[4], topFace[4];
  vtkIdType lowerFace[4], rightFace[4], bottomFace[4];

    // Upper Inclusion Face
  upperFace[0] = vertex->InsertNextPoint(left,  bottom, upper);
  upperFace[1] = vertex->InsertNextPoint(left,  top,    upper);
  upperFace[2] = vertex->InsertNextPoint(right, top,    upper);
  upperFace[3] = vertex->InsertNextPoint(right, bottom, upper);
  faces->InsertNextCell(4, upperFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Lower Exclusion Face
  lowerFace[0] = vertex->InsertNextPoint(left,  bottom, lower);
  lowerFace[1] = vertex->InsertNextPoint(left,  top,    lower);
  lowerFace[2] = vertex->InsertNextPoint(right, top,    lower);
  lowerFace[3] = vertex->InsertNextPoint(right, bottom, lower);
  faces->InsertNextCell(4, lowerFace);
  faceData->InsertNextValue(EXCLUSION_FACE);

  // Left Inclusion Face
  leftFace[0] = upperFace[0];
  leftFace[1] = upperFace[1];
  leftFace[2] = lowerFace[1];
  leftFace[3] = lowerFace[0];
  faces->InsertNextCell(4, leftFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Right Exclusion Face
  rightFace[0] = upperFace[2];
  rightFace[1] = upperFace[3];
  rightFace[2] = lowerFace[3];
  rightFace[3] = lowerFace[2];
  faces->InsertNextCell(4, rightFace);
  faceData->InsertNextValue(EXCLUSION_FACE);

  // Top Inclusion Face
  topFace[0] = upperFace[1];
  topFace[1] = upperFace[2];
  topFace[2] = lowerFace[2];
  topFace[3] = lowerFace[1];
  faces->InsertNextCell(4, topFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Bottom Exclusion Face
  bottomFace[0] = upperFace[3];
  bottomFace[1] = upperFace[0];
  bottomFace[2] = lowerFace[0];
  bottomFace[3] = lowerFace[3];
  faces->InsertNextCell(4, bottomFace);
  faceData->InsertNextValue(EXCLUSION_FACE);

  region->SetPoints(vertex);
  region->SetPolys(faces);
  vtkCellData *data = region->GetCellData();
  data->SetScalars(faceData);
  data->GetScalars()->SetName("Type");

  return region;
}
