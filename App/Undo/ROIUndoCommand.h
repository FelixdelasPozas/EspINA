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

#ifndef ESPINA_ROI_UNDO_COMMAND_H_
#define ESPINA_ROI_UNDO_COMMAND_H_

// EspINA
#include <App/ToolGroups/VOI/VolumeOfInterestTools.h>
#include <Support/ViewManager.h>

// Qt
#include <QUndoStack>

namespace EspINA
{
  class ModifyROIUndoCommand
  : public QUndoCommand
  {
    public:
      /* \brief ModifyROIUndoCommand class constructor.
       * \param[in] toolgroup VOIToolsGroup containing VOI accumulator.
       * \param[in] mask Mask to create/modify ROI.
       */
      explicit ModifyROIUndoCommand(VOIToolsGroup *toolgroup, const BinaryMaskSPtr<unsigned char> mask);

      /* \brief ModifyROIUndoCommand class virtual destructor.
       *
       */
      virtual ~ModifyROIUndoCommand();

      /* \brief Implements QUndoCommand::redo.
       *
       */
      virtual void redo();

      /* \brief Implements QUndoCommand::undo.
       *
       */
      virtual void undo();

    private:
      ROISPtr                       m_newROI;
      VOIToolsGroup                *m_toolGroup;
      BinaryMaskSPtr<unsigned char> m_mask;
  };

  class ClearROIUndoCommand
  : public QUndoCommand
  {
    public:
      /* \brief ClearROIUndoCommand class constructor.
       * \param[in] toolgroup ToolGroup that owns the ROI accumulator.
       *
       */
      explicit ClearROIUndoCommand(VOIToolsGroup *toolgroup);

      /* \brief ClearROIUndoCommand class virtual destructor.
       *
       */
      virtual ~ClearROIUndoCommand();

      /* \brief Implements QUndoCommand::redo.
       *
       */
      virtual void redo();

      /* \brief Implements QUndoCommand::undo.
       *
       */
      virtual void undo();

    private:
      VOIToolsGroup *m_toolGroup;
      ROISPtr        m_roi;
  };

} // namespace EspINA

#endif // ESPINA_ROI_UNDO_COMMAND_H_
