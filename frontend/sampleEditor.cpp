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


#include "sampleEditor.h"

#include "unitExplorer.h"
#include "products.h"
#include <data/modelItem.h>

#include <assert.h>

QWidget* SampleEditor::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  UnitExplorer *unitExplorer = new UnitExplorer();
  unitExplorer->setFocusPolicy(Qt::StrongFocus);
  return unitExplorer;
}

void SampleEditor::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  UnitExplorer *sed = dynamic_cast<UnitExplorer *>(editor);
  IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
  Sample *sample = dynamic_cast<Sample *>(item);
  assert(sample);
  sed->setWindowTitle(index.data(Qt::DisplayRole).toString());
  sed->setSample(sample);
}

void SampleEditor::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  QStyledItemDelegate::setModelData(editor, model, index);
}






