/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 */

#ifndef UNDO_CHANGESEGMENTATIONSSTACK_H_
#define UNDO_CHANGESEGMENTATIONSSTACK_H_

#include "Undo/EspinaUndo_Export.h"

// ESPINA
#include <GUI/Types.h>

// Qt
#include <QMap>
#include <QUndoStack>

namespace ESPINA
{
  namespace Undo
  {
    /** \class ChangeSegmentationsStack
     * \brief Undocommand to change the stack of a list of segmentations.
     *
     */
    class EspinaUndo_EXPORT ChangeSegmentationsStack
    : public QUndoCommand
    {
      public:
        /** \brief ChangeSegmentationsTack class constructor.
         * \param[in] segmentations List of segmentations to change stack.
         * \param[in] stack ChannelAdapter to change the segmentations to.
         * \param[in] parent Raw pointer of parent undo command.
         *
         *
         */
        explicit ChangeSegmentationsStack(const SegmentationAdapterList &segmentations, const ChannelAdapterPtr stack, QUndoCommand *parent = nullptr);

        /** \brief ChangeSegmentationsStack class constructor.
         * \param[in] segmentation SegmentationAdapter to change stack.
         * \param[in] stack ChannelAdapter to change the segmentations to.
         * \param[in] parent Raw pointer of parent undo command.
         *
         */
        explicit ChangeSegmentationsStack(const SegmentationAdapterPtr segmentation, const ChannelAdapterPtr stack, QUndoCommand *parent = nullptr);

        /** \brief ChangeSegmentationsStack class virtual destructor.
         *
         */
        virtual ~ChangeSegmentationsStack()
        {}

        virtual void redo() override;
        virtual void undo() override;

      private:
        struct Data
        {
          SegmentationAdapterPtr segmentation;
          ChannelAdapterSList    stacks;
          SampleAdapterSList     samples;
        };

        QList<struct Data> m_segmentations; /** maps segmentation to its initial stack. */
        ChannelAdapterPtr  m_newStack;      /** stack to change segmentations to.       */
        SampleAdapterSPtr  m_newSample;     /** sample of the new stack.                */
    };
  
  } // namespace Undo
} // namespace ESPINA

#endif // UNDO_CHANGESEGMENTATIONSSTACK_H_
