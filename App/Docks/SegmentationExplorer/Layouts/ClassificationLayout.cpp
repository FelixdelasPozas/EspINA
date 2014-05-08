/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
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


#include "ClassificationLayout.h"

#include <GUI/Model/ModelAdapter.h>
#include <Menus/DefaultContextualMenu.h>
#include <Undo/ChangeCategoryCommand.h>
#include <Undo/ReparentCategoryCommand.h>
#include <Undo/AddCategoryCommand.h>
#include <Undo/RemoveCategoryCommand.h>

#include <QMessageBox>
#include <QUndoStack>
#include <QColorDialog>
#include <QItemDelegate>

using namespace EspINA;

//------------------------------------------------------------------------
class RenameTaxonomyCommand
: public QUndoCommand
{
public:
  explicit RenameTaxonomyCommand(CategoryAdapterPtr categoryItem,
                                 const QString      &name,
                                 ModelAdapterSPtr   model,
                                 QUndoCommand      *parent = 0)
  : QUndoCommand(parent)
  , m_model(model)
  , m_categoryItem(categoryItem)
  , m_name(name)
  {}

  virtual void redo() { swapName(); }

  virtual void undo() { swapName(); }

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
class CategorytemDelegate
: public QItemDelegate
{
public:
  explicit CategorytemDelegate(ModelAdapterSPtr model,
                                QUndoStack     *undoStack,
                                QObject        *parent = 0)
  : QItemDelegate(parent)
  , m_model(model)
  , m_undoStack(undoStack)
  {
  }

  virtual void setModelData(QWidget            *editor,
                            QAbstractItemModel *model,
                            const QModelIndex  &index) const
  {
//     QSortFilterProxyModel *proxy = static_cast<QSortFilterProxyModel *>(model);
//     ItemAdapterPtr item = itemAdapter(proxy->mapToSource(index));
//     if (EspINA::TAXONOMY == item->type())
//     {
//       QLineEdit *textEditor = static_cast<QLineEdit *>(editor);
//       QString name = textEditor->text();
// 
//       CategoryAdapterPtr taxonomy = taxonomyElementPtr(item);
// 
//       if (!taxonomy->parent()->element(name))
//       {
//         m_undoStack->beginMacro("Rename Taxonomy");
//         m_undoStack->push(new RenameTaxonomyCommand(taxonomy, name, m_model));
//         m_undoStack->endMacro();
//       }
//     }
  }

private:
  ModelAdapterSPtr m_model;
  QUndoStack      *m_undoStack;
};

//------------------------------------------------------------------------
bool ClassificationLayout::SortFilter::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  ItemAdapterPtr leftItem  = itemAdapter(left);
  ItemAdapterPtr rightItem = itemAdapter(right);

  if (leftItem->type() == rightItem->type())
    if (ItemAdapter::Type::SEGMENTATION == leftItem->type())
      return sortSegmentationLessThan(leftItem, rightItem);
    else
      return leftItem->data(Qt::DisplayRole).toString() < rightItem->data(Qt::DisplayRole).toString();
    else
      return leftItem->type() == ItemAdapter::Type::CATEGORY;
}

//------------------------------------------------------------------------
ClassificationLayout::ClassificationLayout(CheckableTreeView *view,
                                           ModelAdapterSPtr   model,
                                           ModelFactorySPtr   factory,
                                           ViewManagerSPtr    viewManager,
                                           QUndoStack        *undoStack)
: Layout    (view, model, factory, viewManager, undoStack)
, m_proxy   (new ClassificationProxy(model))
, m_sort    (new SortFilter())
, m_delegate(new CategorytemDelegate(model, undoStack, this))
, m_createCategory(nullptr)
, m_createSubCategory(nullptr)
, m_changeCategoryColor(nullptr)
{
  m_proxy->setSourceModel(m_model);
  m_sort->setSourceModel(m_proxy.get());
  m_sort->setDynamicSortFilter(true);

  connect(m_proxy.get(), SIGNAL(segmentationsDragged(SegmentationAdapterList,CategoryAdapterPtr)),
          this,           SLOT  (segmentationsDragged(SegmentationAdapterList,CategoryAdapterPtr)));
  connect(m_proxy.get(), SIGNAL(categoriesDragged(CategoryAdapterList,CategoryAdapterPtr)),
          this,           SLOT  (categoriesDragged(CategoryAdapterList,CategoryAdapterPtr)));

  connect(m_model.get(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this,  SLOT(updateSelection()));
  connect(m_model.get(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
          this,  SLOT(updateSelection()));
  connect(m_model.get(), SIGNAL(modelReset()),
          this,  SLOT(updateSelection()));
}

//------------------------------------------------------------------------
ClassificationLayout::~ClassificationLayout()
{
//   qDebug() << "Destroying Taxonomy Layout";
}

//------------------------------------------------------------------------
void ClassificationLayout::createSpecificControls(QHBoxLayout *specificControlLayout)
{
  m_createCategory = new QPushButton();
  m_createCategory->setIcon(QIcon(":espina/create_node.png"));
  m_createCategory->setIconSize(QSize(22,22));
  m_createCategory->setBaseSize(32, 32);
  m_createCategory->setMaximumSize(32, 32);
  m_createCategory->setMinimumSize(32, 32);
  m_createCategory->setFlat(true);
  m_createCategory->setEnabled(false);
  m_createCategory->setToolTip(tr("Create Category"));

  connect(m_createCategory, SIGNAL(clicked(bool)),
          this,             SLOT(createCategory()));
  specificControlLayout->addWidget(m_createCategory);

  m_createSubCategory = new QPushButton();
  m_createSubCategory->setIcon(QIcon(":espina/create_subnode.png"));
  m_createSubCategory->setIconSize(QSize(22,22));
  m_createSubCategory->setBaseSize(32, 32);
  m_createSubCategory->setMaximumSize(32, 32);
  m_createSubCategory->setMinimumSize(32, 32);
  m_createSubCategory->setFlat(true);
  m_createSubCategory->setEnabled(false);
  m_createSubCategory->setToolTip(tr("Create Subcategory"));

  connect(m_createSubCategory, SIGNAL(clicked(bool)),
          this,                SLOT(createSubCategory()));
  specificControlLayout->addWidget(m_createSubCategory);

  m_changeCategoryColor = new QPushButton();
  m_changeCategoryColor->setIcon(QIcon(":espina/rainbow.svg"));
  m_changeCategoryColor->setIconSize(QSize(22,22));
  m_changeCategoryColor->setBaseSize(32, 32);
  m_changeCategoryColor->setMaximumSize(32, 32);
  m_changeCategoryColor->setMinimumSize(32, 32);
  m_changeCategoryColor->setFlat(true);
  m_changeCategoryColor->setEnabled(false);
  m_changeCategoryColor->setToolTip(tr("Change Category Color"));

  connect(m_changeCategoryColor, SIGNAL(clicked(bool)),
          this,                  SLOT(changeCategoryColor()));
  specificControlLayout->addWidget(m_changeCategoryColor);

  // the model of CheckableTreeView has been set by now (wasn't in constructor): connect signals
  connect(m_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this,                     SLOT(updateSelection()));


  connect(m_createCategory, SIGNAL(destroyed()),
          this,             SLOT(disconnectSelectionModel()));
}

//------------------------------------------------------------------------
void ClassificationLayout::contextMenu(const QPoint &pos)
{
  CategoryAdapterList    categories;
  SegmentationAdapterSet segmentations;

  if (!selectedItems(categories, segmentations))
    return;

  if (categories.isEmpty())
  {
    DefaultContextualMenu contextMenu(segmentations.toList(),
                                      m_model,
                                      m_viewManager,
                                      m_undoStack);

    contextMenu.addSeparator();

    QAction *selectAll = contextMenu.addAction(tr("Select segmentations of the same category"));
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

  if (!selectedItems(categories, segmentations))
    return;

  if (!categories.isEmpty())
  {
    QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();

    m_view->selectionModel()->clear();

    QModelIndexList additionalIndexes;
    for(QModelIndex index : selectedIndexes)
    {
      if (!index.isValid())
        continue;

      ItemAdapterPtr item = ClassificationLayout::item(index);
      if (ItemAdapter::Type::CATEGORY == item->type())
      {
        additionalIndexes = indices(index, true);
        for(QModelIndex additionalIndex : additionalIndexes)
        {
          ItemAdapterPtr additionalItem = ClassificationLayout::item(additionalIndex);
          if (ItemAdapter::Type::SEGMENTATION == additionalItem->type())
          {
            auto segmentation = segmentationPtr(additionalItem);
            if (!segmentations.contains(segmentation))
              segmentations << segmentation;
          }
          else
          {
            auto category = categoryPtr(additionalItem);
            if (!additionalCategories.contains(category))
              additionalCategories << category;
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
    m_undoStack->beginMacro("Remove Category");
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
  else if (!segmentations.empty())
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
    QModelIndexList subIndexes;
    for(QModelIndex index : selectedIndexes)
    {
      ItemAdapterPtr item = ClassificationLayout::item(index);
      if (ItemAdapter::Type::CATEGORY == item->type())
      {
        subIndexes << indices(index, true);
        for(QModelIndex subIndex : subIndexes)
        {
          ItemAdapterPtr subItem = ClassificationLayout::item(subIndex);
          if (ItemAdapter::Type::SEGMENTATION == subItem->type())
          {
            auto seg = segmentationPtr(subItem);
            if (!segmentations.contains(seg))
              segmentations << seg;
          }
        }
      }
    }
  }

  if (segmentations.empty())
    return;

  showSegmentationInformation(segmentations.toList());
}

//------------------------------------------------------------------------
QItemDelegate *ClassificationLayout::itemDelegate() const
{
  return m_delegate;
}

//------------------------------------------------------------------------
bool ClassificationLayout::selectedItems(CategoryAdapterList &categories, SegmentationAdapterSet &segmentations)
{
  for(auto index : m_view->selectionModel()->selectedIndexes())
  {
    auto item = ClassificationLayout::item(index);
    switch (item->type())
    {
      case ItemAdapter::Type::SEGMENTATION:
        segmentations << segmentationPtr(item);
        break;
      case ItemAdapter::Type::CATEGORY:
        categories << categoryPtr(item);
        break;
      default:
        Q_ASSERT(false);
        break;
    }
  }

  return !categories.isEmpty() || !segmentations.isEmpty();
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
      m_undoStack->push(new AddCategoryCommand(parentCategory, name, m_model, parentCategory->color()));
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
      m_undoStack->push(new AddCategoryCommand(category, name, m_model, category->color()));
      m_undoStack->endMacro();
    }
  }
}

//------------------------------------------------------------------------
void ClassificationLayout::segmentationsDragged(SegmentationAdapterList   segmentations,
                                                CategoryAdapterPtr        category)
{
  m_undoStack->beginMacro(tr("Change Segmentation's Category"));
  {
    m_undoStack->push(new ChangeCategoryCommand(segmentations, category, m_model, m_viewManager));
  }
  m_undoStack->endMacro();
}

//------------------------------------------------------------------------
void ClassificationLayout::categoriesDragged(CategoryAdapterList subCategories,
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

  QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();

  for(auto index : selectedIndexes)
  {
    ItemAdapterPtr item = ClassificationLayout::item(index);

    if (isCategory(item))
    {
      numCategories++;
    }
  }

  bool enabled = (numCategories == 1);
  m_createCategory->setEnabled(m_model->classification().get());
  m_createSubCategory->setEnabled(enabled);
  m_changeCategoryColor->setEnabled(enabled);
}

//------------------------------------------------------------------------
void ClassificationLayout::disconnectSelectionModel()
{
  m_createCategory = m_createSubCategory = m_changeCategoryColor = nullptr;

  disconnect(m_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(updateSelection()));
}

//------------------------------------------------------------------------
void ClassificationLayout::changeCategoryColor()
{
  // sanity checks, not really necessary
  QModelIndexList indexList = m_view->selectionModel()->selection().indexes();

  if (indexList.size() != 1)
    return;

  ItemAdapterPtr item = ClassificationLayout::item(indexList.first());

  if (!isCategory(item))
    return;

  auto category = categoryPtr(item);

  QColorDialog colorSelector(m_view->parentWidget());
  colorSelector.setCurrentColor(category->color());

  if(colorSelector.exec() == QDialog::Accepted)
  {
    category->setData(colorSelector.selectedColor(),
                      Qt::DecorationRole);

    m_viewManager->updateSegmentationRepresentations();
    m_viewManager->updateViews();
  }
}

//------------------------------------------------------------------------
void ClassificationLayout::selectCategoryAdapters()
{
   QModelIndex index = m_view->selectionModel()->currentIndex();

   if (!index.isValid())
     return;

   ItemAdapterPtr itemptr = item(index);
   if (ItemAdapter::Type::CATEGORY != itemptr->type())
   {
     index = index.parent();
     if (!index.isValid())
       return;

     itemptr = item(index);

     Q_ASSERT(itemptr->type() == ItemAdapter::Type::CATEGORY);
   }

   QItemSelection newSelection;
   foreach(QModelIndex sortIndex, indices(index, true))
   {
     if (!sortIndex.isValid())
       continue;

     ItemAdapterPtr sortItem = item(sortIndex);
     if (ItemAdapter::Type::SEGMENTATION != sortItem->type())
       continue;

     QItemSelection selectedItem(sortIndex, sortIndex);
     newSelection.merge(selectedItem, QItemSelectionModel::Select);
   }

   m_view->selectionModel()->clearSelection();
   m_view->selectionModel()->select(newSelection, QItemSelectionModel::Select);
}

//------------------------------------------------------------------------
bool ClassificationLayout::hasInformationToShow()
{
  QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();
  for(auto index : selectedIndexes)
  {
    ItemAdapterPtr item = ClassificationLayout::item(index);
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

    return false;
  }

  return false;
}
