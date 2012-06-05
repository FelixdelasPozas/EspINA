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

#include "vtkRectangularBoundingVolumeWidget.h"
#include "vtkRectangularBoundingVolumeRepresentation.h"
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
#include "vtkRectangularBoundingVolumeRepresentation.h"
#include <vtkPolyDataAlgorithm.h>


vtkStandardNewMacro(vtkRectangularBoundingVolumeWidget);

//----------------------------------------------------------------------------
vtkRectangularBoundingVolumeWidget::vtkRectangularBoundingVolumeWidget()
: Volume(NULL)
{
  this->WidgetState = vtkRectangularBoundingVolumeWidget::Start;
  this->ManagesCursor = 1;

  memset(InclusionOffset, 0, 3*sizeof(double));
  memset(ExclusionOffset, 0, 3*sizeof(double));
  
  // Define widget events
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::Select,
                                          this, vtkRectangularBoundingVolumeWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkEvent::NoModifier,
                                          0, 0, NULL,
                                          vtkWidgetEvent::EndSelect,
                                          this, vtkRectangularBoundingVolumeWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonReleaseEvent,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkRectangularBoundingVolumeWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                            vtkEvent::ControlModifier,
                                            0, 0, NULL,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkRectangularBoundingVolumeWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                            vtkEvent::ShiftModifier,
                                            0, 0, NULL,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkRectangularBoundingVolumeWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
                                          vtkWidgetEvent::EndScale,
                                          this, vtkRectangularBoundingVolumeWidget::EndSelectAction);
}

//----------------------------------------------------------------------------
vtkRectangularBoundingVolumeWidget::~vtkRectangularBoundingVolumeWidget()
{  
}

//----------------------------------------------------------------------
void vtkRectangularBoundingVolumeWidget::SelectAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkRectangularBoundingVolumeWidget *self = reinterpret_cast<vtkRectangularBoundingVolumeWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  
  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkRectangularBoundingVolumeWidget::Start;
    return;
    }
  
  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == vtkRectangularBoundingVolumeRepresentation::Outside )
    {
    return;
    }
  
  // We are definitely selected
  self->WidgetState = vtkRectangularBoundingVolumeWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);
  
  // The SetInteractionState has the side effect of highlighting the widget
  reinterpret_cast<vtkRectangularBoundingVolumeRepresentation*>(self->WidgetRep)->
    SetInteractionState(interactionState);
 
  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkRectangularBoundingVolumeWidget::SetCursor(int state)
{
    switch (state)
    {
      case vtkRectangularBoundingVolumeRepresentation::MoveLeft:
      case vtkRectangularBoundingVolumeRepresentation::MoveRight:
	  this->RequestCursorShape(VTK_CURSOR_SIZEWE);
	break;
      case vtkRectangularBoundingVolumeRepresentation::MoveTop:
      case vtkRectangularBoundingVolumeRepresentation::MoveBottom:
	  this->RequestCursorShape(VTK_CURSOR_SIZENS);
	break;
      case vtkRectangularBoundingVolumeRepresentation::MoveUpper:
      case vtkRectangularBoundingVolumeRepresentation::MoveLower:
	  this->RequestCursorShape(VTK_CURSOR_SIZENS);
	break;
      case vtkRectangularBoundingVolumeRepresentation::Outside:
	this->RequestCursorShape(VTK_CURSOR_DEFAULT);
	break;
      default:
	this->RequestCursorShape(VTK_CURSOR_DEFAULT);
    };
}


//----------------------------------------------------------------------
void vtkRectangularBoundingVolumeWidget::EndSelectAction(vtkAbstractWidget *w)
{
  vtkRectangularBoundingVolumeWidget *self = reinterpret_cast<vtkRectangularBoundingVolumeWidget*>(w);
  if ( self->WidgetState == vtkRectangularBoundingVolumeWidget::Start )
    {
    return;
    }
  
  // Return state to not active
  self->WidgetState = vtkRectangularBoundingVolumeWidget::Start;
  reinterpret_cast<vtkRectangularBoundingVolumeRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkRectangularBoundingVolumeRepresentation::Outside);
  self->ReleaseFocus();

  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  self->Render();
  self->SetCursor(9999);
}

//----------------------------------------------------------------------
void vtkRectangularBoundingVolumeWidget::SetVolume(vtkPolyDataAlgorithm *region)
{
  Volume = region;
  if (WidgetRep)
  {
    vtkRectangularBoundingVolumeRepresentation *rep =
      reinterpret_cast<vtkRectangularBoundingVolumeRepresentation*>(this->WidgetRep);
    rep->SetVolume(region);
    rep->reset();
  }
  else
    std::cout << "There is no representation" << std::endl;
}

  
//----------------------------------------------------------------------
void vtkRectangularBoundingVolumeWidget::CreateDefaultRepresentation()
{
  if ( ! this->WidgetRep )
    {
    this->WidgetRep = vtkRectangularBoundingVolumeRepresentation::New();
    reinterpret_cast<vtkRectangularBoundingVolumeRepresentation*>(this->WidgetRep)->SetVolume(Volume);
    }
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingVolumeWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


