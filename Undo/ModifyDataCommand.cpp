/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include "ModifyDataCommand.h"

namespace ESPINA
{
  //----------------------------------------------------------------------------
  RemoveDataCommand::RemoveDataCommand(OutputSPtr output, const Data::Type &type)
  : m_output{output}
  {
    Q_ASSERT(output->hasData(type));
    //TODO BUG m_data = output->writeLockData<Data>(type);
  }

  //----------------------------------------------------------------------------
  void RemoveDataCommand::redo()
  {
    m_output->removeData(m_data->type());
  }

  //----------------------------------------------------------------------------
  void RemoveDataCommand::undo()
  {
    m_output->setData(m_data);
  }

  //----------------------------------------------------------------------------
  AddDataCommand::AddDataCommand(OutputSPtr output, DataSPtr data)
  : m_output {output}
  , m_data   {data}
  , m_oldData{nullptr}
  {
  }

  //----------------------------------------------------------------------------
  void AddDataCommand::redo()
  {
    if(m_output->hasData(m_data->type()))
    {
      //TODO BUG m_oldData = m_output->data(m_data->type());
    }

    m_output->setData(m_data);
  }

  //----------------------------------------------------------------------------
  void AddDataCommand::undo()
  {
    if(m_oldData != nullptr)
    {
      m_output->setData(m_oldData);
    }
    else
    {
      m_output->removeData(m_data->type());
    }
  }

} // namespace EspINA
