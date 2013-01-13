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


#include "LayoutComposition.h"

#include <Core/Model/Segmentation.h>
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
, m_proxy(new CompositionProxy())
, m_sort (new SortFilter())
{
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
SegmentationList CompositionLayout::deletedSegmentations(QModelIndexList selection)
{
  QSet<SegmentationPtr> toDelete;
  foreach(QModelIndex sortIndex, selection)
  {
    bool recursive = false;
    if (m_sort->rowCount(sortIndex) > 0)
    {
      QMessageBox msgBox;
      msgBox.setText(SEGMENTATION_MESSAGE.arg(sortIndex.data().toString()));
      msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
      msgBox.setDefaultButton(QMessageBox::No);

      recursive = msgBox.exec() == QMessageBox::Yes;
    }

    ModelItemPtr    selectedItem = item(sortIndex);
    SegmentationPtr seg          = segmentationPtr(selectedItem);
    Q_ASSERT(seg);
    toDelete << seg;

    if (recursive)
    {
      foreach(QModelIndex subIndex, indices(sortIndex, true))
      {
        ModelItemPtr     subItem = item(subIndex);
        SegmentationPtr  seg     = segmentationPtr(subItem);
        Q_ASSERT(seg);
        toDelete << seg;
      }
    }
  }

  return toDelete.toList();
}