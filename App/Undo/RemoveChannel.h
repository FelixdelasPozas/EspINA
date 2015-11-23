/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_UNDO_REMOVECHANNEL_H_
#define APP_UNDO_REMOVECHANNEL_H_

// ESPINA
#include <GUI/Types.h>
#include <Support/Context.h>

// Qt
#include <QUndoStack>

namespace ESPINA
{
  /** \class RemoveChannel
   * \brief Undo-able action to remove channels from the model.
   *
   */
  class RemoveChannel
  : public QUndoCommand
  {
    public:
      /** \brief RemoveChannelCommand class constructor.
       * \param[in] channel channel item to remove.
       * \param[in] model model containing the channel.
       *
       */
      explicit RemoveChannel(ChannelAdapterSPtr channel, Support::Context &context, QUndoCommand *parent = nullptr);

      /** \brief RemoveChannelCommand class virtual destuctor.
       *
       */
      virtual ~RemoveChannel();

      virtual void redo() override;

      virtual void undo() override;

    private:
      ChannelAdapterSPtr m_channel;   /** channel to remove. */
      SampleAdapterSPtr  m_sample;    /** sample containing the channel. */
      RelationList       m_relations; /** channel relations in the model. */
      Support::Context  &m_context;   /** current context. */
      bool               m_active;    /** true if the channel we're removing is the active one. */
  };

} // namespace ESPINA

#endif // APP_UNDO_REMOVECHANNEL_H_
