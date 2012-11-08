/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkBoundingRegionSliceWidget.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkBoundingRegionSliceWidget.h"

#include "vtkBoundingRegionSliceRepresentation.h"
#include "vtkBoundingRegionAxialSliceRepresentation.h"
#include "vtkBoundingRegionCoronalSliceRepresentation.h"
#include "vtkBoundingRegionSagittalSliceRepresentation.h"

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
#include <vtkPolyData.h>


vtkStandardNewMacro(vtkBoundingRegionSliceWidget);

typedef vtkBoundingRegionSliceRepresentation         SliceRepresentation;
typedef vtkBoundingRegionAxialSliceRepresentation    AxialSliceRepresentation;
typedef vtkBoundingRegionCoronalSliceRepresentation  CoronalSliceRepresentation;
typedef vtkBoundingRegionSagittalSliceRepresentation SagittalSliceRepresentation;

//----------------------------------------------------------------------------
vtkBoundingRegionSliceWidget::vtkBoundingRegionSliceWidget()
{
  this->WidgetState = vtkBoundingRegionSliceWidget::Start;
  this->ManagesCursor = 1;

  memset(InclusionOffset, 0, 3*sizeof(double));
  memset(ExclusionOffset, 0, 3*sizeof(double));

  // Define widget events
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Select,
                                          this, vtkBoundingRegionSliceWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::EndSelect,
                                          this, vtkBoundingRegionSliceWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonPressEvent,
                                          vtkWidgetEvent::Translate,
                                          this, vtkBoundingRegionSliceWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonReleaseEvent,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkBoundingRegionSliceWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::ControlModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Translate,
                                          this, vtkBoundingRegionSliceWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                            vtkEvent::ControlModifier,
                                            0, 0, NULL,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkBoundingRegionSliceWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::ShiftModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Translate,
                                          this, vtkBoundingRegionSliceWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                            vtkEvent::ShiftModifier,
                                            0, 0, NULL,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkBoundingRegionSliceWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
                                          vtkWidgetEvent::EndScale,
                                          this, vtkBoundingRegionSliceWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
                                          vtkWidgetEvent::Move,
                                          this, vtkBoundingRegionSliceWidget::MoveAction);
}

//----------------------------------------------------------------------------
vtkBoundingRegionSliceWidget::~vtkBoundingRegionSliceWidget()
{
}

//----------------------------------------------------------------------
void vtkBoundingRegionSliceWidget::SelectAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkBoundingRegionSliceWidget *self = reinterpret_cast<vtkBoundingRegionSliceWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkBoundingRegionSliceWidget::Start;
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
  self->WidgetState = vtkBoundingRegionSliceWidget::Active;
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
void vtkBoundingRegionSliceWidget::TranslateAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkBoundingRegionSliceWidget *self = reinterpret_cast<vtkBoundingRegionSliceWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkBoundingRegionSliceWidget::Start;
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
  self->WidgetState = vtkBoundingRegionSliceWidget::Active;
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
void vtkBoundingRegionSliceWidget::MoveAction(vtkAbstractWidget *w)
{
  vtkBoundingRegionSliceWidget *self = reinterpret_cast<vtkBoundingRegionSliceWidget*>(w);

  // compute some info we need for all cases
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // See whether we're active
  if ( self->WidgetState == vtkBoundingRegionSliceWidget::Start )
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
void vtkBoundingRegionSliceWidget::SetCursor(int state)
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
  };
}


//----------------------------------------------------------------------
void vtkBoundingRegionSliceWidget::EndSelectAction(vtkAbstractWidget *w)
{
  vtkBoundingRegionSliceWidget *self = reinterpret_cast<vtkBoundingRegionSliceWidget*>(w);
  if ( self->WidgetState == vtkBoundingRegionSliceWidget::Start )
    {
    return;
    }

  // Return state to not active
  self->WidgetState = vtkBoundingRegionSliceWidget::Start;
  reinterpret_cast<SliceRepresentation*>(self->WidgetRep)->
    SetInteractionState(SliceRepresentation::Outside);
  self->ReleaseFocus();

  SliceRepresentation *rep = SliceRepresentation::SafeDownCast(self->WidgetRep);
  if (rep)
  {
    //std::cout << "updating offset" << std::endl;
    rep->GetInclusionOffset(self->InclusionOffset);
    //std::cout << "Inclusion Offset: " << self->InclusionOffset[0] << " " << self->InclusionOffset[1]  << " " << self->InclusionOffset[2] << std::endl;
    rep->GetExclusionOffset(self->ExclusionOffset);
    //std::cout << "Exclusion Offset: " << self->ExclusionOffset[0] << " " << self->ExclusionOffset[1]  << " " << self->ExclusionOffset[2] << std::endl;
  }

  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  self->Render();
  self->SetCursor(9999);
}

//----------------------------------------------------------------------
void vtkBoundingRegionSliceWidget::SetPlane(PlaneType plane)
{
  Plane = plane;

  if (!this->WidgetRep)
    CreateDefaultRepresentation();

  SliceRepresentation *rep =
    reinterpret_cast<SliceRepresentation*>(this->WidgetRep);
}

//----------------------------------------------------------------------
void vtkBoundingRegionSliceWidget::SetSlice(Nm pos)
{
  if (!this->WidgetRep)
    CreateDefaultRepresentation();

  SliceRepresentation *rep = reinterpret_cast<SliceRepresentation*>(this->WidgetRep);
  rep->SetSlice(pos);
  Slice = pos;
  this->Render();
}

//----------------------------------------------------------------------
void vtkBoundingRegionSliceWidget::SetSlicingStep(Nm slicingStep[3])
{
  memcpy(SlicingStep, slicingStep, 3*sizeof(Nm));
}

//----------------------------------------------------------------------
void vtkBoundingRegionSliceWidget::SetBoundingRegion(vtkSmartPointer<vtkPolyData> region,
                                                     Nm inclusionOffset[3],
                                                     Nm exclusionOffset[3])
{
  if (!this->WidgetRep)
    CreateDefaultRepresentation();

  memcpy(InclusionOffset, inclusionOffset, 3*sizeof(Nm));
  memcpy(ExclusionOffset, exclusionOffset, 3*sizeof(Nm));

  SliceRepresentation *rep = reinterpret_cast<SliceRepresentation*>(this->WidgetRep);
  rep->SetBoundingRegion(region, inclusionOffset, exclusionOffset, SlicingStep);
  this->Render();
}

//----------------------------------------------------------------------
void vtkBoundingRegionSliceWidget::CreateDefaultRepresentation()
{
  if ( ! this->WidgetRep )
  {
    switch (Plane)
    {
      case AXIAL:
        this->WidgetRep = AxialSliceRepresentation::New();
        break;
      case CORONAL:
        this->WidgetRep = CoronalSliceRepresentation::New();
        break;
      case SAGITTAL:
        this->WidgetRep = SagittalSliceRepresentation::New();
        break;
    }
  }
}

//----------------------------------------------------------------------------
void vtkBoundingRegionSliceWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


