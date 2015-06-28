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
#include <Dialogs/HueSelector/HueSelector.h>
#include "ClassificationLayout.h"
#include <GUI/Model/ModelAdapter.h>
#include <GUI/ColorEngines/IntensitySelectionHighlighter.h>
#include <Menus/DefaultContextualMenu.h>
#include <Undo/ChangeCategoryCommand.h>
#include <Undo/ReparentCategoryCommand.h>
#include <Undo/AddCategoryCommand.h>
#include <Undo/RemoveCategoryCommand.h>
#include <Undo/ChangeCategoryColorCommand.h>
#include <GUI/Model/Utils/SegmentationUtils.h>

// Qt
#include <QMessageBox>
#include <QUndoStack>
#include <QColorDialog>
#include <QItemDelegate>

using namespace ESPINA;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::ColorEngines;

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
      auto category   = toCategoryAdapterPtr(item);

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
ClassificationLayout::ClassificationLayout(CheckableTreeView              *view,
                                           Support::FilterRefinerRegister &filterRefiners,
                                           Support::Context               &context)
: Layout               {view, filterRefiners, context}
, m_proxy              {new ClassificationProxy(context.model(), context.viewState().representationInvalidator())}
, m_sort               {new SortFilter()}
, m_delegate           {new CategoryItemDelegate(context.model(), context.undoStack(), this)}
{
  auto model = context.model();

  m_proxy->setSourceModel(model);
  m_sort->setSourceModel(m_proxy.get());
  m_sort->setDynamicSortFilter(true);

  connect(m_proxy.get(), SIGNAL(segmentationsDropped(SegmentationAdapterList,CategoryAdapterPtr)),
          this,          SLOT(segmentationsDropped(SegmentationAdapterList,CategoryAdapterPtr)));
  connect(m_proxy.get(), SIGNAL(categoriesDropped(CategoryAdapterList,CategoryAdapterPtr)),
          this,          SLOT  (categoriesDropped(CategoryAdapterList,CategoryAdapterPtr)));

  connect(model.get(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this,  SLOT(updateSelection()));
  connect(model.get(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
          this,  SLOT(updateSelection()));
  connect(model.get(), SIGNAL(modelReset()),
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
  updateSelectedItems();

  bool segmentationsSelected     = !m_selectedSegmentations.isEmpty();
  bool categoriesSelected        = !m_selectedCategories.isEmpty();
  bool enableSegmentationActions = !categoriesSelected && segmentationsSelected;

  QMenu *contextMenu = nullptr;

  if (segmentationsSelected)
  {
    contextMenu = new DefaultContextualMenu(m_selectedSegmentations, context());

    for (auto action : contextMenu->actions())
    {
      action->setEnabled(enableSegmentationActions);
    }

    contextMenu->addSeparator();
  }

  if (categoriesSelected)
  {
    if (!segmentationsSelected)
    {
      contextMenu = new QMenu();
    }

    auto createNode = createCategoryAction(contextMenu,
                                           tr("Create Category"),
                                           ":espina/create_node.png");
    connect(createNode, SIGNAL(triggered(bool)),
            this,       SLOT(createCategory()));

    auto createSubNode = createCategoryAction(contextMenu,
                                              tr("Create Subcategory"),
                                              ":espina/create_subnode.png");
    connect(createSubNode, SIGNAL(triggered(bool)),
            this,          SLOT(createSubCategory()));

    auto changeColor = createCategoryAction(contextMenu,
                                            tr("Change Category Color"),
                                            ":espina/rainbow.svg");
    connect(changeColor, SIGNAL(triggered(bool)),
            this,        SLOT(changeCategoryColor()));

    contextMenu->addSeparator();
  }


  if (segmentationsSelected)
  {
    auto selectSameCategory = contextMenu->addAction(tr("Select segmentations of the same category"));
    selectSameCategory->setEnabled(!categoriesSelected);
    connect(selectSameCategory, SIGNAL(triggered(bool)),
            this,               SLOT(selectSameCategorySegmentations()));
  }

  if (categoriesSelected)
  {
    auto category = m_selectedCategories.first();

    auto selectFromCategory = createCategoryAction(contextMenu, tr("Select %1 segmentations").arg(category->name()));
    connect(selectFromCategory, SIGNAL(triggered(bool)),
            this,               SLOT(selectCategorySegmentations()));

    auto selectFromSubCategories = createCategoryAction(contextMenu, tr("Select %1 and sub-categories segmentations").arg(category->name()));
    connect(selectFromSubCategories, SIGNAL(triggered(bool)),
            this,                    SLOT(selectCategoryAndSubcategoriesSegmentations()));

    selectFromSubCategories->setEnabled(selectFromSubCategories->isEnabled() && !category->subCategories().isEmpty());
  }

  contextMenu->exec(pos);

  delete contextMenu;
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
            auto category = toCategoryAdapterPtr(additionalItem);
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

    auto undoStack = getUndoStack();

    // assuming categories are empty, because if they weren't then !segmentations.empty()
    undoStack->beginMacro(tr("Remove Categories and Segmentations"));
    deleteSegmentations(segmentations.toList());

    categories << additionalCategories;

    for(auto category : categories)
    {
      auto model = getModel();
      if (model->classification()->category(category->classificationName()))
      {
        undoStack->push(new RemoveCategoryCommand(category, model));
      }
    }
    undoStack->endMacro();
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
  ItemAdapterPtr categoryItem = nullptr;

  auto model        = getModel();
  auto currentIndex = m_view->currentIndex();

  if (currentIndex.isValid())
  {
    categoryItem = item(currentIndex);
  }
  else if (m_view->model()->rowCount() > 0)
  {
    categoryItem = model->classification()->categories().first().get();
  }

  if (!categoryItem) return;

  if (isCategory(categoryItem))
  {
    QString name = tr("New Category");

    auto selectedCategory = toCategoryAdapterPtr(categoryItem);
    auto parentCategory   = selectedCategory->parent();

    if (!parentCategory->subCategory(name))
    {
      auto undoStack = getUndoStack();
      undoStack->beginMacro("Create Category");
      undoStack->push(new AddCategoryCommand(model->smartPointer(parentCategory), name, model, parentCategory->color()));
      undoStack->endMacro();
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

    auto category = toCategoryAdapterPtr(categorytItem);
    if (!category->subCategory(name))
    {
      auto undoStack = getUndoStack();

      undoStack->beginMacro(tr("Create Category"));
      undoStack->push(new AddCategoryCommand(getModel()->smartPointer(category), name, getModel(), category->color()));
      undoStack->endMacro();
    }
  }
}

//------------------------------------------------------------------------
void ClassificationLayout::segmentationsDropped(SegmentationAdapterList   segmentations,
                                                CategoryAdapterPtr        category)
{
  auto undoStack = getUndoStack();

  undoStack->beginMacro(tr("Change Segmentation's Category"));
  undoStack->push(new ChangeCategoryCommand(segmentations, category, context()));
  undoStack->endMacro();
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
    auto undoStack = getUndoStack();
    undoStack->beginMacro(tr("Modify Classification"));
    undoStack->push(new ReparentCategoryCommand(validSubCategories, category, getModel()));
    undoStack->endMacro();
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

  emit canCreateCategory(hasClassification());
  emit categorySelected(numCategories == 1);

  m_sort->sort(m_sort->sortColumn(), m_sort->sortOrder());
}

//------------------------------------------------------------------------
void ClassificationLayout::changeCategoryColor()
{
  // sanity checks, not really necessary
  QModelIndexList indexList = m_view->selectionModel()->selection().indexes();
  auto item = ClassificationLayout::item(indexList.first());

  if (indexList.size() == 1 && isCategory(item))
  {
    auto category = toCategoryAdapterPtr(item);

    HueSelectorDialog hueSelector(category->color().hue());
    hueSelector.setModal(true);

    if(hueSelector.exec() == QDialog::Accepted)
    {
      auto undoStack = getUndoStack();
      undoStack->beginMacro(tr("Change category color"));
      undoStack->push(new ChangeCategoryColorCommand(getModel(),
                                                     context().representationInvalidator(),
                                                     category,
                                                     hueSelector.hueValue()));
      undoStack->endMacro();
    }
  }
}

//------------------------------------------------------------------------
void ClassificationLayout::selectSameCategorySegmentations()
{
  Q_ASSERT(!m_selectedSegmentations.isEmpty());

  QSet<CategoryAdapterPtr> categories;

  for (auto segmentation : m_selectedSegmentations)
  {
    categories << segmentation->category().get();
  }

  QItemSelection selection;

  for (auto category : categories)
  {
    selection.merge(selectCategorySegmentations(category), QItemSelectionModel::Select);
  }

  m_view->selectionModel()->clearSelection();
  m_view->selectionModel()->select(selection, QItemSelectionModel::Select);
}

//------------------------------------------------------------------------
void ClassificationLayout::selectCategorySegmentations()
{
  Q_ASSERT(!m_selectedCategories.isEmpty());

  auto category  = m_selectedCategories.first();
  auto selection = selectCategorySegmentations(category);

  m_view->selectionModel()->clearSelection();
  m_view->selectionModel()->select(selection, QItemSelectionModel::Select);
  m_view->selectionModel()->setCurrentIndex(selection.first().topLeft(), QItemSelectionModel::Select);

  displayCurrentIndex();
}

//------------------------------------------------------------------------
void ClassificationLayout::selectCategoryAndSubcategoriesSegmentations()
{
  Q_ASSERT(!m_selectedCategories.isEmpty());

  QList<CategoryAdapterPtr> selectionCategories;

  selectionCategories << m_selectedCategories.first();

  QItemSelection selection;

  while (!selectionCategories.isEmpty())
  {
    auto category = selectionCategories.takeFirst();

    selection.merge(selectCategorySegmentations(category), QItemSelectionModel::Select);

    for (auto subCategory : category->subCategories())
    {
      selectionCategories << subCategory.get();
    }
  }

  m_view->selectionModel()->clearSelection();
  m_view->selectionModel()->select(selection, QItemSelectionModel::Select);
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
      categories << toCategoryAdapterPtr(item);
    }
    else
    {
      Q_ASSERT(false);
    }
  }

  return !categories.isEmpty() || !segmentations.isEmpty();
}

//------------------------------------------------------------------------
void ClassificationLayout::updateSelectedItems()
{
  m_selectedCategories.clear();
  m_selectedSegmentations.clear();

  for(auto index : m_view->selectionModel()->selectedIndexes())
  {
    auto selectedItem = item(index);

    if (isSegmentation(selectedItem))
    {
      m_selectedSegmentations << segmentationPtr(selectedItem);
    }
    else if (isCategory(selectedItem))
    {
      m_selectedCategories << toCategoryAdapterPtr(selectedItem);
    }
    else
    {
      Q_ASSERT(false);
    }
  }
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

//------------------------------------------------------------------------
bool ClassificationLayout::hasClassification() const
{
 return getModel()->classification().get();
}

//------------------------------------------------------------------------
QItemSelection ClassificationLayout::selectCategorySegmentations(CategoryAdapterPtr category) const
{
  auto categoryIndex    = index(category);
  auto numSubCategories = category->subCategories().size();
  auto rows             = categoryIndex.model()->rowCount(categoryIndex);
  auto segmentations    = rows - numSubCategories;

  QModelIndex topLeft, bottomRight;

  if (segmentations > 0)
  {
    topLeft     = categoryIndex.child(numSubCategories, 0);
    bottomRight = categoryIndex.child(rows - 1, 0);
  }

  return QItemSelection(topLeft, bottomRight);
}

//------------------------------------------------------------------------
QAction *ClassificationLayout::createCategoryAction(QMenu *contextMenu, const QString &text)
{
  auto enableCategoryActions = m_selectedCategories.size() == 1
                            && m_selectedSegmentations.isEmpty();

  auto action = contextMenu->addAction(text);

  action->setEnabled(enableCategoryActions);

  return action;
}

//------------------------------------------------------------------------
QAction *ClassificationLayout::createCategoryAction(QMenu *contextMenu, const QString &text, const QString &icon)
{
  auto action = createCategoryAction(contextMenu, text);

  action->setIcon(QIcon(icon));

  return action;
}

//------------------------------------------------------------------------
void ClassificationLayout::displayCurrentIndex()
{
  auto selectionModel = m_view->selectionModel();
  auto selection      = selectionModel->selectedIndexes();

  if (!selection.isEmpty())
  {
    selectionModel->setCurrentIndex(selection.first(), QItemSelectionModel::Select);
  }
}
