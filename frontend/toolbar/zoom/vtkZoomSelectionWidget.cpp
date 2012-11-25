/*
 * vtkZoomSelectionWidget.cpp
 *
 *  Created on: Nov 14, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include "toolbar/zoom/vtkZoomSelectionWidget.h"
#include "toolbar/zoom/vtkZoomSelectionWidgetRepresentation.h"

// vtk
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkWidgetEvent.h>
#include <vtkWidgetEventTranslator.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(vtkZoomSelectionWidget);

//----------------------------------------------------------------------------
vtkZoomSelectionWidget::vtkZoomSelectionWidget()
: WidgetState(Start)
, m_type(NONE)
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
    return;

  m_type = type;

  if (WidgetRep)
    reinterpret_cast<vtkZoomSelectionWidgetRepresentation*>(this->WidgetRep)->SetWidgetType(m_type);
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidget::CreateDefaultRepresentation()
{
  if(!WidgetRep)
    this->WidgetRep = vtkZoomSelectionWidgetRepresentation::New();

  this->WidgetRep->BuildRepresentation();
  reinterpret_cast<vtkZoomSelectionWidgetRepresentation*>(this->WidgetRep)->SetWidgetType(m_type);
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidget::SetEnabled(int enabling)
{
  if (enabling) //-------------------------------------------------------------
  {
    if (this->Enabled) //already enabled, just return
      return;

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
      if (this->CurrentRenderer == NULL)
        return;
    }

    // We're ready to enable
    this->Enabled = 1;
    this->CreateDefaultRepresentation();
    this->WidgetRep->SetRenderer(this->CurrentRenderer);

    // listen for the events found in the EventTranslator
    if (!this->Parent)
      this->EventTranslator->AddEventsToInteractor(this->Interactor, this->EventCallbackCommand, this->Priority);
    else
      this->EventTranslator->AddEventsToParent(this->Parent, this->EventCallbackCommand, this->Priority);

    this->WidgetRep->BuildRepresentation();
    this->CurrentRenderer->AddViewProp(this->WidgetRep);

    if (this->WidgetState == vtkZoomSelectionWidget::Start)
      reinterpret_cast<vtkZoomSelectionWidgetRepresentation*>(this->WidgetRep)->VisibilityOff();

    this->InvokeEvent(vtkCommand::EnableEvent, NULL);
  }
  else
  {
    vtkDebugMacro(<<"Disabling widget");

    if (!this->Enabled) //already disabled, just return
      return;

    this->Enabled = 0;

    // don't listen for events any more
    if (!this->Parent)
      this->Interactor->RemoveObserver(this->EventCallbackCommand);
    else
      this->Parent->RemoveObserver(this->EventCallbackCommand);

    this->CurrentRenderer->RemoveViewProp(this->WidgetRep);

    this->InvokeEvent(vtkCommand::DisableEvent, NULL);
    this->SetCurrentRenderer(NULL);
  }

  // Should only render if there is no parent
  if (this->Interactor && !this->Parent)
    this->Interactor->Render();
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidget::SelectAction(vtkAbstractWidget *widget)
{
  vtkZoomSelectionWidget *self = reinterpret_cast<vtkZoomSelectionWidget*>(widget);

  if (self->WidgetState != vtkZoomSelectionWidget::Start)
    return;

  // place first point
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  self->GrabFocus(self->EventCallbackCommand);
  self->WidgetState = vtkZoomSelectionWidget::Define;
  self->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
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
  if (self->WidgetState != vtkZoomSelectionWidget::Define)
    return;

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  reinterpret_cast<vtkZoomSelectionWidgetRepresentation*>(self->WidgetRep)->WidgetInteraction(e);
  self->InvokeEvent(vtkCommand::InteractionEvent, NULL);
  self->EventCallbackCommand->SetAbortFlag(1);
  self->WidgetRep->BuildRepresentation();
  self->Render();
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidget::EndSelectAction(vtkAbstractWidget *widget)
{
  vtkZoomSelectionWidget *self = reinterpret_cast<vtkZoomSelectionWidget*>(widget);

  // Delegate the event consistent with the state
  if (self->WidgetState != vtkZoomSelectionWidget::Define)
    return;

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  reinterpret_cast<vtkZoomSelectionWidgetRepresentation*>(self->WidgetRep)->EndWidgetInteraction(e);
  self->WidgetState = vtkZoomSelectionWidget::Start;
  self->ReleaseFocus();
  self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  self->EventCallbackCommand->SetAbortFlag(1);
  self->Render();
}

