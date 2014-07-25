/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef UNLOADCHANNELCOMMAND_H
#define UNLOADCHANNELCOMMAND_H

#include <QtGui/QUndoStack>
#include <Core/Model/Channel.h>

namespace ESPINA
{
  class UnloadChannelCommand
  : public QUndoCommand
  {
  public:
    explicit UnloadChannelCommand(ChannelPtr   channel,
                                  EspinaModel  *model,
                                  QUndoCommand *parent = 0);

    virtual void redo();

    virtual void undo();

  private:
    EspinaModel *m_model;

    SampleSPtr  m_sample;
    FilterSPtr  m_reader;
    ChannelSPtr m_channel;
  };

} // namespace ESPINA

#endif // UNLOADCHANNELCOMMAND_H
