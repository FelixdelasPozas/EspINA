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


#include "LayoutLocation.h"
#include <Menus/SegmentationContextualMenu.h>

#include <Core/Model/Sample.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/EspinaModel.h>
#include <GUI/QtWidget/SegmentationContextualMenu.h>

#include <QMessageBox>

using namespace ESPINA;

bool LocationLayout::SortFilter::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  ModelItemPtr leftItem  = indexPtr(left);
  ModelItemPtr rightItem = indexPtr(right);

  if (leftItem->type() == rightItem->type())
    if (ESPINA::SEGMENTATION == leftItem->type())
      return sortSegmentationLessThan(leftItem, rightItem);
    else
      return leftItem->data(Qt::DisplayRole).toString() < rightItem->data(Qt::DisplayRole).toString();
    else
      return leftItem->type() == ESPINA::TAXONOMY;
}

//------------------------------------------------------------------------
LocationLayout::LocationLayout(CheckableTreeView *view,
                           EspinaModel       *model,
                           QUndoStack        *undoStack,
                           ViewManager       *viewManager)
: Layout (view, model, undoStack, viewManager)
, m_proxy(new LocationProxy())
, m_sort (new SortFilter())
, m_delegate(new QItemDelegate())
{
  m_proxy->setSourceModel(m_model);
  m_sort->setSourceModel(m_proxy.get());
  m_sort->setDynamicSortFilter(true);
}

//------------------------------------------------------------------------
LocationLayout::~LocationLayout()
{
//   qDebug() << "Destroying Sample Layout";
}

//------------------------------------------------------------------------
ModelItemPtr LocationLayout::item(const QModelIndex& index) const
{
  return indexPtr(m_sort->mapToSource(index)); 
}

//------------------------------------------------------------------------
QModelIndex LocationLayout::index(ModelItemPtr item) const
{
  return m_sort->mapFromSource(m_proxy->mapFromSource(Layout::index(item)));
}

//------------------------------------------------------------------------
void LocationLayout::contextMenu(const QPoint &pos)
{
  SampleList      samples;
  SegmentationSet segmentations;

  if (!selectedItems(samples, segmentations))
    return;

  if (samples.isEmpty())
  {
    DefaultContextualMenu contextMenu(segmentations.toList(),
                                           m_model,
                                           m_undoStack,
                                           m_viewManager);

    contextMenu.addSeparator();

    QAction *selectAll = contextMenu.addAction(tr("Select segmentations in the same location"));
    connect(selectAll, SIGNAL(triggered(bool)), this, SLOT(selectLocationElements()));

    contextMenu.exec(pos);
  }
}

//------------------------------------------------------------------------
void LocationLayout::deleteSelectedItems()
{
  SampleList      samples;
  SegmentationSet segmentations;

  if (!selectedItems(samples, segmentations))
    return;

  if (samples.isEmpty())
  {
    deleteSegmentations(segmentations.toList());
  }
}

//------------------------------------------------------------------------
void LocationLayout::showSelectedItemsInformation()
{
  SampleList      samples;
  SegmentationSet segmentations;

  if (!selectedItems(samples, segmentations))
    return;

  if (!samples.empty())
  {
    QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();
    QModelIndexList subIndexes;
    foreach(QModelIndex index, selectedIndexes)
    {
      ModelItemPtr itemPtr = item(index);
      if (ESPINA::SAMPLE == itemPtr->type())
      {
        subIndexes << indices(index, true);
        foreach(QModelIndex subIndex, subIndexes)
        {
          ModelItemPtr subItem = item(subIndex);
          if (ESPINA::SEGMENTATION == subItem->type())
          {
            SegmentationPtr seg = segmentationPtr(subItem);
            if (!segmentations.contains(seg))
              segmentations << seg;
          }
        }
      }
    }
  }

  if (!segmentations.isEmpty())
  {
    QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();
    QModelIndexList subIndexes;
    foreach(QModelIndex index, selectedIndexes)
    {
      ModelItemPtr itemPtr = item(index);
      if (ESPINA::SEGMENTATION == itemPtr->type())
      {
        subIndexes << indices(index, true);
        foreach(QModelIndex subIndex, subIndexes)
        {
          ModelItemPtr subItem = item(subIndex);
          if (ESPINA::SEGMENTATION == subItem->type())
          {
            SegmentationPtr seg = segmentationPtr(subItem);
            if (!segmentations.contains(seg))
              segmentations << seg;
          }
        }
      }
    }
  }

  if (!segmentations.empty())
    showSegmentationInformation(segmentations.toList());
}

//------------------------------------------------------------------------
QItemDelegate *LocationLayout::itemDelegate() const
{
  return m_delegate;
}

//------------------------------------------------------------------------
bool LocationLayout::selectedItems(SampleList &samples, SegmentationSet &segmentations)
{
  QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();
  foreach(QModelIndex index, selectedIndexes)
  {
    ModelItemPtr item = LocationLayout::item(index);
    switch (item->type())
    {
      case ESPINA::SEGMENTATION:
        segmentations << segmentationPtr(item);
        break;
      case ESPINA::SAMPLE:
        samples << samplePtr(item);
        break;
      default:
        Q_ASSERT(false);
        break;
    }
  }

  return !samples.isEmpty() || !segmentations.isEmpty();
}

//------------------------------------------------------------------------
void LocationLayout::selectLocationElements()
{
  QModelIndex index = m_view->selectionModel()->currentIndex();

  if (!index.isValid() || !index.parent().isValid())
    return;

  QItemSelection newSelection;
  foreach(QModelIndex sortIndex, indices(index.parent(), false))
  {
    if (!sortIndex.isValid())
      continue;

    ModelItemPtr sortItem = item(sortIndex);
    if (ESPINA::SEGMENTATION != sortItem->type())
      continue;

    QItemSelection selectedItem(sortIndex, sortIndex);
    newSelection.merge(selectedItem, QItemSelectionModel::Select);
  }

  m_view->selectionModel()->clearSelection();
  m_view->selectionModel()->select(newSelection, QItemSelectionModel::Select);
}

//------------------------------------------------------------------------
bool LocationLayout::hasInformationToShow()
{
  SampleList samples;
  SegmentationSet segmentations;
  selectedItems(samples, segmentations);

  bool sampleHasSegmentations = false;
  if (!samples.empty())
  {
    QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();
    QModelIndexList subIndexes;
    foreach(QModelIndex index, selectedIndexes)
    {
      ModelItemPtr itemPtr = item(index);
      if (ESPINA::SAMPLE == itemPtr->type())
      {
        subIndexes << indices(index, true);
        if (!subIndexes.empty())
          sampleHasSegmentations = true;
      }
    }
  }

  return !segmentations.empty() || sampleHasSegmentations;
}
