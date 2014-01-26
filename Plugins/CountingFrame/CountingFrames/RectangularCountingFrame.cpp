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

#include "Extensions/CountingFrameExtension.h"
#include "CountingFrames/vtkCountingFrameSliceWidget.h"
#include "CountingFrames/vtkCountingFrame3DWidget.h"

#include <Core/Analysis/Channel.h>
#include <GUI/View/View2D.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkPolyData.h>

#include <vtkSmartPointer.h>


using namespace EspINA;
using namespace EspINA::CF;

//-----------------------------------------------------------------------------
RectangularCountingFrame::RectangularCountingFrame(CountingFrameExtension *channelExt,
                                                   const Bounds &bounds,
                                                   Nm inclusion[3],
                                                   Nm exclusion[3])
: CountingFrame(channelExt, inclusion, exclusion)
, m_bounds(bounds)
{
  updateCountingFrameImplementation();
}


//-----------------------------------------------------------------------------
RectangularCountingFrame::~RectangularCountingFrame()
{
  // 2D widgets have already been deleted by 2D Views
//   foreach(vtkAbstractWidget *w, m_widgets2D)
//   {
//     w->EnabledOff();
//     w->Delete();
//   }
  foreach(vtkAbstractWidget *w, m_widgets3D)
  {
    w->EnabledOff();
    w->Delete();
  }

  m_widgets2D.clear();
  m_widgets3D.clear();
}

//-----------------------------------------------------------------------------
QVariant RectangularCountingFrame::data(int role) const
{
  if (role == Qt::DisplayRole)
    return tr("%1 - CF %2: Rectangular")
             .arg(m_extension->channel()->name())
             .arg(m_id);

  return CountingFrame::data(role);
}

//-----------------------------------------------------------------------------
vtkAbstractWidget *RectangularCountingFrame::create3DWidget(View3D *view)
{
  CountingFrame3DWidgetAdapter *wa = new CountingFrame3DWidgetAdapter();
  Q_ASSERT(wa);
  wa->SetCountingFrame(m_countingFrame, m_inclusion, m_exclusion);

  m_widgets3D << wa;

  return wa;
}

// //-----------------------------------------------------------------------------
// void RectangularCountingFrame::deleteWidget(vtkAbstractWidget* widget)
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
SliceWidget* RectangularCountingFrame::createSliceWidget(View2D *view)
{
  auto wa = new CountingFrame2DWidgetAdapter();
  Q_ASSERT(wa);
  wa->AddObserver(vtkCommand::EndInteractionEvent, this);
  wa->SetPlane(view->plane());
  wa->SetSlicingStep(m_extension->channel()->output()->spacing());
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

  Nm Left   = m_bounds[0] + m_inclusion[0];
  Nm Top    = m_bounds[2] + m_inclusion[1];
  Nm Front  = m_bounds[4] + m_inclusion[2];
  Nm Right  = m_bounds[1] - m_exclusion[0];
  Nm Bottom = m_bounds[3] - m_exclusion[1];
  Nm Back   = m_bounds[5] - m_exclusion[2];

  m_countingFrame = createRectangularRegion(Left, Top, Front,
                                            Right, Bottom, Back);

  m_representation = createRectangularRegion(m_bounds[0], m_bounds[2], m_bounds[4],
                                             m_bounds[1], m_bounds[3], m_bounds[5]);

  m_totalVolume = (m_bounds[1]-m_bounds[0]+1)*
                  (m_bounds[3]-m_bounds[2]+1)*
                  (m_bounds[5]-m_bounds[4]+1);

  m_inclusionVolume = (Right-Left)*(Top-Bottom)*(Front-Back);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> RectangularCountingFrame::createRectangularRegion(Nm left,  Nm top,    Nm front,
                                                                               Nm right, Nm bottom, Nm back)
{
  vtkSmartPointer<vtkPolyData>  region   = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints>    vertex   = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> faces    = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkIntArray>  faceData = vtkSmartPointer<vtkIntArray>::New();

  vtkIdType frontFace[4], leftFace[4] , topFace[4];
  vtkIdType backFace[4] , rightFace[4], bottomFace[4];

    // Front Inclusion Face
  frontFace[0] = vertex->InsertNextPoint(left,  bottom, front );
  frontFace[1] = vertex->InsertNextPoint(left,  top,    front );
  frontFace[2] = vertex->InsertNextPoint(right, top,    front );
  frontFace[3] = vertex->InsertNextPoint(right, bottom, front );
  faces->InsertNextCell(4, frontFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Back Exclusion Face
  backFace[0] = vertex->InsertNextPoint(left,  bottom, back);
  backFace[1] = vertex->InsertNextPoint(left,  top,    back);
  backFace[2] = vertex->InsertNextPoint(right, top,    back);
  backFace[3] = vertex->InsertNextPoint(right, bottom, back);
  faces->InsertNextCell(4, backFace);
  faceData->InsertNextValue(EXCLUSION_FACE);

  // Left Inclusion Face
  leftFace[0] = frontFace[0];
  leftFace[1] = frontFace[1];
  leftFace[2] = backFace[1];
  leftFace[3] = backFace[0];
  faces->InsertNextCell(4, leftFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Right Exclusion Face
  rightFace[0] = frontFace[2];
  rightFace[1] = frontFace[3];
  rightFace[2] = backFace[3];
  rightFace[3] = backFace[2];
  faces->InsertNextCell(4, rightFace);
  faceData->InsertNextValue(EXCLUSION_FACE);

  // Top Inclusion Face
  topFace[0] = frontFace[1];
  topFace[1] = frontFace[2];
  topFace[2] = backFace[2];
  topFace[3] = backFace[1];
  faces->InsertNextCell(4, topFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Bottom Exclusion Face
  bottomFace[0] = frontFace[3];
  bottomFace[1] = frontFace[0];
  bottomFace[2] = backFace[0];
  bottomFace[3] = backFace[3];
  faces->InsertNextCell(4, bottomFace);
  faceData->InsertNextValue(EXCLUSION_FACE);

  region->SetPoints(vertex);
  region->SetPolys(faces);
  vtkCellData *data = region->GetCellData();
  data->SetScalars(faceData);
  data->GetScalars()->SetName("Type");

  return region;
}
