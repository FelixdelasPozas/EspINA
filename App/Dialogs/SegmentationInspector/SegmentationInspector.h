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

namespace EspINA
{
  class VolumeView;
  class ViewManager;

  class SegmentationInspector
  : public QWidget
  , public Ui::SegmentationInspector
  {
    Q_OBJECT
  public:
    SegmentationInspector(SegmentationPtr seg,
                          EspinaModelPtr  model,
                          QUndoStack     *undoStack,
                          ViewManager    *vm,
                          QWidget        *parent = 0,
                          Qt::WindowFlags flags  = 0);
    virtual ~SegmentationInspector();

  public slots:
    void updateScene();

  signals:
    void inspectorClosed(SegmentationInspector *);

  protected:
    virtual void closeEvent(QCloseEvent *e);

  private:
    EspinaModelPtr m_model;
    QUndoStack    *m_undoStack;
    ViewManager   *m_viewManager;

    SegmentationPtr m_seg;

    QSharedPointer<InformationProxy>      m_info;
    QSharedPointer<QSortFilterProxyModel> m_sort;

    VolumeView *m_view;
  };

} // namespace EspINA

#endif // SEGMENTATIONINSPECTOR_H
