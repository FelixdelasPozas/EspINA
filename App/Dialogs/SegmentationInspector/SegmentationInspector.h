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


#ifndef SEGMENTATIONINSPECTOR_H
#define SEGMENTATIONINSPECTOR_H

#include <QDialog>
#include "ui_SegmentationInspector.h"

#include <Core/Model/EspinaModel.h>
#include <Core/Model/Proxies/InformationProxy.h>

#include <QSortFilterProxyModel>

class QUndoStack;
class VolumeView;
class ViewManager;
class Segmentation;

class SegmentationInspector
: public QWidget
, public Ui::SegmentationInspector
{
  Q_OBJECT
public:
  SegmentationInspector(Segmentation *seg,
                        EspinaModel *model,
                        QUndoStack *undoStack,
                        ViewManager *vm,
                        QWidget* parent = 0,
                        Qt::WindowFlags f = 0);
  virtual ~SegmentationInspector();

public slots:
  void updateScene();

signals:
  void inspectorClosed(SegmentationInspector *);

protected:
  virtual void closeEvent(QCloseEvent *e);

private:
  QUndoStack   *m_undoStack;
  ViewManager  *m_viewManager;
  Segmentation *m_seg;
  EspinaModel  *m_model;
  VolumeView   *m_view;
  QSharedPointer<InformationProxy> m_info;
  QSharedPointer<QSortFilterProxyModel> m_sort;
};

#endif // SEGMENTATIONINSPECTOR_H
