/*
    <one line to give the program's name an a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "EspinaView.h"

#include <QDebug>

#include "gui/SliceView.h"
#include "gui/VolumeView.h"
#include "model/IModelItem.h"
#include "model/Channel.h"
#include "processing/pqData.h"

#include <QDockWidget>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QSettings>

#include <pqServer.h>
#include <pqObjectBuilder.h>
#include <pqActiveObjects.h>
#include <pqApplicationCore.h>
#include <pqPipelineSource.h>
#include <QDir>
#include <QMenu>

//----------------------------------------------------------------------------
EspinaView::EspinaView( QMainWindow* parent, const QString activity)
: QAbstractItemView(parent)
, m_activity(activity)
, m_window(parent)
{
}

//----------------------------------------------------------------------------
DefaultEspinaView::DefaultEspinaView(QMainWindow* parent, const QString activity)
: EspinaView(parent, activity)
, first(true)
{
  double cyan[3]    = {0, 1, 1};
  double blue[3]    = {0, 0, 1};
  double magenta[3] = {1, 0, 1};

  setObjectName("xyView");

  double ranges[6] = {0,0,0,0,0,0};
  qDebug() << "New Default EspinaView";
  xyView = new SliceView(vtkPVSliceView::AXIAL);
  xyView->setCrossHairColors(blue, magenta);
//   xyView->setRanges(ranges);
  xyView->setFitToGrid(true);
  connect(xyView, SIGNAL(centerChanged(double,double,double)),
	  this, SLOT(setCenter(double,double,double)));
  this->setLayout(new QVBoxLayout());
  this->layout()->addWidget(xyView);
  this->layout()->setMargin(0);

  volDock = QSharedPointer<QDockWidget>(new QDockWidget(tr("3D"),parent));
  volDock->setObjectName("volDock");
  volView = new VolumeView(this);
  volDock->setWidget(volView);
  
  yzDock = QSharedPointer<QDockWidget>(new QDockWidget(tr("YZ"),parent));
  yzDock->setObjectName("yzDock");
  yzView = new SliceView(vtkPVSliceView::SAGITTAL);
  yzView->setCrossHairColors(blue, cyan);
  yzView->setFitToGrid(true);
  connect(yzView, SIGNAL(centerChanged(double,double,double)),
	  this, SLOT(setCenter(double,double,double)));
  yzDock->setWidget(yzView);

  xzDock = QSharedPointer<QDockWidget>(new QDockWidget(tr("XZ"),parent));
  xzDock->setObjectName("xzDock");
  xzView = new SliceView(vtkPVSliceView::CORONAL);
  xzView->setCrossHairColors(cyan, magenta);
  xzView->setFitToGrid(true);
  connect(xzView, SIGNAL(centerChanged(double,double,double)),
	  this, SLOT(setCenter(double,double,double)));
  xzDock->setWidget(xzView);

  parent->addDockWidget(Qt::RightDockWidgetArea, volDock.data());
  parent->addDockWidget(Qt::RightDockWidgetArea, yzDock.data());
  parent->addDockWidget(Qt::RightDockWidgetArea, xzDock.data());

  parent->setCentralWidget(this);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::createViewMenu(QMenu* menu)
{
  menu->addAction(volDock->toggleViewAction());
  menu->addAction(yzDock->toggleViewAction());
  menu->addAction(xzDock->toggleViewAction());
  menu->addSeparator();
  
  QAction *showSegmentations = new QAction(tr("Show Segmentations"),menu);
  menu->addAction(showSegmentations);
  QAction *showRuler = new QAction(tr("Show Ruler"),menu);
  showRuler->setCheckable(true);
  showRuler->setChecked(true);
  menu->addAction(showRuler);
  connect(showRuler, SIGNAL(toggled(bool)),
	  this, SLOT(setRulerVisibility(bool)));
  QAction *fitToSlices = new QAction(tr("Fit To Slices"),menu);
  fitToSlices->setCheckable(true);
  fitToSlices->setChecked(true);
  menu->addAction(fitToSlices);
  connect(fitToSlices, SIGNAL(toggled(bool)),
	  this, SLOT(setFitToSlices(bool)));
}

//----------------------------------------------------------------------------
void DefaultEspinaView::restoreLayout()
{
  qDebug() << "Restore " << m_activity << volDock->objectName();
  QSettings settings("CeSViMa", "EspINA");

  m_window->restoreState(settings.value(m_activity+"/state").toByteArray());
  m_window->restoreGeometry(settings.value(m_activity+"/geometry").toByteArray());
}

//----------------------------------------------------------------------------
QSize DefaultEspinaView::sizeHint() const
{
  return QSize(500,500);
}


//----------------------------------------------------------------------------
void DefaultEspinaView::saveLayout()
{
  qDebug() << "Save " << m_activity << volDock->objectName();
  QSettings settings("CeSViMa", "EspINA");

  settings.setValue(m_activity+"/state", m_window->saveState());
  settings.setValue(m_activity+"/geometry", m_window->saveGeometry());
}

//----------------------------------------------------------------------------
void DefaultEspinaView::setShowSegmentations(bool visibility)
{
  xyView->setSegmentationVisibility(visibility);
  yzView->setSegmentationVisibility(visibility);
  xzView->setSegmentationVisibility(visibility);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::setCenter(double x, double y, double z)
{
//   qDebug() << "Espina View Updating centers";
  double center[3] = {x,y,z};
  xyView->centerViewOn(center);
  yzView->centerViewOn(center);
  xzView->centerViewOn(center);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::rowsInserted(const QModelIndex& parent, int start, int end)
{
  if (!parent.isValid())
    return;

  QModelIndex index = parent.child(start, 0);
  IModelItem *item  = static_cast<IModelItem *>(index.internalPointer());
  switch (item->type())
  {
    case IModelItem::CHANNEL:
    {
      Q_ASSERT(start == end);// Only 1-row-at-a-time insertions are allowed
      Channel *channel = dynamic_cast<Channel *>(item);
      qDebug() << "Add Channel:" << channel->data(Qt::DisplayRole).toString();
      xyView->addChannelRepresentation(channel);
      yzView->addChannelRepresentation(channel);
      xzView->addChannelRepresentation(channel);
      break;
    }
    default:
      break;
  };
  xyView->forceRender();
  yzView->forceRender();
  xzView->forceRender();
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
  if (!parent.isValid())
    return;

  qDebug() << parent.data(Qt::DisplayRole).toString();
  QModelIndex index = parent.child(start, 0);
  IModelItem *item  = static_cast<IModelItem *>(index.internalPointer());
  switch (item->type())
  {
    case IModelItem::CHANNEL:
    {
      Q_ASSERT(start == end);// Only 1-row-at-a-time insertions are allowed
      Channel *channel = dynamic_cast<Channel *>(item);
      qDebug() << "Remove Channel:" << channel->data(Qt::DisplayRole).toString();
      xyView->removeChannelRepresentation(channel);
      yzView->removeChannelRepresentation(channel);
      xzView->removeChannelRepresentation(channel);

      double emptyBounds[6] = {0,0,0,0,0,0};
      xyView->setRanges(emptyBounds);
      yzView->setRanges(emptyBounds);
      xzView->setRanges(emptyBounds);
      break;
    }
    default:
      break;
  };
  xyView->forceRender();
  yzView->forceRender();
  xzView->forceRender();
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::setFitToSlices(bool fit )
{
  xyView->setFitToGrid(fit);
  yzView->setFitToGrid(fit);
  xzView->setFitToGrid(fit);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::setRulerVisibility(bool visible)
{
  xyView->setRulerVisibility(visible);
  yzView->setRulerVisibility(visible);
  xzView->setRulerVisibility(visible);
}
