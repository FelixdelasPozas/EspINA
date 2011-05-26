/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#include "segmentationEditor.h"

#include "SegmentationExplorer.h"
#include <data/modelItem.h>
#include <products.h>

#include <QDebug>

QWidget* SegmentationEditor::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
  Sample *sample = dynamic_cast<Sample *>(item);
  if (sample)
  {
    return QStyledItemDelegate::createEditor(parent, option, index);
  }
  
  //NOTE: Maybe use a generic product explorer...
  Segmentation *seg = dynamic_cast<Segmentation *>(item);
  if (seg)
  {
    SegmentationExplorer *explorer = new SegmentationExplorer(seg);
    explorer->setFocusPolicy(Qt::StrongFocus);
    return explorer;
  }

  return QStyledItemDelegate::createEditor(parent, option, index);

}

void SegmentationEditor::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  QStyledItemDelegate::setEditorData(editor, index);
}

void SegmentationEditor::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  QStyledItemDelegate::setModelData(editor, model, index);
}

