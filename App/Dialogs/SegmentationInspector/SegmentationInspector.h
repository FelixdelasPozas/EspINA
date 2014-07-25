/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_SEGMENTATION_INSPECTOR_H
#define EPSINA_SEGMENTATION_INSPECTOR_H

// ESPINA
#include "ui_SegmentationInspector.h"
#include <Docks/SegmentationExplorer/SegmentationExplorerLayout.h>

// Qt
#include <QDialog>
#include <QScrollArea>
#include <QSortFilterProxyModel>

class QUndoStack;

namespace ESPINA
{
  class TabularReport;
  class View3D;

  class SegmentationInspector
  : public QWidget
  , public Ui::SegmentationInspector
  {
    Q_OBJECT
  public:
    SegmentationInspector(SegmentationAdapterList  segmentation,
                          ModelAdapterSPtr         model,
                          ModelFactorySPtr         factory,
                          ViewManagerSPtr          viewManager,
                          QUndoStack*              undoStack,
                          QWidget*                 parent = nullptr,
                          Qt::WindowFlags          flags  = 0);

    virtual ~SegmentationInspector();

    virtual void addSegmentation(SegmentationAdapterPtr segmentation);

    virtual void removeSegmentation(SegmentationAdapterPtr segmentation);

    virtual void addChannel(ChannelAdapterPtr channel);
    virtual void removeChannel(ChannelAdapterPtr channel);

    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);

  public slots:
    void updateScene(ItemAdapterPtr item);
    void updateSelection(SelectionSPtr selection);

  signals:
    void inspectorClosed(SegmentationInspector *);

  protected:
    virtual void showEvent(QShowEvent *event);

  protected:
    virtual void closeEvent(QCloseEvent *e);

  private:

    // helpher methods
    void generateWindowTitle();

    ModelAdapterSPtr m_model;
    ViewManagerSPtr  m_viewManager;
    QUndoStack*      m_undoStack;

    SegmentationAdapterList m_segmentations;
    ChannelAdapterList      m_channels;

    View3D*        m_view;
    QScrollArea*   m_filterArea;
    TabularReport* m_tabularReport;
  };

} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_INSPECTOR_H
