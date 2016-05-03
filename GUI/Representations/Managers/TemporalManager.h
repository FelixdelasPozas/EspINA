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

          /** \brief Initializes temporal representation for the given view
           * \param[in] view view to show the temporal representations.
           *
           */
          virtual void initialize(RenderView *view) = 0;

          /** \brief De-initializes the temporal representation.
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

          /** \brief Returns true if the representation needs to be updated after a crosshair change.
           * \param[in] crosshair scene's new crosshair.
           *
           */
          virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const = 0;

          /** \brief Returns true if the representation needs to be updated after a resolution change.
           * \param[in] resolution scene's new resolution.
           *
           */
          virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const = 0;

          /** \brief Returns true if the representation needs to be updated after a bounds change.
           * \param[in] bounds scene's new bounds.
           *
           */
          virtual bool acceptSceneBoundsChange(const Bounds &bounds) const = 0;

          /** \brief Returns true if the representation needs to be updated after an invalidation frame.
           * \param[in] frame const invalidation frame.
           *
           */
          virtual bool acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const = 0;

          /** \brief Updates the representation for the given frame.
           * \param[in] frame const frame object.
           *
           */
          virtual void display(const FrameCSPtr &frame) {};
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
            /** \brief Sets the orientation of the 2D representations.
             * \param[in] plane plane enum value.
             *
             */
            virtual void setPlane(Plane plane) = 0;

            /** \brief Sets the depth of the representations in relation to the current scene crosshair.
             * \param[in] depth distace from the current crosshair where the representations should be shown.
             *
             */
            virtual void setRepresentationDepth(Nm depth) = 0;

            /** \brief Clones the representation.
             *
             */
            virtual TemporalRepresentation2DSPtr clone() = 0;
        };

        class TemporalRepresentation3D
        : public TemporalRepresentation
        {
          public:
            /** \brief Clones the representation.
             *
             */
            virtual TemporalRepresentation3DSPtr clone() = 0;
        };

        class TemporalPrototypes
        {
          public:
            /** \brief TemporalPrototypes class constructor.
             * \param[in] prototype2D temporal representation for 2D views.
             * \param[in] prototype3D temporal representation for 3D views.
             * \param[in] name name of the temporal prototypes object.
             *
             */
            explicit TemporalPrototypes(TemporalRepresentation2DSPtr prototype2D, TemporalRepresentation3DSPtr prototype3D, const QString &name);

            /** \brief Returns the types of views supported by the temporal representations.
             *
             */
            ViewTypeFlags supportedViews() const;

            /** \brief Clones and returns a 2D representation.
             *
             */
            TemporalRepresentation2DSPtr createRepresentation2D() const;

            /** \brief Clones and returns a 3D representation.
             *
             */
            TemporalRepresentation3DSPtr createRepresentation3D() const;

            /** \brief Returns the name of the temporal prototypes object.
             *
             */
            QString name() const;

          private:
            TemporalRepresentation2DSPtr m_prototype2D; /** 2D temporal representation. */
            TemporalRepresentation3DSPtr m_prototype3D; /** 3D temporal representation. */
            QString                      m_name;        /** name of the object.         */
        };

        using TemporalPrototypesSPtr = std::shared_ptr<TemporalPrototypes>;

        class EspinaGUI_EXPORT TemporalManager
        : public RepresentationManager
        , public RepresentationManager2D
        {
          public:
            /** \brief TemporalManager class constructor.
             * \param[in] factory temporal prototypes object.
             * \param[in] flags manager's flags values.
             *
             */
            explicit TemporalManager(TemporalPrototypesSPtr factory, ManagerFlags flags = ManagerFlags());

            /** \brief TemporalManager class virtual destructor.
             *
             */
            virtual ~TemporalManager();

            virtual ViewItemAdapterList pick(const NmVector3 &point, vtkProp *actor) const override;

            virtual void setPlane(Plane plane) override;

            virtual void setRepresentationDepth(Nm depth) override;

          protected:
            virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const override;

            virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const override;

            virtual bool acceptSceneBoundsChange(const Bounds &bounds) const override;

            virtual bool acceptInvalidationFrame(const FrameCSPtr frame) const override;

          private:
            virtual bool hasRepresentations() const override;

            virtual void updateFrameRepresentations(const FrameCSPtr frame) override;

            virtual void onShow(const FrameCSPtr frame) override;

            virtual void onHide(const FrameCSPtr frame) override;

            virtual void displayRepresentations(const GUI::Representations::FrameCSPtr frame) override;

            virtual void hideRepresentations(const GUI::Representations::FrameCSPtr frame) override;

            virtual RepresentationManagerSPtr cloneImplementation() override;

          private:
            TemporalPrototypesSPtr m_prototypes;          /** manager's temporal representations. */
            Plane m_plane;                                /** plane of the representations. */
            Nm    m_depth;                                /** distance from the current crosshair to show the representations. */
            TemporalRepresentationSPtr  m_representation; /** representation currently managed. */
        };
      }
    }
  }
}

#endif // ESPINA_TEMPORAL_MANAGER_H
