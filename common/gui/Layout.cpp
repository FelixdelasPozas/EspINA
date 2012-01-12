/*
    <one line to give the program's name and a brief idea of what it does.>
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


#include "Layout.h"

#include <QDebug>

#include "gui/SliceView.h"
#include "gui/VolumeView.h"

#include <QDockWidget>
#include <QMainWindow>
#include <QVBoxLayout>

//----------------------------------------------------------------------------
Layout::Layout(QMainWindow* parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
}

//----------------------------------------------------------------------------
DefaultLayout::DefaultLayout(QMainWindow* parent, Qt::WindowFlags f)
: Layout(parent, f)
{
  qDebug() << "New Default Layout";
  xyView = new SliceView();
  xyView = new SliceView(vtkPVEspinaView::CORONAL);
  xyView = new SliceView(vtkPVEspinaView::AXIAL);
  this->setLayout(new QVBoxLayout());
  this->layout()->addWidget(xyView);

  volDock = QSharedPointer<QDockWidget>(new QDockWidget(tr("3D"),parent));
  volDock->setObjectName("volDock");
  VolumeView *volView = new VolumeView(this);
  volDock->setWidget(volView);
  
  yzDock = QSharedPointer<QDockWidget>(new QDockWidget(tr("YZ"),parent));
  yzDock->setObjectName("yzDock");
  yzView = new SliceView(vtkPVEspinaView::SAGITTAL);
  yzDock->setWidget(yzView);

  xzDock = QSharedPointer<QDockWidget>(new QDockWidget(tr("XZ"),parent));
  xzDock->setObjectName("xzDock");
  xzView = new SliceView(vtkPVEspinaView::CORONAL);
  xzDock->setWidget(xzView);

  parent->addDockWidget(Qt::RightDockWidgetArea, volDock.data());
  parent->addDockWidget(Qt::RightDockWidgetArea, yzDock.data());
  parent->addDockWidget(Qt::RightDockWidgetArea, xzDock.data());

  parent->setCentralWidget(this);
}

//----------------------------------------------------------------------------
void DefaultLayout::setShowSegmentations(bool visibility)
{
  xyView->setShowSegmentations(visibility);
  yzView->setShowSegmentations(visibility);
  xzView->setShowSegmentations(visibility);
}
