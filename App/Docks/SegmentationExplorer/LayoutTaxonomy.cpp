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
#include <Undo/ChangeTaxonomyCommand.h>
#include <Undo/MoveTaxonomiesCommand.h>
#include <Undo/RemoveTaxonomyElementCommand.h>

#include <Core/Model/Segmentation.h>
#include <Core/Model/Taxonomy.h>
#include <GUI/QtWidget/SegmentationContextualMenu.h>
#include <Undo/AddTaxonomyElement.h>

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

      if (taxonomy->parent()->element(name).isNull())
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
{
  m_proxy->setSourceModel(m_model);
  m_sort->setSourceModel(m_proxy.data());
  m_sort->setDynamicSortFilter(true);

  connect(m_proxy.data(), SIGNAL(segmentationsDragged(SegmentationList,TaxonomyElementPtr)),
          this,           SLOT  (segmentationsDragged(SegmentationList,TaxonomyElementPtr)));
  connect(m_proxy.data(), SIGNAL(taxonomiesDragged(TaxonomyElementList,TaxonomyElementPtr)),
          this,           SLOT  (taxonomiesDragged(TaxonomyElementList,TaxonomyElementPtr)));
}

//------------------------------------------------------------------------
TaxonomyLayout::~TaxonomyLayout()
{
  qDebug() << "Destroying Taxonomy Layout";
}

//------------------------------------------------------------------------
void TaxonomyLayout::createSpecificControls(QHBoxLayout *specificControlLayout)
{
  QPushButton *createTaxonomy = new QPushButton();
  createTaxonomy->setIcon(QIcon(":espina/create_node.png"));
  createTaxonomy->setIconSize(QSize(22,22));
  createTaxonomy->setBaseSize(32, 32);
  createTaxonomy->setMaximumSize(32, 32);
  createTaxonomy->setMinimumSize(32, 32);
  createTaxonomy->setFlat(true);
  createTaxonomy->setEnabled(false);

  connect(createTaxonomy, SIGNAL(clicked(bool)),
          this, SLOT(createTaxonomy()));

  specificControlLayout->addWidget(createTaxonomy);

  QPushButton *createSubTaxonomy = new QPushButton();
  createSubTaxonomy->setIcon(QIcon(":espina/create_subnode.png"));
  createSubTaxonomy->setIconSize(QSize(22,22));
  createSubTaxonomy->setBaseSize(32, 32);
  createSubTaxonomy->setMaximumSize(32, 32);
  createSubTaxonomy->setMinimumSize(32, 32);
  createSubTaxonomy->setFlat(true);
  createSubTaxonomy->setEnabled(false);

  connect(createSubTaxonomy, SIGNAL(clicked(bool)),
          this, SLOT(createSubTaxonomy()));

  specificControlLayout->addWidget(createSubTaxonomy);

  // the model of CheckableTreeView has been set by now (wasn't in constructor): connect signals
  connect(m_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(updateSelection(QItemSelection,QItemSelection)));

  m_createTaxonomy = createTaxonomy;
  m_createSubTaxonomy = createSubTaxonomy;
  connect(m_createTaxonomy, SIGNAL(destroyed()), this, SLOT(disconnectSelectionModel()));
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
    SegmentationContextualMenu contextMenu(segmentations.toList(),
                                           m_model,
                                           m_undoStack,
                                           m_viewManager);

    contextMenu.exec(pos);
  }

  if (segmentations.isEmpty())
  {
    QMenu contextMenu;

    QAction *createNode = contextMenu.addAction(tr("Create Taxonomy"));
    createNode->setIcon(QIcon(":espina/create_node.png"));
    connect(createNode, SIGNAL(triggered(bool)),
            this, SLOT(createTaxonomy()));

    QAction *createSubNode = contextMenu.addAction(tr("Create SubTaxonomy"));
    createSubNode->setIcon(QIcon(":espina/create_subnode.png"));
    connect(createSubNode, SIGNAL(triggered(bool)),
            this, SLOT(createSubTaxonomy()));

    contextMenu.exec(pos);
  }
}

//------------------------------------------------------------------------
void TaxonomyLayout::deleteSelectedItems()
{
  TaxonomyElementList  taxonomies;
  SegmentationSet      segmentations;

  if (!selectedItems(taxonomies, segmentations))
    return;

  if (taxonomies.isEmpty())
  {
    deleteSegmentations(segmentations.toList());
  }
  else
  {
    foreach(TaxonomyElementPtr taxonomy, taxonomies)
    {
      foreach(SegmentationPtr segmentation, m_proxy->segmentations(taxonomy, true))
      {
        segmentations << segmentation;
      }
    }

    bool recursiveRemoval = false;

    if (!segmentations.isEmpty())
    {
      QMessageBox msg;
      msg.setText(tr("Delete Selected Items. Warning: all elements under selected items will also be deleted"));
      QPushButton *recursiveTax = msg.addButton(tr("Taxonomies"), QMessageBox::AcceptRole);
      QPushButton *onlySeg      = msg.addButton(tr("Only Segmemtations"), QMessageBox::AcceptRole);
      QPushButton *none         = msg.addButton(tr("Cancel"), QMessageBox::RejectRole);

      msg.exec();

      if (msg.clickedButton() == none)
        return;

      recursiveRemoval = msg.clickedButton() == recursiveTax;

      if (recursiveRemoval)
        m_undoStack->beginMacro("Remove Taxonomy");

      // Delete Segmentations
      deleteSegmentations(segmentations.toList());

      if (recursiveRemoval)
        segmentations.clear();
    }

    if (segmentations.isEmpty())
    {
      // Remove Taxonomies
      if (!recursiveRemoval)
        m_undoStack->beginMacro(tr("Remove Taxonomy"));

      foreach(TaxonomyElementPtr taxonomy, taxonomies)
      {
        if (!m_model->taxonomy()->element(taxonomy->qualifiedName()).isNull())
        {
          m_undoStack->push(new RemoveTaxonomyElementCommand(taxonomy, m_model));
        }
      }
      m_undoStack->endMacro();
    }
  }
}

//------------------------------------------------------------------------
void TaxonomyLayout::showSelectedItemsInformation()
{
  TaxonomyElementList  taxonomies;
  SegmentationSet      segmentations;

  if (!selectedItems(taxonomies, segmentations))
    return;

  if (taxonomies.size() == 1 && segmentations.isEmpty())
  {
    // Change Taxonomy Color
    QColorDialog colorSelector;
    if( colorSelector.exec() == QDialog::Accepted)
    {
      TaxonomyElementPtr taxonomy = taxonomies.first();
      taxonomy->setData(colorSelector.selectedColor(),
                        Qt::DecorationRole);

      m_viewManager->updateSegmentationRepresentations();
      m_viewManager->updateViews();
    }
  } else if (!segmentations.isEmpty())
  {
    showSegmentationInformation(segmentations.toList());
  }

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
    taxonomyItem = m_model->taxonomy()->elements().first().data();
  else
    return;


  if (EspINA::TAXONOMY == taxonomyItem->type())
  {
    QString name = tr("New Taxonomy");

    TaxonomyElementPtr selectedTaxonomy = taxonomyElementPtr(taxonomyItem);
    TaxonomyElementPtr parentTaxonomy   = selectedTaxonomy->parent();
    if (parentTaxonomy->element(name).isNull())
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
    if (taxonomy->element(name).isNull())
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
    if (taxonomy->element(subTaxonomy->name()).isNull())
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
void TaxonomyLayout::updateSelection(QItemSelection selected, QItemSelection deselected)
{
  int numSeg = 0;
  int numTax = 0;

  QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();
  foreach(QModelIndex index, selectedIndexes)
  {
    ModelItemPtr item = TaxonomyLayout::item(index);
    switch (item->type())
    {
      case EspINA::SEGMENTATION:
        numSeg++;
        break;
      case EspINA::TAXONOMY:
        numTax++;
        break;
      default:
        Q_ASSERT(false);
        break;
    }
  }

  m_createTaxonomy->setEnabled((numSeg == 0) && (numTax == 1));
  m_createSubTaxonomy->setEnabled((numSeg == 0) && (numTax == 1));
}

//------------------------------------------------------------------------
void TaxonomyLayout::disconnectSelectionModel()
{
  m_createTaxonomy = m_createSubTaxonomy = NULL;

  disconnect(m_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(updateSelection(QItemSelection,QItemSelection)));
}
