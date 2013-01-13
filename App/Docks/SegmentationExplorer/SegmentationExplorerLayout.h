/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  <copyright holder> <email>
 *
 *    This program is free software: you can redistribute it and/or modify
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


#ifndef SEGMENTATIONEXPLORERLAYOUT_H
#define SEGMENTATIONEXPLORERLAYOUT_H

#include "SegmentationExplorer.h"

#include <Core/Model/ModelItem.h>
#include <Core/Model/EspinaModel.h>

namespace EspINA
{
  class SegmentationExplorer::Layout
  {
  protected:
    static const QString SEGMENTATION_MESSAGE;
    static const QString RECURSIVE_MESSAGE;
    static const QString MIXED_MESSAGE;

  public:
    explicit Layout(CheckableTreeView *view,
                    EspinaModel *model,
                    QUndoStack  *undoStack)
    : m_model    (model    )
    , m_undoStack(undoStack)
    , m_view     (view     )
    {}

    virtual ~Layout(){}

    virtual void createSpecificControls(QHBoxLayout *specificControlLayout);

    virtual QAbstractItemModel *model()
    {return m_model; }

    virtual ModelItemPtr item(const QModelIndex &index) const {return indexPtr(index);}

    virtual QModelIndex index(ModelItemPtr item) const
    { return m_model->index(item); }

    virtual void deleteSelectedItems() = 0;
    virtual void showSelectedItemsInformation() = 0;

  protected:
    void deleteSegmentations(SegmentationList segmentations);
    void showSegmentationInformation();

    QModelIndexList indices(const QModelIndex &index, bool recursive=false);

  protected:
    EspinaModel *m_model;
    QUndoStack  *m_undoStack;

    CheckableTreeView *m_view;
  };

  bool sortSegmentationLessThan(ModelItemPtr left, ModelItemPtr right);

} // namespace EspINA

#endif // SEGMENTATIONEXPLORERLAYOUT_H
