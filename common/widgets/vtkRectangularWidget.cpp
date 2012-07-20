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

#include "vtkRectangularWidget.h"

#include "vtkRectangularRepresentation.h"

#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkEvent.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkRectangularRepresentation.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkWidgetEvent.h>
#include <vtkWidgetEventTranslator.h>


vtkStandardNewMacro(vtkRectangularWidget);

//----------------------------------------------------------------------------
vtkRectangularWidget::vtkRectangularWidget()
{
  this->WidgetState = vtkRectangularWidget::Start;
  this->ManagesCursor = 1;

  this->TranslationEnabled = 1;
  this->ScalingEnabled = 1;

  // Define widget events
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Select,
                                          this, vtkRectangularWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::EndSelect,
                                          this, vtkRectangularWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonPressEvent,
                                          vtkWidgetEvent::Translate,
                                          this, vtkRectangularWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonReleaseEvent,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkRectangularWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::ControlModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Translate,
                                          this, vtkRectangularWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                            vtkEvent::ControlModifier,
                                            0, 0, NULL,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkRectangularWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::ShiftModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Translate,
                                          this, vtkRectangularWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                            vtkEvent::ShiftModifier,
                                            0, 0, NULL,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkRectangularWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent,
                                          vtkWidgetEvent::Scale,
                                          this, vtkRectangularWidget::ScaleAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
                                          vtkWidgetEvent::EndScale,
                                          this, vtkRectangularWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
                                          vtkWidgetEvent::Move,
                                          this, vtkRectangularWidget::MoveAction);
}

//----------------------------------------------------------------------------
vtkRectangularWidget::~vtkRectangularWidget()
{  
}

//----------------------------------------------------------------------
void vtkRectangularWidget::SelectAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkRectangularWidget *self = reinterpret_cast<vtkRectangularWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
  {
    self->WidgetState = vtkRectangularWidget::Start;
    return;
  }

  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == vtkRectangularRepresentation::Outside )
  {
    return;
  }

  // We are definitely selected
  self->WidgetState = vtkRectangularWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);
  
  // The SetInteractionState has the side effect of highlighting the widget
  reinterpret_cast<vtkRectangularRepresentation*>(self->WidgetRep)->
    SetInteractionState(interactionState);
 
  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkRectangularWidget::TranslateAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkRectangularWidget *self = reinterpret_cast<vtkRectangularWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkRectangularWidget::Start;
    return;
    }
  
  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == vtkRectangularRepresentation::Outside )
    {
    return;
    }
  
  // We are definitely selected
  self->WidgetState = vtkRectangularWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);
  reinterpret_cast<vtkRectangularRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkRectangularRepresentation::Translating);
  
  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkRectangularWidget::ScaleAction(vtkAbstractWidget *w)
{
  return; //NOTE: Disabled
  // We are in a static method, cast to ourself
  vtkRectangularWidget *self = reinterpret_cast<vtkRectangularWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  
  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkRectangularWidget::Start;
    return;
    }
  
  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == vtkRectangularRepresentation::Outside )
    {
    return;
    }
  
  // We are definitely selected
  self->WidgetState = vtkRectangularWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);
  reinterpret_cast<vtkRectangularRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkRectangularRepresentation::Scaling);

  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkRectangularWidget::MoveAction(vtkAbstractWidget *w)
{
  vtkRectangularWidget *self = reinterpret_cast<vtkRectangularWidget*>(w);

  // compute some info we need for all cases
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // See whether we're active
  if ( self->WidgetState == vtkRectangularWidget::Start )
  {
    self->WidgetRep->ComputeInteractionState(X, Y);
    int stateAfter = self->WidgetRep->GetInteractionState();
    self->SetCursor(stateAfter);
    return;
  }

  // Okay, adjust the representation
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->WidgetInteraction(e);


  // moving something
  self->EventCallbackCommand->SetAbortFlag(1);
  self->InvokeEvent(vtkCommand::InteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkRectangularWidget::SetCursor(int state)
{
    switch (state)
    {
      case vtkRectangularRepresentation::Translating:
	this->RequestCursorShape(VTK_CURSOR_SIZEALL);
	break;
      case vtkRectangularRepresentation::MoveLeft:
      case vtkRectangularRepresentation::MoveRight:
// 	if (this->Plane == vtkSliceView::CORONAL)
// 	  this->RequestCursorShape(VTK_CURSOR_SIZENS);
// 	else
	  this->RequestCursorShape(VTK_CURSOR_SIZEWE);
	break;
      case vtkRectangularRepresentation::MoveTop:
      case vtkRectangularRepresentation::MoveBottom:
// 	if (this->Plane == vtkSliceView::CORONAL)
// 	  this->RequestCursorShape(VTK_CURSOR_SIZEWE);
// 	else
	  this->RequestCursorShape(VTK_CURSOR_SIZENS);
	break;
      case vtkRectangularRepresentation::Outside:
	this->RequestCursorShape(VTK_CURSOR_DEFAULT);
	break;
      default:
	this->RequestCursorShape(VTK_CURSOR_DEFAULT);
    };
}


//----------------------------------------------------------------------
void vtkRectangularWidget::EndSelectAction(vtkAbstractWidget *w)
{
  vtkRectangularWidget *self = reinterpret_cast<vtkRectangularWidget*>(w);
  if ( self->WidgetState == vtkRectangularWidget::Start )
    {
    return;
    }
  
  // Return state to not active
  self->WidgetState = vtkRectangularWidget::Start;
  reinterpret_cast<vtkRectangularRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkRectangularRepresentation::Outside);
  self->ReleaseFocus();

  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  self->Render();
  self->SetCursor(9999);
}

//----------------------------------------------------------------------
void vtkRectangularWidget::SetPlane(PlaneType plane)
{
  if (!this->WidgetRep)
    this->CreateDefaultRepresentation();

  vtkRectangularRepresentation *rep =
    reinterpret_cast<vtkRectangularRepresentation*>(this->WidgetRep);
    rep->SetPlane(plane);

  Plane = plane;
}

//----------------------------------------------------------------------
void vtkRectangularWidget::CreateDefaultRepresentation()
{
  if ( ! this->WidgetRep )
    {
    this->WidgetRep = vtkRectangularRepresentation::New();
//     reinterpret_cast<vtkRectangularRepresentation*>(this->WidgetRep)->SetRegion(Region);
    }
}

//----------------------------------------------------------------------------
void vtkRectangularWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Translation Enabled: " << (this->TranslationEnabled ? "On\n" : "Off\n");
  os << indent << "Scaling Enabled: " << (this->ScalingEnabled ? "On\n" : "Off\n");
}