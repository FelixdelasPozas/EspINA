/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_REPLACE_OUTPUT_COMMAND_H
#define ESPINA_REPLACE_OUTPUT_COMMAND_H

#include "Undo/EspinaUndo_Export.h"

// ESPINA
#include <GUI/Model/SegmentationAdapter.h>

// Qt
#include <QUndoCommand>

namespace ESPINA
{
  class EspinaUndo_EXPORT ReplaceOutputCommand
  : public QUndoCommand
  {
  public:
  	/** \brief ReplaceOutputCommand class constructor.
  	 * \param[in] segmentation, raw pointer of the segmentation to change output.
  	 * \param[in] input, smart pointer of the new output.
  	 * \param[in] parent, raw pointer of the QUndoCommand parent of this one.
  	 *
  	 */
    explicit ReplaceOutputCommand(SegmentationAdapterPtr segmentation,
                                  InputSPtr              input,
                                  QUndoCommand*          parent = nullptr);

    /** \brief Overrides QUndoCommand::redo().
     *
     */
    virtual void redo() override;

    /** \brief Overrides QUndoCommand::undo().
     *
     */
    virtual void undo() override;

  private:
    /** \brief Helper method to swap new-old outputs.
     *
     */
    void swapInputs();

  private:
    SegmentationAdapterPtr m_segmentation;
    InputSPtr              m_input;
  };

} // namespace ESPINA;

#endif // ESPINA_REPLACE_OUTPUT_COMMAND_H
