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


#ifndef ESPINA_SEGMENTATION_EXPLORER_LAYOUT_H
#define ESPINA_SEGMENTATION_EXPLORER_LAYOUT_H

#include "SegmentationExplorer.h"

#include <GUI/Widgets/CheckableTreeView.h>

#include <QItemDelegate>
#include <QSortFilterProxyModel>

namespace EspINA
{

  class SegmentationInspector;

  class SegmentationFilterProxyModel
  : public QSortFilterProxyModel
  {
  public:
    SegmentationFilterProxyModel(QObject *parent = 0); 

  protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
  };

  class SegmentationExplorer::Layout
  : public QObject
  {
    Q_OBJECT
  protected:
    static const QString SEGMENTATION_MESSAGE;
    static const QString RECURSIVE_MESSAGE;
    static const QString MIXED_MESSAGE;

  public:
    explicit Layout(CheckableTreeView *view,
                    ModelAdapterSPtr  model,
                    ModelFactorySPtr  factory,
                    ViewManagerSPtr   viewManager,
                    QUndoStack        *undoStack);

    virtual ~Layout();

    virtual void createSpecificControls(QHBoxLayout *specificControlLayout);

    virtual QAbstractItemModel *model()
    {return m_model.get(); }

    virtual ItemAdapterPtr item(const QModelIndex &index) const
    {return itemAdapter(index);}

    virtual QModelIndex index(ItemAdapterPtr item) const
    { return m_model->index(item); }

    virtual void setFilterRegExp(const QString &regExp) = 0;

    virtual void contextMenu(const QPoint &pos) = 0;
    virtual void deleteSelectedItems() = 0;
    virtual void showSelectedItemsInformation() = 0;
    virtual bool hasInformationToShow() = 0;

    virtual QItemDelegate *itemDelegate() const = 0;

    using SegmentationInspectorKey = QString;

    static SegmentationInspectorKey toKey(SegmentationAdapterList segmentations);
    static SegmentationInspectorKey toKey(SegmentationAdapterPtr segmentation);

    virtual void reset();

  protected:
    void deleteSegmentations(SegmentationAdapterList segmentations);
    void showSegmentationInformation(SegmentationAdapterList segmentations);

    QModelIndexList indices(const QModelIndex &index, bool recursive=false);

  protected slots:
    void releaseInspectorResources(SegmentationInspector *inspector);
    void rowsAboutToBeRemoved(const QModelIndex parent, int start, int end);

  protected:
    ModelAdapterSPtr m_model;
    ModelFactorySPtr m_factory;
    ViewManagerSPtr  m_viewManager;
    QUndoStack      *m_undoStack;

    CheckableTreeView *m_view;

    QMap<SegmentationInspectorKey, SegmentationInspector *> m_inspectors;
  };

  bool sortSegmentationLessThan(ItemAdapterPtr left, ItemAdapterPtr right);

} // namespace EspINA

#endif // SEGMENTATIONEXPLORERLAYOUT_H
