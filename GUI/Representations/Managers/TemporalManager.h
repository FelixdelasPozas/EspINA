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

          virtual ViewItemAdapterList pick(const NmVector3 &point, vtkProp *actor) const override;

          virtual void setPlane(Plane plane) override;

          virtual void setRepresentationDepth(Nm depth) override;

        protected:
          virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const override;

          virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const override;

          virtual bool acceptSceneBoundsChange(const Bounds &bounds) const override;

        private:
          virtual bool hasRepresentations() const override;

          virtual void updateFrameRepresentations(const FrameCSPtr frame) override;

          virtual void changeCrosshair(const FrameCSPtr frame) override;

          virtual void changeSceneResolution(const FrameCSPtr frame) override;

          virtual void onShow(TimeStamp t) override;

          virtual void onHide(TimeStamp t) override;

          virtual void displayRepresentations(TimeStamp t) override;

          virtual void hideRepresentations(TimeStamp t) override;

          virtual RepresentationManagerSPtr cloneImplementation() override;

        private:
          TemporalPrototypesSPtr m_prototypes;

          Plane m_plane;
          Nm    m_depth;

          TemporalRepresentationSPtr  m_representation;
        };

        class AcceptOnlyPlaneCrosshairChanges
        {
        protected:
          AcceptOnlyPlaneCrosshairChanges();

          void acceptChangesOnPlane(Plane plane);

          bool acceptPlaneCrosshairChange(const NmVector3 &crosshair) const;

          void changeReslicePosition(const NmVector3 &crosshair);

          int slicingNormal() const
          { return m_normalIndex; }

          Nm reslicePosition() const
          { return m_reslicePosition; }

          Plane reslicePlane() const
          { return m_reslicePlane; }

          Nm normalCoordinate(const NmVector3 &value) const;

        private:
          int   m_normalIndex;
          Nm    m_reslicePosition;
          Plane m_reslicePlane;
        };
      }
    }
  }
}

#endif // ESPINA_TEMPORAL_MANAGER_H
