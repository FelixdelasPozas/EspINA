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


#ifndef ESPINA_BRUSH_UNDOCOMMAND_H
#define ESPINA_BRUSH_UNDOCOMMAND_H

// ESPINA
#include <Core/Utils/BinaryMask.hxx>
#include <GUI/Model/SegmentationAdapter.h>

// Qt
#include <QUndoCommand>

namespace ESPINA
{
  class DrawUndoCommand
  : public QObject
  , public QUndoCommand
  {
    Q_OBJECT
  public:
    /** \brief DrawUndoCommand class constructor.
     * \param[in] seg, segmentation adapter pointer of the segmentation going to be modified.
     * \param[in] mask, Mask to modify segmentation with.
     */
    explicit DrawUndoCommand(SegmentationAdapterSPtr seg,
                             BinaryMaskSPtr<unsigned char> mask);

    /** \brief Overrides QUndoCommand::redo().
     *
     */
    virtual void redo() override;

    /** \brief Overrides QUndoCommand::undo().
     *
     */
    virtual void undo() override;

  private:
    SegmentationAdapterSPtr       m_segmentation;
    BinaryMaskSPtr<unsigned char> m_mask;
    Bounds                        m_bounds;
  };

} // namespace ESPINA

#endif // ESPINA_BRUSH_UNDOCOMMAND_H
