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

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        class EspinaWidget;
        class WidgetFactory;
      }
    }

    namespace Representations
    {
      namespace Managers
      {
        using EspinaWidgetSPtr  = std::shared_ptr<GUI::View::Widgets::EspinaWidget>;
        using WidgetFactorySPtr = std::shared_ptr<GUI::View::Widgets::WidgetFactory>;

        class EspinaGUI_EXPORT WidgetManager
        : public RepresentationManager
        , public RepresentationManager2D
        {
        public:
          explicit WidgetManager(WidgetFactorySPtr factory, ManagerFlags flags = ManagerFlags());

          virtual ~WidgetManager();

          virtual TimeRange readyRangeImplementation() const;

          virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const;

          virtual void setPlane(Plane plane);

          virtual void setRepresentationDepth(Nm depth);

        protected:
          virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const;

          virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const;

          virtual bool acceptSceneBoundsChange(const Bounds &bounds) const;

        private:
          virtual bool hasRepresentations() const;

          virtual void updateRepresentations(const NmVector3 &crosshair, const NmVector3 &resolution, const Bounds &bounds, TimeStamp t);

          virtual void changeCrosshair(const NmVector3 &crosshair, TimeStamp t);

          virtual void changeSceneResolution(const NmVector3 &resolution, TimeStamp t);

          virtual void onShow(TimeStamp t);

          virtual void onHide(TimeStamp t);

          virtual void displayRepresentations(TimeStamp t);

          virtual void hideRepresentations(TimeStamp t);

          virtual RepresentationManagerSPtr cloneImplementation();

        private:
          WidgetFactorySPtr m_factory;

          Plane m_plane;
          Nm    m_depth;

          enum Type {INIT, CROSSHAIR, RESOLUTION };

          using Action = QPair<Type, NmVector3>;

          RangedValue<Action> m_pendingActions;

          EspinaWidgetSPtr  m_widget;
        };
      }
    }
  }
}

#endif // ESPINA_WIDGET_MANAGER_H
