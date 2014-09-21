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
    	/** \brief Overrides QSortFilterProxyModel::lessThan for sorting purposes.
    	 * \param[in] left, QModelIndex const reference.
    	 * \param[in] right, QModelIndex const reference.
    	 *
    	 */
      virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
    };

  public:
    /** \brief ClassificationLayout class constructor.
     * \param[in] view, QTreeView raw pointer.
     * \param[in] model, model adapter smart pointer.
     * \param[in] factory, factory smart pointer.
     * \param[in] viewManager, view manager smart pointer.
     * \param[in] undoStack, QUndoStack object raw pointer.
     *
     */
    explicit ClassificationLayout(CheckableTreeView *view,
                                  ModelAdapterSPtr   model,
                                  ModelFactorySPtr   factory,
                                  ViewManagerSPtr    viewManager,
                                  QUndoStack        *undoStack);

    /** \brief ClassificationLayour class virtual destructor.
     *
     */
    virtual ~ClassificationLayout();

    /** \brief Overrides SegmentationExplorer::Layout::createSpecificControls().
     *
     */
    virtual void createSpecificControls(QHBoxLayout *specificControlLayout) override;

    /** \brief Overrides SegmentationExplorer::Layout:: model()
     *
     */
    virtual QAbstractItemModel* model() override
    { return m_sort.get(); }

    /** \brief Overrides SegmentationExplorer::Layout::item().
     *
     */
    virtual ItemAdapterPtr item(const QModelIndex& index) const override
    { return itemAdapter(m_sort->mapToSource(index)); }

    /** \brief Overrides SegmentationExplorer::Layout::index().
     *
     */
    virtual QModelIndex index(ItemAdapterPtr item) const override
    { return m_sort->mapFromSource(m_proxy->mapFromSource(Layout::index(item))); }

    /** \brief Overrides SegmentationExplorer::Layout::setFilterRegExp()
     *
     */
    virtual void setFilterRegExp(const QString &regExp) override
    { m_sort->setFilterRegExp(regExp); }

    /** \brief Overrides SegmentationExplorer::Layout::contextMenu().
     *
     */
    virtual void contextMenu(const QPoint &pos) override;

    /** \brief Overrides SegmentationExplorer::Layout::deleteSelectedItems().
     *
     */
    virtual void deleteSelectedItems() override;

    /** \brief Overrides SegmentationExplorer::Layout::showSelectedItemsInformation().
     *
     */
    virtual void showSelectedItemsInformation() override;

    /** \brief Overrides SegmentationExplorer::Layout::hasInformationToShow().
     *
     */
    virtual bool hasInformationToShow() override;

    /** \brief Overrides SegmentationExplorer::Layout::itemDelegate().
     *
     */
    virtual QItemDelegate *itemDelegate() const override;

  private:
    /** \brief Returns selected categories and/or segmentations.
     * \param[out] categories, list of selected category adapters.
     * \param[out] segmentations, set of selected segmentation adapters.
     */
    bool selectedItems(CategoryAdapterList &categories, SegmentationAdapterSet &segmentations);

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
     * \param[in] segmentations, list of segmentation adapters raw pointers of the elements that have been dropped.
     * \param[in] category, category adatpter raw pointer.
     *
     */
    void segmentationsDropped(SegmentationAdapterList segmentations,
                              CategoryAdapterPtr      category);

    /** \brief Manages the cagories drag-and-drop into another category.
     * \param[in] subcategories, list of category adapters raw pointers of the elements that have been dropped.
     * \param[in] category, category adapter raw pointer.
     *
     */
    void categoriesDropped(CategoryAdapterList subCategories,
                           CategoryAdapterPtr  category);

    /** \brief Updated the controls of the UI based on the current selection.
     *
     */
    void updateSelection();

    /** \brief Disconnects the selection model of the view.
     *
     */
    void disconnectSelectionModel();

  private:
    std::shared_ptr<ClassificationProxy> m_proxy;
    std::shared_ptr<SortFilter>          m_sort;

    CategoryItemDelegate *m_delegate;
    QPushButton          *m_createCategory;
    QPushButton          *m_createSubCategory;
    QPushButton          *m_changeCategoryColor;
  };

} // namespace ESPINA

#endif // ESPINA_CLASSIFICATION_LAYOUT_H
