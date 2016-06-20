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

#ifndef ESPINA_CF_REPRESENTATION_MANAGER_2D_H
#define ESPINA_CF_REPRESENTATION_MANAGER_2D_H

// ESPINA
#include <GUI/Representations/RepresentationManager.h>
#include <GUI/Types.h>
#include "CountingFrameManager.h"

namespace ESPINA
{
  namespace CF
  {
    class RepresentationManager2D
    : public GUI::Representations::RepresentationManager
    , public GUI::Representations::RepresentationManager2D
    {
        Q_OBJECT
      public:
        /** \brief RepresentationManager2D class constructor.
         * \param[in] manager counting frame manager.
         * \param[in] supportedViews views supported by this manager.
         *
         */
        explicit RepresentationManager2D(CountingFrameManager *manager, ViewTypeFlags supportedViews);

        /** \brief RepresentationManager2D class virtual destructor.
         *
         */
        virtual ~RepresentationManager2D();

        virtual ViewItemAdapterList pick(const NmVector3 &point, vtkProp *actor) const override;

        virtual void setPlane(Plane plane) override;

        virtual void setRepresentationDepth(Nm depth) override;

      protected:
        virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const override;

        virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const override
        { return true; }

        virtual bool acceptSceneBoundsChange(const Bounds &bounds) const override
        { return false; }

        virtual bool acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const
        { return false; }

      private slots:
        /** \brief Helper method to update internal data when a CF is created.
         * \param[in] cf Pointer of the created counting frame.
         *
         */
        void onCountingFrameCreated(CountingFrame *cf);

        /** \brief Helper method to update internal data when a CF is removed.
         * \param[in] cf Pointer of the deleted counting frame.
         *
         */
        void onCountingFrameDeleted(CountingFrame *cf);

      private:
        virtual bool hasRepresentations() const override;

        virtual bool needsRepresentationUpdate(const GUI::Representations::FrameCSPtr frame) override;

        virtual void updateFrameRepresentations(const GUI::Representations::FrameCSPtr frame) override;

        virtual void onShow(const GUI::Representations::FrameCSPtr frame) override;

        virtual void onHide(const GUI::Representations::FrameCSPtr frame) override;

        virtual void displayRepresentations(const GUI::Representations::FrameCSPtr frame) override;

        virtual void hideRepresentations(const GUI::Representations::FrameCSPtr frame) override;

        virtual GUI::Representations::RepresentationManagerSPtr cloneImplementation() override;

        /** \brief Returns the slicing position for a given time.
         * \param[in] t timestamp.
         *
         */
        Nm slicingPosition(TimeStamp t) const;

        /** \brief Creates an returns an slice widget for the given counting frame.
         * \param[in] cf counting frame object pointer to create the widget from.
         *
         */
        vtkCountingFrameSliceWidget *createWidget(CountingFrame *cf);

        /** \brief Shows the given widget for the given time.
         * \param[in] widget slice widget to show.
         * \param[in] t timestamp.
         *
         */
        void showWidget(vtkCountingFrameSliceWidget *widget, TimeStamp t);

        /** \brief Hides the given widget.
         * \param[in] widget slice widget to hide.
         *
         */
        void hideWidget(vtkCountingFrameSliceWidget *widget);

        /** \brief Hides the slice widgets for the given counting frame and deletes them.
         * \param[in] cf counting frame object pointer to remove the widgets.
         *
         */
        void deleteWidget(CountingFrame *cf);

        /** \brief Returns true if the value in the normal plane is different between two given points.
         * \param[in] p1 point 1 coordinates.
         * \param[in] p2 point 2 coordinates.
         *
         */
        bool isNormalDifferent(const NmVector3 &p1, const NmVector3 &p2) const;

      private:
        Plane  m_plane;  /** plane of the representations. */
        Nm     m_depth;  /** distance in Nm from the current crosshair where the representations will be shown. */

        CountingFrameManager  *m_manager;                                 /** counting frame manager. */
        QList<CountingFrame *> m_pendingCFs;                              /** list of counting frames pending widget creation. */
        QMap<CountingFrame *, vtkCountingFrameSliceWidget *> m_widgets;   /** map of counting frame - corresponding widget. */
    };
  }
}

#endif // ESPINA_CF_REPRESENTATION_MANAGER_2D_H
