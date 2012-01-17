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

  QPushButton *fakeLoad = new QPushButton(tr("Load Test Stack"));
  connect(fakeLoad,SIGNAL(clicked(bool)),
          this, SLOT(loadTestImage()));

  double gridSize[3] = {12.644, 12.644, 20};
  double ranges[6] = {0,798,0,797,0,272};
  ranges[1] = gridSize[0]*ranges[1];
  ranges[3] = gridSize[1]*ranges[3];
  ranges[5] = gridSize[2]*ranges[5];
  qDebug() << "New Default EspinaView";
  xyView = new SliceView(vtkPVSliceView::AXIAL);
  xyView->setCrossHairColors(blue, magenta);
  xyView->setRanges(ranges);
  xyView->setGridSize(gridSize);
  xyView->setFitToGrid(true);
  connect(xyView, SIGNAL(centerChanged(double,double,double)),
	  this, SLOT(setCenter(double,double,double)));
  this->setLayout(new QVBoxLayout());
  this->layout()->addWidget(xyView);
  this->layout()->addWidget(fakeLoad);
  this->layout()->setMargin(0);

  volDock = QSharedPointer<QDockWidget>(new QDockWidget(tr("3D"),parent));
  volDock->setObjectName("volDock");
  volView = new VolumeView(this);
  volDock->setWidget(volView);
  
  yzDock = QSharedPointer<QDockWidget>(new QDockWidget(tr("YZ"),parent));
  yzDock->setObjectName("yzDock");
  yzView = new SliceView(vtkPVSliceView::SAGITTAL);
  yzView->setCrossHairColors(blue, cyan);
  yzView->setRanges(ranges);
  yzView->setGridSize(gridSize);
  yzView->setFitToGrid(true);
  connect(yzView, SIGNAL(centerChanged(double,double,double)),
	  this, SLOT(setCenter(double,double,double)));
  yzDock->setWidget(yzView);

  xzDock = QSharedPointer<QDockWidget>(new QDockWidget(tr("XZ"),parent));
  xzDock->setObjectName("xzDock");
  xzView = new SliceView(vtkPVSliceView::CORONAL);
  xzView->setCrossHairColors(cyan, magenta);
  xzView->setRanges(ranges);
  xzView->setGridSize(gridSize);
  xzView->setFitToGrid(true);
  connect(xzView, SIGNAL(centerChanged(double,double,double)),
	  this, SLOT(setCenter(double,double,double)));
  xzDock->setWidget(xzView);

  parent->addDockWidget(Qt::RightDockWidgetArea, volDock.data());
  parent->addDockWidget(Qt::RightDockWidgetArea, yzDock.data());
  parent->addDockWidget(Qt::RightDockWidgetArea, xzDock.data());

  parent->setCentralWidget(this);
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
  xyView->setShowSegmentations(visibility);
  yzView->setShowSegmentations(visibility);
  xzView->setShowSegmentations(visibility);
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
void DefaultEspinaView::loadTestImage()
{

  qDebug() << this << ": Loading Test Image";
  pqServer *server = pqActiveObjects::instance().activeServer();
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();

  pqPipelineSource *img;
  if (first)
  {
    img = builder->createReader("sources","MetaImageReader",
// 				QStringList("/home/jpena/Stacks/paraPeque.pvd"),
// 				QStringList("/home/jpena/Stacks/AlzheimerE-reg-affine-012-510/AlzheimerE.pvd"),
// 				QStringList("/home/cbbp/Primeras Series Hechas/19-12wtSerie1/19-12wt Rigid-Body gaussian1-1.mhd"),
				QStringList("/home/cbbp/Primeras Series Hechas/19-12tgSerie4/19-12TG.mhd"),
					    server);
    first = false;
    xyView->addChannelRepresentation(img->getOutputPort(0));
    yzView->addChannelRepresentation(img->getOutputPort(0));
    xzView->addChannelRepresentation(img->getOutputPort(0));
  }else{
    //QDir segDir("/home/cbbp/Primeras Series Hechas/19-12wtSerie1/savegordo/");
    QDir segDir("/home/cbbp/Primeras Series Hechas/19-12tgSerie4/19-12TG-Serie4/");
    QString file;
    QStringList entries = segDir.entryList(QStringList("*.pvd"));
    int total = entries.size();
    int loaded = 1;
    foreach(file, entries)
    {
      img = builder->createReader("sources","PVDReader",
      QStringList(segDir.path()+"/"+file),
      //     QStringList("/home/jpena/Stacks/Peque/pequeFromSegmha/fa40f2b8d6b3bdd039fe2bd7086229eb61c9605e.pvd"),
      server);
      xyView->addSegmentationRepresentation(img->getOutputPort(0));
      yzView->addSegmentationRepresentation(img->getOutputPort(0));
      xzView->addSegmentationRepresentation(img->getOutputPort(0));
      volView->addSegmentationRepresentation(img->getOutputPort(0));

      emit statusMsg(QString("Loaded %1/%2 Segmentations.").arg(loaded++).arg(total));
      if (loaded > 40)
	break;
    }
  }
  xyView->forceRender();
  yzView->forceRender();
  xzView->forceRender();
  emit statusMsg(QString());
}