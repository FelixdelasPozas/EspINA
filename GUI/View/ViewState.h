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

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "CoordinateSystem.h"
#include "EventHandler.h"
#include <Core/Utils/Bounds.h>
#include <Core/Types.h>
#include <Core/Utils/Vector3.hxx>
#include <GUI/Representations/Frame.h>
#include <GUI/Types.h>
#include <GUI/Utils/Timer.h>
#include <GUI/View/Selection.h>
#include <GUI/Widgets/SliceSelector.h>

// Qt
#include <QObject>

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      /** \class ViewState
       * \brief Contains and exposes the current View's state.
       *
       */
      class EspinaGUI_EXPORT ViewState
      : public QObject
      {
          Q_OBJECT
        public:
          enum class Invalidate: char { SELECTED_ITEMS, DEPENDENT_ITEMS };

        public:
          /** \brief Class ViewState class constructor.
           *
           */
          explicit ViewState();

          /** \brief Returns the current view's state crosshair value.
           *
           */
          NmVector3 crosshair() const;

          /** \brief Returns the current view's state selection.
           *
           */
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
           * \param[in] factory temporal representations factory.
           *
           */
          void addTemporalRepresentations(Representations::Managers::TemporalPrototypesSPtr factory);

          /** \brief Removes a widget to the view.
           * \param[in] factory temporal representations factory.
           *
           */
          void removeTemporalRepresentations(Representations::Managers::TemporalPrototypesSPtr factory);

          /** \brief Returns true if the views are currently using the given representation factory.
           * \param[in] factory representation factory object.
           *
           */
          bool hasTemporalRepresentation(const Representations::Managers::TemporalPrototypesSPtr factory) const;

          /** \brief Adds slice selector to all views sharing the view state
           *
           */
          void addSliceSelectors(GUI::Widgets::SliceSelectorSPtr selector, GUI::Widgets::SliceSelectionType type);

          /** \brief Removes slice selector from all views sharing the view state
           *
           */
          void removeSliceSelectors(GUI::Widgets::SliceSelectorSPtr selector);

          /** \brief Returns the list of active widgets in the scene actually.
           *
           */
          const QList<GUI::Representations::Managers::TemporalPrototypesSPtr> activeWidgets() const
          { return m_activeWidgets; };

          CoordinateSystemSPtr coordinateSystem() const;

          /** \brief Invalidates item representations and returns the invalidation frame.
           * \param[in] items to invalide their representations
           * \param[in] scope of the invalidation.
           *
           */
          GUI::Representations::FrameCSPtr invalidateRepresentations(const ViewItemAdapterList &items,
                                                                     const Invalidate scope = Invalidate::SELECTED_ITEMS);

          /** \brief Update item representation colors and returns the invalidation frame.
           * \param[in] items to invalide their representation colors
           * \param[in] scope of the invalidation.
           *
           */
          GUI::Representations::FrameCSPtr invalidateRepresentationColors(const ViewItemAdapterList &items,
                                                                          const Invalidate scope = Invalidate::SELECTED_ITEMS);

          /** \brief Resets the cameras of all the views.
           *
           */
          void resetCamera();

          /** \brief Updates all the views.
           *
           */
          void refresh();

          /** \brief Sets the parameters of the scene.
           * \param[in] crosshair crosshair point.
           * \param[in] resolution scene's resolution.
           * \param[in] bounds scene's bounds.
           *
           */
          void setScene(const NmVector3 &crosshair, const NmVector3 &resolution, const Bounds &bounds);

          /** \brief Creates and returns a new frame.
           *
           */
          Representations::FrameCSPtr createFrame();

          /** \brief Returns the frame with the specified timestamp.
           * \param[in] t TimeStamp of the required frame.
           *
           */
          Representations::FrameCSPtr frame(TimeStamp t);

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

          /** \brief Invalidates item representations.
           *
           */
          void invalidateRepresentations(ViewItemAdapterPtr item);

        signals:
          void eventHandlerChanged();

          void cursorChanged();

          void frameChanged(const GUI::Representations::FrameCSPtr frame);

          void afterFrameChanged(const GUI::Representations::FrameCSPtr frame);

          void widgetsAdded(GUI::Representations::Managers::TemporalPrototypesSPtr factory, const GUI::Representations::FrameCSPtr frame);

          void widgetsRemoved(GUI::Representations::Managers::TemporalPrototypesSPtr factory, const GUI::Representations::FrameCSPtr frame);

          void sliceSelectorAdded(GUI::Widgets::SliceSelectorSPtr selector, GUI::Widgets::SliceSelectionType type);

          void sliceSelectorRemoved(GUI::Widgets::SliceSelectorSPtr selector);

          void resetViewCamera();

          void refreshRequested();

          void representationsInvalidated(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame);

          void representationColorsInvalidated(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame);

        private slots:
          void selectionChanged(SegmentationAdapterList segmentations);

          void updateSelection(ViewItemAdapterSList items);

        private:
          /** \brief Creates a frame with the given parameters and returns it.
           * \param[in] point frames crosshair point.
           * \param[in] options frame options
           *
           * NOTE: a frame with focus or reset to true is always a keyframe.
           *
           */
          Representations::FrameCSPtr createFrame(const NmVector3 &point, const GUI::Representations::Frame::Options options = GUI::Representations::Frame::Options());

          /** \brief Creates an invalidation frame for the given items and returns it.
           *
           */
          Representations::FrameCSPtr createInvalidationFrame(ViewItemAdapterList items);

          NmVector3 crosshairPoint(const NmVector3 &point) const;

          /** \brief Returns the voxel center point to the given point using the state's resolution.
           * \param[in] point point coordinates.
           *
           */
          NmVector3 voxelCenter(const NmVector3 &point) const;

          /** \brief Changes the state crosshair point.
           * \param[in] point new crosshair point coordinates.
           * \param[in] focus true to focus the views on the new crosshair point.
           *
           */
          void changeCrosshair(const NmVector3 &point, bool focus = false);

          /** \brief Returns the list of items related to the given ones using the model relationship graph.
           * \param[in] item view items list.
           * \param[in] scope scope of the relationship.
           *
           */
          ViewItemAdapterList scopedItems(const ViewItemAdapterList &items,
                                          const Invalidate scope = Invalidate::SELECTED_ITEMS);

          /** \brief Emits the frameChanged signal with the given frame.
           * \param[in] frame frame to emit as parameter.
           *
           */
          void emitFrameChanged(const GUI::Representations::FrameCSPtr frame);

        private:
          Timer                m_timer;             /** timer for frame numbers.                                                                      */
          bool                 m_fitToSlices;       /** true if the movement from a slice to the next is done in spacing increments, false otherwise. */
          NmVector3            m_crosshair;         /** position of the crosshair.                                                                    */
          CoordinateSystemSPtr m_coordinateSystem;  /** view's coordinate system (origin, resolution & bounds).                                       */
          SelectionSPtr        m_selection;         /** current view's selected items.                                                                */
          EventHandlerSPtr     m_eventHandler;      /** view's event handler.                                                                         */

          QList<GUI::Representations::Managers::TemporalPrototypesSPtr> m_activeWidgets; /** currently active widgets on the views. */

          QList<GUI::Representations::FrameCSPtr> m_frames; /** last frames buffer. */
      };

      /** \brief Compute scene bounds and minimal resolution in the given state for the given items.
       * \param[in] crosshair crosshair point.
       * \param[inout] state view state reference to modify.
       * \param[in] viewItems list of view items.
       *
       */
      void EspinaGUI_EXPORT updateSceneState(const NmVector3 &crosshair, ViewState & state, ViewItemAdapterSList viewItems);
    }
  }
}

#endif // ESPINA_VIEWSTATE_H
