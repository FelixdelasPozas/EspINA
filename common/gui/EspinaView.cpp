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
{
  double cyan[3]    = {0, 1, 1};
  double blue[3]    = {0, 0, 1};
  double magenta[3] = {1, 0, 1};

  setObjectName("xyView");
  
  qDebug() << "New Default EspinaView";
  xyView = new SliceView(vtkPVSliceView::AXIAL);
  xyView->setCrossHairColors(blue, magenta);
  this->setLayout(new QVBoxLayout());
  this->layout()->addWidget(xyView);
  this->layout()->setMargin(0);

  volDock = QSharedPointer<QDockWidget>(new QDockWidget(tr("3D"),parent));
  volDock->setObjectName("volDock");
  VolumeView *volView = new VolumeView(this);
  volDock->setWidget(volView);
  
  yzDock = QSharedPointer<QDockWidget>(new QDockWidget(tr("YZ"),parent));
  yzDock->setObjectName("yzDock");
  yzView = new SliceView(vtkPVSliceView::SAGITTAL);
  yzView->setCrossHairColors(blue, cyan);
  yzDock->setWidget(yzView);

  xzDock = QSharedPointer<QDockWidget>(new QDockWidget(tr("XZ"),parent));
  xzDock->setObjectName("xzDock");
  xzView = new SliceView(vtkPVSliceView::CORONAL);
  xzView->setCrossHairColors(cyan, magenta);
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
