/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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

// EspINA
#include "RectangularROI.h"
#include <Support/Settings/EspinaSettings.h>
#include <App/Settings/ROI/ROISettingsPanel.h>
#include <App/Settings/ROI/ROISettings.h>
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/ModelAdapter.h>
#include <Core/Factory/CoreFactory.h>
#include <Support/ViewManager.h>
#include <GUI/View/Widgets/RectangularRegion/RectangularRegion.h>
#include <GUI/View/Widgets/RectangularRegion/RectangularRegionSliceSelector.h>

// Qt
#include <QPixmap>

using namespace EspINA;

//-----------------------------------------------------------------------------
RectangularROI::RectangularROI(ModelAdapterSPtr model,
                               ViewManagerSPtr  viewManager)
: m_model        {model}
, m_viewManager  {viewManager}
, m_inUse        {false}
, m_enabled      {true}
, m_widget       {nullptr}
, m_sliceSelector{nullptr}
, m_settings     {new ROISettings()}
, m_settingsPanel{new ROISettingsPanel(m_model, m_settings, m_viewManager)}
{
  m_selector.setCursor(QCursor(QPixmap(":/espina/roi_go.svg").scaled(32,32)));
  m_selector.setMultiSelection(false);
  m_selector.setSelectionTag(Selector::CHANNEL, true);
  connect(&m_selector, SIGNAL(itemsSelected(Selector::Selection)),
          this, SLOT(defineVOI(Selector::Selection)));
}

//-----------------------------------------------------------------------------
RectangularROI::~RectangularROI()
{
//   qDebug() << "Destroy RVOI";
  delete m_settings;

//  if (m_sliceSelector)
//  {
//    m_viewManager->removeSliceSelectors(m_sliceSelector);
//    delete m_sliceSelector;
//  }

  if (m_widget)
    delete m_widget;
}

//-----------------------------------------------------------------------------
QCursor RectangularROI::cursor() const
{
  QCursor voiCursor(Qt::ArrowCursor);

  if (!m_widget)
    voiCursor= m_selector.cursor();

  return voiCursor;
}


//-----------------------------------------------------------------------------
bool RectangularROI::filterEvent(QEvent* e, RenderView* view)
{
  bool eventHandled = false;

  if (m_enabled)
  {
//    if (m_widget)
//      eventHandled = m_widget->filterEvent(e,view);

    if (!eventHandled && !m_widget)
      eventHandled = m_selector.filterEvent(e, view);
  }

  return eventHandled;
}

//-----------------------------------------------------------------------------
void RectangularROI::setInUse(bool value)
{
  if (m_inUse == value)
    return;

  m_inUse = value;

  if (m_inUse)
  {
    m_enabled = true;
  }
  else
  {
    if (m_widget)
    {
//      m_viewManager->removeSliceSelectors(m_sliceSelector);
//      delete m_sliceSelector;
//      m_sliceSelector = NULL;

      delete m_widget;
      m_widget = nullptr;

      m_viewManager->updateViews();
    }

    emit voiDeactivated();
  }
}

//-----------------------------------------------------------------------------
void RectangularROI::setEnabled(bool enable)
{
  if (m_enabled == enable)
    return;

  m_enabled = enable;
  if (m_widget)
    m_widget->setEnabled(enable);
}

////-----------------------------------------------------------------------------
//IVOI::Region RectangularROI::region()
//{
//  if (m_widget)
//    m_widget->bounds(m_bounds);
//
//  return m_bounds;
//}

//-----------------------------------------------------------------------------
void RectangularROI::defineVOI(Selector::Selection channels)
{
  if (channels.isEmpty())
    return;

  // Compute default bounds
  Q_ASSERT(channels.size() == 1); //Only one element is selected
  Selector::SelectionItem pickedItem = channels.first();

  Q_ASSERT(pickedItem.first.numberOfVoxels() == 1); //Only one pixel's selected
  auto maskBounds = pickedItem.first.bounds();
  double pos[3] = { (maskBounds[0]+maskBounds[1])/2, (maskBounds[2]+maskBounds[3])/2, (maskBounds[4]+maskBounds[5])/2 };

  ViewItemAdapterPtr pItem = pickedItem.second;
  if (ItemAdapter::Type::CHANNEL != pItem->type())
    return;

  ChannelAdapterPtr pickedChannel = channelPtr(pItem);
  NmVector3 spacing = pickedChannel->output()->spacing();

  Bounds bounds{ pos[0] - m_settings->xSize()/2.0, pos[0] + m_settings->xSize()/2.0,
                 pos[1] - m_settings->ySize()/2.0, pos[1] + m_settings->ySize()/2.0,
                 pos[2] - m_settings->zSize()/2.0, pos[2] + m_settings->zSize()/2.0 };

  m_widget = new RectangularRegion(bounds, m_viewManager);
  Q_ASSERT(m_widget);
  m_widget->setResolution(spacing);
  m_widget->setRepresentationPattern(0xFFF0);
  for(auto view: m_viewManager->renderViews())
    m_widget->registerView(view);

//  m_sliceSelector = new RectangularRegionSliceSelector(m_widget);
//  m_sliceSelector->setLeftLabel ("From");
//  m_sliceSelector->setRightLabel("To");
//  m_viewManager->addSliceSelectors(m_sliceSelector, ViewManager::From|ViewManager::To);
  m_viewManager->updateViews();
}
