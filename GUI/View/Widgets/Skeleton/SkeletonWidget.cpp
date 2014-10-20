/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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
#include "SkeletonWidget.h"
#include "vtkSkeletonWidget.h"
#include <GUI/View/View2D.h>

// VTK
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

// Qt
#include <QKeyEvent>
#include <QEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>

namespace ESPINA
{
  //-----------------------------------------------------------------------------
  SkeletonWidget::SkeletonWidget()
  : m_command  {vtkSmartPointer<vtkSkeletonWidgetCommand>::New()}
  , m_tolerance{1}
  {
    m_command->setWidget(this);
  }
  
  //-----------------------------------------------------------------------------
  SkeletonWidget::~SkeletonWidget()
  {
    for(auto view: m_widgets.keys())
      unregisterView(view);

    m_widgets.clear();
  }

  //-----------------------------------------------------------------------------
  void SkeletonWidget::registerView(RenderView* view)
  {
    auto view2d = dynamic_cast<View2D *>(view);

    if (!view2d || m_widgets.keys().contains(view))
      return;

    auto plane = view2d->plane();
    auto slice = view2d->crosshairPoint()[normalCoordinateIndex(plane)];

    connect(view2d, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(changeSlice(Plane, Nm)), Qt::QueuedConnection);
    m_widgets[view] = vtkSkeletonWidget::New();
    m_widgets[view]->setParentWidget(this);
    m_widgets[view]->SetOrientation(plane);
    m_widgets[view]->changeSlice(plane, slice);
    m_widgets[view]->SetShift(view2d->widgetDepth());
    m_widgets[view]->SetCurrentRenderer(view->mainRenderer());
    m_widgets[view]->SetInteractor(view->renderWindow()->GetInteractor());
    m_widgets[view]->AddObserver(vtkCommand::EndInteractionEvent, m_command);
    m_widgets[view]->AddObserver(vtkCommand::ModifiedEvent, m_command);
  }

  //-----------------------------------------------------------------------------
  void SkeletonWidget::unregisterView(RenderView* view)
  {
    if (!m_widgets.keys().contains(view))
      return;

    auto view2d = dynamic_cast<View2D *>(view);
    disconnect(view2d, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(changeSlice(Plane, Nm)));

    m_widgets[view]->EnabledOff();
    m_widgets[view]->RemoveObserver(m_command);
    m_widgets[view]->SetInteractor(nullptr);
    m_widgets[view]->SetCurrentRenderer(nullptr);
    m_widgets[view]->Delete();
    m_widgets.remove(view);
  }

  //-----------------------------------------------------------------------------
  void SkeletonWidget::setEnabled(bool enable)
  {
    for(auto vtkWidget: m_widgets)
      vtkWidget->SetEnabled(enable);
  }

  //-----------------------------------------------------------------------------
  bool SkeletonWidget::filterEvent(QEvent* e, RenderView* view)
  {
    switch(e->type())
    {
      case QEvent::MouseButtonRelease:
      case QEvent::MouseButtonPress:
      {
        QMouseEvent *me = reinterpret_cast<QMouseEvent*>(e);
        if(me->button() == Qt::RightButton || me->button() == Qt::LeftButton)
        {
          for(auto view: m_widgets.keys())
          {
            if(view->rect().contains(view->mapFromGlobal(QCursor::pos())) && view->isVisible())
            {
              m_widgets[view]->GetInteractor()->SetEventInformationFlipY(me->x(),
                                                                         me->y(),
                                                                         Qt::ControlModifier == QApplication::keyboardModifiers(),
                                                                         Qt::ShiftModifier == QApplication::keyboardModifiers());

              if(e->type() == QEvent::MouseButtonPress)
              {
                if(me->button() == Qt::RightButton)
                  m_widgets[view]->GetInteractor()->RightButtonPressEvent();
                else
                  m_widgets[view]->GetInteractor()->LeftButtonPressEvent();
              }
              else
              {
                if(me->button() == Qt::RightButton)
                  m_widgets[view]->GetInteractor()->RightButtonReleaseEvent();
                else
                  m_widgets[view]->GetInteractor()->LeftButtonReleaseEvent();
              }
              return true;
            }
          }
        }
      }
        break;
      case QEvent::MouseMove:
      {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
        for(auto view: m_widgets.keys())
        {
          if(view->rect().contains(view->mapFromGlobal(QCursor::pos())) && view->isVisible())
          {
            m_widgets[view]->GetInteractor()->SetEventInformationFlipY(me->x(),
                                                                       me->y(),
                                                                       Qt::ControlModifier == QApplication::keyboardModifiers(),
                                                                       Qt::ShiftModifier == QApplication::keyboardModifiers());

            m_widgets[view]->GetInteractor()->MouseMoveEvent();
            return true;
          }
        }
      }
        break;
      case QEvent::KeyPress:
      case QEvent::KeyRelease:
      {
        QKeyEvent *ke = reinterpret_cast<QKeyEvent*>(e);

        if(ke->key() == Qt::Key_Control || ke->key() == Qt::Key_Backspace || ke->key() == Qt::Key_Delete)
        {
          const char *keyString;
          switch(ke->key())
          {
            case Qt::Key_Control:
              keyString = "Control_L";
              break;
            case Qt::Key_Delete:
              keyString = "Delete";
              break;
            case Qt::Key_Backspace:
              keyString = "BackSpace";
              break;
            default:
              break;
          }

          for(auto view: m_widgets.keys())
          {
            if(view->rect().contains(view->mapFromGlobal(QCursor::pos())) && view->isVisible())
            {
              int eventPos[2];
              view->renderWindow()->GetInteractor()->GetEventPosition(eventPos);
              m_widgets[view]->GetInteractor()->SetEventInformation(eventPos[0],
                                                                    eventPos[1],
                                                                    ke->key()==Qt::Key_Control,
                                                                    ke->key()==Qt::Key_Shift,
                                                                    ke->nativeScanCode(),
                                                                    1,
                                                                    keyString);

              if(e->type() == QEvent::KeyPress)
              {
                m_widgets[view]->GetInteractor()->KeyPressEvent();
              }
              else
              {
                m_widgets[view]->GetInteractor()->KeyReleaseEvent();
              }
              return true;
            }
          }
        }
      }
        break;
      default:
        break;
    }

    return false;
  }

  //-----------------------------------------------------------------------------
  void SkeletonWidget::setInUse(bool value)
  {
    if(m_inUse == value)
      return;

    m_inUse = value;

    emit eventHandlerInUse(value);
  }

  //-----------------------------------------------------------------------------
  void SkeletonWidget::setTolerance(const double value)
  {
    if(this->m_tolerance == value)
      return;

    this->m_tolerance = value;

    for(auto vtkWidget: this->m_widgets)
      vtkWidget->SetTolerance(m_tolerance);
  }

  //-----------------------------------------------------------------------------
  void SkeletonWidget::changeSlice(Plane plane, Nm slice)
  {
    for(auto vtkWidget: this->m_widgets)
      vtkWidget->changeSlice(plane, slice);
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidgetCommand::Execute(vtkObject* caller, unsigned long int eventId, void *callData)
  {
    if(strcmp("vtkSkeletonWidget", caller->GetClassName()) == 0)
    {
      if((eventId == vtkCommand::EndInteractionEvent) || (eventId == vtkCommand::ModifiedEvent))
      {
        auto callerWidget = dynamic_cast<vtkSkeletonWidget *>(caller);
        for(auto vtkWidget: m_widget->m_widgets)
        {
          if(vtkWidget == callerWidget)
            continue;

          vtkWidget->UpdateRepresentation();
        }

      }
    }
  }

  //-----------------------------------------------------------------------------
  vtkSmartPointer<vtkPolyData> SkeletonWidget::getSkeleton()
  {
    // all the vtkSkeletonWidgets should have the same data so anyone can suffice.
    if(m_widgets.isEmpty())
      return nullptr;

    return m_widgets.values().first()->getSkeleton();
  }

  //-----------------------------------------------------------------------------
  void SkeletonWidget::setRepresentationColor(const QColor &color)
  {
    for(auto vtkWidget: this->m_widgets)
      vtkWidget->setRepresentationColor(color);
  }

} // namespace ESPINA

