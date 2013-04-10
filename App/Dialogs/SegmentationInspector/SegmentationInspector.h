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

// EspINA
#include "ui_SegmentationInspector.h"
#include <Core/Model/EspinaModel.h>
#include <GUI/ViewManager.h>
#include <Docks/SegmentationExplorer/SegmentationExplorerLayout.h>

// Qt
#include <QDialog>
#include <QScrollArea>
#include <QSortFilterProxyModel>

class QUndoStack;

namespace EspINA
{

  class TabularReport;
  class VolumeView;

  class SegmentationInspector
  : public QWidget
  , public Ui::SegmentationInspector
  {
    Q_OBJECT
  public:
    SegmentationInspector(SegmentationList seg,
                          EspinaModel     *model,
                          QUndoStack      *undoStack,
                          ViewManager     *vm,
                          QWidget         *parent = 0,
                          Qt::WindowFlags  flags  = 0);
    virtual ~SegmentationInspector();

    virtual void addSegmentation(SegmentationPtr segmentation);
    virtual void removeSegmentation(SegmentationPtr segmentation);

    virtual void addChannel(ChannelPtr channel);
    virtual void removeChannel(ChannelPtr channel);

    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);

  public slots:
    void updateScene(ModelItemPtr);
    void updateSelection(ViewManager::Selection selection);

  signals:
    void inspectorClosed(SegmentationInspector *);

  protected:
    virtual void showEvent(QShowEvent *event);

  protected:
    virtual void closeEvent(QCloseEvent *e);

  private:

    // helpher methods
    void generateWindowTitle();

    EspinaModel *m_model;
    QUndoStack  *m_undoStack;
    ViewManager *m_viewManager;

    SegmentationList m_segmentations;
    ChannelList      m_channels;

    TabularReport *m_tabularReport;

    VolumeView *m_view;
    QScrollArea *m_filterArea;
  };

} // namespace EspINA

#endif // SEGMENTATIONINSPECTOR_H
