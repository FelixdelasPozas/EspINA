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
#include <Tools/VOI/RectangularVOI.h>

// EspINA
#include <GUI/QtWidget/ActionSelector.h>

// Qt
#include <QDebug>

using namespace EspINA;

//-----------------------------------------------------------------------------
VolumeOfInterest::VolumeOfInterest(EspinaModelSPtr model,
                                   ViewManager   *viewManager,
                                   QWidget       *parent)
: IToolBar     (parent)
, m_model      (model)
, m_viewManager(viewManager)
, m_voiSelector(new ActionSelector(this))
{
  setObjectName("VolumeOfInterest");

  setWindowTitle(tr("Volume Of Interest"));

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
void VolumeOfInterest::initToolBar(EspinaModelSPtr model,
                                   QUndoStack     *undoStack,
                                   ViewManager    *viewManager)
{

}


//-----------------------------------------------------------------------------
void VolumeOfInterest::buildVOIs()
{
  QAction *action;

  // Exact Pixel Selector
  action = new QAction(QIcon(":/espina/voi.svg"), tr("Volume Of Interest"), m_voiSelector);

  m_voiSelector->addAction(action);
  RectangularVOISPtr voi(new RectangularVOI(m_model, m_viewManager));
  m_vois[action] = voi;
  connect(voi.data(), SIGNAL(voiDeactivated()),
          this, SLOT(cancelVOI()));
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
  m_voiSelector->cancel();
  m_viewManager->unsetActiveVOI();
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::reset()
{
  cancelVOI();
}
