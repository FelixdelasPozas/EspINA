/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include <Docks/SegmentationExplorer/SegmentationExplorerLayout.h>
#include <GUI/Widgets/CheckableTreeView.h>
#include <GUI/Model/Proxies/ClassificationProxy.h>

// Qt
#include <QSortFilterProxyModel>

class CategoryItemDelegate;

namespace ESPINA
{
  class ClassificationLayout
  : public SegmentationExplorer::Layout
  {
    Q_OBJECT

    class SortFilter
    : public SegmentationFilterProxyModel
    {
    protected:
      virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
    };

  public:
    /** \brief ClassificationLayout class constructor.
     * \param[in] view QTreeView raw pointer.
     * \param[in] delegateFactory
     * \param[in] context ESPINA context
     *
     */
    explicit ClassificationLayout(CheckableTreeView        *view,
                                  FilterDelegateFactorySPtr delegateFactory,
                                  Support::Context   &context);

    virtual ~ClassificationLayout();

    virtual void createSpecificControls(QHBoxLayout *specificControlLayout) override;

    virtual QAbstractItemModel* model() override
    { return m_sort.get(); }

    virtual ItemAdapterPtr item(const QModelIndex& index) const override
    { return itemAdapter(m_sort->mapToSource(index)); }

    virtual QModelIndex index(ItemAdapterPtr item) const override
    { return m_sort->mapFromSource(m_proxy->mapFromSource(Layout::index(item))); }

    virtual void setFilterRegExp(const QString &regExp) override
    { m_sort->setFilterRegExp(regExp); }

    virtual void contextMenu(const QPoint &pos) override;

    virtual void deleteSelectedItems() override;

    virtual void showSelectedItemsInformation() override;

    virtual bool hasInformationToShow() override;

    virtual QItemDelegate *itemDelegate() const override;

  signals:
    void categorySelected(bool value);
    void canCreateCategory(bool value);

  private slots:
    /** \brief Creates a category and adds it to the model.
     *
     */
    void createCategory();

    /** \brief Creates a sub-category and adds it to the model.
     *
     */
    void createSubCategory();

    /** \brief Changes the color of the selected category.
     *
     */
    void changeCategoryColor();

    /** \brief Selects all segmentations from a category.
     *
     */
    void selectCategoryAdapters();

    /** \brief Manages the segmentation drag-and-drop into a category.
     * \param[in] segmentations list of segmentation adapters raw pointers of the elements that have been dropped.
     * \param[in] category category adatpter raw pointer.
     *
     */
    void segmentationsDropped(SegmentationAdapterList segmentations,
                              CategoryAdapterPtr      category);

    /** \brief Manages the cagories drag-and-drop into another category.
     * \param[in] subcategories list of category adapters raw pointers of the elements that have been dropped.
     * \param[in] category category adapter raw pointer.
     *
     */
    void categoriesDropped(CategoryAdapterList subCategories,
                           CategoryAdapterPtr  category);

    /** \brief Updated the controls of the UI based on the current selection.
     *
     */
    void updateSelection();

  private:
    /** \brief Returns selected categories and/or segmentations.
     * \param[out] categories list of selected category adapters.
     * \param[out] segmentations set of selected segmentation adapters.
     */
    bool selectedItems(CategoryAdapterList &categories, SegmentationAdapterSet &segmentations);

    void addCreateCategoryButton(QPushButton *button, QHBoxLayout *layout);

    void addCategoryDependentButton(QPushButton *button, QHBoxLayout *layout);

    void addDockButton(QPushButton *button, QHBoxLayout *layout);

    bool hasClassification() const;

  private:
    std::shared_ptr<ClassificationProxy> m_proxy;
    std::shared_ptr<SortFilter>          m_sort;

    CategoryItemDelegate *m_delegate;
  };

} // namespace ESPINA

#endif // ESPINA_CLASSIFICATION_LAYOUT_H
