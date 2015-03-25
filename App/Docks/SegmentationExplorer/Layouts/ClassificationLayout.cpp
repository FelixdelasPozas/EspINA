/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// ESPINA
#include "ClassificationLayout.h"
#include <GUI/Model/ModelAdapter.h>
#include <Menus/DefaultContextualMenu.h>
#include <Undo/ChangeCategoryCommand.h>
#include <Undo/ReparentCategoryCommand.h>
#include <Undo/AddCategoryCommand.h>
#include <Undo/RemoveCategoryCommand.h>
#include <GUI/Model/Utils/SegmentationUtils.h>

// Qt
#include <QMessageBox>
#include <QUndoStack>
#include <QColorDialog>
#include <QItemDelegate>

using namespace ESPINA;
using namespace ESPINA::GUI::Model::Utils;

//------------------------------------------------------------------------
class RenameCategoryCommand
: public QUndoCommand
{
public:
  /** \brief RenameCategoryCommand class constructor.
   * \param[in] categoryItem category adapter raw pointer of the element to rename.
   * \param[in] name string reference to the new name.
   * \param[in] model model adapter smart pointer of the model containing the category.
   * \param[in] parent parent QUndoCommand raw pointer.
   *
   */
  explicit RenameCategoryCommand(CategoryAdapterPtr categoryItem,
                                 const QString      &name,
                                 ModelAdapterSPtr   model,
                                 QUndoCommand      *parent = 0)
  : QUndoCommand  {parent}
  , m_model       {model}
  , m_categoryItem{categoryItem}
  , m_name        {name}
  {}

  virtual void redo() override
  { swapName(); }

  virtual void undo() override
  { swapName(); }

private:
  void swapName()
  {
    QString     tmp   = m_categoryItem->name();
    QModelIndex index = m_model->categoryIndex(m_categoryItem);

    m_model->setData(index, m_name, Qt::EditRole);

    m_name = tmp;
  }

private:
  ModelAdapterSPtr   m_model;
  CategoryAdapterPtr m_categoryItem;
  QString            m_name;
};


//------------------------------------------------------------------------
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

  virtual void setModelData(QWidget            *editor,
                            QAbstractItemModel *model,
                            const QModelIndex  &index) const override
  {
    auto proxy = static_cast<QSortFilterProxyModel *>(model);
    auto item  = itemAdapter(proxy->mapToSource(index));

    if (isCategory(item))
    {
      auto textEditor = static_cast<QLineEdit *>(editor);
      auto name       = textEditor->text();
      auto category   = categoryPtr(item);

      if (!category->parent()->subCategory(name))
      {
        m_undoStack->beginMacro(tr("Rename Category"));
        m_undoStack->push(new RenameCategoryCommand(category, name, m_model));
        m_undoStack->endMacro();
      }
    }
  }

private:
  ModelAdapterSPtr m_model;
  QUndoStack      *m_undoStack;
};

//------------------------------------------------------------------------
bool ClassificationLayout::SortFilter::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  auto leftItem  = itemAdapter(left);
  auto rightItem = itemAdapter(right);

  if (leftItem->type() == rightItem->type())
  {
    if (isSegmentation(leftItem))
    {
      return sortSegmentationLessThan(leftItem, rightItem);
    }
    else
    {
      auto stringLeft  = leftItem->data(Qt::DisplayRole).toString();
      auto stringRight = rightItem->data(Qt::DisplayRole).toString();

      QRegExp categoryExtractor("(\\D+)");
      categoryExtractor.setMinimal(false);

      if ((categoryExtractor.indexIn(stringLeft) == -1) || (categoryExtractor.indexIn(stringRight) == -1))
      {
        return stringLeft < stringRight;
      }

      categoryExtractor.indexIn(stringLeft);
      auto categoryLeft = categoryExtractor.cap(1);

      categoryExtractor.indexIn(stringRight);
      auto categoryRight = categoryExtractor.cap(1);

      if(categoryLeft != categoryRight)
      {
        return categoryLeft < categoryRight;
      }

      QRegExp numExtractor("(\\d+)");
      numExtractor.setMinimal(false);

      if ((numExtractor.indexIn(stringLeft) == -1) || (numExtractor.indexIn(stringRight) == -1))
      {
        return stringLeft < stringRight;
      }

      numExtractor.indexIn(stringLeft);
      auto numLeft = numExtractor.cap(1).toInt();

      numExtractor.indexIn(stringRight);
      auto numRight = numExtractor.cap(1).toInt();

      return numLeft < numRight;
    }
  }
  else
  {
    return isCategory(leftItem);
  }
}

//------------------------------------------------------------------------
ClassificationLayout::ClassificationLayout(CheckableTreeView        *view,
                                           RepresentationFactorySList &representations,
                                           ModelAdapterSPtr          model,
                                           ModelFactorySPtr          factory,
                                           FilterDelegateFactorySPtr delegateFactory,
                                           ViewManagerSPtr           viewManager,
                                           QUndoStack                *undoStack)
: Layout               {view, representations, model, factory, delegateFactory, viewManager, undoStack}
, m_proxy              {new ClassificationProxy(model)}
, m_sort               {new SortFilter()}
, m_delegate           {new CategoryItemDelegate(model, undoStack, this)}
{
  m_proxy->setSourceModel(m_model);
  m_sort ->setSourceModel(m_proxy.get());
  m_sort ->setDynamicSortFilter(true);

  connect(m_proxy.get(), SIGNAL(segmentationsDropped(SegmentationAdapterList,CategoryAdapterPtr)),
          this,          SLOT(segmentationsDropped(SegmentationAdapterList,CategoryAdapterPtr)));
  connect(m_proxy.get(), SIGNAL(categoriesDropped(CategoryAdapterList,CategoryAdapterPtr)),
          this,          SLOT  (categoriesDropped(CategoryAdapterList,CategoryAdapterPtr)));

  connect(m_model.get(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this,  SLOT(updateSelection()));
  connect(m_model.get(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
          this,  SLOT(updateSelection()));
  connect(m_model.get(), SIGNAL(modelReset()),
          this,  SLOT(updateSelection()));

  m_sort->sort(m_sort->sortColumn(), m_sort->sortOrder());
}

//------------------------------------------------------------------------
ClassificationLayout::~ClassificationLayout()
{
}

//------------------------------------------------------------------------
void ClassificationLayout::createSpecificControls(QHBoxLayout *specificControlLayout)
{
  auto createCategory = DockWidget::createDockButton(":espina/create_node.png",
                                                       tr("Create Category"));
  addCreateCategoryButton(createCategory, specificControlLayout);
  connect(createCategory, SIGNAL(clicked(bool)),
          this,           SLOT(createCategory()));

  auto createSubCategory = DockWidget::createDockButton(":espina/create_subnode.png",
                                                       tr("Create Subcategory"));
  addCategoryDependentButton(createSubCategory, specificControlLayout);
  connect(createSubCategory, SIGNAL(clicked(bool)),
          this,                SLOT(createSubCategory()));

  auto changeCategoryColor = DockWidget::createDockButton(":espina/rainbow.svg",
                                                       tr("Change Category Color"));
  addCategoryDependentButton(changeCategoryColor, specificControlLayout);
  connect(changeCategoryColor, SIGNAL(clicked(bool)),
          this,                SLOT(changeCategoryColor()));

  // the model of CheckableTreeView has been set by now (wasn't in constructor): connect signals
  connect(m_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this,                     SLOT(updateSelection()));
}

//------------------------------------------------------------------------
void ClassificationLayout::contextMenu(const QPoint &pos)
{
  CategoryAdapterList    categories;
  SegmentationAdapterSet segmentations;

  if (!selectedItems(categories, segmentations)) return;

  if (categories.isEmpty())
  {
    DefaultContextualMenu contextMenu(segmentations.toList(),
                                      m_model,
                                      m_viewManager,
                                      m_undoStack);

    contextMenu.addSeparator();

    auto selectAll = contextMenu.addAction(tr("Select segmentations of the same category"));
    connect(selectAll, SIGNAL(triggered(bool)),
            this,      SLOT(selectCategoryAdapters()));

    contextMenu.exec(pos);
    return;
  }

  QMenu contextMenu;

  QAction *createNode = contextMenu.addAction(tr("Create Category"));
  createNode->setIcon(QIcon(":espina/create_node.png"));
  connect(createNode, SIGNAL(triggered(bool)),
          this,       SLOT(createCategory()));

  QAction *createSubNode = contextMenu.addAction(tr("Create Subcategory"));
  createSubNode->setIcon(QIcon(":espina/create_subnode.png"));
  connect(createSubNode, SIGNAL(triggered(bool)),
          this,          SLOT(createSubCategory()));

  QAction *changeColor = contextMenu.addAction(tr("Change Category Color"));
  changeColor->setIcon(QIcon(":espina/rainbow.svg"));
  connect(changeColor, SIGNAL(triggered(bool)),
          this,        SLOT(changeCategoryColor()));

  contextMenu.addSeparator();

  QAction *selectAll = contextMenu.addAction(tr("Select category segmentations"));
  connect(selectAll, SIGNAL(triggered(bool)),
          this,      SLOT(selectCategoryAdapters()));

  contextMenu.exec(pos);
}

//------------------------------------------------------------------------
void ClassificationLayout::deleteSelectedItems()
{
  CategoryAdapterList    categories, additionalCategories;
  SegmentationAdapterSet segmentations;

  if (!selectedItems(categories, segmentations)) return;

  if (!categories.isEmpty())
  {
    auto selectedIndexes = m_view->selectionModel()->selectedIndexes();

    m_view->selectionModel()->clear();

    for(QModelIndex index : selectedIndexes)
    {
      if (!index.isValid()) continue;

      auto item = ClassificationLayout::item(index);
      if (isCategory(item))
      {
        for(auto additionalIndex : indices(index, true))
        {
          auto additionalItem = ClassificationLayout::item(additionalIndex);
          if (isSegmentation(additionalItem))
          {
            segmentations << segmentationPtr(additionalItem);
          }
          else
          {
            auto category = categoryPtr(additionalItem);
            if (!additionalCategories.contains(category))
            {
              additionalCategories << category;
            }
          }
        }
      }
    }

    if (!segmentations.isEmpty())
    {
      QMessageBox msg;
      msg.setWindowTitle(tr("Delete Selected Items"));
      msg.setText(tr("Warning: all elements under selected items will also be deleted"));
      /*QPushButton *cancel =*/msg.addButton(tr("Cancel"), QMessageBox::RejectRole);
      QPushButton *recursive = msg.addButton(tr("Categories and Segmentations"), QMessageBox::AcceptRole);
      QPushButton *onlySeg   = msg.addButton(tr("Only Segmentations"), QMessageBox::AcceptRole);

      msg.exec();


      if(msg.clickedButton() == onlySeg)
      {
        deleteSegmentations(segmentations.toList());
      }

      if (msg.clickedButton() != recursive)
      {
        return;
      }
    }

    // assuming categories are empty, because if they weren't then !segmentations.empty()
    m_undoStack->beginMacro(tr("Remove Categories and Segmentations"));
    deleteSegmentations(segmentations.toList());

    categories << additionalCategories;

    for(auto category : categories)
    {
      if (m_model->classification()->category(category->classificationName()))
      {
        m_undoStack->push(new RemoveCategoryCommand(category, m_model));
      }
    }
    m_undoStack->endMacro();
  }
  else if (!segmentations.isEmpty())
  {
    deleteSegmentations(segmentations.toList());
  }
}

//------------------------------------------------------------------------
void ClassificationLayout::showSelectedItemsInformation()
{
  CategoryAdapterList    categories;
  SegmentationAdapterSet segmentations;

  if (!selectedItems(categories, segmentations))
    return;

  if (!categories.empty())
  {
    QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();
    QModelIndexList subIndexes; // BUG?
    for(QModelIndex index : selectedIndexes)
    {
      auto item = ClassificationLayout::item(index);
      if (isCategory(item))
      {
        subIndexes << indices(index, true);
        for(QModelIndex subIndex : subIndexes)
        {
          auto subItem = ClassificationLayout::item(subIndex);
          if (isSegmentation(subItem))
          {
            segmentations << segmentationPtr(subItem);
          }
        }
      }
    }
  }

  if (!segmentations.isEmpty())
  {
    showSegmentationInformation(segmentations.toList());
  }
}

//------------------------------------------------------------------------
QItemDelegate *ClassificationLayout::itemDelegate() const
{
  return m_delegate;
}

//------------------------------------------------------------------------
void ClassificationLayout::createCategory()
{
  QModelIndex currentIndex = m_view->currentIndex();

  ItemAdapterPtr categoryItem = nullptr;

  if (currentIndex.isValid())
  {
    categoryItem = item(currentIndex);
  }
  else if (m_view->model()->rowCount() > 0)
  {
    categoryItem = m_model->classification()->categories().first().get();
  }

  if (!categoryItem) return;

  if (isCategory(categoryItem))
  {
    QString name = tr("New Category");

    auto selectedCategory = categoryPtr(categoryItem);
    auto parentCategory   = selectedCategory->parent();

    if (!parentCategory->subCategory(name))
    {
      m_undoStack->beginMacro("Create Category");
      m_undoStack->push(new AddCategoryCommand(m_model->smartPointer(parentCategory), name, m_model, parentCategory->color()));
      m_undoStack->endMacro();
    }
  }
}

//------------------------------------------------------------------------
void ClassificationLayout::createSubCategory()
{
  QModelIndex currentIndex = m_view->currentIndex();

  if (!currentIndex.isValid())
    return;

  auto categorytItem = item(currentIndex);

  if (isCategory(categorytItem))
  {
    QString name = tr("New Category");

    auto category = categoryPtr(categorytItem);
    if (!category->subCategory(name))
    {
      m_undoStack->beginMacro("Create Category");
      m_undoStack->push(new AddCategoryCommand(m_model->smartPointer(category), name, m_model, category->color()));
      m_undoStack->endMacro();
    }
  }
}

//------------------------------------------------------------------------
void ClassificationLayout::segmentationsDropped(SegmentationAdapterList   segmentations,
                                                CategoryAdapterPtr        category)
{
  m_undoStack->beginMacro(tr("Change Segmentation's Category"));
  {
    m_undoStack->push(new ChangeCategoryCommand(segmentations, category, m_model, m_viewManager));
  }
  m_undoStack->endMacro();
}

//------------------------------------------------------------------------
void ClassificationLayout::categoriesDropped(CategoryAdapterList subCategories,
                                             CategoryAdapterPtr  category)
{
  CategoryAdapterList validSubCategories;
  for(auto subCategory : subCategories)
  {
    if (!category->subCategory(subCategory->name()))
    {
      bool nameConflict = false;
      for(auto validSubCategory : validSubCategories)
      {
        if (validSubCategory->name() == subCategory->name())
        {
          nameConflict = true;
        }
      }

      if (!nameConflict)
      {
        validSubCategories << subCategory;
      }
    }
  }

  if (!validSubCategories.isEmpty())
  {
    m_undoStack->beginMacro(tr("Modify Classification"));
    m_undoStack->push(new ReparentCategoryCommand(validSubCategories, category, m_model));
    m_undoStack->endMacro();
  }
}

//------------------------------------------------------------------------
void ClassificationLayout::updateSelection()
{
  int numCategories = 0;

  auto selectedIndexes = m_view->selectionModel()->selectedIndexes();

  for(auto index : selectedIndexes)
  {
    if (isCategory(item(index)))
    {
      numCategories++;
    }
  }

  bool hasClassification = m_model->classification() != nullptr;

  emit canCreateCategory(hasClassification);
  emit categorySelected(numCategories == 1);

  m_sort->sort(m_sort->sortColumn(), m_sort->sortOrder());
}

//------------------------------------------------------------------------
void ClassificationLayout::changeCategoryColor()
{
  // sanity checks, not really necessary
  QModelIndexList indexList = m_view->selectionModel()->selection().indexes();

  if (indexList.size() != 1)
    return;

  auto item = ClassificationLayout::item(indexList.first());

  if (!isCategory(item))
    return;

  auto category = categoryPtr(item);

  QColorDialog colorSelector(m_view->parentWidget());
  colorSelector.setCurrentColor(category->color());

  if(colorSelector.exec() == QDialog::Accepted)
  {
    category->setData(colorSelector.selectedColor(),
                      Qt::DecorationRole);

    // TODO: invalidate representations of seletected categories
  }
}

//------------------------------------------------------------------------
void ClassificationLayout::selectCategoryAdapters()
{
   auto index = m_view->selectionModel()->currentIndex();

   if (!index.isValid()) return;

   auto item = ClassificationLayout::item(index);

   if (!isCategory(item))
   {
     index = index.parent();

     if (!index.isValid()) return;

     item = ClassificationLayout::item(index);

     Q_ASSERT(isCategory(item));
   }

   QItemSelection newSelection;
   for(auto sortIndex: indices(index, true))
   {
     if (!sortIndex.isValid()) continue;

     auto sortItem = ClassificationLayout::item(sortIndex);

     if (isSegmentation(sortItem))
     {
       QItemSelection selectedItem(sortIndex, sortIndex);
       newSelection.merge(selectedItem, QItemSelectionModel::Select);
     }
   }

   m_view->selectionModel()->clearSelection();
   m_view->selectionModel()->select(newSelection, QItemSelectionModel::Select);
}

//------------------------------------------------------------------------
bool ClassificationLayout::hasInformationToShow()
{
  auto selectedIndexes = m_view->selectionModel()->selectedIndexes();

  for(auto index : selectedIndexes)
  {
    auto item = ClassificationLayout::item(index);
    if (isSegmentation(item))
    {
      return true;
    }
    else if (isCategory(item))
    {
      for(auto subIndex : indices(index, true))
      {
        if (isSegmentation(ClassificationLayout::item(subIndex)))
        {
          return true;
        }
      }
    }
  }

  return false;
}

//------------------------------------------------------------------------
bool ClassificationLayout::selectedItems(CategoryAdapterList &categories, SegmentationAdapterSet &segmentations)
{
  for(auto index : m_view->selectionModel()->selectedIndexes())
  {
    auto item = ClassificationLayout::item(index);

    if (isSegmentation(item))
    {
      segmentations << segmentationPtr(item);
    }
    else if (isCategory(item))
    {
      categories << categoryPtr(item);
    }
    else
    {
      Q_ASSERT(false);
    }
  }

  return !categories.isEmpty() || !segmentations.isEmpty();
}

//------------------------------------------------------------------------
void ClassificationLayout::addCreateCategoryButton(QPushButton *button, QHBoxLayout *layout)
{
  connect(this,   SIGNAL(canCreateCategory(bool)),
          button, SLOT(setEnabled(bool)));

  addDockButton(button, layout);
}

//------------------------------------------------------------------------
void ClassificationLayout::addCategoryDependentButton(QPushButton *button, QHBoxLayout *layout)
{
  connect(this,   SIGNAL(categorySelected(bool)),
          button, SLOT(setEnabled(bool)));

  addDockButton(button, layout);
}

//------------------------------------------------------------------------
void ClassificationLayout::addDockButton(QPushButton *button, QHBoxLayout *layout)
{
  button->setEnabled(false);
  layout->addWidget(button);
}
