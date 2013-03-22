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
                    EspinaModel       *model,
                    QUndoStack        *undoStack,
                    ViewManager       *viewManager);

    virtual ~Layout();

    virtual void createSpecificControls(QHBoxLayout *specificControlLayout);

    virtual QAbstractItemModel *model()
    {return m_model; }

    virtual ModelItemPtr item(const QModelIndex &index) const {return indexPtr(index);}

    virtual QModelIndex index(ModelItemPtr item) const
    { return m_model->index(item); }

    virtual void setFilterRegExp(const QString &regExp) = 0;

    virtual void contextMenu(const QPoint &pos) = 0;
    virtual void deleteSelectedItems() = 0;
    virtual void showSelectedItemsInformation() = 0;
    virtual bool hasInformationToShow() = 0;

    virtual QItemDelegate *itemDelegate() const = 0;

    typedef QString SegmentationInspectorKey;
    static SegmentationInspectorKey toKey(SegmentationList segmentations);
    static SegmentationInspectorKey toKey(SegmentationPtr segmentation);

    virtual void reset();

  protected:
    void deleteSegmentations(SegmentationList segmentations);
    void showSegmentationInformation(SegmentationList segmentations);

    QModelIndexList indices(const QModelIndex &index, bool recursive=false);

  protected slots:
    void releaseInspectorResources(SegmentationInspector *inspector);
    void rowsAboutToBeRemoved(const QModelIndex parent, int start, int end);

  protected:
    EspinaModel *m_model;
    QUndoStack  *m_undoStack;
    ViewManager *m_viewManager;

    CheckableTreeView *m_view;

    QMap<SegmentationInspectorKey, SegmentationInspector *> m_inspectors;
  };

  bool sortSegmentationLessThan(ModelItemPtr left, ModelItemPtr right);

} // namespace EspINA

#endif // SEGMENTATIONEXPLORERLAYOUT_H
