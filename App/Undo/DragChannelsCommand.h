/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_UNDO_DRAGCHANNELSCOMMAND_H_
#define APP_UNDO_DRAGCHANNELSCOMMAND_H_

// Qt
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Types.h>
#include <QUndoStack>

namespace ESPINA
{
  class ChannelProxy;

  /** \class DragChannelsCommand
   * \brief Handles the changes in the model when a channel is moved from one sample to another.
   *
   */
  class DragChannelsCommand
  : public QUndoCommand
  {
    public:
      /** \brief DragChannelsCommand class constructor.
       * \param[in] model model of the items.
       * \param[in] channels channels being dragged from one sample to the other.
       * \param[in] sample Sample destination of the channels.
       * \param[in] proxy proxy to signal modification.
       *
       */
      explicit DragChannelsCommand(ModelAdapterSPtr model, ChannelAdapterList channels, SampleAdapterSPtr sample, ChannelProxy *proxy = nullptr);

      /** \brief DragChannelsCommand class virtual destructor.
       *
       */
      virtual ~DragChannelsCommand()
      {};

      virtual void redo() override;

      virtual void undo() override;

    private:
      /** \brief Helper method to emit the modified signals to the items in the model.
       *
       */
      void emitSignals();

      /** \brief Helper method to move a channel from a sample to another.
       * \param[in] channel channel to move.
       * \param[in] from sample origin of the channels.
       * \param[in] to sample destination of the channels.
       *
       */
      void move(ChannelAdapterPtr channel, SampleAdapterSPtr from, SampleAdapterSPtr to);

      QMap<ChannelAdapterPtr, SampleAdapterSPtr> m_channels; /** map to channel and corresponding origin sample. */
      ModelAdapterSPtr                           m_model;    /** model that contains the items.                  */
      SampleAdapterSPtr                          m_sample;   /** sample destination of the channels.             */
      ChannelProxy                              *m_proxy;    /** channel proxy to signal modification.           */
  };

} // namespace ESPINA

#endif // APP_UNDO_DRAGCHANNELSCOMMAND_H_
