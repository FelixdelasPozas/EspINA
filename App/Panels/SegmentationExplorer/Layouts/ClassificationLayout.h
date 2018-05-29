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
#include <Panels/SegmentationExplorer/SegmentationExplorerLayout.h>
#include <GUI/Widgets/CheckableTreeView.h>
#include <GUI/Model/Proxies/ClassificationProxy.h>

// Qt
#include <QSortFilterProxyModel>
#include <QItemDelegate>

class QUndoStack;

namespace ESPINA
{
  /** \class CategoryItemDelegate
   * \brief Implements display and editing facilities for category items in the model.
   *
   */
  class CategoryItemDelegate
  : public QItemDelegate
  {
    public:
      /** \brief Class CategoryItemDelegate class constructor.
       * \param[in] model model adapter smart pointer.
       * \param[in] undoStack QUndoStack object raw pointer.
       * \param[in] parent parent object raw pointer.
       *
       */
      explicit CategoryItemDelegate(ModelAdapterSPtr model,
                                    QUndoStack      *undoStack,
                                    QObject         *parent = nullptr)
      : QItemDelegate{parent}
      , m_model      {model}
      , m_undoStack  {undoStack}
      {}

      virtual ~CategoryItemDelegate()
      {};

      virtual void setModelData(QWidget            *editor,
                                QAbstractItemModel *model,
                                const QModelIndex  &index) const override;

    private:
      ModelAdapterSPtr m_model;
      QUndoStack      *m_undoStack;
  };

  /** \class ClassificationLayout
   * \brief Implements the layout in the segmentation explorer based on the classification order.
   *
   */
  class ClassificationLayout
  : public SegmentationExplorer::Layout
  {
      Q_OBJECT

      /** \class SortFilter
       * \brief Implements the sorting algorithm for segmentations in the layout.
       *
       */
      class SortFilter
      : public SegmentationFilterProxyModel
      {
        protected:
          virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
      };

    public:
      /** \brief ClassificationLayout class constructor.
       * \param[in] view QTreeView raw pointer.
       * \param[in] context ESPINA context
       *
       */
      explicit ClassificationLayout(CheckableTreeView              *view,
                                    Support::Context               &context);

      /** \brief ClassificationLayout virtual destructor.
       *
       */
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

      /** \brief Selects the segmentations of the same category as the currently selected one.
       *
       */
      void selectSameCategorySegmentations();

      /** \brief Selects the segmentations of the current selected category.
       *
       */
      void selectCategorySegmentations();

      /** \brief Selectes the segmentations of the current selected category and its subcategories.
       *
       */
      void selectCategoryAndSubcategoriesSegmentations();

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

      /** \brief Updates the internal selection data.
       *
       */
      void updateSelectedItems();

      /** \brief Helper method to add the "create category" button of this layout specific controls.
       * \param[in] button button to add.
       * \param[in] layout layout to add the button.
       *
       */
      void addCreateCategoryButton(QPushButton *button, QHBoxLayout *layout);

      /** \brief Helper method to add the "create sub category" button of this layout specific controls.
       * \param[in] button button to add.
       * \param[in] layout layout to add the button.
       *
       */
      void addCategoryDependentButton(QPushButton *button, QHBoxLayout *layout);

      /** \brief Helper method to add a button to the layout
       * \param[in] button button to add.
       * \param[in] layout layout to add the button.
       *
       */
      void addDockButton(QPushButton *button, QHBoxLayout *layout);

      /** \brief Returns true if the current model has a classification.
       *
       */
      bool hasClassification() const;

      /** \brief Helper method that returns the model indexes of the segmentations of the given category.
       * \param[in] category category adapter object.
       *
       */
      QItemSelection selectCategorySegmentations(CategoryAdapterPtr category) const;

      /** \brief Helper method to create an entry for the contextual menu.
       * \param[in] contextMenu contextual menu object.
       * \param[in] text text of the entry.
       *
       */
      QAction *createCategoryAction(QMenu *contextMenu, const QString &text);

      /** \brief Helper method to create an entry for the contextual menu.
       * \param[in] contextMenu contextual menu object.
       * \param[in] text text of the entry.
       * \param[in] icon icon of the entry.
       *
       */
      QAction *createCategoryAction(QMenu *contextMenu, const QString &text, const QString &icon);

      /** \brief Selects the current index in the selection model.
       *
       */
      void displayCurrentIndex();

    private:
      /** \brief Helper method that returns a name with no collisions in the parent sub-categories.
       * \param[in] category Category adapter pointer.
       * \param[in] suggested Suggested category name.
       *
       */
      const QString uniqueCategoryName(const CategoryAdapterPtr category, const QString &suggested);

      std::shared_ptr<ClassificationProxy> m_proxy;                 /** proxy model.                              */
      std::shared_ptr<SortFilter>          m_sort;                  /** model filter and sorter.                  */
      CategoryItemDelegate                *m_delegate;              /** delegate item for categories.             */
      CategoryAdapterList                  m_selectedCategories;    /** list of currently selected categories.    */
      SegmentationAdapterList              m_selectedSegmentations; /** list of currently selected segmentations. */
  };

} // namespace ESPINA

#endif // ESPINA_CLASSIFICATION_LAYOUT_H
