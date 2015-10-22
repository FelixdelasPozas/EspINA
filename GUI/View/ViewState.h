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

#ifndef ESPINA_VIEW_STATE_H
#define ESPINA_VIEW_STATE_H

// ESPINA
#include <Core/Utils/Bounds.h>
#include <Core/Types.h>
#include <Core/Utils/Vector3.hxx>
#include <GUI/Types.h>
#include <GUI/Utils/Timer.h>
#include <GUI/View/Selection.h>
#include "CoordinateSystem.h"
#include "EventHandler.h"
#include <GUI/Widgets/SliceSelector.h>

// Qt
#include <QObject>

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      class ViewState
      : public QObject
      {
        Q_OBJECT
      public:
        enum class Invalidate
        {
          SELECTED_ITEMS,
          DEPENDENT_ITEMS
        };

      public:
        /** \brief Class ViewState class constructor.
         * \param[in] timer state timer object.
         *
         */
        explicit ViewState();

        NmVector3 crosshair() const;

        SelectionSPtr selection() const;

        /** \brief Enables/disables the "fit to slices" flag.
         * \param[in] value true to enable false otherwise.
         *
         * If fit to slices is enabled the movement between slices is the resolution of the scene.
         *
         */
        void setFitToSlices(bool value);

        /** \brief Returns the value of the "fit to slices" flag.
         *
         */
        bool fitToSlices() const;

        /** \brief Sets the view event handler.
         * \param[in] handler active event handler
         *
         */
        void setEventHandler(EventHandlerSPtr handler);

        /** \brief Sets the view event handler.
         * \param[in] handler event handler to be deactivated
         *
         */
        void unsetEventHandler(EventHandlerSPtr handler);

        /** \brief Returns current event handler
         *
         */
        EventHandlerSPtr eventHandler() const;

        /** \brief Adds a widget to the view.
         * \param[in] widget espina widget smart pointer.
         *
         */
        void addTemporalRepresentations(Representations::Managers::TemporalPrototypesSPtr factory);

        /** \brief Removes a widget to the view.
         * \param[in] widget espina widget smart pointer.
         *
         */
        void removeTemporalRepresentations(Representations::Managers::TemporalPrototypesSPtr factory);

        /** \brief Adds slice selector to all views sharing the view state
         *
         */
        void addSliceSelectors(SliceSelectorSPtr selector, SliceSelectionType type);

        /** \brief Removes slice selector from all views sharing the view state
         *
         */
        void removeSliceSelectors(SliceSelectorSPtr selector);

        CoordinateSystemSPtr coordinateSystem() const;

        /** \brief Invalidates item representations
         * \param[in] items to invalide their representations
         * \param[in] scope of the invalidation.
         *
         */
        void invalidateRepresentations(const ViewItemAdapterList &items,
                                       const Invalidate scope = Invalidate::SELECTED_ITEMS);

        /** \brief Update item representation colors
         * \param[in] items to invalide their representation colors
         * \param[in] scope of the invalidation.
         *
         */
        void invalidateRepresentationColors(const ViewItemAdapterList &items,
                                            const Invalidate scope = Invalidate::SELECTED_ITEMS);

        void resetCamera();

        void refresh();

        void setScene(const NmVector3 &crosshair, const NmVector3 &resolution, const Bounds &bounds);

        Representations::FrameSPtr createFrame();

        Representations::FrameSPtr createFrame(const NmVector3 &point);

      public slots:
        /** \brief Changes the crosshair position to point
         *
         */
        void setCrosshair(const NmVector3 &point);

        /** \brief Changes the crosshair position of the given plane
         *
         */
        void setCrosshairPlane(const Plane plane, const Nm position);

        /** \brief Ensure point position is visible
         *
         */
        void focusViewOn(const NmVector3 &point);

        /** \brief Invalidates item representations
         *
         */
        void invalidateRepresentations(ViewItemAdapterPtr item);

      signals:
        void eventHandlerChanged();

        void frameChanged(const GUI::Representations::FrameCSPtr frame);

        void afterFrameChanged(const GUI::Representations::FrameCSPtr frame);

        void widgetsAdded(GUI::Representations::Managers::TemporalPrototypesSPtr factory, const GUI::Representations::FrameCSPtr frame);

        void widgetsRemoved(GUI::Representations::Managers::TemporalPrototypesSPtr factory, const GUI::Representations::FrameCSPtr frame);

        void sliceSelectorAdded(SliceSelectorSPtr selector, SliceSelectionType type);

        void sliceSelectorRemoved(SliceSelectorSPtr selector);

        void resetCamera(const GUI::Representations::FrameCSPtr frame);

        void refreshRequested();

        void representationsInvalidated(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame);

        void representationColorsInvalidated(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame);

      private slots:
        void selectionChanged(SegmentationAdapterList segmentations);

      private:
        NmVector3 crosshairPoint(const NmVector3 &point) const;

        NmVector3 voxelCenter(const NmVector3 &point) const;

        void changeCrosshair(const NmVector3 &point, bool focus = false);

        ViewItemAdapterList scopedItems(const ViewItemAdapterList &items,
                                        const Invalidate scope = Invalidate::SELECTED_ITEMS);

        void emitFrameChanged(const GUI::Representations::FrameCSPtr frame);

      private:
        Timer m_timer;

        bool                 m_fitToSlices;
        NmVector3            m_crosshair;
        CoordinateSystemSPtr m_coordinateSystem;
        SelectionSPtr        m_selection;

        EventHandlerSPtr m_eventHandler;
      };

      /** \brief Compute scene bounds and minimal resolution
       *
       */
      void updateSceneState(const NmVector3 &crosshair, ViewState & state, ViewItemAdapterSList viewItems);
    }
  }
}

#endif // ESPINA_VIEWSTATE_H
