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

#include "common/gui/ActionSelector.h"
#include "common/selection/SelectionManager.h"
#include "common/gui/EspinaView.h"
#include "common/EspinaCore.h"

#include <QDebug>

//-----------------------------------------------------------------------------
VolumeOfInterest::VolumeOfInterest(QObject* parent)
: QActionGroup (parent)
, m_voi        (new ActionSelector(this))
, m_selector   (new PixelSelector())
{
  setObjectName("VolumeOfInterest");

  m_selector->setCursor(QCursor(QPixmap(":roi_go.svg").scaled(32,32)));
  m_selector->setMultiSelection(false);
  m_selector->setSelectable(SelectionHandler::EspINA_Channel);

  buildVOIs();

  connect(m_voi, SIGNAL(triggered(QAction*)),
	  this, SLOT(changeVOISelector(QAction*)));
  connect(m_selector.data(), SIGNAL(selectionChanged(SelectionHandler::MultiSelection)),
	  this, SLOT(defineVOI(SelectionHandler::MultiSelection)));
  connect(m_voi, SIGNAL(actionCanceled()),
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
    m_voi);

  m_voi->addAction(action);
//   voi = new RectangularVOI();
//   addVOI(action, voi);
//   connect(voi, SIGNAL(voiCancelled()),this,SLOT(cancelVOI()));
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::changeVOISelector(QAction* action)
{
  SelectionManager::instance()->setSelectionHandler(m_selector.data());
  EspinaView *currentView = EspinaCore::instance()->viewManger()->currentView();
  currentView->setSliceSelectors(SliceView::From|SliceView::To);
  connect(currentView, SIGNAL(selectedFromSlice(double, vtkPVSliceView::VIEW_PLANE)),
	  this, SLOT(setBorderFrom(double, vtkPVSliceView::VIEW_PLANE)));
  connect(currentView, SIGNAL(selectedToSlice(double, vtkPVSliceView::VIEW_PLANE)),
	  this, SLOT(setBorderTo(double, vtkPVSliceView::VIEW_PLANE)));
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::defineVOI(SelectionHandler::MultiSelection msel)
{
  if (msel.size() == 0)
    return;

  // Compute default bounds
  Q_ASSERT(msel.size() == 1); //Only one element is selected
  SelectionHandler::Selelection selection = msel.first();

  Q_ASSERT(selection.first.size() == 1); //Only one pixel's selected
  QVector3D pos = selection.first.first();

  QSharedPointer<ViewManager> vm = EspinaCore::instance()->viewManger();
  EspinaView *view = vm->currentView();
  m_voiWidget = QSharedPointer<EspinaWidget>(new RectangularRegion());
  Q_ASSERT(m_voiWidget);
  view->addWidget(m_voiWidget.data());

  const double XHSIZE = 40;
  const double YHSIZE = 40;
  const double ZHSIZE = 40;
  double spacing[3];
  view->gridSize(spacing);
  double bounds[6] = {
     (pos.x() - XHSIZE)*spacing[0], (pos.x() + XHSIZE)*spacing[0],
     (pos.y() - YHSIZE)*spacing[1], (pos.y() + YHSIZE)*spacing[1],
     (pos.z() - ZHSIZE)*spacing[2], (pos.z() + ZHSIZE)*spacing[2]};
  m_voiWidget->setBounds(bounds);
  //BEGIN TODO:
//   m_voiWidget->setEnabled(false);
  //END TODO
  SelectionManager *selectionManager = SelectionManager::instance();
  selectionManager->unsetSelectionHandler(m_selector.data());
  selectionManager->setVOI(m_voiWidget.data());
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::cancelVOI()
{
  if (!m_voiWidget.isNull())
  {
    QSharedPointer<ViewManager> vm = EspinaCore::instance()->viewManger();
    EspinaView *view = vm->currentView();
    view->removeWidget(m_voiWidget.data());
    m_voiWidget.clear();
  }

  SelectionManager *selectorManager = SelectionManager::instance();
  selectorManager->unsetSelectionHandler(m_selector.data());
  selectorManager->setVOI(NULL);
  EspinaView *currentView = EspinaCore::instance()->viewManger()->currentView();
  currentView->setSliceSelectors(SliceView::NoSelector);
  disconnect(currentView, SIGNAL(selectedFromSlice(double, vtkPVSliceView::VIEW_PLANE)),
	  this, SLOT(setBorderFrom(double, vtkPVSliceView::VIEW_PLANE)));
  disconnect(currentView, SIGNAL(selectedToSlice(double, vtkPVSliceView::VIEW_PLANE)),
	  this, SLOT(setBorderTo(double, vtkPVSliceView::VIEW_PLANE)));
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::setBorderFrom(double pos, vtkPVSliceView::VIEW_PLANE plane)
{
  if (!m_voiWidget.isNull())
  {
    EspinaView *view = EspinaCore::instance()->viewManger()->currentView();
    double bounds[6];
    m_voiWidget->bounds(bounds);
    bounds[plane*2] = pos;
    m_voiWidget->setBounds(bounds);
  }
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::setBorderTo(double pos, vtkPVSliceView::VIEW_PLANE plane)
{
  if (!m_voiWidget.isNull())
  {
    EspinaView *view = EspinaCore::instance()->viewManger()->currentView();
    double bounds[6];
    m_voiWidget->bounds(bounds);
    bounds[plane*2+1] = pos;
    m_voiWidget->setBounds(bounds);
  }

}
