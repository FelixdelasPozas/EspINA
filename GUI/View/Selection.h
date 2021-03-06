/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_SELECTION_H
#define ESPINA_SELECTION_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>

// Qt
#include <QObject>

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      class EspinaGUI_EXPORT Selection: public QObject
      {
        Q_OBJECT
      public:
        /** \brief Selection class constructor.
         * \param[in] invalidator reference to the view state representation invalidator.
         *
         */
        explicit Selection();

        /** \brief Sets the given list of channels as selected.
         * \param[in] selection, list of channel adapter raw pointers.
         *
         */
        void set(ChannelAdapterList selection);

        /** \brief Sets the given list of segmentations as selected.
         * \param[in] selection, list of segmentation adapter raw pointers.
         *
         */
        void set(SegmentationAdapterList selection);

        /** \brief Sets the given list of view items as selected.
         * \param[in] selection, list of view item adapter raw pointers.
         *
         */
        void set(ViewItemAdapterList selection);

        /** \brief Deselects the given items (if the selection contains the items).
         * \param[in] items items to deselect.
         *
         */
        void unset(ViewItemAdapterList itemList);

        /** \brief Sets the given channel as the active one (the one used for segmentation).
         * \param[in] channel channel item.
         *
         */
        void setActiveChannel(ChannelAdapterPtr channel);

        /** \brief Returns the active channel.
         *
         */
        ChannelAdapterPtr activeChannel() const;

        /** \brief Returns the list of selected channels.
         *
         */
        ChannelAdapterList channels() const;

        /** \brief Returns the list of selected segmentations.
         *
         */
        SegmentationAdapterList segmentations() const;

        /** \brief Returns the list of selected items.
         *
         */
        ViewItemAdapterList items() const;

        /** \brief Clears the selection.
         *
         */
        void clear();

        /** \brief Emit selection state changed
         *
         */
        void modified();

      signals:
        /** \brief These signals are emitted when the selection changes in any way
         *  (contents, ordering of the selection, etc). The channels or segmentations
         *  emitted as value are the ones that have changed its state in any way
         *  (passed from selected to not selected and vice-versa).
         *
         */
        void selectionStateChanged();
        void selectionStateChanged(ChannelAdapterList);
        void selectionStateChanged(SegmentationAdapterList);

        /** \brief These signals are emitted when the contents of the selection have
         * changed. The channels and segmentations emitted as value are the elements
         * contained in the selection.
         *
         */
        void selectionChanged(ChannelAdapterList);
        void selectionChanged(SegmentationAdapterList);
        void selectionChanged();

        void activeChannelChanged(ChannelAdapterPtr);

      private:
        /** \brief Helper method to set the given list of channels as selected.
         * \param[in] list list of channel adapter raw pointers.
         */
        ChannelAdapterList setChannels(ChannelAdapterList list);

        /** \brief Helper method to set the given list of segmentations as selected.
         * \param[in] list list of segmentation adapter raw pointers.
         */
        SegmentationAdapterList setSegmentations(SegmentationAdapterList list);

        /** \brief Helper method to set the given list of items as selected.
         * \param[in] list list of view items raw pointers.
         *
         */
        void setItems(ViewItemAdapterList items);

        /** \brief Helper method to signal the selection state change when the selected
         * channels list is modified.
         * \param[in] channels list of channels or empty.
         *
         */
        void onChannelsModified(ChannelAdapterList channels);

        /** \brief Helper method to signal the selection state change when the selected
         * segmentations list is modified.
         * \param[in] segmentations list of segmentations or empty.
         *
         */
        void onSegmentationsModified(SegmentationAdapterList segmentations);

        /** \brief Helper method that implements items() method without mutex.
         *
         */
        ViewItemAdapterList items_implementation() const;

        ChannelAdapterList      m_channels;
        SegmentationAdapterList m_segmentations;
        ChannelAdapterPtr       m_activeChannel;
        mutable QMutex          m_mutex;
      };

      using SelectionSPtr = std::shared_ptr<Selection>;
    }
  }
}

#endif // ESPINA_SELECTION_H
