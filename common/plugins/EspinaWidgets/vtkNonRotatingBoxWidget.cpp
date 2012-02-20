/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkNonRotatingBoxWidget.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkNonRotatingBoxWidget.h"
#include "vtkBoxRepresentation.h"
#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkObjectFactory.h"
#include "vtkWidgetEventTranslator.h"
#include "vtkWidgetCallbackMapper.h" 
#include "vtkEvent.h"
#include "vtkWidgetEvent.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkNonRotatingBoxRepresentation.h"


vtkStandardNewMacro(vtkNonRotatingBoxWidget);

//----------------------------------------------------------------------------
vtkNonRotatingBoxWidget::vtkNonRotatingBoxWidget()
{
  this->WidgetState = vtkNonRotatingBoxWidget::Start;
  this->ManagesCursor = 1;

  this->TranslationEnabled = 1;
  this->ScalingEnabled = 1;
  this->RotationEnabled = 1;
  
  this->InvertXCursor = 0;
  this->InvertYCursor = 0;
  this->InvertZCursor = 0;

  // Define widget events
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Select,
                                          this, vtkNonRotatingBoxWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::EndSelect,
                                          this, vtkNonRotatingBoxWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonPressEvent,
                                          vtkWidgetEvent::Translate,
                                          this, vtkNonRotatingBoxWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonReleaseEvent,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkNonRotatingBoxWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::ControlModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Translate,
                                          this, vtkNonRotatingBoxWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                            vtkEvent::ControlModifier,
                                            0, 0, NULL,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkNonRotatingBoxWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::ShiftModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Translate,
                                          this, vtkNonRotatingBoxWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                            vtkEvent::ShiftModifier,
                                            0, 0, NULL,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkNonRotatingBoxWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent,
                                          vtkWidgetEvent::Scale,
                                          this, vtkNonRotatingBoxWidget::ScaleAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
                                          vtkWidgetEvent::EndScale,
                                          this, vtkNonRotatingBoxWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
                                          vtkWidgetEvent::Move,
                                          this, vtkNonRotatingBoxWidget::MoveAction);
}

//----------------------------------------------------------------------------
vtkNonRotatingBoxWidget::~vtkNonRotatingBoxWidget()
{  
}

//----------------------------------------------------------------------
void vtkNonRotatingBoxWidget::SelectAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkNonRotatingBoxWidget *self = reinterpret_cast<vtkNonRotatingBoxWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  
  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkNonRotatingBoxWidget::Start;
    return;
    }
  
  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == vtkBoxRepresentation::Outside )
    {
    return;
    }
  
  // We are definitely selected
  self->WidgetState = vtkNonRotatingBoxWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);
  
  // The SetInteractionState has the side effect of highlighting the widget
  reinterpret_cast<vtkBoxRepresentation*>(self->WidgetRep)->
    SetInteractionState(interactionState);
 
  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkNonRotatingBoxWidget::TranslateAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkNonRotatingBoxWidget *self = reinterpret_cast<vtkNonRotatingBoxWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  
  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkNonRotatingBoxWidget::Start;
    return;
    }
  
  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == vtkBoxRepresentation::Outside )
    {
    return;
    }
  
  // We are definitely selected
  self->WidgetState = vtkNonRotatingBoxWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);
  reinterpret_cast<vtkBoxRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkBoxRepresentation::Translating);
  
  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkNonRotatingBoxWidget::ScaleAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkNonRotatingBoxWidget *self = reinterpret_cast<vtkNonRotatingBoxWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  
  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkNonRotatingBoxWidget::Start;
    return;
    }
  
  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == vtkBoxRepresentation::Outside )
    {
    return;
    }
  
  // We are definitely selected
  self->WidgetState = vtkNonRotatingBoxWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);
  reinterpret_cast<vtkBoxRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkBoxRepresentation::Scaling);

  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkNonRotatingBoxWidget::MoveAction(vtkAbstractWidget *w)
{
  vtkNonRotatingBoxWidget *self = reinterpret_cast<vtkNonRotatingBoxWidget*>(w);

  // compute some info we need for all cases
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  
  // See whether we're active
  if ( self->WidgetState == vtkNonRotatingBoxWidget::Start )
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
void vtkNonRotatingBoxWidget::SetCursor(int state)
{
    switch (state)
    {
      case vtkNonRotatingBoxRepresentation::Translating:
	this->RequestCursorShape(VTK_CURSOR_SIZEALL);
	break;
      case vtkNonRotatingBoxRepresentation::MoveF0:
      case vtkNonRotatingBoxRepresentation::MoveF1:
	if (this->InvertXCursor)
	  this->RequestCursorShape(VTK_CURSOR_SIZENS);
	else
	  this->RequestCursorShape(VTK_CURSOR_SIZEWE);
	break;
      case vtkNonRotatingBoxRepresentation::MoveF2:
      case vtkNonRotatingBoxRepresentation::MoveF3:
	if (this->InvertYCursor)
	  this->RequestCursorShape(VTK_CURSOR_SIZEWE);
	else
	  this->RequestCursorShape(VTK_CURSOR_SIZENS);
	break;
      case vtkNonRotatingBoxRepresentation::MoveF4:
      case vtkNonRotatingBoxRepresentation::MoveF5:
	if (this->InvertZCursor)
	  this->RequestCursorShape(VTK_CURSOR_SIZEWE);
	else
	  this->RequestCursorShape(VTK_CURSOR_SIZENS);
	break;
      case vtkNonRotatingBoxRepresentation::Outside:
	this->RequestCursorShape(VTK_CURSOR_DEFAULT);
	break;
      default:
	this->RequestCursorShape(VTK_CURSOR_DEFAULT);
    };
}


//----------------------------------------------------------------------
void vtkNonRotatingBoxWidget::EndSelectAction(vtkAbstractWidget *w)
{
  vtkNonRotatingBoxWidget *self = reinterpret_cast<vtkNonRotatingBoxWidget*>(w);
  if ( self->WidgetState == vtkNonRotatingBoxWidget::Start )
    {
    return;
    }
  
  // Return state to not active
  self->WidgetState = vtkNonRotatingBoxWidget::Start;
  reinterpret_cast<vtkBoxRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkBoxRepresentation::Outside);
  self->ReleaseFocus();

  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  self->Render();
  self->SetCursor(9999);
}

//----------------------------------------------------------------------
void vtkNonRotatingBoxWidget::CreateDefaultRepresentation()
{
  if ( ! this->WidgetRep )
    {
    this->WidgetRep = vtkBoxRepresentation::New();
    }
}

//----------------------------------------------------------------------------
void vtkNonRotatingBoxWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Translation Enabled: " << (this->TranslationEnabled ? "On\n" : "Off\n");
  os << indent << "Scaling Enabled: " << (this->ScalingEnabled ? "On\n" : "Off\n");
  os << indent << "Rotation Enabled: " << (this->RotationEnabled ? "On\n" : "Off\n");
}


