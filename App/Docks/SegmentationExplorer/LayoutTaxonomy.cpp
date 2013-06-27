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


#include "LayoutTaxonomy.h"
#include <Menus/SegmentationContextualMenu.h>


#include <Core/Model/Segmentation.h>
#include <Core/Model/Taxonomy.h>
#include <GUI/QtWidget/SegmentationContextualMenu.h>
#include <Undo/ChangeTaxonomyCommand.h>
#include <Undo/TaxonomiesCommand.h>

#include <QMessageBox>
#include <QUndoStack>
#include <QColorDialog>

using namespace EspINA;


//------------------------------------------------------------------------
class RenameTaxonomyCommand
: public QUndoCommand
{
public:
  explicit RenameTaxonomyCommand(TaxonomyElementPtr taxonomyItem,
                                 const QString &name,
                                 EspinaModel   *model,
                                 QUndoCommand  *parent = 0)
  : QUndoCommand(parent)
  , m_model(model)
  , m_taxonomyItem(taxonomyItem)
  , m_name(name)
  {}

  virtual void redo() { swapName(); }

  virtual void undo() { swapName(); }

private:
  void swapName()
  {
    QString     tmp   = m_taxonomyItem->name();
    QModelIndex index = m_model->taxonomyIndex(m_taxonomyItem);

    m_model->setData(index, m_name, Qt::EditRole);

    m_name = tmp;
  }

private:
  EspinaModel       *m_model;
  TaxonomyElementPtr m_taxonomyItem;
  QString            m_name;
};



//------------------------------------------------------------------------
class TaxonomyItemDelegate
: public QItemDelegate
{
public:
  explicit TaxonomyItemDelegate(EspinaModel *model,
                                QUndoStack  *undoStack,
                                QObject     *parent = 0)
  : QItemDelegate(parent)
  , m_model(model)
  , m_undoStack(undoStack)
  {
  }

  virtual void setModelData(QWidget            *editor,
                            QAbstractItemModel *model,
                            const QModelIndex  &index) const
  {
    QSortFilterProxyModel *proxy = static_cast<QSortFilterProxyModel *>(model);
    ModelItemPtr item = indexPtr(proxy->mapToSource(index));
    if (EspINA::TAXONOMY == item->type())
    {
      QLineEdit *textEditor = static_cast<QLineEdit *>(editor);
      QString name = textEditor->text();

      TaxonomyElementPtr taxonomy = taxonomyElementPtr(item);

      if (!taxonomy->parent()->element(name))
      {
        m_undoStack->beginMacro("Rename Taxonomy");
        m_undoStack->push(new RenameTaxonomyCommand(taxonomy, name, m_model));
        m_undoStack->endMacro();
      }
    }
  }

private:
  EspinaModel *m_model;
  QUndoStack  *m_undoStack;
};

//------------------------------------------------------------------------
bool TaxonomyLayout::SortFilter::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  ModelItemPtr leftItem  = indexPtr(left);
  ModelItemPtr rightItem = indexPtr(right);

  if (leftItem->type() == rightItem->type())
    if (EspINA::SEGMENTATION == leftItem->type())
      return sortSegmentationLessThan(leftItem, rightItem);
    else
      return leftItem->data(Qt::DisplayRole).toString() < rightItem->data(Qt::DisplayRole).toString();
    else
      return leftItem->type() == EspINA::TAXONOMY;
}

//------------------------------------------------------------------------
TaxonomyLayout::TaxonomyLayout(CheckableTreeView     *view,
                               EspinaModel           *model,
                               QUndoStack            *undoStack,
                               ViewManager           *viewManager)
: Layout    (view, model, undoStack, viewManager)
, m_proxy   (new TaxonomyProxy())
, m_sort    (new SortFilter())
, m_delegate(new TaxonomyItemDelegate(model, undoStack, this))
, m_createTaxonomy(NULL)
, m_createSubTaxonomy(NULL)
, m_changeTaxonomyColor(NULL)
{
  m_proxy->setSourceModel(m_model);
  m_sort->setSourceModel(m_proxy.get());
  m_sort->setDynamicSortFilter(true);

  connect(m_proxy.get(), SIGNAL(segmentationsDragged(SegmentationList,TaxonomyElementPtr)),
          this,           SLOT  (segmentationsDragged(SegmentationList,TaxonomyElementPtr)));
  connect(m_proxy.get(), SIGNAL(taxonomiesDragged(TaxonomyElementList,TaxonomyElementPtr)),
          this,           SLOT  (taxonomiesDragged(TaxonomyElementList,TaxonomyElementPtr)));

  connect(m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this,  SLOT(updateSelection()));
  connect(m_model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
          this,  SLOT(updateSelection()));
  connect(m_model, SIGNAL(modelReset()),
          this,  SLOT(updateSelection()));
}

//------------------------------------------------------------------------
TaxonomyLayout::~TaxonomyLayout()
{
//   qDebug() << "Destroying Taxonomy Layout";
}

//------------------------------------------------------------------------
void TaxonomyLayout::createSpecificControls(QHBoxLayout *specificControlLayout)
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
void TaxonomyLayout::contextMenu(const QPoint &pos)
{
  TaxonomyElementList taxonomies;
  SegmentationSet     segmentations;

  if (!selectedItems(taxonomies, segmentations))
    return;

  if (taxonomies.isEmpty())
  {
    DefaultContextualMenu contextMenu(segmentations.toList(),
                                           m_model,
                                           m_undoStack,
                                           m_viewManager);

    contextMenu.addSeparator();

    QAction *selectAll = contextMenu.addAction(tr("Select segmentations of the same category"));
    connect(selectAll, SIGNAL(triggered(bool)),
            this,      SLOT(selectTaxonomyElements()));

    contextMenu.exec(pos);
    return;
  }

  QMenu contextMenu;

  QAction *createNode = contextMenu.addAction(tr("Create Category"));
  createNode->setIcon(QIcon(":espina/create_node.png"));
  connect(createNode, SIGNAL(triggered(bool)),
          this,       SLOT(createTaxonomy()));

  QAction *createSubNode = contextMenu.addAction(tr("Create Subcategory"));
  createSubNode->setIcon(QIcon(":espina/create_subnode.png"));
  connect(createSubNode, SIGNAL(triggered(bool)),
          this,          SLOT(createSubTaxonomy()));

  QAction *changeColor = contextMenu.addAction(tr("Change Category Color"));
  changeColor->setIcon(QIcon(":espina/rainbow.svg"));
  connect(changeColor, SIGNAL(triggered(bool)),
          this,        SLOT(changeTaxonomyColor()));

  contextMenu.addSeparator();

  QAction *selectAll = contextMenu.addAction(tr("Select category segmentations"));
  connect(selectAll, SIGNAL(triggered(bool)),
          this,      SLOT(selectTaxonomyElements()));

  contextMenu.exec(pos);
}

//------------------------------------------------------------------------
void TaxonomyLayout::deleteSelectedItems()
{
  TaxonomyElementList  taxonomies, additionalTaxonomies;
  SegmentationSet      segmentations;

  if (!selectedItems(taxonomies, segmentations))
    return;

  if (!taxonomies.isEmpty())
  {
    QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();
    QModelIndexList additionalIndexes;
    foreach(QModelIndex index, selectedIndexes)
    {
      if (!index.isValid())
        continue;

      ModelItemPtr item = TaxonomyLayout::item(index);
      switch (item->type())
      {
        case EspINA::TAXONOMY:
          additionalIndexes = indices(index, true);
          foreach(QModelIndex additionalIndex, additionalIndexes)
          {
            ModelItemPtr additionalItem = TaxonomyLayout::item(additionalIndex);
            if (additionalItem->type() == EspINA::SEGMENTATION)
            {
              if (!segmentations.contains(segmentationPtr(additionalItem)))
                segmentations << segmentationPtr(additionalItem);
            }
            else
            {
              if (!additionalTaxonomies.contains(taxonomyElementPtr(additionalItem)))
                additionalTaxonomies << taxonomyElementPtr(additionalItem);
            }
          }
          break;
        default:
          continue;
          break;
      }
    }

    if (!segmentations.isEmpty())
    {
      QMessageBox msg;
      msg.setText(tr("Delete Selected Items. Warning: all elements under selected items will also be deleted"));
      QPushButton *none             = msg.addButton(tr("Cancel"), QMessageBox::RejectRole);
      /*QPushButton *recursiveTax =*/ msg.addButton(tr("Taxonomies and Segmentations"), QMessageBox::AcceptRole);
      QPushButton *onlySeg          = msg.addButton(tr("Only Segmentations"), QMessageBox::AcceptRole);

      msg.exec();

      if (msg.clickedButton() == none)
        return;

      if(msg.clickedButton() == onlySeg)
      {
        deleteSegmentations(segmentations.toList());
        return;
      }
    }

    // assuming taxonomies are empty, because if they weren't then !segmentations.empty()
    m_undoStack->beginMacro("Remove Taxonomy");
    deleteSegmentations(segmentations.toList());
    foreach(TaxonomyElementPtr taxonomy, additionalTaxonomies)
    {
      if (m_model->taxonomy()->element(taxonomy->qualifiedName()))
      {
        m_undoStack->push(new RemoveTaxonomyElementCommand(taxonomy, m_model));
      }
    }

    foreach(TaxonomyElementPtr taxonomy, taxonomies)
    {
      if (m_model->taxonomy()->element(taxonomy->qualifiedName()))
      {
        m_undoStack->push(new RemoveTaxonomyElementCommand(taxonomy, m_model));
      }
    }
    m_undoStack->endMacro();
  }
  else
    if (!segmentations.empty())
      deleteSegmentations(segmentations.toList());
}

//------------------------------------------------------------------------
void TaxonomyLayout::showSelectedItemsInformation()
{
  TaxonomyElementList  taxonomies;
  SegmentationSet      segmentations;

  if (!selectedItems(taxonomies, segmentations))
    return;

  if (!taxonomies.empty())
  {
    QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();
    QModelIndexList subIndexes;
    foreach(QModelIndex index, selectedIndexes)
    {
      ModelItemPtr item = TaxonomyLayout::item(index);
      if (EspINA::TAXONOMY == item->type())
      {
        subIndexes << indices(index, true);
        foreach(QModelIndex subIndex, subIndexes)
        {
          ModelItemPtr subItem = TaxonomyLayout::item(subIndex);
          if (EspINA::SEGMENTATION == subItem->type())
          {
            SegmentationPtr seg = segmentationPtr(subItem);
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
QItemDelegate *TaxonomyLayout::itemDelegate() const
{
  return m_delegate;
}

//------------------------------------------------------------------------
bool TaxonomyLayout::selectedItems(TaxonomyElementList &taxonomies, SegmentationSet &segmentations)
{
  QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();
  foreach(QModelIndex index, selectedIndexes)
  {
    ModelItemPtr item = TaxonomyLayout::item(index);
    switch (item->type())
    {
      case EspINA::SEGMENTATION:
        segmentations << segmentationPtr(item);
        break;
      case EspINA::TAXONOMY:
        taxonomies << taxonomyElementPtr(item);
        break;
      default:
        Q_ASSERT(false);
        break;
    }
  }

  return !taxonomies.isEmpty() || !segmentations.isEmpty();
}

//------------------------------------------------------------------------
void TaxonomyLayout::createTaxonomy()
{
  QModelIndex currentIndex = m_view->currentIndex();
  ModelItemPtr taxonomyItem;
  if (currentIndex.isValid())
    taxonomyItem = item(currentIndex);
  else if (m_view->model()->rowCount() > 0)
    taxonomyItem = m_model->taxonomy()->elements().first().get();
  else
    return;

  if (EspINA::TAXONOMY == taxonomyItem->type())
  {
    QString name = tr("New Taxonomy");

    TaxonomyElementPtr selectedTaxonomy = taxonomyElementPtr(taxonomyItem);
    TaxonomyElementPtr parentTaxonomy   = selectedTaxonomy->parent();
    if (!parentTaxonomy->element(name))
    {
      m_undoStack->beginMacro("Create Taxonomy");
      m_undoStack->push(new AddTaxonomyElement(parentTaxonomy, name, m_model, parentTaxonomy->color()));
      m_undoStack->endMacro();
    }
  }
}

//------------------------------------------------------------------------
void TaxonomyLayout::createSubTaxonomy()
{
  QModelIndex currentIndex = m_view->currentIndex();
  if (!currentIndex.isValid())
    return;

  ModelItemPtr taxonomyItem = item(currentIndex);

  if (EspINA::TAXONOMY == taxonomyItem->type())
  {
    QString name = tr("New Taxonomy");

    TaxonomyElementPtr taxonomy = taxonomyElementPtr(taxonomyItem);
    if (!taxonomy->element(name))
    {
      m_undoStack->beginMacro("Create Taxonomy");
      m_undoStack->push(new AddTaxonomyElement(taxonomy, name, m_model, taxonomy->color()));
      m_undoStack->endMacro();
    }
  }
}

//------------------------------------------------------------------------
void TaxonomyLayout::segmentationsDragged(SegmentationList   segmentations,
                                          TaxonomyElementPtr taxonomy)
{
  m_undoStack->beginMacro(tr("Change Segmentation's Taxonomy"));
  {
    m_undoStack->push(new ChangeTaxonomyCommand(segmentations,
                                                taxonomy,
                                                m_model,
                                                m_viewManager));
  }
  m_undoStack->endMacro();
}

//------------------------------------------------------------------------
void TaxonomyLayout::taxonomiesDragged(TaxonomyElementList subTaxonomies,
                                       TaxonomyElementPtr  taxonomy)
{
  TaxonomyElementList validSubTaxonomies;
  foreach(TaxonomyElementPtr subTaxonomy, subTaxonomies)
  {
    if (!taxonomy->element(subTaxonomy->name()))
    {
      bool nameConflict = false;
      foreach (TaxonomyElementPtr validSubTaxonomy, validSubTaxonomies)
      {
        if (validSubTaxonomy->name() == subTaxonomy->name())
          nameConflict = true;
      }
      if (!nameConflict)
        validSubTaxonomies << subTaxonomy;
    }
  }
  if (!validSubTaxonomies.isEmpty())
  {
    m_undoStack->beginMacro(tr("Modify Taxonomy"));
    m_undoStack->push(new MoveTaxonomiesCommand(validSubTaxonomies, taxonomy, m_model));
    m_undoStack->endMacro();
  }
}

//------------------------------------------------------------------------
void TaxonomyLayout::updateSelection()
{
  int numTax = 0;

  QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();

  foreach(QModelIndex index, selectedIndexes)
  {
    ModelItemPtr item = TaxonomyLayout::item(index);
    switch (item->type())
    {
      case EspINA::TAXONOMY:
        numTax++;
        break;
      default:
        break;
    }
  }

  bool enabled = (numTax == 1);
  m_createTaxonomy->setEnabled(m_model->taxonomy());
  m_createSubTaxonomy->setEnabled(enabled);
  m_changeTaxonomyColor->setEnabled(enabled);
}

//------------------------------------------------------------------------
void TaxonomyLayout::disconnectSelectionModel()
{
  m_createTaxonomy = m_createSubTaxonomy = m_changeTaxonomyColor = NULL;

  disconnect(m_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(updateSelection()));
}

//------------------------------------------------------------------------
void TaxonomyLayout::changeTaxonomyColor()
{
  // sanity checks, not really necessary
  QModelIndexList indexList = m_view->selectionModel()->selection().indexes();
  if (indexList.size() != 1)
    return;

  ModelItemPtr item = TaxonomyLayout::item(indexList.first());
  if (EspINA::TAXONOMY != item->type())
    return;

  TaxonomyElementPtr taxonomy = taxonomyElementPtr(item);

  QColorDialog colorSelector(m_view->parentWidget());
  colorSelector.setCurrentColor(taxonomy->color());
  if(colorSelector.exec() == QDialog::Accepted)
  {
    taxonomy->setData(colorSelector.selectedColor(),
                      Qt::DecorationRole);

    m_viewManager->updateSegmentationRepresentations();
    m_viewManager->updateViews();
  }
}

//------------------------------------------------------------------------
void TaxonomyLayout::selectTaxonomyElements()
{
  QModelIndex index = m_view->selectionModel()->currentIndex();

  if (!index.isValid())
    return;

  ModelItemPtr itemptr = item(index);
  if (EspINA::TAXONOMY != itemptr->type())
  {
    index = index.parent();
    if (!index.isValid())
      return;

    itemptr = item(index);

    Q_ASSERT(itemptr->type() == EspINA::TAXONOMY);
  }

  QItemSelection newSelection;
  foreach(QModelIndex sortIndex, indices(index, true))
  {
    if (!sortIndex.isValid())
      continue;

    ModelItemPtr sortItem = item(sortIndex);
    if (EspINA::SEGMENTATION != sortItem->type())
      continue;

    QItemSelection selectedItem(sortIndex, sortIndex);
    newSelection.merge(selectedItem, QItemSelectionModel::Select);
  }

  m_view->selectionModel()->clearSelection();
  m_view->selectionModel()->select(newSelection, QItemSelectionModel::Select);
}

//------------------------------------------------------------------------
bool TaxonomyLayout::hasInformationToShow()
{
  QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();
  foreach(QModelIndex index, selectedIndexes)
  {
    QModelIndexList subIndexes;
    ModelItemPtr item = TaxonomyLayout::item(index);
    switch (item->type())
    {
      case EspINA::TAXONOMY:
        subIndexes = indices(index, true);
        foreach(QModelIndex subIndex, subIndexes)
        {
          if (EspINA::SEGMENTATION == TaxonomyLayout::item(subIndex)->type())
            return true;
        }
        break;
      case EspINA::SEGMENTATION:
        return true;
        break;
      default:
        break;
    }
  }

  return false;
}
