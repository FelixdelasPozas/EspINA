/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "DefaultEspinaView.h"

#include <QDebug>

#include "common/gui/SliceView.h"
#include "common/gui/VolumeView.h"
#include "common/model/ModelItem.h"
#include "common/model/Channel.h"
#include "common/processing/pqData.h"
#include "common/model/Segmentation.h"
#include "common/plugins/EspinaWidgets/RectangularSelection.h"

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
#include <QApplication>

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
  connect(xyView, SIGNAL(selectedFromSlice(int,vtkPVSliceView::VIEW_PLANE)),
	  this, SLOT(selectFromSlice(int,vtkPVSliceView::VIEW_PLANE)));
  connect(xyView, SIGNAL(selectedToSlice(int,vtkPVSliceView::VIEW_PLANE)),
	  this, SLOT(selectToSlice(int,vtkPVSliceView::VIEW_PLANE)));
  this->setLayout(new QVBoxLayout());
  this->layout()->addWidget(xyView);
  this->layout()->setMargin(0);

  volDock = new QDockWidget(tr("3D"),parent);
  volDock->setObjectName("volDock");
  volView = new VolumeView(this);
  volDock->setWidget(volView);
  
  yzDock = new QDockWidget(tr("YZ"),parent);
  yzDock->setObjectName("yzDock");
  yzView = new SliceView(vtkPVSliceView::SAGITTAL);
  yzView->setCrossHairColors(blue, cyan);
  yzView->setFitToGrid(true);
  connect(yzView, SIGNAL(centerChanged(double,double,double)),
	  this, SLOT(setCenter(double,double,double)));
  connect(yzView, SIGNAL(selectedFromSlice(int,vtkPVSliceView::VIEW_PLANE)),
	  this, SLOT(selectFromSlice(int,vtkPVSliceView::VIEW_PLANE)));
  connect(yzView, SIGNAL(selectedToSlice(int,vtkPVSliceView::VIEW_PLANE)),
	  this, SLOT(selectToSlice(int,vtkPVSliceView::VIEW_PLANE)));
  yzDock->setWidget(yzView);

  xzDock = new QDockWidget(tr("XZ"),parent);
  xzDock->setObjectName("xzDock");
  xzView = new SliceView(vtkPVSliceView::CORONAL);
  xzView->setCrossHairColors(cyan, magenta);
  xzView->setFitToGrid(true);
  connect(xzView, SIGNAL(centerChanged(double,double,double)),
	  this, SLOT(setCenter(double,double,double)));
  connect(xzView, SIGNAL(selectedFromSlice(int,vtkPVSliceView::VIEW_PLANE)),
	  this, SLOT(selectFromSlice(int,vtkPVSliceView::VIEW_PLANE)));
  connect(xzView, SIGNAL(selectedToSlice(int,vtkPVSliceView::VIEW_PLANE)),
	  this, SLOT(selectToSlice(int,vtkPVSliceView::VIEW_PLANE)));
  xzDock->setWidget(xzView);

  parent->addDockWidget(Qt::RightDockWidgetArea, volDock);
  parent->addDockWidget(Qt::RightDockWidgetArea, yzDock);
  parent->addDockWidget(Qt::RightDockWidgetArea, xzDock);

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
void DefaultEspinaView::resetCamera()
{
  xyView->resetCamera();
  yzView->resetCamera();
  xzView->resetCamera();
}

//----------------------------------------------------------------------------
void DefaultEspinaView::gridSize(double size[3])
{
  memcpy(size, m_gridSize, 3*sizeof(double));
}

//----------------------------------------------------------------------------
void DefaultEspinaView::setGridSize(double size[3])
{
  xyView->setGridSize(size);
  yzView->setGridSize(size);
  xzView->setGridSize(size);
  memcpy(m_gridSize, size, 3*sizeof(double));
}


//----------------------------------------------------------------------------
void DefaultEspinaView::addWidget(EspinaWidget* widget)
{
  xyView->addWidget(widget->createSliceWidget(vtkPVSliceView::AXIAL));
  yzView->addWidget(widget->createSliceWidget(vtkPVSliceView::SAGITTAL));
  xzView->addWidget(widget->createSliceWidget(vtkPVSliceView::CORONAL));
  volView->addWidget(widget->createWidget());
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
void DefaultEspinaView::setSliceSelectors(SliceView::SliceSelectors selectors)
{
  xyView->setSliceSelectors(selectors);
  yzView->setSliceSelectors(selectors);
  xzView->setSliceSelectors(selectors);
}


//-----------------------------------------------------------------------------
void DefaultEspinaView::addSegmentation(Segmentation* seg)
{
  xyView->addSegmentationRepresentation(seg);
  yzView->addSegmentationRepresentation(seg);
  xzView->addSegmentationRepresentation(seg);
  volView->addSegmentationRepresentation(seg);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::removeSegmentation(Segmentation* seg)
{
  xyView->removeSegmentationRepresentation(seg);
  yzView->removeSegmentationRepresentation(seg);
  xzView->removeSegmentationRepresentation(seg);
  volView->removeSegmentationRepresentation(seg);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::rowsInserted(const QModelIndex& parent, int start, int end)
{
  if (!parent.isValid())
    return;

  for(int child = start; child <= end; child++)
  {
    QModelIndex index = parent.child(child, 0);
    ModelItem *item  = indexPtr(index);
    switch (item->type())
    {
      case ModelItem::CHANNEL:
      {
	QApplication::setOverrideCursor(Qt::WaitCursor);
	Q_ASSERT(start == end);// Only 1-row-at-a-time insertions are allowed
	Channel *channel = dynamic_cast<Channel *>(item);
	//       item.dynamicCast<ChannelPtr>();
// 	qDebug() << "Add Channel:" << channel->data(Qt::DisplayRole).toString();
	
	//BEGIN Only at sample LOD
	double spacing[3];
	channel->spacing(spacing);
	setGridSize(spacing);
	double bounds[6];
	channel->bounds(bounds);
	xyView->setRanges(bounds);
	yzView->setRanges(bounds);
	xzView->setRanges(bounds);
	//END
	xyView->addChannelRepresentation(channel);
	yzView->addChannelRepresentation(channel);
	xzView->addChannelRepresentation(channel);
	QApplication::restoreOverrideCursor();
	resetCamera();
	break;
      }
      case ModelItem::SEGMENTATION:
      {
	Segmentation *seg = dynamic_cast<Segmentation *>(item);
// 	qDebug() << "Add Segmentation:" << seg->data(Qt::DisplayRole).toString();
	addSegmentation(seg);
	break;
      }
      default:
	break;
    };
  }
  xyView->forceRender();
  yzView->forceRender();
  xzView->forceRender();
  volView->forceRender();
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
  if (!parent.isValid())
    return;

  qDebug() << parent.data(Qt::DisplayRole).toString();
  for(int child = start; child <= end; child++)
  {
    QModelIndex index = parent.child(child, 0);
    ModelItem *item  = indexPtr(index);
    switch (item->type())
    {
      case ModelItem::CHANNEL:
      {
	Channel *channel = dynamic_cast<Channel *>(item);
// 	qDebug() << "Remove Channel:" << channel->data(Qt::DisplayRole).toString();
	xyView->removeChannelRepresentation(channel);
	yzView->removeChannelRepresentation(channel);
	xzView->removeChannelRepresentation(channel);

	double emptyBounds[6] = {0,0,0,0,0,0};
	xyView->setRanges(emptyBounds);
	yzView->setRanges(emptyBounds);
	xzView->setRanges(emptyBounds);
	break;
      }
      case ModelItem::SEGMENTATION:
      {
	Segmentation *seg = dynamic_cast<Segmentation *>(item);
// 	qDebug() << "Remove Segmentation:" << seg->data(Qt::DisplayRole).toString();
	removeSegmentation(seg);
	break;
      }
      default:
	break;
    };
  }
  xyView->forceRender();
  yzView->forceRender();
  xzView->forceRender();
  volView->forceRender();
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
  if (!topLeft.isValid() || !topLeft.parent().isValid())
    return;

  ModelItem *item = indexPtr(topLeft);
  if (ModelItem::SEGMENTATION == item->type())
  {
    Segmentation *seg = dynamic_cast<Segmentation *>(item);
    xyView->updateSegmentationRepresentation(seg);
    yzView->updateSegmentationRepresentation(seg);
    xzView->updateSegmentationRepresentation(seg);
  }
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

//-----------------------------------------------------------------------------
void DefaultEspinaView::selectFromSlice(int slice, vtkPVSliceView::VIEW_PLANE plane)
{
  emit selectedFromSlice(slice, plane);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::selectToSlice(int slice, vtkPVSliceView::VIEW_PLANE plane)
{
  emit selectedToSlice(slice, plane);
}