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


#ifndef ESPINA_CLASSIFICATION_LAYOUT_H
#define ESPINA_CLASSIFICATION_LAYOUT_H

#include <Docks/SegmentationExplorer/SegmentationExplorerLayout.h>
#include <GUI/Widgets/CheckableTreeView.h>
#include <GUI/Model/Proxies/ClassificationProxy.h>

#include <QSortFilterProxyModel>


class CategorytemDelegate;

namespace EspINA
{
  class ClassificationLayout
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
    explicit ClassificationLayout(CheckableTreeView *view,
                                  ModelAdapterSPtr   model,
                                  ViewManagerSPtr    viewManager,
                                  QUndoStack        *undoStack);
    virtual ~ClassificationLayout();

    virtual void createSpecificControls(QHBoxLayout *specificControlLayout);

    virtual QAbstractItemModel* model()
    { return m_sort.get(); }

    virtual ItemAdapterPtr item(const QModelIndex& index) const
    { return itemAdapter(m_sort->mapToSource(index)); }

    virtual QModelIndex index(ItemAdapterPtr item) const
    { return m_sort->mapFromSource(m_proxy->mapFromSource(Layout::index(item))); }

    virtual void setFilterRegExp(const QString &regExp)
    { m_sort->setFilterRegExp(regExp);}

    virtual void contextMenu(const QPoint &pos);
    virtual void deleteSelectedItems();
    virtual void showSelectedItemsInformation();
    virtual bool hasInformationToShow();

    virtual QItemDelegate *itemDelegate() const;

  private:
    bool selectedItems(CategoryAdapterList &categories, SegmentationAdapterSet &segmentations);

  private slots:
    void createCategory();
    void createSubCategory();
    void changeCategoryColor();
    void selectCategoryAdapters();

    void segmentationsDragged(SegmentationAdapterList segmentations,
                              CategoryAdapterPtr      category);

    void categoriesDragged(CategoryAdapterList subCategories,
                           CategoryAdapterPtr  category);

    void updateSelection();
    void disconnectSelectionModel();

  private:
    std::shared_ptr<ClassificationProxy> m_proxy;
    std::shared_ptr<SortFilter>          m_sort;

    CategorytemDelegate *m_delegate;
    QPushButton *m_createCategory;
    QPushButton *m_createSubCategory;
    QPushButton *m_changeCategoryColor;
  };

} // namespace EspINA

#endif // ESPINA_CLASSIFICATION_LAYOUT_H