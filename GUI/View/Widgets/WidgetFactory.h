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

#ifndef ESPINA_VIEW_WIDGETS_WIDGETFACTORY_H
#define ESPINA_VIEW_WIDGETS_WIDGETFACTORY_H

#include <GUI/View/ViewTypeFlags.h>
#include "GUI/View/Widgets/EspinaWidget2.h"

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        class WidgetFactory
        {
        public:
          explicit WidgetFactory(EspinaWidget2DSPtr prototype2D, EspinaWidget3DSPtr prototype3D);

          ViewTypeFlags supportedViews() const;

          EspinaWidget2DSPtr createWidget2D() const;

          EspinaWidget3DSPtr createWidget3D() const;

        private:
          EspinaWidget2DSPtr m_prototype2D;
          EspinaWidget3DSPtr m_prototype3D;
        };
      }
    }
  }
}

#endif // ESPINA_VIEW_WIDGETS_WIDGETFACTORY_H
