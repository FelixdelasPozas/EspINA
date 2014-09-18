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

#ifndef ESPINA_RENAME_SEGMENTATIONS_COMMAND_H_
#define ESPINA_RENAME_SEGMENTATIONS_COMMAND_H_

// ESPINA
#include <GUI/Model/SegmentationAdapter.h>

//Qt
#include <QUndoStack>
#include <QMap>

namespace ESPINA
{
  class RenameSegmentationsCommand
  : public QUndoCommand
  {
    public:
  		/* \brief RenameSegmentationsCommand class constructor.
  		 * \param[in] renames, map of segmentation names and new names.
  		 */
      explicit RenameSegmentationsCommand(QMap<SegmentationAdapterPtr, QString> renames);

      /* \brief RenameSegmentationsCommand class virtual destructor.
       *
       */
      virtual ~RenameSegmentationsCommand();

      /* \brief Overrides QUndoCommand::redo().
       *
       */
      virtual void redo() override;

      /* \brief Overrides QUndoCommand::undo().
       *
       */
      virtual void undo() override;

    private:
      QMap<SegmentationAdapterPtr, QString> m_renames;
  };

} // namespace ESPINA

#endif // ESPINA_RENAME_SEGMENTATIONS_COMMAND_H_
