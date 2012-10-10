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


#include "SegmentationDelegate.h"

#include <QDebug>

#include <model/ModelItem.h>
#include <model/Segmentation.h>
#include "SegmentationInspector.h"

//------------------------------------------------------------------------
SegmentationDelegate::SegmentationDelegate(EspinaModel *model,
                                           ViewManager *vm)
: QStyledItemDelegate()
, m_model(model)
, m_viewManager(vm)
{
}

//------------------------------------------------------------------------
QWidget* SegmentationDelegate::createEditor(QWidget* parent,
                                            const QStyleOptionViewItem& option,
                                            const QModelIndex& index) const
{
  ModelItem *item =  indexPtr(index);
  Q_ASSERT(item);

  if (ModelItem::SEGMENTATION == item->type())
  {
    Segmentation *seg = dynamic_cast<Segmentation *>(item);
    if (!m_inspectors.contains(seg))
    {
      m_inspectors[seg] = new SegmentationInspector(seg, m_model, m_viewManager);
      connect(m_inspectors[seg], SIGNAL(inspectorClosed(SegmentationInspector*)),
              this, SLOT(freeInspector(SegmentationInspector*)));
    }

    m_inspectors[seg]->setFocusPolicy(Qt::StrongFocus);
    return m_inspectors[seg];
  } else
    return QStyledItemDelegate::createEditor(parent, option, index);

}

//------------------------------------------------------------------------
void SegmentationDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  QStyledItemDelegate::setEditorData(editor, index);
}

//------------------------------------------------------------------------
void SegmentationDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  QStyledItemDelegate::setModelData(editor, model, index);
}

//------------------------------------------------------------------------
void SegmentationDelegate::freeInspector(SegmentationInspector* inspector)
{
  Segmentation *seg = m_inspectors.key(inspector);
  m_inspectors.remove(seg);
  delete inspector;
}

