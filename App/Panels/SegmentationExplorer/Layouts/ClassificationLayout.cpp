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
#include <Dialogs/CreateCategoryDialog/CreateCategoryDialog.h>
#include "ClassificationLayout.h"
#include <Core/Utils/Vector3.hxx>
#include <Core/Analysis/Category.h>
#include <Core/Utils/ListUtils.hxx>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/ColorEngines/IntensitySelectionHighlighter.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <Menus/DefaultContextualMenu.h>
#include <Undo/ChangeCategoryCommand.h>
#include <Undo/ReparentCategoryCommand.h>
#include <Undo/AddCategoryCommand.h>
#include <Undo/RemoveCategoryCommand.h>
#include <Undo/ChangeCategoryColorCommand.h>
#include <Undo/RemoveSegmentations.h>

// Qt
#include <QMessageBox>
#include <QUndoStack>
#include <QColorDialog>
#include <QItemDelegate>

// C++
#include <random>
#include <chrono>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::ColorEngines;

namespace ESPINA
{
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
      /** \brief Helper method to swap the old and new name.
       *
       */
      void swapName()
      {
        QString     tmp   = m_categoryItem->name();
        QModelIndex index = m_model->categoryIndex(m_categoryItem);

        m_model->setData(index, m_name, Qt::EditRole);

        m_name = tmp;
      }

    private:
      ModelAdapterSPtr   m_model;        /** model containing the classification.        */
      CategoryAdapterPtr m_categoryItem; /** category to modify.                         */
      QString            m_name;         /** buffer that contains the new and old names. */
  };
}

//------------------------------------------------------------------------
void CategoryItemDelegate::setModelData(QWidget            *editor,
                                        QAbstractItemModel *model,
                                        const QModelIndex  &index) const
{
  auto proxy = static_cast<QSortFilterProxyModel *>(model);
  auto item = itemAdapter(proxy->mapToSource(index));

  if (item && isCategory(item))
  {
    auto textEditor = static_cast<QLineEdit *>(editor);
    auto name = textEditor->text();
    auto category = toCategoryAdapterPtr(item);

    if (!category->parent()->subCategory(name))
    {
      m_undoStack->beginMacro(tr("Rename category '%1' to '%2'.").arg(category->name()).arg(name));
      m_undoStack->push(new RenameCategoryCommand(category, name, m_model));
      m_undoStack->endMacro();
    }
    else
    {
      auto message = tr("There is already a category at the same level with the name '%1'").arg(name);
      auto title   = tr("Rename Category");
      GUI::DefaultDialogs::ErrorMessage(message, title);
    }
  }
}

//------------------------------------------------------------------------
bool ClassificationLayout::SortFilter::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  auto leftItem  = itemAdapter(left);
  auto rightItem = itemAdapter(right);

  if(!leftItem || !rightItem) return false;

  if (leftItem->type() == rightItem->type())
  {
    return Core::Utils::lessThan<ItemAdapterPtr>(leftItem, rightItem);
  }
  else
  {
    return isCategory(leftItem);
  }
}

//------------------------------------------------------------------------
ClassificationLayout::ClassificationLayout(CheckableTreeView              *view,
                                           Support::Context               &context)
: Layout               {view, context}
, m_proxy              {new ClassificationProxy(context.model(), context.viewState())}
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
          this,        SLOT(updateSelection()));
  connect(model.get(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
          this,        SLOT(updateSelection()));
  connect(model.get(), SIGNAL(modelReset()),
          this,        SLOT(updateSelection()));

  m_sort->sort(m_sort->sortColumn(), m_sort->sortOrder());
}

//------------------------------------------------------------------------
ClassificationLayout::~ClassificationLayout()
{
}

//------------------------------------------------------------------------
void ClassificationLayout::createSpecificControls(QHBoxLayout *specificControlLayout)
{
  auto createCategory = Panel::createDockButton(":espina/create_category.svg",
                                                       tr("Create Category"));
  addCreateCategoryButton(createCategory, specificControlLayout);
  connect(createCategory, SIGNAL(clicked(bool)),
          this,           SLOT(createCategory()));

  auto createSubCategory = Panel::createDockButton(":espina/create_subcategory.svg",
                                                       tr("Create Subcategory"));
  addCategoryDependentButton(createSubCategory, specificControlLayout);
  connect(createSubCategory, SIGNAL(clicked(bool)),
          this,                SLOT(createSubCategory()));

  auto changeCategoryColor = Panel::createDockButton(":espina/rainbow.svg",
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

  QMenu *contextMenu = nullptr;

  if (segmentationsSelected)
  {
    contextMenu = new DefaultContextualMenu(m_selectedSegmentations, getContext());

    contextMenu->addSeparator();

    connect(contextMenu, SIGNAL(renamedSegmentations()), m_view, SLOT(update()));
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
    auto category         = m_selectedCategories.first();
    auto categoryIndex    = index(category);
    auto rows             = categoryIndex.model()->rowCount(categoryIndex);
    auto numSubCategories = category->subCategories().size();

    auto selectFromCategory = createCategoryAction(contextMenu, tr("Select %1 segmentations").arg(category->name()));
    connect(selectFromCategory, SIGNAL(triggered(bool)),
            this,               SLOT(selectCategorySegmentations()));

    selectFromCategory->setEnabled(selectFromCategory->isEnabled() && (rows > 0) && (numSubCategories < rows));

    auto selectFromSubCategories = createCategoryAction(contextMenu, tr("Select %1 and sub-categories segmentations").arg(category->name()));
    connect(selectFromSubCategories, SIGNAL(triggered(bool)),
            this,                    SLOT(selectCategoryAndSubcategoriesSegmentations()));

    selectFromSubCategories->setEnabled(selectFromSubCategories->isEnabled() && (numSubCategories > 0) && hasInformationToShow());
  }

  if (contextMenu)
  {
    contextMenu->exec(pos);

    delete contextMenu;
  }
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

    categories << additionalCategories;

    QString categoriesNameListText;
    for(int i = 0; i < categories.size(); ++i)
    {
      if(i != 0)
      {
        categoriesNameListText += (i != categories.size() -1 ? ", ": " and ");
      }
      categoriesNameListText += "'" + categories.at(i)->name() + "'";
    }

    auto model = getModel();
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
    else
    {
      auto message  = QObject::tr("Do you really want to delete the selected categories?");
      auto title    = QObject::tr("Delete Selected Items");
      auto details  = QObject::tr("Selected to be deleted:");

      for(int i = 0; i < categories.size(); ++i)
      {
        details.append(QString("\n - %1").arg(categories.at(i)->classificationName()));
      }

      if(QMessageBox::Ok == GUI::DefaultDialogs::UserQuestion(message, QMessageBox::Cancel|QMessageBox::Ok, title, details))
      {
        auto undoStack = getUndoStack();
        undoStack->beginMacro(tr("Remove categor%1 %2.").arg(categories.size() > 1 ? "ies":"y").arg(categoriesNameListText));
        for(auto category : categories)
        {
          if (model->classification()->category(category->classificationName()))
          {
            undoStack->push(new RemoveCategoryCommand(category, model));
          }
        }
        undoStack->endMacro();
      }
      return;
    }

    auto undoStack = getUndoStack();

    // assuming categories are empty, because if they weren't then !segmentations.empty()
    auto macroText = tr("Remove categor%1 %2 and its segmentations.").arg(categories.size() > 1 ? "ies":"y").arg(categoriesNameListText);
    undoStack->beginMacro(macroText);
    if(!segmentations.empty())
    {
      undoStack->push(new RemoveSegmentations(segmentations.toList(), model));
    }

    for(auto category : categories)
    {
      if (model->classification()->category(category->classificationName()))
      {
        undoStack->push(new RemoveCategoryCommand(category, model));
      }
    }
    undoStack->endMacro();
  }
  else
  {
    if (!segmentations.isEmpty())
    {
      auto toDelete = segmentations.toList();
      auto message  = QObject::tr("Do you really want to delete the selected segmentations?");
      auto title    = QObject::tr("Delete Selected Items");
      auto details  = QObject::tr("Selected to be deleted:");

      for(int i = 0; i < toDelete.size(); ++i)
      {
        details.append(QString("\n - %1").arg(toDelete.at(i)->data().toString()));
      }

      if(QMessageBox::Ok == GUI::DefaultDialogs::UserQuestion(message, QMessageBox::Cancel|QMessageBox::Ok, title, details))
      {
        deleteSegmentations(toDelete);
      }
    }
  }
}

//------------------------------------------------------------------------
void ClassificationLayout::showSelectedItemsInformation()
{
  CategoryAdapterList    categories;
  SegmentationAdapterSet segmentations;

  if (!selectedItems(categories, segmentations)) return;

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
    showSegmentationProperties(segmentations.toList());
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
  auto model          = getModel();
  auto parentCategory = toCategoryAdapterPtr(model->classification()->root().get());

  auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::minstd_rand0 rngenerator(seed);
  auto color = QColor::fromHsv(rngenerator() % 360, 255,255);

  auto name = uniqueCategoryName(parentCategory, "New Category");

  CreateCategoryDialog dialog;
  dialog.setWindowTitle(tr("Create New Category"));
  dialog.setOperationText(tr("Create a new root category:"));
  dialog.setCategoryName(name);
  dialog.setColor(color);
  dialog.setROI(Vector3<long long>{500,500,500});

  if(QDialog::Accepted == dialog.exec())
  {
    name = uniqueCategoryName(parentCategory, dialog.categoryName());

    auto undoStack = getUndoStack();
    undoStack->beginMacro(tr("Create category '%1'.").arg(name));
    undoStack->push(new AddCategoryCommand(model->smartPointer(parentCategory), name, model, dialog.categoryColor(), dialog.ROI()));
    undoStack->endMacro();
  }
}

//------------------------------------------------------------------------
void ClassificationLayout::createSubCategory()
{
  auto currentIndex = m_view->currentIndex();

  if (!currentIndex.isValid()) return;

  auto categorytItem = item(currentIndex);

  if (isCategory(categorytItem))
  {
    auto category = toCategoryAdapterPtr(categorytItem);
    auto name     = uniqueCategoryName(category, tr("New Category"));
    auto model    = getModel();

    CreateCategoryDialog dialog;
    dialog.setWindowTitle(tr("Create New Sub-category"));
    dialog.setOperationText(tr("Create a new sub-category of '%1':").arg(category->name()));
    dialog.setCategoryName(name);
    dialog.setColor(category->color());

    bool ok1, ok2, ok3;
    long long xSize = category->property(Category::DIM_X()).toLongLong(&ok1);
    long long ySize = category->property(Category::DIM_Y()).toLongLong(&ok2);
    long long zSize = category->property(Category::DIM_Z()).toLongLong(&ok3);

    if (ok1 && ok2 && ok3 && (xSize > 0) && (ySize > 0) && (zSize > 0))
    {
      dialog.setROI(Vector3<long long>{xSize,ySize,zSize});
    }
    else
    {
      dialog.setROI(Vector3<long long>{500,500,500});
    }

    if(QDialog::Accepted == dialog.exec())
    {
      name = uniqueCategoryName(category, dialog.categoryName());

      auto undoStack = getUndoStack();
      undoStack->beginMacro(tr("Create sub-category '%1' of category '%2'.").arg(name).arg(category->name()));
      undoStack->push(new AddCategoryCommand(model->smartPointer(category), name, model, dialog.categoryColor(), dialog.ROI()));
      undoStack->endMacro();
    }
  }
}

//------------------------------------------------------------------------
void ClassificationLayout::segmentationsDropped(SegmentationAdapterList   segmentations,
                                                CategoryAdapterPtr        category)
{
  if(!segmentations.empty() && category != nullptr)
  {
    auto undoStack = getUndoStack();
    auto names     = segmentationListNames(segmentations);

    undoStack->beginMacro(tr("Change segmentation%1 category to '%2': %3.").arg(segmentations.size() > 1 ? "s":"").arg(category->name()).arg(names));
    undoStack->push(new ChangeCategoryCommand(segmentations, category, getContext()));
    undoStack->endMacro();
  }
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
    QString categoryNames;
    for(auto cat: validSubCategories)
    {
      if(cat != validSubCategories.first())
      {
        categoryNames += (cat != validSubCategories.last() ? ", ":" and ");
      }
      categoryNames += "'" + cat->name() + "'";
    }

    auto undoStack = getUndoStack();
    undoStack->beginMacro(tr("Re-parent categor%1 to category '%2': %3.").arg(validSubCategories.size() > 1 ? "ies":"y").arg(category->name()).arg(categoryNames));
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
      auto hueValue  = hueSelector.hueValue();
      undoStack->beginMacro(tr("Change category '%1' color to RGB %2.").arg(category->name()).arg(QColor::fromHsv(hueValue,255,255).name()));
      undoStack->push(new ChangeCategoryColorCommand(getModel(),
                                                     getViewState(),
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

  if(!selection.isEmpty())
  {
    m_view->selectionModel()->clearSelection();
    m_view->selectionModel()->select(selection, QItemSelectionModel::Select);
    m_view->selectionModel()->setCurrentIndex(selection.first().topLeft(), QItemSelectionModel::Select);

    displayCurrentIndex();
  }
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

  if(!selection.isEmpty())
  {
    m_view->selectionModel()->clearSelection();
    m_view->selectionModel()->select(selection, QItemSelectionModel::Select);
  }
}

//------------------------------------------------------------------------
bool ClassificationLayout::hasInformationToShow()
{
  auto selectedIndexes = m_view->selectionModel()->selectedIndexes();

  for(auto index : selectedIndexes)
  {
    auto item = ClassificationLayout::item(index);
    if (item && isSegmentation(item))
    {
      return true;
    }
    else if (item && isCategory(item))
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

    if (item && isSegmentation(item))
    {
      segmentations << segmentationPtr(item);
    }
    else if (item && isCategory(item))
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

    if (selectedItem && isSegmentation(selectedItem))
    {
      m_selectedSegmentations << segmentationPtr(selectedItem);
    }
    else if (selectedItem && isCategory(selectedItem))
    {
      m_selectedCategories << toCategoryAdapterPtr(selectedItem);
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

//------------------------------------------------------------------------
const QString ClassificationLayout::uniqueCategoryName(const CategoryAdapterPtr category, const QString& suggested)
{
  Q_ASSERT(category);

  int i = 1;
  auto unique = suggested;
  while (category->subCategory(unique))
  {
    unique = suggested + tr("-%1").arg(++i);
  }

  return unique;
}
