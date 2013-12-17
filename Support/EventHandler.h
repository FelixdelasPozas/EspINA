/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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
#ifndef ESPINA_EVENT_HANDLER_H_
#define ESPINA_EVENT_HANDLER_H_

#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QCursor>

class QEvent;

namespace EspINA
{
  class RenderView;

  class EventHandler
  : public QObject
  {
    Q_OBJECT
    public:
      explicit EventHandler();
      virtual ~EventHandler() {};

      virtual void setInUse(bool value);

      virtual bool filterEvent(QEvent *e, RenderView *view=nullptr);

      virtual QCursor cursor() const
      { return m_cursor; }

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

} /* namespace EspINA */

#endif // ESPINA_EVENT_HANDLER_H_
