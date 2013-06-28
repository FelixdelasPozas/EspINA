/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

 This program is free software: you can redistribute it and/or modify
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

// EspINA
#include "vtkTubularWidget.h"
#include "vtkTubularRepresentation.h"

// VTK
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkEvent.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkWidgetEvent.h>
#include <vtkWidgetEventTranslator.h>

namespace EspINA
{

  vtkStandardNewMacro(vtkTubularWidget);

  //----------------------------------------------------------------------------
  vtkTubularWidget::vtkTubularWidget()
  : RoundedExtremes(false)
  , Plane(AXIAL)
  {
    this->WidgetState = vtkTubularWidget::Start;
    this->ManagesCursor = 1;

    this->TranslationEnabled = 1;
    this->ScalingEnabled = 1;

    // Define widget events
    this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier, 0, 0, NULL,
        vtkWidgetEvent::Select, this, vtkTubularWidget::SelectAction);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent, vtkEvent::NoModifier, 0, 0, NULL,
        vtkWidgetEvent::EndSelect, this, vtkTubularWidget::EndSelectAction);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent, vtkEvent::ControlModifier, 0, 0, NULL,
        vtkWidgetEvent::Translate, this, vtkTubularWidget::TranslateAction);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent, vtkEvent::ControlModifier, 0, 0, NULL,
        vtkWidgetEvent::EndTranslate, this, vtkTubularWidget::EndSelectAction);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent, vtkEvent::ShiftModifier, 0, 0, NULL,
        vtkWidgetEvent::Translate, this, vtkTubularWidget::TranslateAction);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent, vtkEvent::ShiftModifier, 0, 0, NULL,
        vtkWidgetEvent::EndTranslate, this, vtkTubularWidget::EndSelectAction);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent, vtkWidgetEvent::Scale, this,
        vtkTubularWidget::ScaleAction);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent, vtkWidgetEvent::EndScale, this,
        vtkTubularWidget::EndSelectAction);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent, vtkWidgetEvent::Move, this,
        vtkTubularWidget::MoveAction);
  }

  //----------------------------------------------------------------------------
  vtkTubularWidget::~vtkTubularWidget()
  {
  }

  //----------------------------------------------------------------------
  void vtkTubularWidget::SelectAction(vtkAbstractWidget *w)
  {
    // We are in a static method, cast to ourself
    vtkTubularWidget *self = reinterpret_cast<vtkTubularWidget*>(w);

    // Get the event position
    int X = self->Interactor->GetEventPosition()[0];
    int Y = self->Interactor->GetEventPosition()[1];

    // Okay, make sure that the pick is in the current renderer
    if (!self->CurrentRenderer || !self->CurrentRenderer->IsInViewport(X, Y))
    {
      self->WidgetState = vtkTubularWidget::Start;
      return;
    }

    // Begin the widget interaction which has the side effect of setting the
    // interaction state.
    double e[2];
    e[0] = static_cast<double>(X);
    e[1] = static_cast<double>(Y);
    self->WidgetRep->StartWidgetInteraction(e);
    self->WidgetRep->WidgetInteraction(e);
    int interactionState = self->WidgetRep->GetInteractionState();

    // We are definitely selected
    self->WidgetState = vtkTubularWidget::Active;
    self->GrabFocus(self->EventCallbackCommand);

    // The SetInteractionState has the side effect of highlighting the widget
    reinterpret_cast<vtkTubularRepresentation*>(self->WidgetRep)->SetInteractionState(interactionState);

    // start the interaction
    self->EventCallbackCommand->SetAbortFlag(1);
    self->StartInteraction();
    self->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
    self->Render();
  }

  //----------------------------------------------------------------------
  void vtkTubularWidget::TranslateAction(vtkAbstractWidget *w)
  {
//   // We are in a static method, cast to ourself
//   vtkTubularWidget *self = reinterpret_cast<vtkTubularWidget*>(w);
// 
//   // Get the event position
//   int X = self->Interactor->GetEventPosition()[0];
//   int Y = self->Interactor->GetEventPosition()[1];
// 
//   // Okay, make sure that the pick is in the current renderer
//   if ( !self->CurrentRenderer || 
//        !self->CurrentRenderer->IsInViewport(X,Y) )
//     {
//     self->WidgetState = vtkTubularWidget::Start;
//     return;
//     }
//   
//   // Begin the widget interaction which has the side effect of setting the
//   // interaction state.
//   double e[2];
//   e[0] = static_cast<double>(X);
//   e[1] = static_cast<double>(Y);
//   self->WidgetRep->StartWidgetInteraction(e);
//   int interactionState = self->WidgetRep->GetInteractionState();
//   if ( interactionState == vtkSpineRepresentation::Outside )
//     {
//     return;
//     }
//   
//   // We are definitely selected
//   self->WidgetState = vtkTubularWidget::Active;
//   self->GrabFocus(self->EventCallbackCommand);
//   reinterpret_cast<vtkSpineRepresentation*>(self->WidgetRep)->
//     SetInteractionState(vtkSpineRepresentation::Translating);
//   
//   // start the interaction
//   self->EventCallbackCommand->SetAbortFlag(1);
//   self->StartInteraction();
//   self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
//   self->Render();
  }

  //----------------------------------------------------------------------
  void vtkTubularWidget::ScaleAction(vtkAbstractWidget *w)
  {
//   return; //NOTE: Disabled
//   // We are in a static method, cast to ourself
//   vtkTubularWidget *self = reinterpret_cast<vtkTubularWidget*>(w);
// 
//   // Get the event position
//   int X = self->Interactor->GetEventPosition()[0];
//   int Y = self->Interactor->GetEventPosition()[1];
//   
//   // Okay, make sure that the pick is in the current renderer
//   if ( !self->CurrentRenderer || 
//        !self->CurrentRenderer->IsInViewport(X,Y) )
//     {
//     self->WidgetState = vtkTubularWidget::Start;
//     return;
//     }
//   
//   // Begin the widget interaction which has the side effect of setting the
//   // interaction state.
//   double e[2];
//   e[0] = static_cast<double>(X);
//   e[1] = static_cast<double>(Y);
//   self->WidgetRep->StartWidgetInteraction(e);
//   int interactionState = self->WidgetRep->GetInteractionState();
//   if ( interactionState == vtkSpineRepresentation::Outside )
//     {
//     return;
//     }
//   
//   // We are definitely selected
//   self->WidgetState = vtkTubularWidget::Active;
//   self->GrabFocus(self->EventCallbackCommand);
//   reinterpret_cast<vtkSpineRepresentation*>(self->WidgetRep)->
//     SetInteractionState(vtkSpineRepresentation::Scaling);
// 
//   // start the interaction
//   self->EventCallbackCommand->SetAbortFlag(1);
//   self->StartInteraction();
//   self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
//   self->Render();
  }

  //----------------------------------------------------------------------
  void vtkTubularWidget::MoveAction(vtkAbstractWidget *w)
  {
    vtkTubularWidget *self = reinterpret_cast<vtkTubularWidget*>(w);

    // compute some info we need for all cases
    int X = self->Interactor->GetEventPosition()[0];
    int Y = self->Interactor->GetEventPosition()[1];

    int interactionState = self->WidgetRep->GetInteractionState();
    if (self->WidgetState == vtkTubularWidget::Active && vtkTubularRepresentation::CreatingNode == interactionState)
      return;
    // See whether we're active
    if (self->WidgetState == vtkTubularWidget::Start)
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
    self->InvokeEvent(vtkCommand::InteractionEvent, NULL);
    self->Render();
  }

  //----------------------------------------------------------------------
  void vtkTubularWidget::SetCursor(int state)
  {
    switch (state)
    {
      case vtkTubularRepresentation::CreatingNode:
        this->RequestCursorShape(VTK_CURSOR_DEFAULT);
        break;
      case vtkTubularRepresentation::MovingNode:
        this->RequestCursorShape(VTK_CURSOR_SIZEALL);
        break;
      case vtkTubularRepresentation::ChangingRadius:
        this->RequestCursorShape(VTK_CURSOR_SIZEWE);
        break;
      default:
        this->RequestCursorShape(VTK_CURSOR_DEFAULT);
        break;
    };
  }

  //----------------------------------------------------------------------
  void vtkTubularWidget::EndSelectAction(vtkAbstractWidget *w)
  {
    vtkTubularWidget *self = reinterpret_cast<vtkTubularWidget*>(w);
    if (self->WidgetState == vtkTubularWidget::Start)
      return;

    int interactionState = self->WidgetRep->GetInteractionState();
    if (vtkTubularRepresentation::CreatingNode == interactionState)
    {
      reinterpret_cast<vtkTubularRepresentation*>(self->WidgetRep)->SetInteractionState(
          vtkTubularRepresentation::ChangingRadius);
      self->WidgetState = vtkTubularWidget::Active;
      self->SetCursor(vtkTubularRepresentation::ChangingRadius);
    }
    else
    {
      if (vtkTubularRepresentation::ChangingRadius == interactionState)
      {
        // The SetInteractionState has the side effect of highlighting the widget
        reinterpret_cast<vtkTubularRepresentation*>(self->WidgetRep)->SetInteractionState(
            vtkTubularRepresentation::CreatingNode);
        self->SetCursor(vtkTubularRepresentation::CreatingNode);
      }
      self->WidgetState = vtkTubularWidget::Start;
      self->ReleaseFocus();
    }

    self->EventCallbackCommand->SetAbortFlag(1);
    self->EndInteraction();
    self->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
    self->Render();
    self->SetCursor(9999);
  }

  //----------------------------------------------------------------------
  void vtkTubularWidget::SetPlane(PlaneType plane)
  {
    this->Plane = plane;

    if (!this->WidgetRep)
      this->CreateDefaultRepresentation();

    vtkTubularRepresentation *rep = reinterpret_cast<vtkTubularRepresentation*>(this->WidgetRep);
    rep->SetPlane(plane);
  }

  //----------------------------------------------------------------------
  void vtkTubularWidget::SetSlice(Nm slice)
  {
    if (!this->WidgetRep)
      this->CreateDefaultRepresentation();

    vtkTubularRepresentation *rep = reinterpret_cast<vtkTubularRepresentation*>(this->WidgetRep);
    rep->SetSlice(slice);
  }

  //----------------------------------------------------------------------
  TubularSegmentationFilter::NodeList vtkTubularWidget::GetNodeList()
  {
    if (!this->WidgetRep)
      this->CreateDefaultRepresentation();

    vtkTubularRepresentation *rep = reinterpret_cast<vtkTubularRepresentation*>(this->WidgetRep);
    return rep->GetNodeList();
  }

  //----------------------------------------------------------------------
  void vtkTubularWidget::SetNodeList(TubularSegmentationFilter::NodeList nodes)
  {
    if (!this->WidgetRep)
      this->CreateDefaultRepresentation();

    vtkTubularRepresentation *rep = reinterpret_cast<vtkTubularRepresentation*>(this->WidgetRep);
    rep->SetNodeList(nodes);
  }

  //----------------------------------------------------------------------
  void vtkTubularWidget::CreateDefaultRepresentation()
  {
    if (!this->WidgetRep)
      this->WidgetRep = vtkTubularRepresentation::New();
  }

  //----------------------------------------------------------------------------
  void vtkTubularWidget::PrintSelf(ostream& os, vtkIndent indent)
  {
    this->Superclass::PrintSelf(os, indent);

    os << indent << "Translation Enabled: " << (this->TranslationEnabled ? "On\n" : "Off\n");
    os << indent << "Scaling Enabled: " << (this->ScalingEnabled ? "On\n" : "Off\n");
    os << indent << "Rounded Extremes: " << (this->RoundedExtremes ? "On\n" : "Off\n");
  }

  //----------------------------------------------------------------------------
  void vtkTubularWidget::RoundedExtremesOn()
  {
    RoundedExtremes = true;
    if (!this->WidgetRep)
      this->CreateDefaultRepresentation();

    vtkTubularRepresentation *rep = dynamic_cast<vtkTubularRepresentation*>(this->WidgetRep);
    rep->setRoundExtremes(true);
  }

  //----------------------------------------------------------------------------
  void vtkTubularWidget::RoundedExtremesOff()
  {
    RoundedExtremes = false;
    if (!this->WidgetRep)
      this->CreateDefaultRepresentation();

    vtkTubularRepresentation *rep = dynamic_cast<vtkTubularRepresentation*>(this->WidgetRep);
    rep->setRoundExtremes(false);
  }
}
