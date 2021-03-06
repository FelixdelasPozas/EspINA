/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_ZOOM_EVENT_HANDLER_H_
#define ESPINA_ZOOM_EVENT_HANDLER_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/View/EventHandler.h>

class QEvent;
class QPoint;

namespace ESPINA
{
  class RenderView;
  
  /** \class ZoomEventHandler
   *  \brief Event handler for Zoom tool.
   *
   */
  class EspinaGUI_EXPORT ZoomEventHandler
  : public EventHandler
  {
      Q_OBJECT
    public:
      /** \brief ZoomEventHandler class constructor.
       *
       */
      ZoomEventHandler();

      /** \brief ZoomEventHandler class virtual destructor.
       *
       */
      virtual ~ZoomEventHandler();

      virtual bool filterEvent(QEvent *e, RenderView *view = nullptr);

    signals:
      void movement(QPoint point, RenderView *view);
      void leftPress(QPoint point, RenderView *view);
      void leftRelease(QPoint point, RenderView *view);

    private:
      bool m_isDragging; /** true if there has been a mouse button down event but not a mouse button release event. */
  };

  using ZoomEventHandlerSPtr = std::shared_ptr<ZoomEventHandler>;

} // namespace ESPINA

#endif // ESPINA_ZOOM_EVENT_HANDLER_H_
