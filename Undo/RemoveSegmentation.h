/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

//----------------------------------------------------------------------------
// File:    RemoveSegmentation.h
// Purpose: Undo-able action to remove segmentations from the model
//----------------------------------------------------------------------------
#ifndef REMOVESEGMENTATION_H
#define REMOVESEGMENTATION_H

// EspINA
#include <Core/Model/ModelItem.h>
#include <Core/Model/EspinaModel.h>

// Qt
#include <QUndoCommand>

namespace EspINA
{
  class ViewManager;

  class RemoveSegmentation: public QUndoCommand
  {
    public:
      explicit RemoveSegmentation(SegmentationPtr seg, EspinaModel *model, ViewManager *vm, QUndoCommand *parent = 0);
      explicit RemoveSegmentation(SegmentationList segs, EspinaModel *model, ViewManager *vm, QUndoCommand *parent = 0);

      virtual void redo();
      virtual void undo();

    private:
      bool isADupicatedRelation(Relation relation);
      void addFilterDependencies(FilterSPtr filter);

    private:
      EspinaModel *m_model;
      ViewManager *m_viewManager;

      SegmentationSList m_segmentations;
      FilterSPtrList m_filters;
      RelationList m_relations;
  };

} // namespace EspINA

#endif // REMOVESEGMENTATION_H
