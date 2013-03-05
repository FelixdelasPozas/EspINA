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


#include "LayoutLocation.h"

#include <Core/Model/Sample.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/EspinaModel.h>
#include <GUI/QtWidget/SegmentationContextualMenu.h>

#include <QMessageBox>

using namespace EspINA;

bool LocationLayout::SortFilter::lessThan(const QModelIndex& left, const QModelIndex& right) const
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
  m_sort->setSourceModel(m_proxy.data());
  m_sort->setDynamicSortFilter(true);
}

//------------------------------------------------------------------------
LocationLayout::~LocationLayout()
{
  qDebug() << "Destroying Sample Layout";
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
    SegmentationContextualMenu contextMenu(segmentations.toList(),
                                           m_model,
                                           m_undoStack,
                                           m_viewManager);

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

  if (samples.size() == 1 && segmentations.isEmpty())
  {
    // TODO: Display Sample metadata
  } else if (!segmentations.isEmpty())
  {
    showSegmentationInformation(segmentations.toList());
  }

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
      case EspINA::SEGMENTATION:
        segmentations << segmentationPtr(item);
        break;
      case EspINA::SAMPLE:
        samples << samplePtr(item);
        break;
      default:
        Q_ASSERT(false);
    }
  }

  return !samples.isEmpty() || !segmentations.isEmpty();
}
