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


#include "LayoutSample.h"

#include <Core/Model/Sample.h>
#include <Core/Model/Segmentation.h>
#include <QMessageBox>

bool SampleLayout::SortFilter::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  ModelItem *leftItem  = indexPtr(left);
  ModelItem *rightItem = indexPtr(right);

  if (leftItem->type() == rightItem->type())
    if (ModelItem::SEGMENTATION == leftItem->type())
      return sortSegmentationLessThan(leftItem, rightItem);
    else
      return leftItem->data(Qt::DisplayRole).toString() < rightItem->data(Qt::DisplayRole).toString();
    else
      return leftItem->type() == ModelItem::TAXONOMY;
}

//------------------------------------------------------------------------
SampleLayout::SampleLayout(EspinaModel *model)
: Layout(model)
, m_proxy(new SampleProxy())
, m_sort (new SortFilter())
{
  m_proxy->setSourceModel(m_model);
  m_sort->setSourceModel(m_proxy.data());
  m_sort->setDynamicSortFilter(true);
}

//------------------------------------------------------------------------
ModelItem* SampleLayout::item(const QModelIndex& index) const
{
  return indexPtr(m_sort->mapToSource(index)); 
}

//------------------------------------------------------------------------
QModelIndex SampleLayout::index(ModelItem* item) const
{
  return m_sort->mapFromSource(m_proxy->mapFromSource(index(item)));
}

//------------------------------------------------------------------------
SegmentationList SampleLayout::deletedSegmentations(QModelIndexList selection)
{
  QSet<Segmentation *> toDelete;
  foreach(QModelIndex index, selection)
  {
    index = m_sort->mapToSource(index);
    ModelItem *item = indexPtr(index);
    switch (item->type())
    {
      case ModelItem::SEGMENTATION:
      {
        Segmentation *seg = dynamic_cast<Segmentation *>(item);
        Q_ASSERT(seg);
        toDelete << seg;
        break;
      }
      case ModelItem::SAMPLE:
      {
        int totalSeg  = m_proxy->numSegmentations(index, true);
        int directSeg = m_proxy->numSegmentations(index);

        if (totalSeg == 0)
          continue;

        Sample *sample = dynamic_cast<Sample *>(item);
        QMessageBox msgBox;
        msgBox.setText(SEGMENTATION_MESSAGE.arg(sample->id()));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        if (directSeg > 0)
        {
          if (directSeg < totalSeg)
          {
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesAll |  QMessageBox::No);
            msgBox.setText(MIXED_MESSAGE.arg(sample->id()));
          }
        } else
        {
          msgBox.setText(RECURSIVE_MESSAGE.arg(sample->id()));
          msgBox.setStandardButtons(QMessageBox::YesAll |  QMessageBox::No);
        }

        bool recursive = false;
        switch (msgBox.exec())
        {
          case QMessageBox::YesAll:
            recursive = true;
          case QMessageBox::Yes:
          {
            QModelIndexList subSegs = m_proxy->segmentations(index, recursive);
            foreach(QModelIndex subIndex, subSegs)
            {
              ModelItem *subItem = indexPtr(subIndex);
              Segmentation *seg = dynamic_cast<Segmentation *>(subItem);
              Q_ASSERT(seg);
              toDelete << seg;
            }
            break;
          }
          default:
            break;
        }
        break;
      }
          default:
            Q_ASSERT(false);
            break;
    }
  }

  return toDelete.toList();
}