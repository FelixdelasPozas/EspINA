/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_GUI_VIEW_WIDGETS_MEASURES_MEASUREEVENTHANDLER_H
#define ESPINA_GUI_VIEW_WIDGETS_MEASURES_MEASUREEVENTHANDLER_H

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/View/EventHandler.h>

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        namespace Measures
        {
          /** \class MeasureEventHandler
           * \brief Event handler for the measures tool.
           *
           */
          class EspinaGUI_EXPORT MeasureEventHandler
          : public EventHandler
          {
            Q_OBJECT
          public:
            /** \brief MeasureEventHandler class constructor.
             *
             */
            MeasureEventHandler()
            { setCursor(Qt::CrossCursor); }

            virtual bool filterEvent(QEvent *e, RenderView *view = nullptr);

          signals:
            void clear();
          };

          using MeasureEventHandlerSPtr = std::shared_ptr<MeasureEventHandler>;
        }
      }
    }
  }
}

#endif // ESPINA_GUI_VIEW_WIDGETS_MEASURES_MEASUREEVENTHANDLER_H
