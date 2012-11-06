/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#include "vtkBoundingRegion3DWidget.h"

#include "regions/vtkBoundingRegion3DRepresentation.h"


#include "vtkCallbackCommand.h"
#include "vtkCommand.h"
#include "vtkEvent.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkWidgetCallbackMapper.h" 
#include "vtkWidgetEvent.h"
#include "vtkWidgetEventTranslator.h"
#include <vtkPolyDataAlgorithm.h>

vtkStandardNewMacro(vtkBoundingRegion3DWidget);

//----------------------------------------------------------------------------
vtkBoundingRegion3DWidget::vtkBoundingRegion3DWidget()
: Volume(NULL)
{
  this->WidgetState = vtkBoundingRegion3DWidget::Start;
  this->ManagesCursor = 1;

  memset(InclusionOffset, 0, 3*sizeof(double));
  memset(ExclusionOffset, 0, 3*sizeof(double));

  // Define widget events
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Select,
                                          this, vtkBoundingRegion3DWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::EndSelect,
                                          this, vtkBoundingRegion3DWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonReleaseEvent,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkBoundingRegion3DWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                            vtkEvent::ControlModifier,
                                            0, 0, NULL,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkBoundingRegion3DWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                            vtkEvent::ShiftModifier,
                                            0, 0, NULL,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkBoundingRegion3DWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
                                          vtkWidgetEvent::EndScale,
                                          this, vtkBoundingRegion3DWidget::EndSelectAction);
}

//----------------------------------------------------------------------------
vtkBoundingRegion3DWidget::~vtkBoundingRegion3DWidget()
{
}

//----------------------------------------------------------------------
void vtkBoundingRegion3DWidget::SelectAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkBoundingRegion3DWidget *self = reinterpret_cast<vtkBoundingRegion3DWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkBoundingRegion3DWidget::Start;
    return;
    }

  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == vtkBoundingRegion3DRepresentation::Outside )
    {
    return;
    }

  // We are definitely selected
  self->WidgetState = vtkBoundingRegion3DWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);

  // The SetInteractionState has the side effect of highlighting the widget
  reinterpret_cast<vtkBoundingRegion3DRepresentation*>(self->WidgetRep)->
    SetInteractionState(interactionState);

  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkBoundingRegion3DWidget::SetCursor(int state)
{
  switch (state)
  {
    case vtkBoundingRegion3DRepresentation::MoveLeft:
    case vtkBoundingRegion3DRepresentation::MoveRight:
      this->RequestCursorShape(VTK_CURSOR_SIZEWE);
      break;
    case vtkBoundingRegion3DRepresentation::MoveTop:
    case vtkBoundingRegion3DRepresentation::MoveBottom:
      this->RequestCursorShape(VTK_CURSOR_SIZENS);
      break;
    case vtkBoundingRegion3DRepresentation::MoveUpper:
    case vtkBoundingRegion3DRepresentation::MoveLower:
      this->RequestCursorShape(VTK_CURSOR_SIZENS);
      break;
    case vtkBoundingRegion3DRepresentation::Outside:
      this->RequestCursorShape(VTK_CURSOR_DEFAULT);
      break;
    default:
      this->RequestCursorShape(VTK_CURSOR_DEFAULT);
  };
}


//----------------------------------------------------------------------
void vtkBoundingRegion3DWidget::EndSelectAction(vtkAbstractWidget *w)
{
  vtkBoundingRegion3DWidget *self = reinterpret_cast<vtkBoundingRegion3DWidget*>(w);
  if ( self->WidgetState == vtkBoundingRegion3DWidget::Start )
    {
    return;
    }

  // Return state to not active
  self->WidgetState = vtkBoundingRegion3DWidget::Start;
  reinterpret_cast<vtkBoundingRegion3DRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkBoundingRegion3DRepresentation::Outside);
  self->ReleaseFocus();

  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  self->Render();
  self->SetCursor(9999);
}

//----------------------------------------------------------------------
void vtkBoundingRegion3DWidget::SetBoundingRegion(vtkSmartPointer< vtkPolyData > region, Nm inclusionOffset[3], Nm exclusionOffset[3])
{
  if (!this->WidgetRep)
    CreateDefaultRepresentation();

  vtkBoundingRegion3DRepresentation *rep = reinterpret_cast<vtkBoundingRegion3DRepresentation*>(this->WidgetRep);
  rep->SetBoundingRegion(region);
  rep->reset();
}

//----------------------------------------------------------------------
void vtkBoundingRegion3DWidget::CreateDefaultRepresentation()
{
  if ( ! this->WidgetRep )
    this->WidgetRep = vtkBoundingRegion3DRepresentation::New();
}

//----------------------------------------------------------------------------
void vtkBoundingRegion3DWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


