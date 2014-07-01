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

#include "vtkCountingFrame3DWidget.h"

#include "CountingFrames/vtkCountingFrame3DRepresentation.h"


#include "vtkCallbackCommand.h"
#include "vtkCommand.h"
#include "vtkEvent.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkWidgetCallbackMapper.h" 
#include "vtkWidgetEvent.h"
#include "vtkWidgetEventTranslator.h"
#include <vtkPolyDataAlgorithm.h>

vtkStandardNewMacro(vtkCountingFrame3DWidget);

//----------------------------------------------------------------------------
vtkCountingFrame3DWidget::vtkCountingFrame3DWidget()
: Volume(nullptr)
{
  this->WidgetState = vtkCountingFrame3DWidget::Start;
  this->ManagesCursor = 1;

  memset(InclusionOffset, 0, 3*sizeof(double));
  memset(ExclusionOffset, 0, 3*sizeof(double));

  // Define widget events
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, nullptr,
                                          vtkWidgetEvent::Select,
                                          this, vtkCountingFrame3DWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, nullptr,
                                          vtkWidgetEvent::EndSelect,
                                          this, vtkCountingFrame3DWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonReleaseEvent,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkCountingFrame3DWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                            vtkEvent::ControlModifier,
                                            0, 0, nullptr,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkCountingFrame3DWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                            vtkEvent::ShiftModifier,
                                            0, 0, nullptr,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkCountingFrame3DWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
                                          vtkWidgetEvent::EndScale,
                                          this, vtkCountingFrame3DWidget::EndSelectAction);
}

//----------------------------------------------------------------------------
vtkCountingFrame3DWidget::~vtkCountingFrame3DWidget()
{
}

//----------------------------------------------------------------------
void vtkCountingFrame3DWidget::SelectAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkCountingFrame3DWidget *self = reinterpret_cast<vtkCountingFrame3DWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkCountingFrame3DWidget::Start;
    return;
    }

  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == vtkCountingFrame3DRepresentation::Outside )
    {
    return;
    }

  // We are definitely selected
  self->WidgetState = vtkCountingFrame3DWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);

  // The SetInteractionState has the side effect of highlighting the widget
  reinterpret_cast<vtkCountingFrame3DRepresentation*>(self->WidgetRep)->
    SetInteractionState(interactionState);

  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkCountingFrame3DWidget::SetCursor(int state)
{
  switch (state)
  {
    case vtkCountingFrame3DRepresentation::MoveLeft:
    case vtkCountingFrame3DRepresentation::MoveRight:
      this->RequestCursorShape(VTK_CURSOR_SIZEWE);
      break;
    case vtkCountingFrame3DRepresentation::MoveTop:
    case vtkCountingFrame3DRepresentation::MoveBottom:
      this->RequestCursorShape(VTK_CURSOR_SIZENS);
      break;
    case vtkCountingFrame3DRepresentation::MoveUpper:
    case vtkCountingFrame3DRepresentation::MoveLower:
      this->RequestCursorShape(VTK_CURSOR_SIZENS);
      break;
    case vtkCountingFrame3DRepresentation::Outside:
      this->RequestCursorShape(VTK_CURSOR_DEFAULT);
      break;
    default:
      this->RequestCursorShape(VTK_CURSOR_DEFAULT);
  };
}


//----------------------------------------------------------------------
void vtkCountingFrame3DWidget::EndSelectAction(vtkAbstractWidget *w)
{
  vtkCountingFrame3DWidget *self = reinterpret_cast<vtkCountingFrame3DWidget*>(w);
  if ( self->WidgetState == vtkCountingFrame3DWidget::Start )
    {
    return;
    }

  // Return state to not active
  self->WidgetState = vtkCountingFrame3DWidget::Start;
  reinterpret_cast<vtkCountingFrame3DRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkCountingFrame3DRepresentation::Outside);
  self->ReleaseFocus();

  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent,nullptr);
  self->Render();
  self->SetCursor(9999);
}

//----------------------------------------------------------------------
void vtkCountingFrame3DWidget::SetCountingFrame(vtkSmartPointer< vtkPolyData > region, EspINA::Nm inclusionOffset[3], EspINA::Nm exclusionOffset[3])
{
  if (!this->WidgetRep)
    CreateDefaultRepresentation();

  vtkCountingFrame3DRepresentation *rep = reinterpret_cast<vtkCountingFrame3DRepresentation*>(this->WidgetRep);
  rep->SetCountingFrame(region);
  rep->reset();
}

//----------------------------------------------------------------------
void vtkCountingFrame3DWidget::CreateDefaultRepresentation()
{
  if ( ! this->WidgetRep )
    this->WidgetRep = vtkCountingFrame3DRepresentation::New();
}

//----------------------------------------------------------------------------
void vtkCountingFrame3DWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


