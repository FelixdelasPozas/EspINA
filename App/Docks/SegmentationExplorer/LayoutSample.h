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


#ifndef SAMPLELAYOUT_H
#define SAMPLELAYOUT_H

#include "SegmentationExplorerLayout.h"

#include <Core/Model/Proxies/SampleProxy.h>

#include <QSortFilterProxyModel>

namespace EspINA
{
  //------------------------------------------------------------------------
  class SampleLayout
  : public SegmentationExplorer::Layout
  {
    class SortFilter
    : public QSortFilterProxyModel
    {
    protected:
      virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;
    };

  public:
    explicit SampleLayout(EspinaModel *model, QUndoStack *undoStack);
    virtual ~SampleLayout();

    virtual QAbstractItemModel* model() {return m_sort.data();}
    virtual ModelItemPtr item(const QModelIndex& index) const;
    virtual QModelIndex index(ModelItemPtr item) const;
    virtual SegmentationList deletedSegmentations(QModelIndexList selection);

  private:
    QSharedPointer<SampleProxy> m_proxy;
    QSharedPointer<SortFilter> m_sort;
  };

} // namespace EspINA

#endif // SAMPLELAYOUT_H
