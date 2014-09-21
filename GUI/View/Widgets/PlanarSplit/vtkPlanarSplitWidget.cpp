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
#include "vtkPlanarSplitWidget.h"
#include "vtkPlanarSplitRepresentation2D.h"

// vtk
#include <vtkHandleWidget.h>
#include <vtkHandleRepresentation.h>
#include <vtkCallbackCommand.h>
#include <vtkObjectFactory.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkWidgetEvent.h>
#include <vtkWidgetEventTranslator.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkPoints.h>
#include <vtkWidgetEventTranslator.h>

// Qt
#include <QDebug>

using namespace ESPINA;

vtkStandardNewMacro(vtkPlanarSplitWidget);

// The widget observes its two handles.
// Here we create the command/observer classes to respond to the
// handle widgets.

namespace ESPINA
{
  class vtkPlanarSplitWidgetCallback
  : public vtkCommand
  {
  public:
  	/** \brief Creates a new instance.
  	 *
  	 */
    static vtkPlanarSplitWidgetCallback *New()
    { return new vtkPlanarSplitWidgetCallback; }

    /** \brief Implements vtkCommand::execute().
     *
     */
    virtual void Execute(vtkObject*, unsigned long eventId, void*)
    {
      switch(eventId)
      {
        case vtkCommand::StartInteractionEvent:
          this->m_widget->StartHandleInteraction(this->m_handleNumber);
          break;
        case vtkCommand::InteractionEvent:
          this->m_widget->HandleInteraction(this->m_handleNumber);
          break;
        case vtkCommand::EndInteractionEvent:
          this->m_widget->StopHandleInteraction(this->m_handleNumber);
          break;
      }
    }

    int m_handleNumber;
    vtkPlanarSplitWidget *m_widget;
  };
}

//----------------------------------------------------------------------
vtkPlanarSplitWidget::vtkPlanarSplitWidget()
{
  this->m_slice = 0;
  this->ManagesCursor = 0;

  this->WidgetState = vtkPlanarSplitWidget::Start;
  this->CurrentHandle = 0;

  // The widgets for moving the end points. They observe this widget (i.e.,
  // this widget is the parent to the handles).
  this->m_point1Widget = vtkHandleWidget::New();
  this->m_point1Widget->SetParent(this);
  this->m_point1Widget->GetEventTranslator()->SetTranslation(vtkCommand::LeftButtonPressEvent, vtkCommand::StartInteractionEvent);
  this->m_point1Widget->GetEventTranslator()->SetTranslation(vtkCommand::MouseMoveEvent, vtkCommand::InteractionEvent);
  this->m_point1Widget->GetEventTranslator()->SetTranslation(vtkCommand::LeftButtonReleaseEvent, vtkCommand::EndInteractionEvent);

  this->m_point2Widget = vtkHandleWidget::New();
  this->m_point2Widget->SetParent(this);
  this->m_point2Widget->GetEventTranslator()->SetTranslation(vtkCommand::LeftButtonPressEvent, vtkCommand::StartInteractionEvent);
  this->m_point2Widget->GetEventTranslator()->SetTranslation(vtkCommand::MouseMoveEvent, vtkCommand::InteractionEvent);
  this->m_point2Widget->GetEventTranslator()->SetTranslation(vtkCommand::LeftButtonReleaseEvent, vtkCommand::EndInteractionEvent);


  // Set up the callbacks on the two handles
  this->m_planarSplitWidgetCallback1 = vtkPlanarSplitWidgetCallback::New();
  this->m_planarSplitWidgetCallback1->m_handleNumber = 0;
  this->m_planarSplitWidgetCallback1->m_widget = this;
  this->m_point1Widget->AddObserver(vtkCommand::StartInteractionEvent, this->m_planarSplitWidgetCallback1,
                                  this->Priority);
  this->m_point1Widget->AddObserver(vtkCommand::InteractionEvent, this->m_planarSplitWidgetCallback1,
                                  this->Priority);
  this->m_point1Widget->AddObserver(vtkCommand::EndInteractionEvent, this->m_planarSplitWidgetCallback1,
                                  this->Priority);

  this->m_planarSplitWidgetCallback2 = vtkPlanarSplitWidgetCallback::New();
  this->m_planarSplitWidgetCallback2->m_handleNumber = 1;
  this->m_planarSplitWidgetCallback2->m_widget = this;
  this->m_point2Widget->AddObserver(vtkCommand::StartInteractionEvent, this->m_planarSplitWidgetCallback2,
                                  this->Priority);
  this->m_point2Widget->AddObserver(vtkCommand::InteractionEvent, this->m_planarSplitWidgetCallback2,
                                  this->Priority);
  this->m_point2Widget->AddObserver(vtkCommand::EndInteractionEvent, this->m_planarSplitWidgetCallback2,
                                  this->Priority);


  // These are the event callbacks supported by this widget
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkWidgetEvent::AddPoint,
                                          this, vtkPlanarSplitWidget::AddPointAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
                                          vtkWidgetEvent::Move,
                                          this, vtkPlanarSplitWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkWidgetEvent::EndSelect,
                                          this, vtkPlanarSplitWidget::EndSelectAction);
  m_plane = Plane::XY;
  m_permanentlyDisabled = false;

  m_segmentationBounds[0] = 0;
  m_segmentationBounds[1] = -1;
  m_segmentationBounds[2] = 0;
  m_segmentationBounds[3] = -1;
  m_segmentationBounds[4] = 0;
  m_segmentationBounds[5] = -1;
}

//----------------------------------------------------------------------
vtkPlanarSplitWidget::~vtkPlanarSplitWidget()
{
  this->m_point1Widget->RemoveObserver(this->m_planarSplitWidgetCallback1);
  this->m_point1Widget->Delete();
  this->m_planarSplitWidgetCallback1->Delete();

  this->m_point2Widget->RemoveObserver(this->m_planarSplitWidgetCallback2);
  this->m_point2Widget->Delete();
  this->m_planarSplitWidgetCallback2->Delete();
}

//----------------------------------------------------------------------
void vtkPlanarSplitWidget::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
    this->WidgetRep = vtkPlanarSplitRepresentation2D::New();

  reinterpret_cast<vtkPlanarSplitRepresentation2D*>(this->WidgetRep)->InstantiateHandleRepresentation();
  reinterpret_cast<vtkPlanarSplitRepresentation2D*>(this->WidgetRep)->setOrientation(m_plane);
  reinterpret_cast<vtkPlanarSplitRepresentation2D*>(this->WidgetRep)->setSlice(m_slice);
}

void vtkPlanarSplitWidget::SetEnabled(int enabling)
{
  if(m_permanentlyDisabled)
    return;

  if(this->WidgetRep == nullptr)
    CreateDefaultRepresentation();

  // The handle widgets are not actually enabled until they are placed.
  // The handle widgets take their representation from the
  // vtkDistanceRepresentation.
  if (enabling)
  {
    if (this->WidgetState == vtkPlanarSplitWidget::Start)
      reinterpret_cast<vtkPlanarSplitRepresentation2D*>(this->WidgetRep)->VisibilityOff();
    else
    {
      // The interactor must be set prior to enabling the widget.
      if (this->Interactor)
      {
        this->m_point1Widget->SetInteractor(this->Interactor);
        this->m_point2Widget->SetInteractor(this->Interactor);
      }

      this->m_point1Widget->SetEnabled(1);
      this->m_point2Widget->SetEnabled(1);
    }
  }

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

    // Set the renderer, interactor and representation on the two handle widgets.
    this->m_point1Widget->SetRepresentation(
        reinterpret_cast<vtkPlanarSplitRepresentation2D*>(this->WidgetRep)->GetPoint1Representation());
    this->m_point1Widget->SetInteractor(this->Interactor);
    this->m_point1Widget->GetRepresentation()->SetRenderer(this->CurrentRenderer);

    this->m_point2Widget->SetRepresentation(
        reinterpret_cast<vtkPlanarSplitRepresentation2D*>(this->WidgetRep)->GetPoint2Representation());
    this->m_point2Widget->SetInteractor(this->Interactor);
    this->m_point2Widget->GetRepresentation()->SetRenderer(this->CurrentRenderer);

    // listen for the events found in the EventTranslator
    if (!this->Parent)
      this->EventTranslator->AddEventsToInteractor(this->Interactor, this->EventCallbackCommand, this->Priority);
    else
      this->EventTranslator->AddEventsToParent(this->Parent, this->EventCallbackCommand, this->Priority);

    if (this->ManagesCursor)
    {
      this->WidgetRep->ComputeInteractionState(X, Y);
      this->SetCursor(this->WidgetRep->GetInteractionState());
    }

    this->WidgetRep->BuildRepresentation();
    this->CurrentRenderer->AddViewProp(this->WidgetRep);

    if (this->WidgetState == vtkPlanarSplitWidget::Start)
      reinterpret_cast<vtkPlanarSplitRepresentation2D*>(this->WidgetRep)->VisibilityOff();
    else
    {
      this->m_point1Widget->SetEnabled(1);
      this->m_point2Widget->SetEnabled(1);
    }

    this->InvokeEvent(vtkCommand::EnableEvent, NULL);
  }
  else //disabling-------------------------------------------------------------
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

    this->m_point1Widget->SetEnabled(0);
    this->m_point2Widget->SetEnabled(0);

    this->InvokeEvent(vtkCommand::DisableEvent, NULL);
    this->SetCurrentRenderer(NULL);
  }

  // Should only render if there is no parent
  if (this->Interactor && !this->Parent)
    this->Interactor->Render();
}

// The following methods are the callbacks that the measure widget responds to
//-------------------------------------------------------------------------
void vtkPlanarSplitWidget::AddPointAction(vtkAbstractWidget *w)
{
  vtkPlanarSplitWidget *self = reinterpret_cast<vtkPlanarSplitWidget*>(w);
  if(self->m_permanentlyDisabled)
    return;

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Freshly enabled and placing the first point
  if (self->WidgetState == vtkPlanarSplitWidget::Start)
  {
    self->GrabFocus(self->EventCallbackCommand);
    self->WidgetState = vtkPlanarSplitWidget::Define;
    self->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
    reinterpret_cast<vtkPlanarSplitRepresentation2D*>(self->WidgetRep)->VisibilityOn();
    double e[2];
    e[0] = static_cast<double>(X);
    e[1] = static_cast<double>(Y);
    reinterpret_cast<vtkPlanarSplitRepresentation2D*>(self->WidgetRep)->StartWidgetInteraction(e);
    self->CurrentHandle = 0;
    self->InvokeEvent(vtkCommand::PlacePointEvent, &(self->CurrentHandle));
  }

  // Placing the second point is easy
  else
    if (self->WidgetState == vtkPlanarSplitWidget::Define)
    {
      self->CurrentHandle = 1;
      self->InvokeEvent(vtkCommand::PlacePointEvent, &(self->CurrentHandle));
      self->WidgetState = vtkPlanarSplitWidget::Manipulate;
      self->m_point1Widget->SetEnabled(1);
      self->m_point2Widget->SetEnabled(1);
      self->CurrentHandle = -1;
      self->ReleaseFocus();
      self->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
    }

    // Maybe we are trying to manipulate the widget handles
    else
    {
      int state = self->WidgetRep->ComputeInteractionState(X, Y);
      if (state == vtkPlanarSplitRepresentation2D::Outside)
      {
        self->CurrentHandle = -1;
        return;
      }

      self->GrabFocus(self->EventCallbackCommand);
      if (state == vtkPlanarSplitRepresentation2D::NearP1)
        self->CurrentHandle = 0;
      else
        if (state == vtkPlanarSplitRepresentation2D::NearP2)
          self->CurrentHandle = 1;

      self->InvokeEvent(vtkCommand::LeftButtonPressEvent, NULL);
    }

  // Clean up
  self->EventCallbackCommand->SetAbortFlag(1);
  self->Render();
}

//-------------------------------------------------------------------------
void vtkPlanarSplitWidget::MoveAction(vtkAbstractWidget *w)
{
  vtkPlanarSplitWidget *self = reinterpret_cast<vtkPlanarSplitWidget*>(w);
  if(self->m_permanentlyDisabled)
    return;

  // Do nothing if in start mode or valid handle not selected
  if (self->WidgetState == vtkPlanarSplitWidget::Start)
    return;

  // Delegate the event consistent with the state
  if (self->WidgetState == vtkPlanarSplitWidget::Define)
  {
    int X = self->Interactor->GetEventPosition()[0];
    int Y = self->Interactor->GetEventPosition()[1];
    double e[2];
    e[0] = static_cast<double>(X);
    e[1] = static_cast<double>(Y);
    reinterpret_cast<vtkPlanarSplitRepresentation2D*>(self->WidgetRep)->WidgetInteraction(e);
    self->InvokeEvent(vtkCommand::InteractionEvent, NULL);
    self->EventCallbackCommand->SetAbortFlag(1);
  }
  else
    //must be moving a handle, invoke a event for the handle widgets
    self->InvokeEvent(vtkCommand::MouseMoveEvent, NULL);

  self->WidgetRep->BuildRepresentation();
  self->Render();
}

//-------------------------------------------------------------------------
void vtkPlanarSplitWidget::EndSelectAction(vtkAbstractWidget *w)
{
  vtkPlanarSplitWidget *self = reinterpret_cast<vtkPlanarSplitWidget*>(w);
  if(self->m_permanentlyDisabled)
    return;

  // Do nothing if outside
  if ( self->WidgetState == vtkPlanarSplitWidget::Start ||
       self->WidgetState == vtkPlanarSplitWidget::Define ||
       self->CurrentHandle < 0 )
    {
    return;
    }

  self->ReleaseFocus();
  self->InvokeEvent(vtkCommand::LeftButtonReleaseEvent,NULL);
  self->CurrentHandle = -1;
  self->WidgetRep->BuildRepresentation();
  self->EventCallbackCommand->SetAbortFlag(1);
  self->Render();
}

// This is a callback that is active when the user is manipulating the
// handles of the widget.
//----------------------------------------------------------------------
void vtkPlanarSplitWidget::StartHandleInteraction(int handle)
{
  if(m_permanentlyDisabled)
    return;

  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];
  this->CurrentHandle = handle;
  reinterpret_cast<vtkPlanarSplitRepresentation2D*>(this->WidgetRep)->MoveHandle(handle, X,Y);
}

//----------------------------------------------------------------------
void vtkPlanarSplitWidget::HandleInteraction(int handle)
{
  if(m_permanentlyDisabled)
    return;

  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  reinterpret_cast<vtkPlanarSplitRepresentation2D*>(this->WidgetRep)->MoveHandle(handle, X,Y);
}

//----------------------------------------------------------------------
void vtkPlanarSplitWidget::StopHandleInteraction(int handle)
{
  if(m_permanentlyDisabled)
    return;

  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];
  this->CurrentHandle = -1;
  reinterpret_cast<vtkPlanarSplitRepresentation2D*>(this->WidgetRep)->MoveHandle(handle, X,Y);
}

//----------------------------------------------------------------------
void vtkPlanarSplitWidget::SetProcessEvents(int pe)
{
  this->Superclass::SetProcessEvents(pe);

  this->m_point1Widget->SetProcessEvents(pe);
  this->m_point2Widget->SetProcessEvents(pe);
}

//----------------------------------------------------------------------
void vtkPlanarSplitWidget::SetWidgetStateToStart()
{
  this->WidgetState = vtkPlanarSplitWidget::Start;
  this->CurrentHandle = -1;
  this->ReleaseFocus();
  this->GetRepresentation()->BuildRepresentation();
  this->SetEnabled(this->GetEnabled()); // show/hide the handles properly
}

//----------------------------------------------------------------------
void vtkPlanarSplitWidget::SetWidgetStateToManipulate()
{
  this->WidgetState = vtkPlanarSplitWidget::Manipulate;
  this->CurrentHandle = -1;
  this->ReleaseFocus();
  this->GetRepresentation()->BuildRepresentation();
  this->SetEnabled(this->GetEnabled()); // show/hide the handles properly
}

//----------------------------------------------------------------------
void vtkPlanarSplitWidget::setSlice(double slice)
{
  if(!this->WidgetRep)
    return;

  m_slice = slice;
  vtkPlanarSplitRepresentation2D *widget = reinterpret_cast<vtkPlanarSplitRepresentation2D*>(this->WidgetRep);
  widget->setSlice(slice);
}

//----------------------------------------------------------------------
void vtkPlanarSplitWidget::setPoints(vtkSmartPointer<vtkPoints> points)
{
  vtkPlanarSplitRepresentation2D *widget = reinterpret_cast<vtkPlanarSplitRepresentation2D*>(this->WidgetRep);
  widget->setPoints(points);
}

//----------------------------------------------------------------------
vtkSmartPointer<vtkPoints> vtkPlanarSplitWidget::getPoints()
{
  if (this->WidgetState != vtkPlanarSplitWidget::Manipulate)
  {
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    return points;
  }

  vtkPlanarSplitRepresentation2D *widget = reinterpret_cast<vtkPlanarSplitRepresentation2D*>(this->WidgetRep);
  return widget->getPoints();
}

//----------------------------------------------------------------------
void vtkPlanarSplitWidget::setOrientation(Plane plane)
{
  m_plane = plane;
  if (this->WidgetRep)
    reinterpret_cast<vtkPlanarSplitRepresentation2D*>(this->WidgetRep)->setOrientation(m_plane);
}

//----------------------------------------------------------------------
void vtkPlanarSplitWidget::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------
void vtkPlanarSplitWidget::disableWidget()
{
  this->SetEnabled(false);
  this->SetProcessEvents(false);
  m_permanentlyDisabled = true;
  static_cast<vtkPlanarSplitRepresentation2D*>(this->WidgetRep)->removeBoundsActor();
}

//----------------------------------------------------------------------
void vtkPlanarSplitWidget::setSegmentationBounds(double *bounds)
{
  memcpy(m_segmentationBounds, bounds, sizeof(double)*6);
  if (this->WidgetRep)
    static_cast<vtkPlanarSplitRepresentation2D*>(this->WidgetRep)->setSegmentationBounds(bounds);
}
