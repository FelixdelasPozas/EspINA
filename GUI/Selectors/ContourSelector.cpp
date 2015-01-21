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
#include "ContourSelector.h"
#include <GUI/View/View2D.h>
#include <GUI/View/Widgets/Contour/vtkPlaneContourWidget.h>

// Qt
#include <QEvent>
#include <QCursor>
#include <QMouseEvent>
#include <QApplication>

// VTK
#include <vtkRenderWindowInteractor.h>

using namespace ESPINA;

ContourSelector::ContourSelector()
: m_widget{nullptr}
{
  setMultiSelection(false);
  setCursor(Qt::CrossCursor);
}

//-----------------------------------------------------------------------------
void ContourSelector::buildCursor()
{
  if(m_widget)
  {
    auto mode = (m_drawing ? BrushMode::BRUSH : BrushMode::ERASER);
    for(auto vtkWidget: m_widget->m_widgets.values())
    {
      vtkWidget->setPolygonColor(m_brushColor);
      vtkWidget->setContourMode(mode);
    }
  }
}

//-----------------------------------------------------------------------------
bool ContourSelector::filterEvent(QEvent* e, RenderView *view)
{
  auto ke = static_cast<QKeyEvent *>(e);
  auto me = static_cast<QMouseEvent *>(e);
  auto view2d = dynamic_cast<View2D*>(view);

  switch (e->type())
  {
    case QEvent::Leave:
    case QEvent::Enter:
      {
        updateCurrentDrawingMode(view);
      }
      break;
    case QEvent::KeyPress:
      {
        if ((ke->key() == Qt::Key_Shift) && !m_tracking && m_item && (m_item->type() == ViewItemAdapter::Type::SEGMENTATION))
        {
          updateCurrentDrawingMode(view);
          return true;
        }
      }
      break;
    case QEvent::KeyRelease:
      {
        if ((ke->key() == Qt::Key_Shift) && !m_tracking)
        {
          updateCurrentDrawingMode(view);
          return true;
        }
      }
      break;
    case QEvent::MouseButtonPress:
      {
        if(!(me->button() == Qt::MiddleButton) && view->rect().contains(view->mapFromGlobal(QCursor::pos())) && view->isVisible() && m_widget)
        {
          m_widget->m_widgets[view2d]->GetInteractor()->SetEventInformationFlipY(me->x(),
                                                                                 me->y(),
                                                                                 Qt::ControlModifier == QApplication::keyboardModifiers(),
                                                                                 Qt::ShiftModifier == QApplication::keyboardModifiers());
          // the crtl check is to avoid interference with View2D ctrl+click
          if (!QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
          {
            if (me->button() == Qt::LeftButton)
            {
              m_tracking = true;
              startStroke(me->pos(), view);
              m_widget->m_widgets[view2d]->GetInteractor()->LeftButtonPressEvent();
            }
            else
            {
              if (me->button() == Qt::RightButton)
              {
                m_widget->m_widgets[view2d]->GetInteractor()->RightButtonPressEvent();
              }
            }
            return true;
          }
        }
      }
      break;
    case QEvent::MouseMove:
      {
        if(view->rect().contains(view->mapFromGlobal(QCursor::pos())) && view->isVisible() && m_widget)
        {
          m_widget->m_widgets[view2d]->GetInteractor()->SetEventInformationFlipY(me->x(),
                                                                                 me->y(),
                                                                                 Qt::ControlModifier == QApplication::keyboardModifiers(),
                                                                                 Qt::ShiftModifier == QApplication::keyboardModifiers());

          if(m_tracking)
          {
            updateStroke(me->pos(), view);
          }

          m_widget->m_widgets[view2d]->GetInteractor()->MouseMoveEvent();
          return true;
        }
      }
      break;
    case QEvent::MouseButtonRelease:
      {
        if(!(me->button() == Qt::MiddleButton) && view->rect().contains(view->mapFromGlobal(QCursor::pos())) && view->isVisible() && m_widget)
        {
          m_widget->m_widgets[view2d]->GetInteractor()->SetEventInformationFlipY(me->x(),
                                                                                 me->y(),
                                                                                 Qt::ControlModifier == QApplication::keyboardModifiers(),
                                                                                 Qt::ShiftModifier == QApplication::keyboardModifiers());

          if (m_tracking && me->button() == Qt::LeftButton)
          {
            m_tracking = false;
            stopStroke(view);
          }

          if(me->button() == Qt::LeftButton)
          {
            m_widget->m_widgets[view2d]->GetInteractor()->LeftButtonReleaseEvent();
          }
          else
          {
            m_widget->m_widgets[view2d]->GetInteractor()->RightButtonReleaseEvent();
          }

          return true;
        }
      }
      break;
    default:
      break;
  }

  return false;
}

