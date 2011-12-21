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


#include "SegmentationExplorer.h"

#include <common/model/EspINA.h>
#include <iostream>
#include <cstdio>

class SegmentationExplorer::GUI : public QWidget, public Ui::SegmentationExplorer
{
public:
  GUI(){setupUi(this);}
};

class State
{
public:
  void deleteSegmentation(){}
};


SegmentationExplorer::SegmentationExplorer(QSharedPointer< EspINA> model, QWidget* parent)
: EspinaDockWidget(parent)
, m_baseModel(model)
{
  setWindowTitle(tr("Segmentation Explorer"));
  m_gui = new GUI();
  m_gui->view->setModel(m_baseModel.data());

  connect(m_gui->deleteSegmentation, SIGNAL(clicked(bool)),
          this, SLOT(deleteSegmentation()));

  setWidget(m_gui);
}

SegmentationExplorer::~SegmentationExplorer()
{
}

void SegmentationExplorer::deleteSegmentation()
{
}
