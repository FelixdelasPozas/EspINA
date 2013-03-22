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


#ifndef TAXONOMYLAYOUT_H
#define TAXONOMYLAYOUT_H

#include "SegmentationExplorerLayout.h"

#include <Core/Model/Proxies/TaxonomyProxy.h>
#include <Core/ColorEngines/TaxonomyColorEngine.h>

#include <QSortFilterProxyModel>

class TaxonomyItemDelegate;

namespace EspINA
{
  class TaxonomyLayout
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
    explicit TaxonomyLayout(CheckableTreeView     *view,
                            EspinaModel           *model,
                            QUndoStack            *undoStack,
                            ViewManager           *viewManager);
    virtual ~TaxonomyLayout();

    virtual void createSpecificControls(QHBoxLayout *specificControlLayout);

    virtual QAbstractItemModel* model()
    { return m_sort.data(); }

    virtual ModelItemPtr item(const QModelIndex& index) const
    { return indexPtr(m_sort->mapToSource(index)); }

    virtual QModelIndex index(ModelItemPtr item) const
    { return m_sort->mapFromSource(m_proxy->mapFromSource(Layout::index(item))); }

    virtual void setFilterRegExp(const QString &regExp)
    { m_sort->setFilterRegExp(regExp);}

    virtual void contextMenu(const QPoint &pos);
    virtual void deleteSelectedItems();
    virtual void showSelectedItemsInformation();
    virtual bool hasInformationToShow();

    virtual QItemDelegate *itemDelegate() const;

  private:
    bool selectedItems(TaxonomyElementList &taxonomies, SegmentationSet &segmentations);

  private slots:
    void createTaxonomy();
    void createSubTaxonomy();
    void changeTaxonomyColor();
    void selectTaxonomyElements();

    void segmentationsDragged(SegmentationList    segmentations,
                              TaxonomyElementPtr  taxonomy);

    void taxonomiesDragged(TaxonomyElementList subTaxonomies,
                           TaxonomyElementPtr  taxonomy);

    void updateSelection();
    void disconnectSelectionModel();

  private:
    QSharedPointer<TaxonomyProxy> m_proxy;
    QSharedPointer<SortFilter>    m_sort;

    TaxonomyItemDelegate *m_delegate;
    QPushButton *m_createTaxonomy;
    QPushButton *m_createSubTaxonomy;
    QPushButton *m_changeTaxonomyColor;
  };

} // namespace EspINA

#endif // TAXONOMYLAYOUT_H
