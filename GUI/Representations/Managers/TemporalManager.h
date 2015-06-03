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

#ifndef ESPINA_TEMPORAL_MANAGER_H
#define ESPINA_TEMPORAL_MANAGER_H

#include <GUI/Representations/RepresentationManager.h>

namespace ESPINA
{
  namespace GUI
  {
    namespace Representations
    {
      namespace Managers
      {
        class EspinaGUI_EXPORT TemporalRepresentation
        : public QObject
        {
        public:
          /** \brief Class constructor.
           *
           */
          explicit TemporalRepresentation() {}

          /** \brief Class virtual destructor.
           *
           */
          virtual ~TemporalRepresentation() {}

          /** \brief Initializes temporal representation for view
           *
           */
          virtual void initialize(RenderView *view) = 0;

          /** \brief
           *
           */
          virtual void uninitialize() = 0;

          /** \brief Displays temporal representation
           *
           */
          virtual void show() = 0;

          /** \brief Hides temporal representation
           *
           */
          virtual void hide() = 0;

          virtual bool isEnabled() = 0;

          virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const = 0;

          virtual void setCrosshair(const NmVector3 &crosshair) {}

          virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const = 0;

          virtual void setSceneResolution(const NmVector3 &resolution) {}
        };

        using TemporalRepresentationSPtr = std::shared_ptr<TemporalRepresentation>;

        class TemporalRepresentation2D;
        using TemporalRepresentation2DSPtr = std::shared_ptr<TemporalRepresentation2D>;

        class TemporalRepresentation3D;
        using TemporalRepresentation3DSPtr = std::shared_ptr<TemporalRepresentation3D>;

        class TemporalRepresentation2D
        : public TemporalRepresentation
        {
        public:
          virtual void setPlane(Plane plane) = 0;

          virtual void setRepresentationDepth(Nm depth) = 0;

          virtual TemporalRepresentation2DSPtr clone() = 0;

        };

        class TemporalRepresentation3D
        : public TemporalRepresentation
        {
        public:
          virtual TemporalRepresentation3DSPtr clone() = 0;
        };

        class TemporalPrototypes
        {
        public:
          explicit TemporalPrototypes(TemporalRepresentation2DSPtr prototype2D, TemporalRepresentation3DSPtr prototype3D);

          ViewTypeFlags supportedViews() const;

          TemporalRepresentation2DSPtr createRepresentation2D() const;

          TemporalRepresentation3DSPtr createRepresentation3D() const;

        private:
          TemporalRepresentation2DSPtr m_prototype2D;
          TemporalRepresentation3DSPtr m_prototype3D;
        };

        using TemporalPrototypesSPtr = std::shared_ptr<TemporalPrototypes>;

        class EspinaGUI_EXPORT TemporalManager
        : public RepresentationManager
        , public RepresentationManager2D
        {
        public:
          explicit TemporalManager(TemporalPrototypesSPtr factory, ManagerFlags flags = ManagerFlags());

          virtual ~TemporalManager();

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
          TemporalPrototypesSPtr m_prototypes;

          Plane m_plane;
          Nm    m_depth;

          enum Type {INIT, CROSSHAIR, RESOLUTION };

          using Action = QPair<Type, NmVector3>;

          RangedValue<Action> m_pendingActions;

          TemporalRepresentationSPtr  m_representation;
        };
      }
    }
  }
}

#endif // ESPINA_TEMPORAL_MANAGER_H
