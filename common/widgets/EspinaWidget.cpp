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


#include "EspinaWidget.h"
#include <EspinaRenderView.h>

#include <vtkAbstractWidget.h>
#include <vtkWidgetRepresentation.h>

#include <QMouseEvent>
#include <vtkCommand.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

//----------------------------------------------------------------------------
SliceWidget::SliceWidget(vtkAbstractWidget *widget)
: m_widget(widget)
{
}

//----------------------------------------------------------------------------
bool EspinaWidget::filterEvent(QEvent* e, EspinaRenderView* view)
{
  if ( QEvent::MouseButtonPress != e->type()
    && QEvent::MouseButtonRelease != e->type()
    && QEvent::MouseMove != e->type() )
    return false;

  QMouseEvent *me = static_cast<QMouseEvent *>(e);

  QMouseEvent* e2 = static_cast<QMouseEvent*>(e);

  // give interactor the event information
  vtkRenderWindowInteractor *iren = view->renderWindow()->GetInteractor();

  int oldPos[2];
  iren->GetEventPosition(oldPos);
  iren->SetEventInformationFlipY(e2->x(), e2->y(),
                                 (e2->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
                                 (e2->modifiers() & Qt::ShiftModifier ) > 0 ? 1 : 0,
                                 0,
                                 e2->type() == QEvent::MouseButtonDblClick ? 1 : 0);
  long unsigned int eventId = 0;

  const QEvent::Type t = e->type();
  if(t == QEvent::MouseMove)
  {
    eventId = vtkCommand::MouseMoveEvent;
  }
  else if(t == QEvent::MouseButtonPress || t == QEvent::MouseButtonDblClick)
  {
    switch(me->button())
    {
      case Qt::LeftButton:
        eventId =vtkCommand::LeftButtonPressEvent;
        break;

      case Qt::MidButton:
        eventId = vtkCommand::MiddleButtonPressEvent;
        break;

      case Qt::RightButton:
        eventId = vtkCommand::RightButtonPressEvent;
        break;

      default:
        break;
    }
  }
  else if(t == QEvent::MouseButtonRelease)
  {
    switch(me->button())
    {
      case Qt::LeftButton:
        eventId = vtkCommand::LeftButtonReleaseEvent;
        break;

      case Qt::MidButton:
        eventId = vtkCommand::MiddleButtonReleaseEvent;
        break;

      case Qt::RightButton:
        eventId = vtkCommand::RightButtonReleaseEvent;
        break;

      default:
        break;
    }
  }

  bool handled = processEvent(iren, eventId);

  if (!handled)
    iren->SetEventPosition(oldPos);

  return handled;
}
