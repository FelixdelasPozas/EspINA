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


#include "CountingRegion.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QWidgetAction>
#include <QPushButton>

#include "CountingRegionExtension.h"
#include "espINAFactory.h"

CountingRegion::CountingRegion(QWidget * parent): QDockWidget(parent)
{
  this->setWindowTitle(tr("Counting Brick"));
  QWidget *dockWidget = new QWidget();
  setupUi(dockWidget);
  setWidget(dockWidget);
  
  connect(createRegion,SIGNAL(clicked()),
    this, SLOT(createNewRegion()));
  
  CountingRegionExtension ext;
  EspINAFactory::instance()->addSegmentationExtension(&ext);
  
  regions->setModel(&m_regionsModel);
}

void CountingRegion::createNewRegion()
{
  regions->addItem("Hola");
}