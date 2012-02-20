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

#include <gui/ActionSelector.h>
#include <selection/SelectionManager.h>
#include <EspinaCore.h>

#include <QDebug>
#include <gui/EspinaView.h>

//-----------------------------------------------------------------------------
VolumeOfInterest::VolumeOfInterest(QObject* parent)
: QActionGroup (parent)
, m_VOI        (new ActionSelector(this))
, m_selector   (new PixelSelector())
{
  setObjectName("VolumeOfInterest");

  m_selector->setCursor(QCursor(QPixmap(":roi_go.svg").scaled(32,32)));
  m_selector->setMultiSelection(false);
  m_selector->setSelectable(SelectionHandler::EspINA_Channel);

  buildVOIs();

  connect(m_VOI, SIGNAL(triggered(QAction*)),
	  this, SLOT(changeVOISelector(QAction*)));
  connect(m_selector.data(), SIGNAL(selectionChanged(SelectionHandler::MultiSelection)),
	  this, SLOT(defineVOI(SelectionHandler::MultiSelection)));
  connect(m_VOI, SIGNAL(actionCanceled()),
	  this, SLOT(cancelVOI()));

//   m_preferences = new VolumeOfInterestPreferences();
//   EspinaPluginManager::instance()->registerPreferencePanel(m_preferences);
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
  action = new QAction(
    QIcon(":roi.svg")
    , tr("Volume Of Interest"),
    m_VOI);

  m_VOI->addAction(action);
//   voi = new RectangularVOI();
//   addVOI(action, voi);
//   connect(voi, SIGNAL(voiCancelled()),this,SLOT(cancelVOI()));
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::changeVOISelector(QAction* action)
{
  SelectionManager::instance()->setSelectionHandler(m_selector.data());
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::defineVOI(SelectionHandler::MultiSelection msel)
{
  qDebug() << "Create VOI";
  QSharedPointer<ViewManager> vm = EspinaCore::instance()->viewManger();
  QSharedPointer<EspinaView> view = vm->currentView();
  RectangularRegion *voi = new RectangularRegion();
  Q_ASSERT(voi);
  view->addWidget(voi);
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::cancelVOI()
{
  SelectionManager::instance()->setSelectionHandler(NULL);
}
