/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkRectangularBoundingRegionWidget.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkRectangularBoundingRegionWidget.h"
#include "vtkRectangularBoundingRegionRepresentation.h"
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
#include "vtkRectangularBoundingRegionRepresentation.h"
#include <vtkPolyDataAlgorithm.h>


vtkStandardNewMacro(vtkRectangularBoundingRegionWidget);

//----------------------------------------------------------------------------
vtkRectangularBoundingRegionWidget::vtkRectangularBoundingRegionWidget()
{
  this->WidgetState = vtkRectangularBoundingRegionWidget::Start;
  this->ManagesCursor = 1;

  this->TranslationEnabled = 1;
  this->ScalingEnabled = 1;
  this->RotationEnabled = 1;
  
  this->InvertXCursor = 0;
  this->InvertYCursor = 0;
  this->InvertZCursor = 0;
  
  memset(InclusionOffset, 0, 3*sizeof(double));
  memset(ExclusionOffset, 0, 3*sizeof(double));
  
  // Define widget events
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Select,
                                          this, vtkRectangularBoundingRegionWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::EndSelect,
                                          this, vtkRectangularBoundingRegionWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonPressEvent,
                                          vtkWidgetEvent::Translate,
                                          this, vtkRectangularBoundingRegionWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonReleaseEvent,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkRectangularBoundingRegionWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::ControlModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Translate,
                                          this, vtkRectangularBoundingRegionWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                            vtkEvent::ControlModifier,
                                            0, 0, NULL,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkRectangularBoundingRegionWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::ShiftModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Translate,
                                          this, vtkRectangularBoundingRegionWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                            vtkEvent::ShiftModifier,
                                            0, 0, NULL,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkRectangularBoundingRegionWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent,
                                          vtkWidgetEvent::Scale,
                                          this, vtkRectangularBoundingRegionWidget::ScaleAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
                                          vtkWidgetEvent::EndScale,
                                          this, vtkRectangularBoundingRegionWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
                                          vtkWidgetEvent::Move,
                                          this, vtkRectangularBoundingRegionWidget::MoveAction);
}

//----------------------------------------------------------------------------
vtkRectangularBoundingRegionWidget::~vtkRectangularBoundingRegionWidget()
{  
}

//----------------------------------------------------------------------
void vtkRectangularBoundingRegionWidget::SelectAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkRectangularBoundingRegionWidget *self = reinterpret_cast<vtkRectangularBoundingRegionWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  
  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkRectangularBoundingRegionWidget::Start;
    return;
    }
  
  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == vtkRectangularBoundingRegionRepresentation::Outside )
    {
    return;
    }
  
  // We are definitely selected
  self->WidgetState = vtkRectangularBoundingRegionWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);
  
  // The SetInteractionState has the side effect of highlighting the widget
  reinterpret_cast<vtkRectangularBoundingRegionRepresentation*>(self->WidgetRep)->
    SetInteractionState(interactionState);
 
  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkRectangularBoundingRegionWidget::TranslateAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkRectangularBoundingRegionWidget *self = reinterpret_cast<vtkRectangularBoundingRegionWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkRectangularBoundingRegionWidget::Start;
    return;
    }
  
  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == vtkRectangularBoundingRegionRepresentation::Outside )
    {
    return;
    }
  
  // We are definitely selected
  self->WidgetState = vtkRectangularBoundingRegionWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);
  reinterpret_cast<vtkRectangularBoundingRegionRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkRectangularBoundingRegionRepresentation::Translating);
  
  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkRectangularBoundingRegionWidget::ScaleAction(vtkAbstractWidget *w)
{
  return; //NOTE: Disabled
  // We are in a static method, cast to ourself
  vtkRectangularBoundingRegionWidget *self = reinterpret_cast<vtkRectangularBoundingRegionWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  
  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkRectangularBoundingRegionWidget::Start;
    return;
    }
  
  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == vtkRectangularBoundingRegionRepresentation::Outside )
    {
    return;
    }
  
  // We are definitely selected
  self->WidgetState = vtkRectangularBoundingRegionWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);
  reinterpret_cast<vtkRectangularBoundingRegionRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkRectangularBoundingRegionRepresentation::Scaling);

  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkRectangularBoundingRegionWidget::MoveAction(vtkAbstractWidget *w)
{
  
  vtkRectangularBoundingRegionWidget *self = reinterpret_cast<vtkRectangularBoundingRegionWidget*>(w);

  // compute some info we need for all cases
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // See whether we're active
  if ( self->WidgetState == vtkRectangularBoundingRegionWidget::Start )
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

  vtkRectangularBoundingRegionRepresentation *rep =
    vtkRectangularBoundingRegionRepresentation::SafeDownCast(self->WidgetRep);
  if (rep)
  {
//     std::cout << "updating offset" << std::endl;
    rep->GetInclusionOffset(self->InclusionOffset);
//     std::cout << "Inclusion Offset: " << self->InclusionOffset[0] << " " << self->InclusionOffset[1]  << " " << self->InclusionOffset[2] << std::endl;
    rep->GetExclusionOffset(self->ExclusionOffset);
//     std::cout << "Exclusion Offset: " << self->ExclusionOffset[0] << " " << self->ExclusionOffset[1]  << " " << self->ExclusionOffset[2] << std::endl;
  }
  // moving something
  self->EventCallbackCommand->SetAbortFlag(1);
  self->InvokeEvent(vtkCommand::InteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkRectangularBoundingRegionWidget::SetCursor(int state)
{
    switch (state)
    {
      case vtkRectangularBoundingRegionRepresentation::Translating:
	this->RequestCursorShape(VTK_CURSOR_SIZEALL);
	break;
      case vtkRectangularBoundingRegionRepresentation::MoveLeft:
      case vtkRectangularBoundingRegionRepresentation::MoveRight:
	if (this->InvertXCursor)
	  this->RequestCursorShape(VTK_CURSOR_SIZENS);
	else
	  this->RequestCursorShape(VTK_CURSOR_SIZEWE);
	break;
      case vtkRectangularBoundingRegionRepresentation::MoveTop:
      case vtkRectangularBoundingRegionRepresentation::MoveBottom:
	if (this->InvertYCursor)
	  this->RequestCursorShape(VTK_CURSOR_SIZEWE);
	else
	  this->RequestCursorShape(VTK_CURSOR_SIZENS);
	break;
      case vtkRectangularBoundingRegionRepresentation::MoveUpper:
      case vtkRectangularBoundingRegionRepresentation::MoveLower:
	if (this->InvertZCursor)
	  this->RequestCursorShape(VTK_CURSOR_SIZEWE);
	else
	  this->RequestCursorShape(VTK_CURSOR_SIZENS);
	break;
      case vtkRectangularBoundingRegionRepresentation::Outside:
	this->RequestCursorShape(VTK_CURSOR_DEFAULT);
	break;
      default:
	this->RequestCursorShape(VTK_CURSOR_DEFAULT);
    };
}


//----------------------------------------------------------------------
void vtkRectangularBoundingRegionWidget::EndSelectAction(vtkAbstractWidget *w)
{
  vtkRectangularBoundingRegionWidget *self = reinterpret_cast<vtkRectangularBoundingRegionWidget*>(w);
  if ( self->WidgetState == vtkRectangularBoundingRegionWidget::Start )
    {
    return;
    }
  
  // Return state to not active
  self->WidgetState = vtkRectangularBoundingRegionWidget::Start;
  reinterpret_cast<vtkRectangularBoundingRegionRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkRectangularBoundingRegionRepresentation::Outside);
  self->ReleaseFocus();

  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  self->Render();
  self->SetCursor(9999);
}

//----------------------------------------------------------------------
void vtkRectangularBoundingRegionWidget::SetViewType(int type)
{
  vtkRectangularBoundingRegionRepresentation *rep =
    reinterpret_cast<vtkRectangularBoundingRegionRepresentation*>(this->WidgetRep);
    rep->SetViewType(type);

  if (0 == type || 2 == type)
    InvertZCursorOn();

  View = type;
}

//----------------------------------------------------------------------
void vtkRectangularBoundingRegionWidget::SetSlice(int slice, double spacing[3])
{
  reinterpret_cast<vtkRectangularBoundingRegionRepresentation*>(this->WidgetRep)->
    SetSlice(slice,spacing);
  Slice = slice;
}

//----------------------------------------------------------------------
void vtkRectangularBoundingRegionWidget::SetRegion(vtkPolyDataAlgorithm *region)
{
  Region = region;
  if (WidgetRep)
  {
    vtkRectangularBoundingRegionRepresentation *rep =
      reinterpret_cast<vtkRectangularBoundingRegionRepresentation*>(this->WidgetRep);
    rep->SetRegion(region);
    rep->reset();
  }
  else
    std::cout << "There is no representation" << std::endl;
}

  
//----------------------------------------------------------------------
void vtkRectangularBoundingRegionWidget::CreateDefaultRepresentation()
{
  if ( ! this->WidgetRep )
    {
    this->WidgetRep = vtkRectangularBoundingRegionRepresentation::New();
    reinterpret_cast<vtkRectangularBoundingRegionRepresentation*>(this->WidgetRep)->SetRegion(Region);
    }
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Translation Enabled: " << (this->TranslationEnabled ? "On\n" : "Off\n");
  os << indent << "Scaling Enabled: " << (this->ScalingEnabled ? "On\n" : "Off\n");
  os << indent << "Rotation Enabled: " << (this->RotationEnabled ? "On\n" : "Off\n");
}


