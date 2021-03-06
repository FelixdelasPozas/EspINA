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

#ifndef ESPINA_ROI_UNDO_COMMAND_H_
#define ESPINA_ROI_UNDO_COMMAND_H_

// ESPINA
#include <App/ToolGroups/Restrict/RestrictToolGroup.h>

// Qt
#include <QUndoStack>

#include "Core/Types.h"
namespace ESPINA
{
  class AddROIUndoCommand
  : public QUndoCommand
  {
    public:
      /** \brief AddROIUndoCommand class constructor.
       * \param[in] toolgroup, raw pointer of the ROIToolsGroup that has the ROI accumulator.
       * \param[in] mask, Mask to create/modify ROI.
       */
      explicit AddROIUndoCommand(RestrictToolGroup *toolgroup, const BinaryMaskSPtr<unsigned char> mask);

      /** \brief AddROIUndoCommand class virtual destructor.
       *
       */
      virtual ~AddROIUndoCommand();

      virtual void redo() override;

      virtual void undo() override;

    private:
      ROISPtr                       m_newROI;
      RestrictToolGroup            *m_toolGroup;
      BinaryMaskSPtr<unsigned char> m_mask;
      Bounds                        m_bounds;
      itkVolumeType::Pointer        m_image;
  };

  class ClearROIUndoCommand
  : public QUndoCommand
  {
    public:
      /** \brief ClearROIUndoCommand class constructor.
       * \param[in] toolgroup, raw pointer of the ROIToolGroup that has the ROI accumulator.
       *
       */
      explicit ClearROIUndoCommand(RestrictToolGroup *toolgroup);

      /** \brief ClearROIUndoCommand class virtual destructor.
       *
       */
      virtual ~ClearROIUndoCommand();

      /** \brief Overrides QUndoCommand::redo().
       *
       */
      virtual void redo() override;

      /** \brief Overrides QUndoCommand::undo().
       *
       */
      virtual void undo() override;

    private:
      RestrictToolGroup *m_toolGroup;
      ROISPtr            m_roi;
  };

} // namespace ESPINA

#endif // ESPINA_ROI_UNDO_COMMAND_H_
