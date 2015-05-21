/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include "vtkZoomSelectionWidget.h"
#include "vtkZoomSelectionWidgetRepresentation.h"

// vtk
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkWidgetEvent.h>
#include <vtkWidgetEventTranslator.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

using namespace ESPINA::GUI::View::Widgets;

vtkStandardNewMacro(vtkZoomSelectionWidget);

//----------------------------------------------------------------------------
vtkZoomSelectionWidget::vtkZoomSelectionWidget()
: WidgetState{Start}
, m_type     {NONE}
, m_depth    {0}
{
  // These are the event callbacks supported by this widget
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkWidgetEvent::Select,
                                          this, vtkZoomSelectionWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
                                          vtkWidgetEvent::Move,
                                          this, vtkZoomSelectionWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkWidgetEvent::EndSelect,
                                          this, vtkZoomSelectionWidget::EndSelectAction);
}

//----------------------------------------------------------------------------
vtkZoomSelectionWidget::~vtkZoomSelectionWidget()
{
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidget::SetWidgetType(vtkZoomSelectionWidget::WidgetType type)
{
  if(this->m_type != NONE)
  {
    return;
  }

  m_type = type;

  if (WidgetRep)
  {
    reinterpret_cast<vtkZoomSelectionWidgetRepresentation*>(this->WidgetRep)->SetWidgetType(m_type);
  }
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidget::CreateDefaultRepresentation()
{
  if(!WidgetRep)
  {
    this->WidgetRep = vtkZoomSelectionWidgetRepresentation::New();
  }

  this->WidgetRep->BuildRepresentation();

  auto rep = reinterpret_cast<vtkZoomSelectionWidgetRepresentation*>(this->WidgetRep);
  rep->SetWidgetType(m_type);
  rep->SetRepresentationDepth(m_depth);
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidget::SetEnabled(int enabling)
{
  if (enabling) //-------------------------------------------------------------
  {
    if (this->Enabled) return; //already enabled, just return

    if (!this->Interactor)
    {
      vtkErrorMacro(<<"The interactor must be set prior to enabling the widget");
      return;
    }

    int X = this->Interactor->GetEventPosition()[0];
    int Y = this->Interactor->GetEventPosition()[1];

    if (!this->CurrentRenderer)
    {
      this->SetCurrentRenderer(this->Interactor->FindPokedRenderer(X, Y));

      if (!this->CurrentRenderer) return;
    }

    // We're ready to enable
    this->Enabled = 1;
    this->CreateDefaultRepresentation();
    this->WidgetRep->SetRenderer(this->CurrentRenderer);

    // listen for the events found in the EventTranslator
    if (!this->Parent)
    {
      this->EventTranslator->AddEventsToInteractor(this->Interactor, this->EventCallbackCommand, this->Priority);
    }
    else
    {
      this->EventTranslator->AddEventsToParent(this->Parent, this->EventCallbackCommand, this->Priority);
    }

    this->WidgetRep->BuildRepresentation();
    this->CurrentRenderer->AddViewProp(this->WidgetRep);

    if (this->WidgetState == vtkZoomSelectionWidget::Start)
    {
      reinterpret_cast<vtkZoomSelectionWidgetRepresentation*>(this->WidgetRep)->VisibilityOff();
    }

    this->InvokeEvent(vtkCommand::EnableEvent, nullptr);
  }
  else
  {
    vtkDebugMacro(<<"Disabling widget");

    if (!this->Enabled) return; //already disabled, just return

    this->Enabled = 0;

    // don't listen for events any more
    if (!this->Parent)
    {
      this->Interactor->RemoveObserver(this->EventCallbackCommand);
    }
    else
    {
      this->Parent->RemoveObserver(this->EventCallbackCommand);
    }

    this->CurrentRenderer->RemoveViewProp(this->WidgetRep);
    this->InvokeEvent(vtkCommand::DisableEvent, nullptr);
    this->SetCurrentRenderer(nullptr);
  }

  // Should only render if there is no parent
  if (this->Interactor && !this->Parent)
  {
    this->Interactor->Render();
  }
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidget::SelectAction(vtkAbstractWidget *widget)
{
  vtkZoomSelectionWidget *self = reinterpret_cast<vtkZoomSelectionWidget*>(widget);

  if (self->WidgetState != vtkZoomSelectionWidget::Start) return;

  // place first point
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  self->GrabFocus(self->EventCallbackCommand);
  self->WidgetState = vtkZoomSelectionWidget::Define;
  self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
  reinterpret_cast<vtkZoomSelectionWidgetRepresentation*>(self->WidgetRep)->VisibilityOn();
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  reinterpret_cast<vtkZoomSelectionWidgetRepresentation*>(self->WidgetRep)->StartWidgetInteraction(e);
  self->EventCallbackCommand->SetAbortFlag(1);
  self->Render();
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidget::MoveAction(vtkAbstractWidget *widget)
{
  vtkZoomSelectionWidget *self = reinterpret_cast<vtkZoomSelectionWidget*>(widget);

  // Do nothing if in start or end mode
  if (self->WidgetState != vtkZoomSelectionWidget::Define) return;

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  reinterpret_cast<vtkZoomSelectionWidgetRepresentation*>(self->WidgetRep)->WidgetInteraction(e);
  self->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
  self->EventCallbackCommand->SetAbortFlag(1);
  self->WidgetRep->BuildRepresentation();
  self->Render();
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidget::EndSelectAction(vtkAbstractWidget *widget)
{
  vtkZoomSelectionWidget *self = reinterpret_cast<vtkZoomSelectionWidget*>(widget);

  // Delegate the event consistent with the state
  if (self->WidgetState != vtkZoomSelectionWidget::Define) return;

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  reinterpret_cast<vtkZoomSelectionWidgetRepresentation*>(self->WidgetRep)->EndWidgetInteraction(e);
  self->WidgetState = vtkZoomSelectionWidget::Start;
  self->ReleaseFocus();
  self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
  self->EventCallbackCommand->SetAbortFlag(1);
  self->Render();
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidget::setRepresentationDepth(ESPINA::Nm depth)
{
  if(m_depth != depth)
  {
    m_depth = depth;

    if(WidgetRep)
    {
      reinterpret_cast<vtkZoomSelectionWidgetRepresentation*>(this->WidgetRep)->SetRepresentationDepth(depth);
    }
  }
}

//----------------------------------------------------------------------------
void ESPINA::GUI::View::Widgets::vtkZoomSelectionWidget::SetSlice(Nm slice)
{
  if(WidgetRep)
  {
    reinterpret_cast<vtkZoomSelectionWidgetRepresentation*>(this->WidgetRep)->SetSlice(slice);
  }
}
