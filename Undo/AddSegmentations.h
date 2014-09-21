/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPIN_ADD_SEGMENTATION_H
#define ESPIN_ADD_SEGMENTATION_H

#include "Undo/EspinaUndo_Export.h"

// ESPINA
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QUndoCommand>

namespace ESPINA
{
  class EspinaUndo_EXPORT AddSegmentations
  : public QUndoCommand
  {
  public:
  	/** \brief AddSegmentations class constructor.
  	 * \param[in] segmentation, smart pointer of the segmentation adapter to add.
  	 * \param[in] samples, list of sample adapter smart pointer related to the segmentation.
  	 * \param[in] model, model adapter smart pointer.
  	 * \param[in] parent, raw pointer of the QUndoCommand parent of this one.
  	 *
  	 */
    explicit AddSegmentations(SegmentationAdapterSPtr segmentation,
                              SampleAdapterSList      samples,
                              ModelAdapterSPtr        model,
                              QUndoCommand           *parent = nullptr);

  	/** \brief AddSegmentations class constructor.
  	 * \param[in] segmentations, list of smart pointers of the segmentation adapters to add.
  	 * \param[in] samples, list of sample adapter smart pointer related to the segmentations.
  	 * \param[in] model, model adapter smart pointer.
  	 * \param[in] parent, raw pointer of the QUndoCommand parent of this one.
  	 *
  	 */
    explicit AddSegmentations(SegmentationAdapterSList segmentations,
                              SampleAdapterSList       samples,
                              ModelAdapterSPtr         model,
                              QUndoCommand            *parent = nullptr);

    /** \brief Overrides QUndoCommand::redo().
     *
     */
    virtual void redo() override;

    /** \brief Overrides QUndoCommand::undo().
     *
     */
    virtual void undo() override;

  private:
    SampleAdapterSList m_samples;
    ModelAdapterSPtr m_model;

    SegmentationAdapterSList m_segmentations;
  };
} // namespace ESPINA

#endif // ESPIN_ADD_SEGMENTATION_H
