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

#include "vtkWidget2D.h"
#include "vtkRepresentation2D.h"

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
using namespace ESPINA::GUI::View::Widgets::OrthogonalRegion;

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        namespace OrthogonalRegion
        {
          // Using namespace prevents collisions with other widgets
          vtkStandardNewMacro(vtkWidget2D);
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
vtkWidget2D::vtkWidget2D()
: m_plane(Plane::XY)
, m_slice(0)
, m_pattern(0xFFFF)
{
  this->WidgetState = vtkWidget2D::Start;
  this->ManagesCursor = 1;

  // Define widget events
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Select,
                                          this, vtkWidget2D::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::EndSelect,
                                          this, vtkWidget2D::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonPressEvent,
                                          vtkWidgetEvent::Translate,
                                          this, vtkWidget2D::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonReleaseEvent,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkWidget2D::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
                                          vtkWidgetEvent::EndScale,
                                          this, vtkWidget2D::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
                                          vtkWidgetEvent::Move,
                                          this, vtkWidget2D::MoveAction);

  m_color[0] = m_color[1] = 1;
  m_color[2] = 0;
}

//----------------------------------------------------------------------------
vtkWidget2D::~vtkWidget2D()
{
}


//----------------------------------------------------------------------
void vtkWidget2D::ensureRepresentationIsAvailable()
{
  if(!this->WidgetRep)
  {
    this->CreateDefaultRepresentation();
  }
}

//----------------------------------------------------------------------
void vtkWidget2D::updateRepresentation()
{
  auto rep = static_cast<vtkRepresentation2D*>(this->WidgetRep);
  rep->setRepresentationColor(m_color);
  rep->setRepresentationPattern(m_pattern);
}

//----------------------------------------------------------------------
void vtkWidget2D::SelectAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkWidget2D *self = reinterpret_cast<vtkWidget2D*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkWidget2D::Start;
    return;
    }

  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState <= vtkRepresentation2D::Inside )
    {
    return;
    }

  // We are definitely selected
  self->WidgetState = vtkWidget2D::Active;
  self->GrabFocus(self->EventCallbackCommand);

  // The SetInteractionState has the side effect of highlighting the widget
  reinterpret_cast<vtkRepresentation2D*>(self->WidgetRep)->
    SetInteractionState(interactionState);

  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkWidget2D::TranslateAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkWidget2D *self = reinterpret_cast<vtkWidget2D*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
  {
    self->WidgetState = vtkWidget2D::Start;
    return;
  }

  if (!self->Interactor->GetControlKey())
  {
    self->WidgetState = vtkWidget2D::Start;
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
  if ( interactionState != vtkRepresentation2D::Inside )
    return;

  // We are definitely selected
  self->WidgetState = vtkWidget2D::Active;
  self->GrabFocus(self->EventCallbackCommand);
  reinterpret_cast<vtkRepresentation2D*>(self->WidgetRep)->
    SetInteractionState(vtkRepresentation2D::Translating);
  self->SetCursor(vtkRepresentation2D::Translating);

  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkWidget2D::SetDepth(double depth)
{
  ensureRepresentationIsAvailable();

  auto rep = reinterpret_cast<vtkRepresentation2D *>(this->WidgetRep);
  rep->SetDepth(depth);
}

//----------------------------------------------------------------------
void vtkWidget2D::MoveAction(vtkAbstractWidget *w)
{
  vtkWidget2D *self = reinterpret_cast<vtkWidget2D*>(w);

  // compute some info we need for all cases
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // See whether we're active
  if ( self->WidgetState == vtkWidget2D::Start )
  {
    self->WidgetRep->ComputeInteractionState(X, Y);
    int stateAfter = self->WidgetRep->GetInteractionState();
    self->SetCursor(stateAfter);

    if (vtkRepresentation2D::Inside < stateAfter)
    {
      self->EventCallbackCommand->SetAbortFlag(1);
    }
  }
  else
  {
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
}

//----------------------------------------------------------------------
void vtkWidget2D::SetCursor(int state)
{
  switch (state)
  {
    case vtkRepresentation2D::Translating:
      this->RequestCursorShape(VTK_CURSOR_SIZEALL);
      break;
    case vtkRepresentation2D::MoveLeft:
    case vtkRepresentation2D::MoveRight:
      this->RequestCursorShape(VTK_CURSOR_SIZEWE);
      break;
    case vtkRepresentation2D::MoveTop:
    case vtkRepresentation2D::MoveBottom:
      this->RequestCursorShape(VTK_CURSOR_SIZENS);
      break;
    case vtkRepresentation2D::Outside:
      this->RequestCursorShape(VTK_CURSOR_DEFAULT);
      break;
    default:
      this->RequestCursorShape(VTK_CURSOR_DEFAULT);
      break;
  };
}


//----------------------------------------------------------------------
void vtkWidget2D::EndSelectAction(vtkAbstractWidget *w)
{
  vtkWidget2D *self = reinterpret_cast<vtkWidget2D*>(w);
  if ( self->WidgetState == vtkWidget2D::Start )
    {
    return;
    }

  // Return state to not active
  self->WidgetState = vtkWidget2D::Start;
  reinterpret_cast<vtkRepresentation2D*>(self->WidgetRep)->
    SetInteractionState(vtkRepresentation2D::Outside);
  self->ReleaseFocus();

  self->EventCallbackCommand->SetAbortFlag(0);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkWidget2D::SetPlane(Plane plane)
{
  ensureRepresentationIsAvailable();

  auto rep = reinterpret_cast<vtkRepresentation2D*>(this->WidgetRep);
  rep->SetPlane(plane);

  m_plane = plane;
}

//----------------------------------------------------------------------
void vtkWidget2D::SetSlice(Nm pos)
{
  ensureRepresentationIsAvailable();

  auto rep = reinterpret_cast<vtkRepresentation2D*>(this->WidgetRep);
  rep->SetSlice(pos);
  m_slice = pos;

  this->Render();
}

//----------------------------------------------------------------------
void vtkWidget2D::SetBounds(Bounds bounds)
{
  ensureRepresentationIsAvailable();

  double dBounds[6]{bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]};
  auto rep = reinterpret_cast<vtkRepresentation2D*>(this->WidgetRep);

  rep->SetOrthogonalBounds(dBounds);
  m_bounds = bounds;

  this->Render();
}

//----------------------------------------------------------------------
Bounds vtkWidget2D::GetBounds()
{
  ensureRepresentationIsAvailable();

  double dBounds[6];
  auto rep = reinterpret_cast<vtkRepresentation2D*>(this->WidgetRep);

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
void vtkWidget2D::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
  {
    this->WidgetRep = vtkRepresentation2D::New();

    updateRepresentation();
  }
}

//----------------------------------------------------------------------------
void vtkWidget2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkWidget2D::setRepresentationColor(double *color)
{
  if (0 == memcmp(m_color, color, sizeof(double)*3))
    return;

  memcpy(m_color, color, 3*sizeof(double));

  if (this->WidgetRep)
  {
    updateRepresentation();
  }
}

//----------------------------------------------------------------------------
void vtkWidget2D::setRepresentationPattern(int pattern)
{
  if (m_pattern == pattern)
    return;

  m_pattern = pattern;

  if (this->WidgetRep)
  {
    updateRepresentation();
  }
}
