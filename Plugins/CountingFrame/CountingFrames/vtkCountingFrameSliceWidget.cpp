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

// Plugin
#include "vtkCountingFrameSliceWidget.h"
#include "vtkCountingFrameSliceRepresentation.h"
#include "vtkCountingFrameRepresentationXY.h"
#include "vtkCountingFrameRepresentationXZ.h"
#include "vtkCountingFrameRepresentationYZ.h"

// VTK
#include <vtkCommand.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkObjectFactory.h>
#include <vtkMath.h>
#include <vtkWidgetEventTranslator.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkEvent.h>
#include <vtkWidgetEvent.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkPolyData.h>

vtkStandardNewMacro(vtkCountingFrameSliceWidget);

using SliceRepresentation   = vtkCountingFrameSliceRepresentation;
using SliceRepresentationXY = vtkCountingFrameRepresentationXY;
using SliceRepresentationXZ = vtkCountingFrameRepresentationXZ;
using SliceRepresentationYZ = vtkCountingFrameRepresentationYZ;

//----------------------------------------------------------------------------
vtkCountingFrameSliceWidget::vtkCountingFrameSliceWidget()
: Plane{ESPINA::Plane::XY}
, Slice{0}
, Depth{0}
{
  this->WidgetState = vtkCountingFrameSliceWidget::Start;
  this->ManagesCursor = 1;

  memset(InclusionOffset, 0, 3*sizeof(double));
  memset(ExclusionOffset, 0, 3*sizeof(double));

  // Define widget events
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, nullptr,
                                          vtkWidgetEvent::Select,
                                          this, vtkCountingFrameSliceWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, nullptr,
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
                                          0, 0, nullptr,
                                          vtkWidgetEvent::Translate,
                                          this, vtkCountingFrameSliceWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                            vtkEvent::ControlModifier,
                                            0, 0, nullptr,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkCountingFrameSliceWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::ShiftModifier,
                                          0, 0, nullptr,
                                          vtkWidgetEvent::Translate,
                                          this, vtkCountingFrameSliceWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                            vtkEvent::ShiftModifier,
                                            0, 0, nullptr,
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
  auto self = reinterpret_cast<vtkCountingFrameSliceWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if (!self->CurrentRenderer || !self->CurrentRenderer->IsInViewport(X,Y))
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
  reinterpret_cast<SliceRepresentation*>(self->WidgetRep)->SetInteractionState(interactionState);

  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkCountingFrameSliceWidget::TranslateAction(vtkAbstractWidget *w)
{
  auto self = reinterpret_cast<vtkCountingFrameSliceWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if (!self->CurrentRenderer || !self->CurrentRenderer->IsInViewport(X,Y))
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
  if (interactionState == SliceRepresentation::Outside)
  {
    return;
  }

  // We are definitely selected
  self->WidgetState = vtkCountingFrameSliceWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);
  reinterpret_cast<SliceRepresentation*>(self->WidgetRep)->SetInteractionState(SliceRepresentation::Translating);

  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkCountingFrameSliceWidget::MoveAction(vtkAbstractWidget *w)
{
  auto self = reinterpret_cast<vtkCountingFrameSliceWidget*>(w);

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
    {
      self->EventCallbackCommand->SetAbortFlag(1);
    }
    return;
  }

  // Okay, adjust the representation
  self->SetHighlighted(true);
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->WidgetInteraction(e);

  // moving something
  self->EventCallbackCommand->SetAbortFlag(1);
  self->InvokeEvent(vtkCommand::InteractionEvent,nullptr);
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
  auto self = reinterpret_cast<vtkCountingFrameSliceWidget*>(w);
  auto rep = SliceRepresentation::SafeDownCast(self->WidgetRep);

  if (self->WidgetState == vtkCountingFrameSliceWidget::Start)
  {
    return;
  }

  // Return state to not active
  self->WidgetState = vtkCountingFrameSliceWidget::Start;
  if(rep)
  {
    rep->SetInteractionState(SliceRepresentation::Outside);
  }
  self->ReleaseFocus();

  if (rep)
  {
    rep->GetInclusionOffset(self->InclusionOffset);
    rep->GetExclusionOffset(self->ExclusionOffset);

    centerMarginsOnVoxelCenter(self);
  }

  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent,nullptr);
  self->Render();
  self->SetCursor(9999);
}

//----------------------------------------------------------------------
void vtkCountingFrameSliceWidget::centerMarginsOnVoxelCenter(vtkCountingFrameSliceWidget *self)
{
  auto voxelCenter = [](double offset, double spacing){ return vtkMath::Floor(offset/spacing)*spacing + 0.5*spacing; };

  for (int i = 0; i < 3; i++)
  {
    self->InclusionOffset[i] = voxelCenter(self->InclusionOffset[i], self->SlicingStep[i]);
    self->ExclusionOffset[i] = voxelCenter(self->ExclusionOffset[i], self->SlicingStep[i]);
  }
}

//----------------------------------------------------------------------
void vtkCountingFrameSliceWidget::SetPlane(ESPINA::Plane plane)
{
  Plane = plane;

  if (!this->WidgetRep)
  {
    CreateDefaultRepresentation();
  }
}

//----------------------------------------------------------------------
void vtkCountingFrameSliceWidget::SetRepresentationDepth(ESPINA::Nm depth)
{
  Depth = depth;

  if (this->WidgetRep)
  {
    dynamic_cast<vtkCountingFrameSliceRepresentation *>(this->WidgetRep)->SetRepresentationDepth(Depth);
  }
}

//----------------------------------------------------------------------
void vtkCountingFrameSliceWidget::SetSlice(ESPINA::Nm pos)
{
  if (!this->WidgetRep)
  {
    CreateDefaultRepresentation();
  }

  if(Slice != pos)
  {
    Slice = pos;
    auto rep = reinterpret_cast<SliceRepresentation*>(this->WidgetRep);
    rep->SetSlice(pos);
  }
}

//----------------------------------------------------------------------
void vtkCountingFrameSliceWidget::SetCountingFrame(vtkSmartPointer<vtkPolyData> region,
                                                   ESPINA::Nm inclusionOffset[3],
                                                   ESPINA::Nm exclusionOffset[3],
                                                   ESPINA::NmVector3 resolution)
{
  SlicingStep = resolution;

  if (!this->WidgetRep)
  {
    CreateDefaultRepresentation();
  }

  memcpy(InclusionOffset, inclusionOffset, 3*sizeof(ESPINA::Nm));
  memcpy(ExclusionOffset, exclusionOffset, 3*sizeof(ESPINA::Nm));

  centerMarginsOnVoxelCenter(this);

  // ensures consistency with the counting frame
  memcpy(inclusionOffset, InclusionOffset, 3*sizeof(ESPINA::Nm));
  memcpy(exclusionOffset, ExclusionOffset, 3*sizeof(ESPINA::Nm));

  auto rep = reinterpret_cast<SliceRepresentation*>(this->WidgetRep);
  rep->SetCountingFrame(region, InclusionOffset, ExclusionOffset, resolution);
  rep->SetSlice(Slice);

  this->Render();
}

//----------------------------------------------------------------------
void vtkCountingFrameSliceWidget::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
  {
    switch (Plane)
    {
      case ESPINA::Plane::XY:
        this->WidgetRep = SliceRepresentationXY::New();
        break;
      case ESPINA::Plane::XZ:
        this->WidgetRep = SliceRepresentationXZ::New();
        break;
      case ESPINA::Plane::YZ:
        this->WidgetRep = SliceRepresentationYZ::New();
        break;
      default:
        Q_ASSERT(false);
        break;
    }
  }

  dynamic_cast<vtkCountingFrameSliceRepresentation *>(this->WidgetRep)->SetRepresentationDepth(Depth);
}

//----------------------------------------------------------------------------
void vtkCountingFrameSliceWidget::SetHighlighted(bool highlight)
{
  if (!this->WidgetRep)
  {
    CreateDefaultRepresentation();
  }

  auto rep = reinterpret_cast<SliceRepresentation*>(this->WidgetRep);
  rep->SetHighlighted(highlight);

  this->Render();
}


//----------------------------------------------------------------------------
void vtkCountingFrameSliceWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkCountingFrameSliceWidget::setVisible(bool visible)
{
  if(Visible != visible)
  {
    Visible = visible;

    auto rep = reinterpret_cast<SliceRepresentation*>(this->WidgetRep);
    rep->SetVisibility(visible);

    this->Render();
  }
}
