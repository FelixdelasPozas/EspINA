/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINAINTERACTORADAPTER_H
#define ESPINAINTERACTORADAPTER_H

#include "GUI/EspinaGUI_Export.h"

// VTK
#include <vtkAbstractWidget.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkCallbackCommand.h>
#include <vtkWidgetEventTranslator.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

// Qt
#include <QEvent>
#include <QMouseEvent>

namespace ESPINA
{
  template<class T>
  class EspinaGUI_EXPORT EspinaInteractorAdapter
  : public T
  {
  public:
    /** \brief Instanciates a new object.
     *
     */
    static EspinaInteractorAdapter* New()
    {
      EspinaInteractorAdapter *result = new EspinaInteractorAdapter;
      vtkObjectFactory::ConstructInstance(result->GetClassName());
      return result;
    }

    /** \brief EspinaInteractorAdapter class virtual destructor.
     *
     */
    virtual ~EspinaInteractorAdapter()
    {}

    vtkTypeMacro(EspinaInteractorAdapter, T);

    /** \brief Process the given event.
     * \param[in] event, vtk event to process.
     *
     */
    bool ProcessEventsHandler(long unsigned int event)
    {
        this->EventCallbackCommand->SetAbortFlag(0);
        this->CallbackMapper->DebugOn();
        unsigned long int widgetEvent = this->CallbackMapper->GetEventTranslator()->GetTranslation(event);
        this->CallbackMapper->InvokeCallback(widgetEvent);

        return this->EventCallbackCommand->GetAbortFlag();
    }

    /** \brief Process Qt events.
     * \param[in] event, raw pointer of the QEvent to process.
     *
     */
    bool ProcessBasicQtEvent(QEvent *event)
    {
      if (QEvent::MouseButtonPress != event->type() &&
          QEvent::MouseButtonRelease != event->type() &&
          QEvent::MouseMove != event->type())
      {
        return false;
      }

      QMouseEvent *me = static_cast<QMouseEvent *>(event);

      // give interactor the event information
      vtkRenderWindowInteractor *iren = this->GetInteractor();

      int oldPos[2];
      iren->GetEventPosition(oldPos);
      iren->SetEventInformationFlipY(me->x(), me->y(), (me->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
          (me->modifiers() & Qt::ShiftModifier) > 0 ? 1 : 0, 0, me->type() == QEvent::MouseButtonDblClick ? 1 : 0);
      long unsigned int eventId = 0;

      const QEvent::Type t = event->type();
      if (t == QEvent::MouseMove)
      {
        eventId = vtkCommand::MouseMoveEvent;
      }
      else
        if (t == QEvent::MouseButtonPress || t == QEvent::MouseButtonDblClick)
        {
          switch (me->button())
          {
            case Qt::LeftButton:
              eventId = vtkCommand::LeftButtonPressEvent;
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
        else
          if (t == QEvent::MouseButtonRelease)
          {
            switch (me->button())
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

      bool handled = this->ProcessEventsHandler(eventId);

      if (!handled)
        iren->SetEventPosition(oldPos);

      return handled;
    }

  private:
    /** \brief EspinaInteractorAdapter class private constructor.
     *
     */
    explicit EspinaInteractorAdapter()
    {}

  };

}// namespace ESPINA

#endif // ESPINAINTERACTORADAPTER_H
