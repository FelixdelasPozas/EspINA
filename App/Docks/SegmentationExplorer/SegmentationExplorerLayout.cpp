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


#include "SegmentationExplorerLayout.h"

#include <Core/Model/Segmentation.h>

using namespace EspINA;

const QString SegmentationExplorer::Layout::SEGMENTATION_MESSAGE
  = QObject::tr("Delete %1's segmentations");
const QString SegmentationExplorer::Layout::RECURSIVE_MESSAGE
  = QObject::tr("Delete %1's segmentations. "
                "If you want to delete recursively select Yes To All");
const QString SegmentationExplorer::Layout::MIXED_MESSAGE
  = QObject::tr("Delete recursively %1's segmentations");

//------------------------------------------------------------------------
QModelIndexList SegmentationExplorer::Layout::indices(const QModelIndex& index, bool recursive)
{
  QModelIndexList res;

  Q_ASSERT(model() == index.model());
  for(int r = 0; r < model()->rowCount(index); r++)
  {
    QModelIndex child = index.child(r, 0);
    res << child;
    if (recursive)
      res << indices(child, recursive);
  }

  return res;
}

//------------------------------------------------------------------------
bool EspINA::sortSegmentationLessThan(ModelItemPtr left, ModelItemPtr right)
{
  SegmentationPtr leftSeg  = segmentationPtr(left);
  SegmentationPtr rightSeg = segmentationPtr(right);

  if (leftSeg->number() == rightSeg->number())
    return left ->data(Qt::ToolTipRole).toString() <
           right->data(Qt::ToolTipRole).toString();
  else
    return leftSeg->number() < rightSeg->number();
}
