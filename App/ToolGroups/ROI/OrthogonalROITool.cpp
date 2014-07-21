/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
#include "OrthogonalROITool.h"
#include "ROITools.h"
#include <App/Settings/ROI/ROISettings.h>
#include <GUI/Selectors/PixelSelector.h>
#include <GUI/View/Widgets/RectangularRegion/RectangularRegion.h>
#include <GUI/View/Widgets/RectangularRegion/RectangularRegionSliceSelector.h>
#include <Undo/ROIUndoCommand.h>

// Qt
#include <QAction>
#include <QDebug>

using namespace EspINA;

//-----------------------------------------------------------------------------
OrthogonalROITool::OrthogonalROITool(ROISettings     *settings,
                                     ModelAdapterSPtr model,
                                     ViewManagerSPtr  viewManager,
                                     QUndoStack      *undoStack,
                                     ROIToolsGroup   *toolGroup)
: m_model        {model}
, m_viewManager  {viewManager}
, m_undoStack    {undoStack}
, m_toolGroup    {toolGroup}
, m_applyROI     {new QAction(QIcon(":/espina/voi_ortogonal.svg"), tr("Orthogonal Volume Of Interest"), this)}
, m_enabled      {true}
, m_widget       {nullptr}
, m_sliceSelector{nullptr}
, m_settings     {settings}
{
  m_applyROI->setCheckable(true);
  m_applyROI->setEnabled(m_toolGroup->currentROI() == nullptr);

  connect(m_viewManager.get(), SIGNAL(eventHandlerChanged()),
          this,                SLOT(commitROI()));

  connect(m_applyROI, SIGNAL(triggered(bool)),
          this,       SLOT(initTool(bool)));

  auto eventHandler = new PixelSelector();
  eventHandler->setCursor(QCursor(QPixmap(":/espina/roi_go.svg").scaled(32,32)));
  eventHandler->setMultiSelection(false);
  eventHandler->setSelectionTag(Selector::CHANNEL, true);

  m_selector = EventHandlerSPtr{eventHandler};
}

//-----------------------------------------------------------------------------
OrthogonalROITool::~OrthogonalROITool()
{
  delete m_settings;

  disconnect(m_viewManager.get(), SIGNAL(eventHandlerChanged()),
             this,                SLOT(commitROI()));

  disconnect(m_applyROI, SIGNAL(triggered(bool)),
             this,       SLOT(initTool(bool)));

  delete m_applyROI;

  if (m_widget != nullptr)
  {
    m_widget->setEnabled(false);
    m_widget = nullptr;
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::setEnabled(bool value)
{
  if(m_enabled == value)
    return;

  m_applyROI->setEnabled(value);
  m_enabled = value;
}

//-----------------------------------------------------------------------------
bool OrthogonalROITool::enabled() const
{
  return m_enabled;
}

//-----------------------------------------------------------------------------
QList<QAction *> OrthogonalROITool::actions() const
{
  QList<QAction *> actions;

  actions << m_applyROI;

  return actions;
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::initTool(bool value)
{
  switch(value)
  {
    case true:
      if(m_widget != nullptr)
        commitROI();

      m_viewManager->setEventHandler(m_selector);
      connect(m_selector.get(), SIGNAL(itemsSelected(Selector::Selection)),
              this,             SLOT(defineROI(Selector::Selection)));
      break;
    case false:
        m_viewManager->unsetEventHandler(m_selector);
        disconnect(m_selector.get(), SIGNAL(itemsSelected(Selector::Selection)),
                   this,             SLOT(defineROI(Selector::Selection)));
        m_applyROI->setChecked(false);
      break;
    default:
      break;
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::commitROI()
{
  if(m_widget != nullptr)
  {
    auto rrWidget = dynamic_cast<RectangularRegion *>(m_widget.get());
    auto mask = BinaryMaskSPtr<unsigned char>{new BinaryMask<unsigned char>{rrWidget->bounds(), m_spacing, m_origin}};
    BinaryMask<unsigned char>::iterator it{mask.get()};
    it.goToBegin();
    while(!it.isAtEnd())
    {
      it.Set();
      ++it;
    }

    if(m_toolGroup->currentROI() == nullptr)
      m_undoStack->beginMacro("Create Region Of Interest");
    else
      m_undoStack->beginMacro("Modify Region Of Interest");

    m_undoStack->push(new ModifyROIUndoCommand{m_toolGroup, mask});
    m_undoStack->endMacro();

    m_viewManager->removeWidget(m_widget);
    m_widget->setEnabled(false);
    m_widget = nullptr;
    m_viewManager->updateViews();
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::defineROI(Selector::Selection channels)
{
  if (channels.isEmpty())
    return;

  auto activeChannel = m_viewManager->activeChannel();
  bool valid = false;
  Selector::SelectionItem selectedChannel;
  for(auto channel: channels)
    if(channel.second == activeChannel)
    {
      valid = true;
      selectedChannel = channel;
      break;
    }

  if(!valid)
    return;

  Q_ASSERT(selectedChannel.first->numberOfVoxels() == 1); //Only one pixel's selected
  auto pointBounds = selectedChannel.first->bounds();
  NmVector3 pos{ (pointBounds[0]+pointBounds[1])/2, (pointBounds[2]+pointBounds[3])/2, (pointBounds[4]+pointBounds[5])/2};

  auto pItem = selectedChannel.second;
  if (ItemAdapter::Type::CHANNEL != pItem->type())
    return;

  auto pickedChannel = channelPtr(pItem);
  m_spacing = pickedChannel->output()->spacing();
  m_origin = pickedChannel->position();

  Bounds bounds{ pos[0] - m_settings->xSize()/2.0, pos[0] + m_settings->xSize()/2.0,
                 pos[1] - m_settings->ySize()/2.0, pos[1] + m_settings->ySize()/2.0,
                 pos[2] - m_settings->zSize()/2.0, pos[2] + m_settings->zSize()/2.0 };

  auto rrWidget = new RectangularRegion(bounds);
  m_widget = EspinaWidgetSPtr(rrWidget);
  Q_ASSERT(m_widget);
  rrWidget->setResolution(m_spacing);
  rrWidget->setRepresentationPattern(0xFFF0);

  m_viewManager->addWidget(m_widget);
  m_viewManager->updateViews();
  rrWidget->setEnabled(true);

  initTool(false);

//  m_sliceSelector = new RectangularRegionSliceSelector(m_widget);
//  m_sliceSelector->setLeftLabel ("From");
//  m_sliceSelector->setRightLabel("To");
//  m_viewManager->addSliceSelectors(m_sliceSelector, ViewManager::From|ViewManager::To);
}
