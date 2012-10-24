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
#include "VolumeOfInterest.h"

#include "RectangularVOI.h"

// EspINA
#include "common/model/Channel.h"
#include "common/gui/ActionSelector.h"
#include "common/gui/ViewManager.h"
#include "common/tools/PickableItem.h"

// Qt
#include <QDebug>

//-----------------------------------------------------------------------------
VolumeOfInterest::VolumeOfInterest(ViewManager *vm, QWidget *parent)
: QToolBar   (parent)
, m_viewManager(vm)
, m_voiSelector(new ActionSelector(this))
{
  setObjectName ("VolumeOfInterest");
  setWindowTitle("Volume Of Interest");

  buildVOIs();

  addAction(m_voiSelector);

  connect(m_voiSelector, SIGNAL(triggered(QAction*)),
          this, SLOT(changeVOI(QAction*)));
  connect(m_voiSelector, SIGNAL(actionCanceled()),
          this, SLOT(cancelVOI()));
}

//-----------------------------------------------------------------------------
VolumeOfInterest::~VolumeOfInterest()
{
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::buildVOIs()
{
  //   IVOI *voi;
  QAction *action;

  // Exact Pixel Selector
  action = new QAction(QIcon(":roi.svg"), tr("Volume Of Interest"), m_voiSelector);

  m_voiSelector->addAction(action);
  m_vois[action] = new RectangularVOI(m_viewManager);
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::changeVOI(QAction* action)
{
  Q_ASSERT(m_vois.contains(action));
  m_viewManager->setVOI(m_vois[action]);
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::cancelVOI()
{
  m_viewManager->setVOI(NULL);
}
