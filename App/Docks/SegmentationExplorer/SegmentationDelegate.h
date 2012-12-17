/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef SEGMENTATIONDELEGATE_H
#define SEGMENTATIONDELEGATE_H

#include <QStyledItemDelegate>

#include <Core/EspinaTypes.h>

class QUndoStack;

namespace EspINA
{
  class ViewManager;
  class SegmentationInspector;

  class SegmentationDelegate
  : public QStyledItemDelegate
  {
  public:
    explicit SegmentationDelegate(EspinaModelPtr model,
                                  QUndoStack *undoStack,
                                  ViewManager *vm);

    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
    virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

  private slots:
    void freeInspector(SegmentationInspector *inspector);

  private:
    EspinaModelPtr m_model;
    QUndoStack  *m_undoStack;
    ViewManager *m_viewManager;
    mutable QMap<SegmentationPtr, SegmentationInspector *> m_inspectors;
  };

} // namespace EspINA

#endif // SEGMENTATIONDELEGATE_H
