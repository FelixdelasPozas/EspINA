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

// ESPINA
#include "OrthogonalROITool.h"
#include "ROITools.h"
#include <App/Settings/ROI/ROISettings.h>
#include <GUI/Selectors/PixelSelector.h>
#include <GUI/View/Widgets/RectangularRegion/RectangularRegion.h>
#include <GUI/View/Widgets/RectangularRegion/RectangularRegionSliceSelector.h>
#include <Undo/ROIUndoCommand.h>

// Qt
#include <QAction>
#include <QMessageBox>

using namespace ESPINA;

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
, m_selector     {new PixelSelector()}
, m_widget       {nullptr}
, m_sliceSelector{nullptr}
, m_settings     {settings}
{
  m_applyROI->setCheckable(true);

  m_selector->setMultiSelection(false);
  m_selector->setSelectionTag(Selector::CHANNEL, true);

  connect(m_applyROI, SIGNAL(triggered(bool)),
          this,       SLOT(activateEventHandler(bool)));

  connect(m_selector.get(), SIGNAL(eventHandlerInUse(bool)),
          this,             SLOT(activateTool(bool)));

}

//-----------------------------------------------------------------------------
OrthogonalROITool::~OrthogonalROITool()
{
  disconnect(m_selector.get(), SIGNAL(eventHandlerInUse(bool)),
             this,             SLOT(activateTool(bool)));

  disconnect(m_applyROI, SIGNAL(triggered(bool)),
             this,       SLOT(activateTool(bool)));

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
  if(m_enabled == value) return;

  enableSelector(value);
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
bool OrthogonalROITool::isDefined() const
{
  return m_widget != nullptr;
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::cancelWidget()
{
  if (m_widget)
  {
    m_viewManager->removeWidget(m_widget);
    m_viewManager->updateViews();

    m_widget->setEnabled(false);
    m_widget = nullptr;

    m_applyROI->blockSignals(true);
    m_applyROI->setChecked(false);
    m_applyROI->blockSignals(false);
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::activateEventHandler(bool value)
{
  if (value)
  {
    if (m_viewManager->eventHandler() != m_selector) {
      m_viewManager->setEventHandler(m_selector);
      connect(m_selector.get(), SIGNAL(itemsSelected(Selector::Selection)),
              this,             SLOT(defineROI(Selector::Selection)));
    }

    commitROI();

    enableSelector(true);
  }
  else
  {
    m_viewManager->unsetEventHandler(m_selector);
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::activateTool(bool value)
{
  if (!value)
  {
    commitROI();
    disconnect(m_selector.get(), SIGNAL(itemsSelected(Selector::Selection)),
               this,             SLOT(defineROI(Selector::Selection)));
  }
  enableSelector(value);
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

    cancelWidget();
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::enableSelector(bool value)
{
  m_selector->setEnabled(value);

  if (value)
  {
    m_selector->setCursor(QCursor(QPixmap(":/espina/roi_go.svg").scaled(32,32)));
  }
  else
  {
    m_selector->setCursor(Qt::ArrowCursor);
  }

  m_applyROI->blockSignals(true);
  m_applyROI->setChecked(value);
  m_applyROI->blockSignals(false);
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

  if(m_settings->xSize() == 0 || m_settings->ySize() == 0 || m_settings->zSize() == 0)
  {
    QMessageBox msgBox;
    msgBox.setText("Invalid ROI size values.");
    msgBox.setInformativeText("At least one of the sizes for the ROI is 0, please\nmodify the ROI size values to a valid quantity.");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);

    return;
  }

  Q_ASSERT(selectedChannel.first->numberOfVoxels() == 1); //Only one pixel's selected
  auto pointBounds = selectedChannel.first->bounds();
  NmVector3 pos{ (pointBounds[0]+pointBounds[1])/2, (pointBounds[2]+pointBounds[3])/2, (pointBounds[4]+pointBounds[5])/2};

  auto pItem = selectedChannel.second;

  if (isChannel(pItem))
  {
    auto pickedChannel = channelPtr(pItem);

    Bounds bounds{ pos[0] - m_settings->xSize()/2.0, pos[0] + m_settings->xSize()/2.0,
      pos[1] - m_settings->ySize()/2.0, pos[1] + m_settings->ySize()/2.0,
      pos[2] - m_settings->zSize()/2.0, pos[2] + m_settings->zSize()/2.0 };


    setROI(bounds, pickedChannel->output()->spacing(), pickedChannel->position(), EDITION);

    emit roiDefined();

    //  m_sliceSelector = new RectangularRegionSliceSelector(m_widget);
    //  m_sliceSelector->setLeftLabel ("From");
    //  m_sliceSelector->setRightLabel("To");
    //  m_viewManager->addSliceSelectors(m_sliceSelector, ViewManager::From|ViewManager::To);
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::setROI(const Bounds    &bounds,
                               const NmVector3 &spacing,
                               const NmVector3 &origin,
                               const Mode      &mode)
{
  m_origin  = origin;
  m_spacing = spacing;

  auto rrWidget = new RectangularRegion(bounds);
  rrWidget->setResolution(m_spacing);

  if (EDITION == mode)
  {
    rrWidget->setRepresentationPattern(0xFFF0);
  }

  m_widget = EspinaWidgetSPtr(rrWidget);
  Q_ASSERT(m_widget);

  m_viewManager->addWidget(m_widget);
  m_viewManager->updateViews();
  rrWidget->setEnabled(true);

  enableSelector(false);
}
