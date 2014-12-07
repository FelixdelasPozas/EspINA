/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_UNDO_MODIFY_DATA_COMMAND_H_
#define ESPINA_UNDO_MODIFY_DATA_COMMAND_H_

#include "Undo/EspinaUndo_Export.h"

// ESPINA
#include <Core/Analysis/Data.h>
#include <Core/Analysis/Output.h>

// Qt
#include <QUndoStack>

namespace ESPINA
{

  class EspinaUndo_EXPORT RemoveDataCommand
  : public QUndoCommand
  {
    public:
      /** \brief RemoveDataCommand class constructor.
       * \param[in] type Data type to remove.
       *
       */
      RemoveDataCommand(OutputSPtr output, const Data::Type& type);

      /** \brief RemoveDataCommand class virtual destructor.
       *
       */
      virtual ~RemoveDataCommand()
      {}

      virtual void redo();
      virtual void undo();

    private:
      OutputSPtr m_output;
      DataSPtr   m_data;
  };

  class EspinaUndo_EXPORT AddDataCommand
  : public QUndoCommand
  {
    public:
      /** \brief RemoveDataCommand class constructor.
       * \param[in] type Data type to remove.
       *
       */
      AddDataCommand(OutputSPtr output, DataSPtr data);

      /** \brief RemoveDataCommand class virtual destructor.
       *
       */
      virtual ~AddDataCommand()
      {}

      virtual void redo();
      virtual void undo();

    private:
      OutputSPtr m_output;
      DataSPtr   m_data;
      DataSPtr   m_oldData;
  };


} // namespace ESPINA

#endif // ESPINA_UNDO_MODIFY_DATA_COMMAND_H_
