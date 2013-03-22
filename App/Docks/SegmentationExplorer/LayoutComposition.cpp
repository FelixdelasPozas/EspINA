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

// EspINA
#include "LayoutComposition.h"
#include <Core/Model/Segmentation.h>
#include <Core/Relations.h>
#include <GUI/QtWidget/SegmentationContextualMenu.h>

// Qt
#include <QMessageBox>

using namespace EspINA;

bool CompositionLayout::SortFilter::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  ModelItemPtr leftItem  = indexPtr(left);
  ModelItemPtr rightItem = indexPtr(right);

  return sortSegmentationLessThan(leftItem, rightItem);
}

//------------------------------------------------------------------------
CompositionLayout::CompositionLayout(CheckableTreeView *view,
                                     EspinaModel       *model,
                                     QUndoStack        *undoStack,
                                     ViewManager       *viewManager)
: Layout (view, model, undoStack, viewManager)
, m_proxy(new RelationProxy())
, m_sort (new SortFilter())
, m_delegate(new QItemDelegate())
{
  m_proxy->setRelation(Relations::COMPOSITION);
  m_proxy->setSourceModel(m_model);
  m_sort->setSourceModel(m_proxy.data());
  m_sort->setDynamicSortFilter(true);
}

//------------------------------------------------------------------------
CompositionLayout::~CompositionLayout()
{
  qDebug() << "Destroying Composition Layout";
}

//------------------------------------------------------------------------
ModelItemPtr CompositionLayout::item(const QModelIndex& index) const
{
  return indexPtr(m_sort->mapToSource(index));
}

//------------------------------------------------------------------------
QModelIndex CompositionLayout::index(ModelItemPtr item) const
{
  return m_sort->mapFromSource(m_proxy->mapFromSource(Layout::index(item)));
}

//------------------------------------------------------------------------
void CompositionLayout::contextMenu(const QPoint &pos)
{
  SegmentationSet segmentations;

  if (!selectedItems(segmentations))
    return;

  SegmentationContextualMenu contextMenu(segmentations.toList(),
                                         m_model,
                                         m_undoStack,
                                         m_viewManager);

  contextMenu.addSeparator();

  QAction *selectAll = contextMenu.addAction(tr("Select all segmentations that compose this one"));
  connect(selectAll, SIGNAL(triggered(bool)), this, SLOT(selectComposeElements()));

  QModelIndex index = m_view->selectionModel()->currentIndex();
  if (segmentations.size() != 1 || indices(index,true).isEmpty())
    selectAll->setEnabled(false);

  contextMenu.exec(pos);
}

//------------------------------------------------------------------------
void CompositionLayout::deleteSelectedItems()
{
  QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();
  SegmentationList segsToDelete = deletedSegmentations(selectedIndexes);
  deleteSegmentations(segsToDelete);
}

//------------------------------------------------------------------------
QItemDelegate *CompositionLayout::itemDelegate() const
{
  return m_delegate;
}

//------------------------------------------------------------------------
void CompositionLayout::showSelectedItemsInformation()
{
  SampleList      samples;
  SegmentationSet segmentations;

  if (!selectedItems(segmentations))
    return;

  showSegmentationInformation(segmentations.toList());
}

//------------------------------------------------------------------------
bool CompositionLayout::selectedItems(SegmentationSet &segmentations)
{
  QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();
  foreach(QModelIndex index, selectedIndexes)
  {
    ModelItemPtr item = CompositionLayout::item(index);
    switch (item->type())
    {
      case EspINA::SEGMENTATION:
        segmentations << segmentationPtr(item);
        break;
      default:
        Q_ASSERT(false);
        break;
    }
  }

  return !segmentations.isEmpty();
}

//------------------------------------------------------------------------
SegmentationList CompositionLayout::deletedSegmentations(QModelIndexList selection)
{
  QSet<SegmentationPtr> toDelete;
  bool recursiveToAll = false;
  bool recursive = false;
  foreach(QModelIndex sortIndex, selection)
  {
    if (m_sort->rowCount(sortIndex) > 0 && !recursiveToAll)
    {
      QMessageBox msgBox(m_view->parentWidget());
      msgBox.setText(SEGMENTATION_MESSAGE.arg(sortIndex.data().toString()));
      if (selection.size() > 1)
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll);
      else
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
      msgBox.setDefaultButton(QMessageBox::No);

      switch(msgBox.exec())
      {
        case QMessageBox::Yes:
          recursive = true;
          break;
        case QMessageBox::YesToAll:
          recursive = true;
          recursiveToAll = true;
          break;
        case QMessageBox::No:
          recursive = false;
          break;
        case QMessageBox::NoToAll:
          recursive = false;
          recursiveToAll = true;
          break;
      }
    }

    ModelItemPtr selectedItem = item(sortIndex);
    SegmentationPtr seg = segmentationPtr(selectedItem);
    Q_ASSERT(seg);
    if (!toDelete.contains(seg))
      toDelete << seg;

    if (recursive)
    {
      foreach(QModelIndex subIndex, indices(sortIndex, true))
      {
        ModelItemPtr subItem = item(subIndex);
        SegmentationPtr seg = segmentationPtr(subItem);
        Q_ASSERT(seg);
        if (!toDelete.contains(seg))
          toDelete << seg;
      }
    }
  }

  return toDelete.toList();
}

//------------------------------------------------------------------------
void CompositionLayout::selectComposeElements()
{
  QModelIndex index = m_view->selectionModel()->currentIndex();
  if (!index.isValid())
    return;

  QItemSelection newSelection;
  foreach(QModelIndex sortIndex, indices(index, false))
  {
    if (!sortIndex.isValid())
      continue;

    QItemSelection selectedItem(sortIndex, sortIndex);
    newSelection.merge(selectedItem, QItemSelectionModel::Select);
  }

  m_view->selectionModel()->clearSelection();
  m_view->selectionModel()->select(newSelection, QItemSelectionModel::Select);
}

//------------------------------------------------------------------------
bool CompositionLayout::hasInformationToShow()
{
  SegmentationSet unused;
  return selectedItems(unused);
}
