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

#include <QUndoCommand>

// EspINA
#include "Core/Model/ModelItem.h"
#include <Core/Model/EspinaModel.h>

namespace EspINA
{

class RemoveSegmentation
: public QUndoCommand
{
public:
  explicit RemoveSegmentation(SegmentationPtr seg,
                              EspinaModelSPtr  model,
                              QUndoCommand *parent=0
                             );
  explicit RemoveSegmentation(SegmentationList segs,
                              EspinaModelSPtr   model,
                              QUndoCommand    *parent=0
                             );

  virtual void redo();
  virtual void undo();

private:
  void addRelations(RelationList list);
  void removeRelations(RelationList list);

  FilterSPtrList removeFilterDependencies(FilterSPtr filter);

private:
  struct FilterInfo
  {
    FilterInfo () : filter(NULL) {}
    FilterInfo(FilterSPtr filter, RelationList list) :
    filter(filter), relations(list)
    {}
    FilterSPtr filter;
    RelationList relations;
  };

  struct SegInfo
  {
    SegInfo(SegmentationSPtr seg);

    FilterSPtr  filter;
    RelationList relations;
    SegmentationSPtr segmentation;
  };

private:
  EspinaModelSPtr    m_model;

  QList<FilterInfo> m_removedFilters;
  QList<SegInfo>    m_segmentations;
};

} // namespace EspINA

#endif // REMOVESEGMENTATION_H
