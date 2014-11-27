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

// ESPINA
#include "RenameSegmentationsCommand.h"

namespace ESPINA
{
  //-------------------------------------------------------------------------
  RenameSegmentationsCommand::RenameSegmentationsCommand(QMap<SegmentationAdapterPtr, QString> renames)
  : m_renames{renames}
  {
  }

  //-------------------------------------------------------------------------
  RenameSegmentationsCommand::~RenameSegmentationsCommand()
  {
  }

  //-------------------------------------------------------------------------
  void RenameSegmentationsCommand::undo()
  {
    redo();
  }

  //-------------------------------------------------------------------------
  void RenameSegmentationsCommand::redo()
  {
    for(auto seg: m_renames.keys())
    {
      auto alias = seg->data().toString();
      seg->setData(QVariant(m_renames[seg]), Qt::EditRole);
      m_renames[seg] = alias;

      seg->notifyModification();
    }
  }

} // namespace ESPINA
