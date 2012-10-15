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

// EspINA
#include "common/gui/ActionSelector.h"
#include <selection/PickableItem.h>
#include "common/gui/ViewManager.h"
#include <model/Channel.h>

// Qt
#include <QDebug>

//-----------------------------------------------------------------------------
VolumeOfInterest::VolumeOfInterest(ViewManager *vm, QWidget *parent)
: QToolBar   (parent)
, m_viewManager(vm)
, m_voi      (new ActionSelector(this))
, m_selector (new PixelSelector())
{
  setObjectName ("VolumeOfInterest");
  setWindowTitle("Volume Of Interest");

  m_selector->setCursor(QCursor(QPixmap(":roi_go.svg").scaled(32,32)));
  m_selector->setMultiSelection(false);
  m_selector->setPickable(IPicker::CHANNEL);

  buildVOIs();

  addAction(m_voi);

  connect(m_voi, SIGNAL(triggered(QAction*)),
          this, SLOT(changeVOISelector(QAction*)));
  connect(m_selector.data(), SIGNAL(itemsPicked(IPicker::PickList)),
          this, SLOT(defineVOI(IPicker::PickList)));
  connect(m_voi, SIGNAL(actionCanceled()),
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
  action = new QAction(QIcon(":roi.svg"), tr("Volume Of Interest"), m_voi);

  m_voi->addAction(action);
  //   voi = new RectangularVOI();
  //   addVOI(action, voi);
  //   connect(voi, SIGNAL(voiCancelled()),this,SLOT(cancelVOI()));
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::changeVOISelector(QAction* action)
{
  m_viewManager->setVOIPicker(m_selector.data());
  /*TODO 2012-10-10 currentView->setSliceSelectors(SliceView::From|SliceView::To);
  connect(currentView, SIGNAL(selectedFromSlice(double, PlaneType)),
          this, SLOT(setBorderFrom(double, PlaneType)));
  connect(currentView, SIGNAL(selectedToSlice(double, PlaneType)),
          this, SLOT(setBorderTo(double, PlaneType)));
          */
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::defineVOI(IPicker::PickList pickList)
{
  if (pickList.size() == 0)
    return;

  // Compute default bounds
  Q_ASSERT(pickList.size() == 1); //Only one element is selected
  IPicker::PickedItem pickedItem = pickList.first();

  Q_ASSERT(pickedItem.first.size() == 1); //Only one pixel's selected
  QVector3D pos = pickedItem.first.first();

  PickableItem *pItem = pickedItem.second;
  if (ModelItem::CHANNEL != pItem->type())
    return;

  Channel *pickedChannel = dynamic_cast<Channel *>(pItem);
  double spacing[3];
  pickedChannel->spacing(spacing);

  const Nm HALF_VOXEL = 0.5;
  const Nm XHSIZE = (40 + HALF_VOXEL)*spacing[0];
  const Nm YHSIZE = (40 + HALF_VOXEL)*spacing[1];
  const Nm ZHSIZE = (40 + HALF_VOXEL)*spacing[2];

  Nm bounds[6] = {
     pos.x() - XHSIZE, pos.x() + XHSIZE,
     pos.y() - YHSIZE, pos.y() + YHSIZE,
     pos.z() - ZHSIZE, pos.z() + ZHSIZE};

  m_voiWidget = QSharedPointer<RectangularRegion>(new RectangularRegion(bounds, m_viewManager));
  Q_ASSERT(m_voiWidget);
  m_viewManager->addWidget(m_voiWidget.data());

  m_viewManager->unsetPicker(m_selector.data());
  // TODO 2012-10-07 Apply VOIs
  m_viewManager->setVOI(m_voiWidget.data());
  m_viewManager->updateViews();
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::cancelVOI()
{
  if (!m_voiWidget.isNull())
  {
    m_viewManager->removeWidget(m_voiWidget.data());
    m_voiWidget.clear();
    m_viewManager->setVOI(NULL);
    m_viewManager->unsetPicker(m_selector.data());
    m_viewManager->updateViews();
  }

  /* BUG TODO 2012-10-05 
  EspinaView *currentView = EspinaCore::instance()->viewManger()->currentView();
  currentView->setSliceSelectors(SliceView::NoSelector);
  disconnect(currentView, SIGNAL(selectedFromSlice(double, PlaneType)),
             this, SLOT(setBorderFrom(double, PlaneType)));
  disconnect(currentView, SIGNAL(selectedToSlice(double, PlaneType)),
             this, SLOT(setBorderTo(double, PlaneType)));
             */
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::setBorderFrom(double pos, PlaneType plane)
{
  if (!m_voiWidget.isNull())
  {
    double bounds[6];
    m_voiWidget->bounds(bounds);
    bounds[plane*2] = pos;
    m_voiWidget->setBounds(bounds);
  }
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::setBorderTo(double pos, PlaneType plane)
{
  if (!m_voiWidget.isNull())
  {
    double bounds[6];
    m_voiWidget->bounds(bounds);
    bounds[plane*2+1] = pos;
    m_voiWidget->setBounds(bounds);
  }
}
