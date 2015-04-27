/*
 * Copyright 2015 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef ESPINA_GUI_VIEW_WIDGETS_MEASURES_MEASUREEVENTHANDLER_H
#define ESPINA_GUI_VIEW_WIDGETS_MEASURES_MEASUREEVENTHANDLER_H

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
          class MeasureEventHandler
          : public EventHandler
          {
            Q_OBJECT
          public:
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
