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

#ifndef ESPINA_PLANAR_SPLIT_EVENTHANDLER_H_
#define ESPINA_PLANAR_SPLIT_EVENTHANDLER_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/View/EventHandler.h>

class QEvent;

namespace ESPINA
{
  class RenderView;

  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        class PlanarSplitWidget;
        using PlanarSplitWidgetPtr = PlanarSplitWidget *;

        /** \class PlanarSplitEventHandler
         * \brief Handle events for planar split widgets.
         *
         */
        class EspinaGUI_EXPORT PlanarSplitEventHandler
        : public EventHandler
        {
            Q_OBJECT
          public:
            /** \brief Class PlanarSplitEventHandler class constructor.
             *
             */
            PlanarSplitEventHandler();

            /** \brief Class PlanarSplitEventHandler class virtual destructor.
             *
             */
            virtual ~PlanarSplitEventHandler();

            virtual bool filterEvent(QEvent *e, RenderView *view);

            /** \brief Helper method to emit the plane defined signal
             * \param[in] widget widget use to define the plane.
             *
             */
            void emitPlaneDefined(PlanarSplitWidgetPtr widget)
            {
              emit planeDefined(widget);
            }

          signals:
            void planeDefined(GUI::View::Widgets::PlanarSplitWidgetPtr widget);
        };

        using PlanarSplitEventHandlerPtr  = PlanarSplitEventHandler *;
        using PlanarSplitEventHandlerSPtr = std::shared_ptr<PlanarSplitEventHandler>;
      
      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_PLANAR_SPLIT_EVENTHANDLER_H_
