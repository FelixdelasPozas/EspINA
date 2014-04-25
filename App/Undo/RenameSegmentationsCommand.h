/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

 This program is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_RENAME_SEGMENTATIONS_COMMAND_H_
#define ESPINA_RENAME_SEGMENTATIONS_COMMAND_H_

// EspINA
#include <GUI/Model/SegmentationAdapter.h>

//Qt
#include <QUndoStack>
#include <QMap>

namespace EspINA
{
  class RenameSegmentationsCommand
  : public QUndoCommand
  {
    public:
      explicit RenameSegmentationsCommand(QMap<SegmentationAdapterPtr, QString> renames);
      virtual ~RenameSegmentationsCommand();

      virtual void redo();
      virtual void undo();

    private:
      QMap<SegmentationAdapterPtr, QString> m_renames;
  };

} // namespace EspINA

#endif // ESPINA_RENAME_SEGMENTATIONS_COMMAND_H_