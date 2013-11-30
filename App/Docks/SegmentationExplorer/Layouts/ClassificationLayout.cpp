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
                                           ViewManagerSPtr    viewManager,
                                           QUndoStack        *undoStack)
: Layout    (view, model, viewManager, undoStack)
, m_proxy   (new ClassificationProxy(model))
, m_sort    (new SortFilter())
, m_delegate(new CategorytemDelegate(model, undoStack, this))
, m_createTaxonomy(NULL)
, m_createSubTaxonomy(NULL)
, m_changeTaxonomyColor(NULL)
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
  QPushButton *createCategory = new QPushButton();
  createCategory->setIcon(QIcon(":espina/create_node.png"));
  createCategory->setIconSize(QSize(22,22));
  createCategory->setBaseSize(32, 32);
  createCategory->setMaximumSize(32, 32);
  createCategory->setMinimumSize(32, 32);
  createCategory->setFlat(true);
  createCategory->setEnabled(false);
  createCategory->setToolTip(tr("Create Category"));

  connect(createCategory, SIGNAL(clicked(bool)),
          this,           SLOT(createTaxonomy()));
  specificControlLayout->addWidget(createCategory);

  QPushButton *createSubcategory = new QPushButton();
  createSubcategory->setIcon(QIcon(":espina/create_subnode.png"));
  createSubcategory->setIconSize(QSize(22,22));
  createSubcategory->setBaseSize(32, 32);
  createSubcategory->setMaximumSize(32, 32);
  createSubcategory->setMinimumSize(32, 32);
  createSubcategory->setFlat(true);
  createSubcategory->setEnabled(false);
  createSubcategory->setToolTip(tr("Create Subcategory"));

  connect(createSubcategory, SIGNAL(clicked(bool)),
          this,              SLOT(createSubTaxonomy()));
  specificControlLayout->addWidget(createSubcategory);

  QPushButton *changeColor = new QPushButton();
  changeColor->setIcon(QIcon(":espina/rainbow.svg"));
  changeColor->setIconSize(QSize(22,22));
  changeColor->setBaseSize(32, 32);
  changeColor->setMaximumSize(32, 32);
  changeColor->setMinimumSize(32, 32);
  changeColor->setFlat(true);
  changeColor->setEnabled(false);
  changeColor->setToolTip(tr("Change Category Color"));

  connect(changeColor, SIGNAL(clicked(bool)),
          this, SLOT(changeTaxonomyColor()));
  specificControlLayout->addWidget(changeColor);

  // the model of CheckableTreeView has been set by now (wasn't in constructor): connect signals
  connect(m_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this,                     SLOT(updateSelection()));

  m_createTaxonomy      = createCategory;
  m_createSubTaxonomy   = createSubcategory;
  m_changeTaxonomyColor = changeColor;

  connect(m_createTaxonomy, SIGNAL(destroyed()),
          this,             SLOT(disconnectSelectionModel()));
}

//------------------------------------------------------------------------
void ClassificationLayout::contextMenu(const QPoint &pos)
{//TODO
//   CategoryAdapterList categories;
//   SegmentationSet     segmentations;
// 
//   if (!selectedItems(categories, segmentations))
//     return;
// 
//   if (categories.isEmpty())
//   {
//     DefaultContextualMenu contextMenu(segmentations.toList(),
//                                            m_model,
//                                            m_undoStack,
//                                            m_viewManager);
// 
//     contextMenu.addSeparator();
// 
//     QAction *selectAll = contextMenu.addAction(tr("Select segmentations of the same category"));
//     connect(selectAll, SIGNAL(triggered(bool)),
//             this,      SLOT(selectCategoryAdapters()));
// 
//     contextMenu.exec(pos);
//     return;
//   }
// 
//   QMenu contextMenu;
// 
//   QAction *createNode = contextMenu.addAction(tr("Create Category"));
//   createNode->setIcon(QIcon(":espina/create_node.png"));
//   connect(createNode, SIGNAL(triggered(bool)),
//           this,       SLOT(createTaxonomy()));
// 
//   QAction *createSubNode = contextMenu.addAction(tr("Create Subcategory"));
//   createSubNode->setIcon(QIcon(":espina/create_subnode.png"));
//   connect(createSubNode, SIGNAL(triggered(bool)),
//           this,          SLOT(createSubTaxonomy()));
// 
//   QAction *changeColor = contextMenu.addAction(tr("Change Category Color"));
//   changeColor->setIcon(QIcon(":espina/rainbow.svg"));
//   connect(changeColor, SIGNAL(triggered(bool)),
//           this,        SLOT(changeTaxonomyColor()));
// 
//   contextMenu.addSeparator();
// 
//   QAction *selectAll = contextMenu.addAction(tr("Select category segmentations"));
//   connect(selectAll, SIGNAL(triggered(bool)),
//           this,      SLOT(selectCategoryAdapters()));
// 
//   contextMenu.exec(pos);
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
        // TODO: m_undoStack->push(new RemoveCategoryAdapterCommand(category, m_model));
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
{//TODO
//   CategoryAdapterList  categories;
//   SegmentationSet      segmentations;
// 
//   if (!selectedItems(categories, segmentations))
//     return;
// 
//   if (!categories.empty())
//   {
//     QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();
//     QModelIndexList subIndexes;
//     foreach(QModelIndex index, selectedIndexes)
//     {
//       ItemAdapterPtr item = ClassificationLayout::item(index);
//       if (EspINA::TAXONOMY == item->type())
//       {
//         subIndexes << indices(index, true);
//         foreach(QModelIndex subIndex, subIndexes)
//         {
//           ItemAdapterPtr subItem = ClassificationLayout::item(subIndex);
//           if (EspINA::SEGMENTATION == subItem->type())
//           {
//             SegmentationPtr seg = segmentationPtr(subItem);
//             if (!segmentations.contains(seg))
//               segmentations << seg;
//           }
//         }
//       }
//     }
//   }
// 
//   if (segmentations.empty())
//     return;
// 
//   showSegmentationInformation(segmentations.toList());
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
void ClassificationLayout::createTaxonomy()
{
//   QModelIndex currentIndex = m_view->currentIndex();
//   ItemAdapterPtr taxonomyItem;
//   if (currentIndex.isValid())
//     taxonomyItem = item(currentIndex);
//   else if (m_view->model()->rowCount() > 0)
//     taxonomyItem = m_model->taxonomy()->elements().first().get();
//   else
//     return;
// 
//   if (EspINA::TAXONOMY == taxonomyItem->type())
//   {
//     QString name = tr("New Taxonomy");
// 
//     CategoryAdapterPtr selectedTaxonomy = taxonomyElementPtr(taxonomyItem);
//     CategoryAdapterPtr parentTaxonomy   = selectedTaxonomy->parent();
//     if (!parentTaxonomy->element(name))
//     {
//       m_undoStack->beginMacro("Create Taxonomy");
//       m_undoStack->push(new AddCategoryAdapter(parentTaxonomy, name, m_model, parentTaxonomy->color()));
//       m_undoStack->endMacro();
//     }
//   }
}

//------------------------------------------------------------------------
void ClassificationLayout::createSubTaxonomy()
{
//   QModelIndex currentIndex = m_view->currentIndex();
//   if (!currentIndex.isValid())
//     return;
// 
//   ItemAdapterPtr taxonomyItem = item(currentIndex);
// 
//   if (EspINA::TAXONOMY == taxonomyItem->type())
//   {
//     QString name = tr("New Taxonomy");
// 
//     CategoryAdapterPtr taxonomy = taxonomyElementPtr(taxonomyItem);
//     if (!taxonomy->element(name))
//     {
//       m_undoStack->beginMacro("Create Taxonomy");
//       m_undoStack->push(new AddCategoryAdapter(taxonomy, name, m_model, taxonomy->color()));
//       m_undoStack->endMacro();
//     }
//   }
}

//------------------------------------------------------------------------
void ClassificationLayout::segmentationsDragged(SegmentationAdapterList   segmentations,
                                                CategoryAdapterPtr category)
{
//   m_undoStack->beginMacro(tr("Change Segmentation's Taxonomy"));
//   {
//     m_undoStack->push(new ChangeTaxonomyCommand(segmentations,
//                                                 taxonomy,
//                                                 m_model,
//                                                 m_viewManager));
//   }
//   m_undoStack->endMacro();
}

//------------------------------------------------------------------------
void ClassificationLayout::categoriesDragged(CategoryAdapterList subTaxonomies,
                                             CategoryAdapterPtr  taxonomy)
{
//   CategoryAdapterList validSubTaxonomies;
//   foreach(CategoryAdapterPtr subTaxonomy, subTaxonomies)
//   {
//     if (!taxonomy->element(subTaxonomy->name()))
//     {
//       bool nameConflict = false;
//       foreach (CategoryAdapterPtr validSubTaxonomy, validSubTaxonomies)
//       {
//         if (validSubTaxonomy->name() == subTaxonomy->name())
//           nameConflict = true;
//       }
//       if (!nameConflict)
//         validSubTaxonomies << subTaxonomy;
//     }
//   }
//   if (!validSubTaxonomies.isEmpty())
//   {
//     m_undoStack->beginMacro(tr("Modify Taxonomy"));
//     m_undoStack->push(new MoveTaxonomiesCommand(validSubTaxonomies, taxonomy, m_model));
//     m_undoStack->endMacro();
//   }
}

//------------------------------------------------------------------------
void ClassificationLayout::updateSelection()
{
//   int numTax = 0;
// 
//   QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();
// 
//   foreach(QModelIndex index, selectedIndexes)
//   {
//     ItemAdapterPtr item = ClassificationLayout::item(index);
//     switch (item->type())
//     {
//       case EspINA::TAXONOMY:
//         numTax++;
//         break;
//       default:
//         break;
//     }
//   }
// 
//   bool enabled = (numTax == 1);
//   m_createTaxonomy->setEnabled(m_model->taxonomy().get());
//   m_createSubTaxonomy->setEnabled(enabled);
//   m_changeTaxonomyColor->setEnabled(enabled);
}

//------------------------------------------------------------------------
void ClassificationLayout::disconnectSelectionModel()
{
  m_createTaxonomy = m_createSubTaxonomy = m_changeTaxonomyColor = NULL;

  disconnect(m_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(updateSelection()));
}

//------------------------------------------------------------------------
void ClassificationLayout::changeTaxonomyColor()
{
//   // sanity checks, not really necessary
//   QModelIndexList indexList = m_view->selectionModel()->selection().indexes();
//   if (indexList.size() != 1)
//     return;
// 
//   ItemAdapterPtr item = ClassificationLayout::item(indexList.first());
//   if (EspINA::TAXONOMY != item->type())
//     return;
// 
//   CategoryAdapterPtr taxonomy = taxonomyElementPtr(item);
// 
//   QColorDialog colorSelector(m_view->parentWidget());
//   colorSelector.setCurrentColor(taxonomy->color());
//   if(colorSelector.exec() == QDialog::Accepted)
//   {
//     taxonomy->setData(colorSelector.selectedColor(),
//                       Qt::DecorationRole);
// 
//     m_viewManager->updateSegmentationRepresentations();
//     m_viewManager->updateViews();
//   }
}

//------------------------------------------------------------------------
void ClassificationLayout::selectCategoryAdapters()
{
//   QModelIndex index = m_view->selectionModel()->currentIndex();
// 
//   if (!index.isValid())
//     return;
// 
//   ItemAdapterPtr itemptr = item(index);
//   if (EspINA::TAXONOMY != itemptr->type())
//   {
//     index = index.parent();
//     if (!index.isValid())
//       return;
// 
//     itemptr = item(index);
// 
//     Q_ASSERT(itemptr->type() == EspINA::TAXONOMY);
//   }
// 
//   QItemSelection newSelection;
//   foreach(QModelIndex sortIndex, indices(index, true))
//   {
//     if (!sortIndex.isValid())
//       continue;
// 
//     ItemAdapterPtr sortItem = item(sortIndex);
//     if (EspINA::SEGMENTATION != sortItem->type())
//       continue;
// 
//     QItemSelection selectedItem(sortIndex, sortIndex);
//     newSelection.merge(selectedItem, QItemSelectionModel::Select);
//   }
// 
//   m_view->selectionModel()->clearSelection();
//   m_view->selectionModel()->select(newSelection, QItemSelectionModel::Select);
}

//------------------------------------------------------------------------
bool ClassificationLayout::hasInformationToShow()
{
  QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();
//   foreach(QModelIndex index, selectedIndexes)
//   {
//     QModelIndexList subIndexes;
//     ItemAdapterPtr item = ClassificationLayout::item(index);
//     switch (item->type())
//     {
//       case EspINA::TAXONOMY:
//         subIndexes = indices(index, true);
//         foreach(QModelIndex subIndex, subIndexes)
//         {
//           if (EspINA::SEGMENTATION == ClassificationLayout::item(subIndex)->type())
//             return true;
//         }
//         break;
//       case EspINA::SEGMENTATION:
//         return true;
//         break;
//       default:
//         break;
//     }
//   }

  return false;
}
