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


#ifndef COMPOSITIONLAYOUT_H
#define COMPOSITIONLAYOUT_H

#include "SegmentationExplorerLayout.h"

// EspINA
#include <Core/Model/Proxies/RelationProxy.h>

// Qt
#include <QSortFilterProxyModel>

namespace EspINA
{
  class CompositionLayout
  : public SegmentationExplorer::Layout
  {
    Q_OBJECT
    class SortFilter
    : public SegmentationFilterProxyModel
    {
    protected:
      virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;
    };

  public:
    explicit CompositionLayout(CheckableTreeView *view,
                               EspinaModel       *model,
                               QUndoStack        *undoStack,
                               ViewManager       *viewManager);
    virtual ~CompositionLayout();

    virtual QAbstractItemModel* model()
    {return m_sort.get();}

    virtual ModelItemPtr item(const QModelIndex& index) const;

    virtual QModelIndex index(ModelItemPtr item) const;

    virtual void setFilterRegExp(const QString &regExp)
    { m_sort->setFilterRegExp(regExp);}

    virtual void contextMenu(const QPoint &pos);

    virtual void deleteSelectedItems();

    virtual void showSelectedItemsInformation();
    virtual bool hasInformationToShow();

    virtual QItemDelegate *itemDelegate() const;

    virtual bool selectedItems(SegmentationSet &segmentations);

  protected:
    SegmentationList deletedSegmentations(QModelIndexList selection);

  private slots:
    void selectComposeElements();

  private:
    boost::shared_ptr<RelationProxy> m_proxy;
    boost::shared_ptr<SortFilter>    m_sort;

    QItemDelegate *m_delegate;
  };

} // namespace EspINA

#endif // COMPOSITIONLAYOUT_H
