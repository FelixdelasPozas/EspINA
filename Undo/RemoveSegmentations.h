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

//----------------------------------------------------------------------------
// File:    RemoveSegmentation.h
// Purpose: Undo-able action to remove segmentations from the model
//----------------------------------------------------------------------------
#ifndef ESPINA_REMOVE_SEGMENTATIONS_H
#define ESPINA_REMOVE_SEGMENTATIONS_H

#include "Undo/EspinaUndo_Export.h"

// EspINA
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QUndoCommand>

namespace EspINA
{
  class ViewManager;

  class EspinaUndo_EXPORT RemoveSegmentations
  : public QUndoCommand
  {
    public:
      explicit RemoveSegmentations(SegmentationAdapterPtr segmentation,
                                   ModelAdapterSPtr       model,
                                   QUndoCommand *         parent = nullptr);

      explicit RemoveSegmentations(SegmentationAdapterList segmentations,
                                   ModelAdapterSPtr        model,
                                   QUndoCommand *          parent = nullptr);

      virtual void redo();
      virtual void undo();

    private:
      void analyzeSegmentation(SegmentationAdapterPtr segmentation);
//       bool isADupicatedRelation(Relation relation);
//       void addFilterDependencies(FilterSPtr filter);

    private:
      ModelAdapterSPtr m_model;

      SegmentationAdapterSList m_segmentations;
      RelationList             m_relations;
  };

} // namespace EspINA

#endif // ESPINA_REMOVE_SEGMENTATIONS_H
