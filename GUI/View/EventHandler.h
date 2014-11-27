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

#ifndef ESPINA_EVENT_HANDLER_H_
#define ESPINA_EVENT_HANDLER_H_

#include "GUI/EspinaGUI_Export.h"

// Qt
#include <QObject>
#include <QCursor>

// C++
#include <memory>

class QEvent;

namespace ESPINA
{
  class RenderView;

  class EspinaGUI_EXPORT EventHandler
  : public QObject
  {
    Q_OBJECT
    public:
      /** \brief EventHandler class constructor.
       *
       */
      explicit EventHandler();

      /** \brief EventHandler class virtual destructor.
       *
       */
      virtual ~EventHandler()
      {};

      /** \brief Activates/deactvates the event handler.
       * \param[in] value true to enable false otherwise.
       */
      virtual void setInUse(bool value);

      /** \brief Perform the operations needed to events in the given view.
       * \param[in] e raw pointer of the event received.
       * \param[in] view raw pointer of the view of the event.
       *
       */
      virtual bool filterEvent(QEvent *e, RenderView *view = nullptr);

      /** \brief Returns the cursor of the event handler.
       *
       */
      virtual QCursor cursor() const
      { return m_cursor; }

      /** \brief Sets the cursor of the event handler.
       *
       */
      virtual void setCursor(const QCursor& cursor)
      { m_cursor = cursor; }

    signals:
      void eventHandlerInUse(bool);

    protected:
      bool    m_inUse;
      QCursor m_cursor;
  };

  using EventHandlerPtr  = EventHandler *;
  using EventHandlerSPtr = std::shared_ptr<EventHandler>;

} /* namespace ESPINA */

#endif // ESPINA_EVENT_HANDLER_H_
