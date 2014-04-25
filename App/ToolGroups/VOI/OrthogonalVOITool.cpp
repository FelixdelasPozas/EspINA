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

// EspINA
#include <App/Settings/ROI/ROISettings.h>
#include <GUI/Selectors/PixelSelector.h>
#include <GUI/View/Widgets/RectangularRegion/RectangularRegion.h>
#include <GUI/View/Widgets/RectangularRegion/RectangularRegionSliceSelector.h>
#include "OrthogonalVOITool.h"

// Qt
#include <QAction>
#include <QDebug>

using namespace EspINA;

//-----------------------------------------------------------------------------
OrthogonalVOITool::OrthogonalVOITool(ModelAdapterSPtr model,
                                   ViewManagerSPtr  viewManager)
: m_model        {model}
, m_viewManager  {viewManager}
, m_applyVOI     {new QAction(QIcon(":/espina/voi_ortogonal.svg"), tr("Orthogonal Volume Of Interest"), this)}
, m_enabled      {true}
, m_widget       {nullptr}
, m_sliceSelector{nullptr}
, m_settings     { new ROISettings()}
{
  m_applyVOI->setCheckable(true);
  m_applyVOI->setEnabled(m_viewManager->currentROI() == nullptr);

  connect(m_viewManager.get(), SIGNAL(ROIChanged()),
          this,                SLOT(ROIChanged()));

  connect(m_applyVOI, SIGNAL(triggered(bool)),
          this,       SLOT(initTool(bool)));

  auto eventHandler = new PixelSelector();
  eventHandler->setCursor(QCursor(QPixmap(":/espina/roi_go.svg").scaled(32,32)));
  eventHandler->setMultiSelection(false);
  eventHandler->setSelectionTag(Selector::CHANNEL, true);

  m_selector = EventHandlerSPtr{eventHandler};
}

//-----------------------------------------------------------------------------
OrthogonalVOITool::~OrthogonalVOITool()
{
  delete m_settings;

  disconnect(m_viewManager.get(), SIGNAL(ROIChanged()),
             this,                SLOT(ROIChanged()));

  disconnect(m_applyVOI, SIGNAL(triggered(bool)),
             this,       SLOT(initTool(bool)));

  delete m_applyVOI;

  if (m_widget != nullptr)
  {
    m_widget->setEnabled(false);
    m_widget = nullptr;
  }
  m_viewManager->setCurrentROI(nullptr);
}

//-----------------------------------------------------------------------------
void OrthogonalVOITool::setEnabled(bool value)
{
  if(m_enabled == value)
    return;

  m_applyVOI->setEnabled(value);
  m_enabled = value;
}

//-----------------------------------------------------------------------------
bool OrthogonalVOITool::enabled() const
{
  return m_enabled;
}

//-----------------------------------------------------------------------------
QList<QAction *> OrthogonalVOITool::actions() const
{
  QList<QAction *> actions;

  actions << m_applyVOI;

  return actions;
}

//-----------------------------------------------------------------------------
void OrthogonalVOITool::ROIChanged()
{
  if(m_viewManager->currentROI() == nullptr && m_widget != nullptr)
  {
    m_viewManager->removeWidget(m_widget);
    m_widget->setEnabled(false);
    m_widget = nullptr;

    if(m_applyVOI->isChecked())
    {
      m_viewManager->setEventHandler(m_selector);
      connect(m_selector.get(), SIGNAL(itemsSelected(Selector::SelectionList)),
              this,        SLOT(defineROI(Selector::SelectionList)));
    }

    m_viewManager->updateViews();
  }

  m_applyVOI->setEnabled(m_viewManager->currentROI() == nullptr);
}

//-----------------------------------------------------------------------------
void OrthogonalVOITool::initTool(bool value)
{
  switch(value)
  {
    case true:
      if(m_widget == nullptr)
      {
        m_viewManager->setEventHandler(m_selector);
        connect(m_selector.get(), SIGNAL(itemsSelected(Selector::SelectionList)),
                this,        SLOT(defineROI(Selector::SelectionList)));
      }
      break;
    case false:
        m_viewManager->unsetEventHandler(m_selector);
        disconnect(m_selector.get(), SIGNAL(itemsSelected(Selector::SelectionList)),
                   this,        SLOT(defineROI(Selector::SelectionList)));
        m_applyVOI->setChecked(false);
      break;
    default:
      break;
  }
}

//-----------------------------------------------------------------------------
void OrthogonalVOITool::defineROI(Selector::SelectionList channels)
{
  if (channels.isEmpty() || m_viewManager->currentROI() != nullptr)
    return;

  Q_ASSERT(channels.size() == 1); //Only one element is selected
  auto pickedItem = channels.first();

  Q_ASSERT(pickedItem.first->GetNumberOfPoints() == 1); //Only one pixel's selected
  double pos[3];
  pickedItem.first->GetPoint(0, pos);

  auto pItem = pickedItem.second;
  if (ItemAdapter::Type::CHANNEL != pItem->type())
    return;

  auto pickedChannel = channelPtr(pItem);
  auto spacing = pickedChannel->output()->spacing();
  auto origin = pickedChannel->position();

  Bounds bounds{ pos[0] - m_settings->xSize()/2.0, pos[0] + m_settings->xSize()/2.0,
                 pos[1] - m_settings->ySize()/2.0, pos[1] + m_settings->ySize()/2.0,
                 pos[2] - m_settings->zSize()/2.0, pos[2] + m_settings->zSize()/2.0 };

  auto rrWidget = new RectangularRegion(bounds, m_viewManager);
  m_widget = EspinaWidgetSPtr(rrWidget);
  Q_ASSERT(m_widget);
  rrWidget->setResolution(spacing);
  rrWidget->setRepresentationPattern(0xFFF0);

  m_viewManager->addWidget(m_widget);
  VolumeBounds vBounds(bounds, spacing, origin);
  m_viewManager->setCurrentROI(ROISPtr(new ROI(vBounds, spacing, origin)));
  m_viewManager->updateViews();
  rrWidget->setEnabled(true);

  initTool(false);

//  m_sliceSelector = new RectangularRegionSliceSelector(m_widget);
//  m_sliceSelector->setLeftLabel ("From");
//  m_sliceSelector->setRightLabel("To");
//  m_viewManager->addSliceSelectors(m_sliceSelector, ViewManager::From|ViewManager::To);
}
