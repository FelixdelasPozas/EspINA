/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_CF_REPRESENTATION_MANAGER_3D_H
#define ESPINA_CF_REPRESENTATION_MANAGER_3D_H

#include <GUI/Representations/RepresentationManager.h>

#include "CountingFrameManager.h"

namespace ESPINA
{
  namespace CF
  {
    class CFRepresentationSwitch;

    class RepresentationManager3D
    : public GUI::Representations::RepresentationManager
    {
        Q_OBJECT
      public:
        /** \brief RepresentationManager3D class constructor.
         * \param[in] manager counting frame manager.
         * \param[in] supportedViews types of views supported by this manager.
         *
         */
        explicit RepresentationManager3D(CountingFrameManager &manager, ViewTypeFlags supportedViews);

        /** \brief RepresentationManager3D class constructor.
         *
         */
        virtual ~RepresentationManager3D();

        virtual ViewItemAdapterList pick(const NmVector3 &point, vtkProp *actor) const override;

        /** \brief Sets the opacity of the representations.
         * \param[in] opacity opacity value in range [0,1].
         *
         */
        void setOpacity(float opacity);

        /** \brief Returns the opacity value of the managed representations.
         *
         */
        const float opacity() const;

        /** \brief Sets the switch managing the opacity options for the representations of this manager.
         * \param[in] cfSwitch
         */
        void setSwitch(CFRepresentationSwitch *cfSwitch);

      protected:
        virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const override
        { return false; }

        virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const override
        { return false; }

        virtual bool acceptSceneBoundsChange(const Bounds &bounds) const override
        { return false; }

        virtual bool acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const
        { return false; }

      private slots:
        /** \brief Helper method to update internal data when a CF is created.
         *
         */
        void onCountingFrameCreated(CountingFrame *cf);

        /** \brief Helper method to update internal data when a CF is removed.
         *
         */
        void onCountingFrameDeleted(CountingFrame *cf);

        /** \brief Updates the representations' opacity.
         * \param[in] opacity opacity value in range [0,1].
         *
         */
        void onOpacityChanged(float opacity);

      private:
        virtual bool hasRepresentations() const override;

        virtual void updateFrameRepresentations(const GUI::Representations::FrameCSPtr frame) override;

        virtual void onShow(const GUI::Representations::FrameCSPtr frame) override;

        virtual void onHide(const GUI::Representations::FrameCSPtr frame) override;

        virtual void displayRepresentations(const GUI::Representations::FrameCSPtr frame) override;

        virtual void hideRepresentations(const GUI::Representations::FrameCSPtr frame) override;

        virtual GUI::Representations::RepresentationManagerSPtr cloneImplementation() override;

        /** \brief Creates and returns a 3D widget for the given counting frame.
         * \param[in] cf counting frame object pointer.
         *
         */
        vtkCountingFrame3DWidget *createWidget(CountingFrame *cf);

        /** \brief Shows the given widget.
         * \param[in] widget 3D widget.
         *
         */
        void showWidget(vtkCountingFrameWidget *widget);

        /** \brief Hides the given widget.
         * \param[in] widget 3D widget.
         *
         */
        void hideWidget(vtkCountingFrameWidget *widget);

        /** \brief Hides and deletes the widgets for the given counting frame.
         * \param[in] cf counting frame object pointer.
         *
         */
        void deleteWidget(CountingFrame *cf);

      private:
        CountingFrameManager  &m_manager;                            /** counting frame manager.                          */
        QList<CountingFrame *> m_pendingCFs;                         /** list of counting frames pending widget creation. */
        QMap<CountingFrame *, vtkCountingFrame3DWidget *> m_widgets; /** map of created widgets - counting frames.        */
        CFRepresentationSwitch* m_switch;                            /** this representation switch.                      */
    };
  }
}

#endif // ESPINA_CF_REPRESENTATION_MANAGER_3D_H
