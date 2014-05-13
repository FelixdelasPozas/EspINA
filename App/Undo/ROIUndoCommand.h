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

#ifndef ESPINA_ROI_UNDO_COMMAND_H_
#define ESPINA_ROI_UNDO_COMMAND_H_

// EspINA
#include <Support/ViewManager.h>

// Qt
#include <QUndoStack>

namespace EspINA
{
  class ModifyROIUndoCommand
  : public QUndoCommand
  {
    public:
      /* \brief ModifyROIUndoCommand templated class constructor.
       * \param[in] vm ViewManager shared pointer.
       * \param[in] mask Templated mask to create ROI.
       * \param[in] value value of the voxels of the mask, defaults to SEG_VOXEL_VALUE
       */
      explicit ModifyROIUndoCommand(const ViewManagerSPtr vm, const BinaryMaskSPtr<unsigned char> mask, unsigned char value = SEG_VOXEL_VALUE);

      /* \brief ModifyROIUndoCommand class virtual destructor.
       *
       */
      virtual ~ModifyROIUndoCommand();

      /* \brief Implements QUndoCommand:redo.
       *
       */
      virtual void redo();

      /* \brief Implements QUndoCommand::undo.
       *
       */
      virtual void undo();

    private:
      ROISPtr                       m_newROI;
      ViewManagerSPtr               m_viewManager;
      BinaryMaskSPtr<unsigned char> m_mask;
      unsigned char                 m_value;
  };

} // namespace EspINA

#endif // ESPINA_ROI_UNDO_COMMAND_H_
