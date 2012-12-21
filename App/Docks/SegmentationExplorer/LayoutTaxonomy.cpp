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

#include <Core/Model/Segmentation.h>
#include <Core/Model/Taxonomy.h>

#include <QMessageBox>

using namespace EspINA;

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
TaxonomyLayout::TaxonomyLayout(EspinaModelSPtr model)
: Layout(model)
, m_proxy(new TaxonomyProxy())
, m_sort (new SortFilter())
{
  m_proxy->setSourceModel(m_model);
  m_sort->setSourceModel(m_proxy.data());
  m_sort->setDynamicSortFilter(true);
}

//------------------------------------------------------------------------
SegmentationList TaxonomyLayout::deletedSegmentations(QModelIndexList selection)
{
  QSet<SegmentationPtr> toDelete;
  foreach(QModelIndex index, selection)
  {
    index = m_sort->mapToSource(index);
    ModelItemPtr item = indexPtr(index);
    switch (item->type())
    {
      case EspINA::SEGMENTATION:
      {
        SegmentationPtr seg = segmentationPtr(item);
        toDelete << seg;
        break;
      }
      case EspINA::TAXONOMY:
      {
        int totalSeg = m_proxy->numSegmentations(index, true);
        int directSeg = m_proxy->numSegmentations(index);

        if (totalSeg == 0)
          continue;

        TaxonomyElementPtr taxonmy = taxonomyElementPtr(item);
        QMessageBox msgBox;
        msgBox.setText(SEGMENTATION_MESSAGE.arg(taxonmy->qualifiedName()));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        if (directSeg > 0)
        {
          if (directSeg < totalSeg)
          {
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesAll |  QMessageBox::No);
            msgBox.setText(MIXED_MESSAGE.arg(taxonmy->qualifiedName()));
          }
        } else
        {
          msgBox.setText(RECURSIVE_MESSAGE.arg(taxonmy->qualifiedName()));
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
              ModelItemPtr subItem = indexPtr(subIndex);
              SegmentationPtr seg = segmentationPtr(subItem);
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