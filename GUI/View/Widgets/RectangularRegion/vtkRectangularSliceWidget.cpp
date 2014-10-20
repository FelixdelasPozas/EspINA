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

#include "vtkRectangularSliceWidget.h"

#include "vtkRectangularSliceRepresentation.h"

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

using namespace ESPINA;

vtkStandardNewMacro(vtkRectangularSliceWidget);

//----------------------------------------------------------------------------
vtkRectangularSliceWidget::vtkRectangularSliceWidget()
: m_plane(Plane::XY)
, Slice(0)
, m_pattern(0xFFFF)
{
  this->WidgetState = vtkRectangularSliceWidget::Start;
  this->ManagesCursor = 1;

  // Define widget events
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Select,
                                          this, vtkRectangularSliceWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::EndSelect,
                                          this, vtkRectangularSliceWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonPressEvent,
                                          vtkWidgetEvent::Translate,
                                          this, vtkRectangularSliceWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonReleaseEvent,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkRectangularSliceWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
                                          vtkWidgetEvent::EndScale,
                                          this, vtkRectangularSliceWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
                                          vtkWidgetEvent::Move,
                                          this, vtkRectangularSliceWidget::MoveAction);

  m_color[0] = m_color[1] = 1;
  m_color[2] = 0;
}

//----------------------------------------------------------------------------
vtkRectangularSliceWidget::~vtkRectangularSliceWidget()
{
}


//----------------------------------------------------------------------
void vtkRectangularSliceWidget::SelectAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkRectangularSliceWidget *self = reinterpret_cast<vtkRectangularSliceWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkRectangularSliceWidget::Start;
    return;
    }

  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState <= vtkRectangularSliceRepresentation::Inside )
    {
    return;
    }

  // We are definitely selected
  self->WidgetState = vtkRectangularSliceWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);

  // The SetInteractionState has the side effect of highlighting the widget
  reinterpret_cast<vtkRectangularSliceRepresentation*>(self->WidgetRep)->
    SetInteractionState(interactionState);

  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkRectangularSliceWidget::TranslateAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkRectangularSliceWidget *self = reinterpret_cast<vtkRectangularSliceWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
  {
    self->WidgetState = vtkRectangularSliceWidget::Start;
    return;
  }

  if (!self->Interactor->GetControlKey())
  {
    self->WidgetState = vtkRectangularSliceWidget::Start;
    return;
  }

  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  // Translate only if we are inside the representation
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState != vtkRectangularSliceRepresentation::Inside )
    return;

  // We are definitely selected
  self->WidgetState = vtkRectangularSliceWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);
  reinterpret_cast<vtkRectangularSliceRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkRectangularSliceRepresentation::Translating);
  self->SetCursor(vtkRectangularSliceRepresentation::Translating);

  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkRectangularSliceWidget::SetView(View2D *view)
{
  if(!this->WidgetRep)
    this->CreateDefaultRepresentation();

  auto rep = reinterpret_cast<vtkRectangularSliceRepresentation *>(this->WidgetRep);
  rep->SetView(view);
}

//----------------------------------------------------------------------
void vtkRectangularSliceWidget::MoveAction(vtkAbstractWidget *w)
{
  vtkRectangularSliceWidget *self = reinterpret_cast<vtkRectangularSliceWidget*>(w);

  // compute some info we need for all cases
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // See whether we're active
  if ( self->WidgetState == vtkRectangularSliceWidget::Start )
  {
    self->WidgetRep->ComputeInteractionState(X, Y);
    int stateAfter = self->WidgetRep->GetInteractionState();
    self->SetCursor(stateAfter);
    if (vtkRectangularSliceRepresentation::Inside < stateAfter
     || stateAfter == vtkRectangularSliceRepresentation::Translating)
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
void vtkRectangularSliceWidget::SetCursor(int state)
{
  switch (state)
  {
    case vtkRectangularSliceRepresentation::Translating:
      this->RequestCursorShape(VTK_CURSOR_SIZEALL);
      break;
    case vtkRectangularSliceRepresentation::MoveLeft:
    case vtkRectangularSliceRepresentation::MoveRight:
      this->RequestCursorShape(VTK_CURSOR_SIZEWE);
      break;
    case vtkRectangularSliceRepresentation::MoveTop:
    case vtkRectangularSliceRepresentation::MoveBottom:
      this->RequestCursorShape(VTK_CURSOR_SIZENS);
      break;
    case vtkRectangularSliceRepresentation::Outside:
      this->RequestCursorShape(VTK_CURSOR_DEFAULT);
      break;
    default:
      this->RequestCursorShape(VTK_CURSOR_DEFAULT);
      break;
  };
}


//----------------------------------------------------------------------
void vtkRectangularSliceWidget::EndSelectAction(vtkAbstractWidget *w)
{
  vtkRectangularSliceWidget *self = reinterpret_cast<vtkRectangularSliceWidget*>(w);
  if ( self->WidgetState == vtkRectangularSliceWidget::Start )
    {
    return;
    }

  // Return state to not active
  self->WidgetState = vtkRectangularSliceWidget::Start;
  reinterpret_cast<vtkRectangularSliceRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkRectangularSliceRepresentation::Outside);
  self->ReleaseFocus();

  self->EventCallbackCommand->SetAbortFlag(0);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkRectangularSliceWidget::SetPlane(Plane plane)
{
  if (!this->WidgetRep)
    CreateDefaultRepresentation();

  vtkRectangularSliceRepresentation *rep =
    reinterpret_cast<vtkRectangularSliceRepresentation*>(this->WidgetRep);
  rep->SetPlane(plane);

  m_plane = plane;
}

//----------------------------------------------------------------------
void vtkRectangularSliceWidget::SetSlice(Nm pos)
{
  if (!this->WidgetRep)
    CreateDefaultRepresentation();

  vtkRectangularSliceRepresentation *rep = reinterpret_cast<vtkRectangularSliceRepresentation*>(this->WidgetRep);
  rep->SetSlice(pos);
  Slice = pos;
  this->Render();
}

//----------------------------------------------------------------------
void vtkRectangularSliceWidget::SetBounds(Bounds bounds)
{
  if (!this->WidgetRep)
    CreateDefaultRepresentation();

  double dBounds[6]{bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]};
  vtkRectangularSliceRepresentation *rep = reinterpret_cast<vtkRectangularSliceRepresentation*>(this->WidgetRep);
  rep->SetOrthogonalBounds(dBounds);
  m_bounds = bounds;
  this->Render();
}

//----------------------------------------------------------------------
Bounds vtkRectangularSliceWidget::GetBounds()
{
  if (!this->WidgetRep)
    CreateDefaultRepresentation();

  vtkRectangularSliceRepresentation *rep = reinterpret_cast<vtkRectangularSliceRepresentation*>(this->WidgetRep);
  double dBounds[6];
  rep->GetOrthogonalBounds(dBounds);
  m_bounds[0] = dBounds[0];
  m_bounds[1] = dBounds[1];
  m_bounds[2] = dBounds[2];
  m_bounds[3] = dBounds[3];
  m_bounds[4] = dBounds[4];
  m_bounds[5] = dBounds[5];

  this->Render();
  this->EventCallbackCommand->SetAbortFlag(0);

  return m_bounds;
}

//----------------------------------------------------------------------
void vtkRectangularSliceWidget::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
  {
    this->WidgetRep = vtkRectangularSliceRepresentation::New();
    static_cast<vtkRectangularSliceRepresentation*>(this->WidgetRep)->setRepresentationColor(m_color);
    static_cast<vtkRectangularSliceRepresentation*>(this->WidgetRep)->setRepresentationPattern(m_pattern);
  }
}

//----------------------------------------------------------------------------
void vtkRectangularSliceWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkRectangularSliceWidget::setRepresentationColor(double *color)
{
  if (0 == memcmp(m_color, color, sizeof(double)*3))
    return;

  memcpy(m_color, color, sizeof(double)*3);
  if (this->WidgetRep)
  {
    static_cast<vtkRectangularSliceRepresentation*>(this->WidgetRep)->setRepresentationColor(m_color);
    static_cast<vtkRectangularSliceRepresentation*>(this->WidgetRep)->setRepresentationPattern(m_pattern);
  }
}

//----------------------------------------------------------------------------
void vtkRectangularSliceWidget::setRepresentationPattern(int pattern)
{
  if (m_pattern == pattern)
    return;

  m_pattern = pattern;
  if (this->WidgetRep)
  {
    static_cast<vtkRectangularSliceRepresentation*>(this->WidgetRep)->setRepresentationColor(m_color);
    static_cast<vtkRectangularSliceRepresentation*>(this->WidgetRep)->setRepresentationPattern(m_pattern);
  }
}
