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

#ifndef ESPINA_REMOVE_SEGMENTATIONS_H
#define ESPINA_REMOVE_SEGMENTATIONS_H

#include "Undo/EspinaUndo_Export.h"

// ESPINA
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QUndoCommand>

namespace ESPINA
{
  class ViewManager;

  /** \class RemoveSegmentations
   * \brief Undo-able action to remove segmentations from the model.
   *
   */
  class EspinaUndo_EXPORT RemoveSegmentations
  : public QUndoCommand
  {
    public:
  		/* \brief RemoveSegmentations class constructor.
  		 * \param[in] segmentation, raw pointer of the segmentation adapter to remove.
  		 * \param[in] model, smart pointer of the model adapter containing the segmentation.
  		 * \param[in] parent, raw pointer of the QUndoCommand parent of this one.
  		 *
  		 */
      explicit RemoveSegmentations(SegmentationAdapterPtr segmentation,
                                   ModelAdapterSPtr       model,
                                   QUndoCommand *         parent = nullptr);

  		/* \brief RemoveSegmentations class constructor.
  		 * \param[in] segmentations, list of raw pointers of the segmentation adapters to remove.
  		 * \param[in] model, smart pointer of the model adapter containing the segmentation.
  		 * \param[in] parent, raw pointer of the QUndoCommand parent of this one.
  		 *
  		 */
      explicit RemoveSegmentations(SegmentationAdapterList segmentations,
                                   ModelAdapterSPtr        model,
                                   QUndoCommand *          parent = nullptr);

      /* \brief Overrides QUndoCommand::redo().
       *
       */
      virtual void redo() override;

      /* \brief Overrides QUndoCommand::undo().
       *
       */
      virtual void undo() override;

    private:
      /* \brief Helper method to analyze the relations of the segmentation to be
       * removed so any other segmentation depending on it can be removed too.
       *
       * TODO: work in progress
       *
       */
      void analyzeSegmentation(SegmentationAdapterPtr segmentation);
//       bool isADupicatedRelation(Relation relation);
//       void addFilterDependencies(FilterSPtr filter);

    private:
      ModelAdapterSPtr m_model;

      SegmentationAdapterSList m_segmentations;
      RelationList             m_relations;
  };

} // namespace ESPINA

#endif // ESPINA_REMOVE_SEGMENTATIONS_H
