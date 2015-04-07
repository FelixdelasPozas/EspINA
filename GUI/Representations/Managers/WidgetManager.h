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

#ifndef ESPINA_WIDGET_MANAGER_H
#define ESPINA_WIDGET_MANAGER_H

#include <GUI/Representations/RepresentationManager.h>
#include <GUI/View/Widgets/EspinaWidget.h>

namespace ESPINA
{
  class EspinaGUI_EXPORT WidgetManager
  : public RepresentationManager
  , public RepresentationManager2D
  {
  public:
    explicit WidgetManager(EspinaWidgetSPtr widget, ViewTypeFlags supportedViews);

    virtual void setResolution(const NmVector3 &resolution);

    virtual TimeRange readyRange() const;

    virtual void display(TimeStamp time);

    virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const;

  private:
    EspinaWidgetSPtr m_widget;
  };
}

#endif // ESPINA_WIDGET_MANAGER_H
