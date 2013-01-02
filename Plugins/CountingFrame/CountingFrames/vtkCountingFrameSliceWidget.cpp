/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkCountingFrameSliceWidget.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCountingFrameSliceWidget.h"

#include "vtkCountingFrameSliceRepresentation.h"
#include "vtkCountingFrameAxialSliceRepresentation.h"
#include "vtkCountingFrameCoronalSliceRepresentation.h"
#include "vtkCountingFrameSagittalSliceRepresentation.h"

#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkObjectFactory.h"
#include <vtkMath.h>
#include "vtkWidgetEventTranslator.h"
#include "vtkWidgetCallbackMapper.h"
#include "vtkEvent.h"
#include "vtkWidgetEvent.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include <vtkPolyData.h>


vtkStandardNewMacro(vtkCountingFrameSliceWidget);

typedef vtkCountingFrameSliceRepresentation         SliceRepresentation;
typedef vtkCountingFrameAxialSliceRepresentation    AxialSliceRepresentation;
typedef vtkCountingFrameCoronalSliceRepresentation  CoronalSliceRepresentation;
typedef vtkCountingFrameSagittalSliceRepresentation SagittalSliceRepresentation;

//----------------------------------------------------------------------------
vtkCountingFrameSliceWidget::vtkCountingFrameSliceWidget()
: Plane(EspINA::AXIAL)
, Slice(0)
{
  this->WidgetState = vtkCountingFrameSliceWidget::Start;
  this->ManagesCursor = 1;

  memset(InclusionOffset, 0, 3*sizeof(double));
  memset(ExclusionOffset, 0, 3*sizeof(double));

  // Define widget events
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Select,
                                          this, vtkCountingFrameSliceWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::EndSelect,
                                          this, vtkCountingFrameSliceWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonPressEvent,
                                          vtkWidgetEvent::Translate,
                                          this, vtkCountingFrameSliceWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonReleaseEvent,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkCountingFrameSliceWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::ControlModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Translate,
                                          this, vtkCountingFrameSliceWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                            vtkEvent::ControlModifier,
                                            0, 0, NULL,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkCountingFrameSliceWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::ShiftModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Translate,
                                          this, vtkCountingFrameSliceWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                            vtkEvent::ShiftModifier,
                                            0, 0, NULL,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkCountingFrameSliceWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
                                          vtkWidgetEvent::EndScale,
                                          this, vtkCountingFrameSliceWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
                                          vtkWidgetEvent::Move,
                                          this, vtkCountingFrameSliceWidget::MoveAction);
}

//----------------------------------------------------------------------------
vtkCountingFrameSliceWidget::~vtkCountingFrameSliceWidget()
{
}

//----------------------------------------------------------------------
void vtkCountingFrameSliceWidget::SelectAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkCountingFrameSliceWidget *self = reinterpret_cast<vtkCountingFrameSliceWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkCountingFrameSliceWidget::Start;
    return;
    }

  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == SliceRepresentation::Outside )
    {
    return;
    }

  // We are definitely selected
  self->WidgetState = vtkCountingFrameSliceWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);

  // The SetInteractionState has the side effect of highlighting the widget
  reinterpret_cast<SliceRepresentation*>(self->WidgetRep)->
    SetInteractionState(interactionState);

  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkCountingFrameSliceWidget::TranslateAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkCountingFrameSliceWidget *self = reinterpret_cast<vtkCountingFrameSliceWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkCountingFrameSliceWidget::Start;
    return;
    }

  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == SliceRepresentation::Outside )
    {
    return;
    }

  // We are definitely selected
  self->WidgetState = vtkCountingFrameSliceWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);
  reinterpret_cast<SliceRepresentation*>(self->WidgetRep)->
    SetInteractionState(SliceRepresentation::Translating);

  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkCountingFrameSliceWidget::MoveAction(vtkAbstractWidget *w)
{
  vtkCountingFrameSliceWidget *self = reinterpret_cast<vtkCountingFrameSliceWidget*>(w);
  Q_ASSERT(self->WidgetRep);
  // compute some info we need for all cases
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // See whether we're active
  if ( self->WidgetState == vtkCountingFrameSliceWidget::Start )
  {
    self->WidgetRep->ComputeInteractionState(X, Y);
    int stateAfter = self->WidgetRep->GetInteractionState();
    self->SetCursor(stateAfter);
    if (stateAfter != SliceRepresentation::Outside)
      self->EventCallbackCommand->SetAbortFlag(1);
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
void vtkCountingFrameSliceWidget::SetCursor(int state)
{
  switch (state)
  {
    case SliceRepresentation::Translating:
      this->RequestCursorShape(VTK_CURSOR_SIZEALL);
      break;
    case SliceRepresentation::MoveLeft:
    case SliceRepresentation::MoveRight:
      this->RequestCursorShape(VTK_CURSOR_SIZEWE);
      break;
    case SliceRepresentation::MoveTop:
    case SliceRepresentation::MoveBottom:
      this->RequestCursorShape(VTK_CURSOR_SIZENS);
      break;
    case SliceRepresentation::Outside:
      this->RequestCursorShape(VTK_CURSOR_DEFAULT);
      break;
    default:
      this->RequestCursorShape(VTK_CURSOR_DEFAULT);
      break;
  };
}


//----------------------------------------------------------------------
void vtkCountingFrameSliceWidget::EndSelectAction(vtkAbstractWidget *w)
{
  vtkCountingFrameSliceWidget *self = reinterpret_cast<vtkCountingFrameSliceWidget*>(w);
  if ( self->WidgetState == vtkCountingFrameSliceWidget::Start )
    {
    return;
    }

  // Return state to not active
  self->WidgetState = vtkCountingFrameSliceWidget::Start;
  reinterpret_cast<SliceRepresentation*>(self->WidgetRep)->
    SetInteractionState(SliceRepresentation::Outside);
  self->ReleaseFocus();

  SliceRepresentation *rep = SliceRepresentation::SafeDownCast(self->WidgetRep);
  if (rep)
  {
    rep->GetInclusionOffset(self->InclusionOffset);
    rep->GetExclusionOffset(self->ExclusionOffset);
    for (int i = 0; i < 3; i++)
    {
      self->InclusionOffset[i] =
        vtkMath::Round(self->InclusionOffset[i]/self->Resolution[i])*self->Resolution[i];
      self->ExclusionOffset[i] =
        vtkMath::Round(self->ExclusionOffset[i]/self->Resolution[i])*self->Resolution[i];
    }
  }

  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  self->Render();
  self->SetCursor(9999);
}

//----------------------------------------------------------------------
void vtkCountingFrameSliceWidget::SetPlane(EspINA::PlaneType plane)
{
  Plane = plane;

  if (!this->WidgetRep)
    CreateDefaultRepresentation();
}

//----------------------------------------------------------------------
void vtkCountingFrameSliceWidget::SetSlice(EspINA::Nm pos)
{
  if (!this->WidgetRep)
    CreateDefaultRepresentation();

  SliceRepresentation *rep = reinterpret_cast<SliceRepresentation*>(this->WidgetRep);
  rep->SetSlice(pos);
  Slice = pos;
  this->Render();
}

//----------------------------------------------------------------------
void vtkCountingFrameSliceWidget::SetSlicingStep(EspINA::Nm slicingStep[3])
{
  memcpy(Resolution, slicingStep, 3*sizeof(EspINA::Nm));
}

//----------------------------------------------------------------------
void vtkCountingFrameSliceWidget::SetCountingFrame(vtkSmartPointer<vtkPolyData> region,
                                                     EspINA::Nm inclusionOffset[3],
                                                     EspINA::Nm exclusionOffset[3])
{
  if (!this->WidgetRep)
    CreateDefaultRepresentation();

  memcpy(InclusionOffset, inclusionOffset, 3*sizeof(EspINA::Nm));
  memcpy(ExclusionOffset, exclusionOffset, 3*sizeof(EspINA::Nm));

  SliceRepresentation *rep = reinterpret_cast<SliceRepresentation*>(this->WidgetRep);
  rep->SetCountingFrame(region, inclusionOffset, exclusionOffset, Resolution);
  this->Render();
}

//----------------------------------------------------------------------
void vtkCountingFrameSliceWidget::CreateDefaultRepresentation()
{
  if ( ! this->WidgetRep )
  {
    switch (Plane)
    {
      case EspINA::AXIAL:
        this->WidgetRep = AxialSliceRepresentation::New();
        break;
      case EspINA::CORONAL:
        this->WidgetRep = CoronalSliceRepresentation::New();
        break;
      case EspINA::SAGITTAL:
        this->WidgetRep = SagittalSliceRepresentation::New();
        break;
      case EspINA::VOLUME:
      default:
        Q_ASSERT(false);
        break;
    }
  }
}

//----------------------------------------------------------------------------
void vtkCountingFrameSliceWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
