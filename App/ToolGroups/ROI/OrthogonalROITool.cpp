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

class ModifyRectangularRegion
: public QUndoCommand
{
public:
  ModifyRectangularRegion(ROISPtr roi, const Bounds &bounds)
  : m_roi(roi)
  , m_swap(bounds)
  {}

  virtual void redo()
  { swapBounds(); }

  virtual void undo()
  { swapBounds(); }

private:
  void swapBounds()
  {
    Bounds tmp = m_roi->bounds();
    m_roi->resize(m_swap);
    m_swap = tmp;
  }

private:
  ROISPtr m_roi;
  Bounds  m_swap;
};


//-----------------------------------------------------------------------------
OrthogonalROITool::OrthogonalROITool(ROISettings     *settings,
                                     ModelAdapterSPtr model,
                                     ViewManagerSPtr  viewManager,
                                     QUndoStack      *undoStack,
                                     ROIToolsGroup   *toolGroup)
: m_model        {model}
, m_viewManager  {viewManager}
, m_undoStack    {undoStack}
, m_activeTool   {new QAction(QIcon(":/espina/roi_orthogonal.svg"), tr("Orthogonal Region of Interest"), this)}
, m_resizeROI    {new QAction(QIcon(":/espina/resize_roi.svg"), tr("Resize Orthogonal Region of Interest"), this)}
, m_applyROI     {new QAction(QIcon(":/espina/roi_go.svg"), tr("Define Orthogonal Region of Interest"), this)}
, m_enabled      {true}
, m_mode         {Mode::FIXED}
, m_widget       {nullptr}
, m_selector     {new PixelSelector()}
, m_sliceSelector{nullptr}
, m_settings     {settings}
{
  m_activeTool->setCheckable(true);
  m_resizeROI ->setCheckable(true);
  m_applyROI  ->setCheckable(true);

  m_resizeROI->setEnabled(false);

  m_selector->setMultiSelection(false);
  m_selector->setSelectionTag(Selector::CHANNEL, true);
  m_selector->setCursor(QCursor(QPixmap(":/espina/roi_go.svg").scaled(32,32)));

  setActionVisibility(false);

  connect(m_activeTool, SIGNAL(toggled(bool)),
          this,         SLOT(setActive(bool)));

  connect(m_resizeROI, SIGNAL(triggered(bool)),
          this,        SLOT(setResizable(bool)));

  connect(m_applyROI, SIGNAL(triggered(bool)),
          this,       SLOT(activateEventHandler(bool)));

  connect(m_viewManager.get(), SIGNAL(eventHandlerChanged()),
          this,                SLOT(onEventHandlerChanged()));

}

//-----------------------------------------------------------------------------
OrthogonalROITool::~OrthogonalROITool()
{
  disconnect(m_viewManager.get(), SIGNAL(eventHandlerChanged()),
             this,                SLOT(onEventHandlerChanged()));

  disconnect(m_applyROI, SIGNAL(triggered(bool)),
             this,       SLOT(activateEventHandler(bool)));

  disconnect(m_resizeROI, SIGNAL(triggered(bool)),
             this,        SLOT(setResizable(bool)));

  disconnect(m_activeTool, SIGNAL(toggled(bool)),
             this,         SLOT(setActive(bool)));

  delete m_applyROI;
  delete m_resizeROI;
  delete m_activeTool;

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

  activateEventHandler(value);

  if (!value)
  {
    setResizable(false);
  }

  m_activeTool->setEnabled(value);
  m_resizeROI ->setEnabled(value && m_roi);
  m_applyROI  ->setEnabled(value);

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

  actions << m_activeTool;
  actions << m_resizeROI;
  actions << m_applyROI;

  return actions;
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::setROI(ROISPtr roi)
{
  if (m_roi)
  {
    destroyOrthogonalWidget();
  }

  bool validRectangularROI = roi && roi->isOrthogonal();
  if (validRectangularROI)
  {
    m_roi = roi;
    createOrthogonalWidget();
  }
  else
  {
    m_roi = nullptr;
  }

  m_resizeROI->setEnabled(validRectangularROI);
  m_viewManager->updateViews();
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::setVisible(bool visible)
{
  if (visible)
  {
    if (!m_rrWidget) createOrthogonalWidget();
  }
  else
  {
    if (m_rrWidget) destroyOrthogonalWidget();
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::destroyOrthogonalWidget()
{
  if (m_widget)
  {
    m_viewManager->removeWidget(m_widget);

    disconnect(m_rrWidget, SIGNAL(modified(Bounds)),
               this,       SLOT(updateBounds(Bounds)));
    disconnect(m_roi.get(), SIGNAL(dataChanged()),
               this,        SLOT(updateRectangularRegion()));

    m_widget->setEnabled(false);
    m_widget   = nullptr;
    m_rrWidget = nullptr;
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::activateEventHandler(bool value)
{
  m_selector->setEnabled(value);

  m_applyROI->blockSignals(true);
  m_applyROI->setChecked(value);
  m_applyROI->blockSignals(false);

  if (value)
  {
    if (m_viewManager->eventHandler() != m_selector)
    {
      m_viewManager->setEventHandler(m_selector);
      connect(m_selector.get(), SIGNAL(itemsSelected(Selector::Selection)),
              this,             SLOT(defineROI(Selector::Selection)));
    }
  }
  else
  {
    disconnect(m_selector.get(), SIGNAL(itemsSelected(Selector::Selection)),
               this,             SLOT(defineROI(Selector::Selection)));
    m_viewManager->unsetEventHandler(m_selector);
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::onEventHandlerChanged()
{
  if (m_viewManager->eventHandler() != m_selector)
  {
    setResizable(false);

    activateEventHandler(false);
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::setActive(bool value)
{
  setActionVisibility(value);
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::setResizable(bool resizable)
{
  if (m_widget)
  {
    if (resizable)
    {
      m_rrWidget->setRepresentationPattern(0xFFF0);
    }
    else
    {
      m_rrWidget->setRepresentationPattern(0xFFFF);
    }

    m_rrWidget   ->setEnabled(resizable);
    m_viewManager->updateViews();
  }

  m_viewManager->setSelectionEnabled(!resizable);

  m_mode = resizable?Mode::RESIZABLE:Mode::FIXED;

  m_resizeROI->blockSignals(true);
  m_resizeROI->setChecked(resizable);
  m_resizeROI->blockSignals(false);
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
    msgBox.setInformativeText("At least one of the sizes for the ROI is 0, please\n"
                              "modify the ROI size values to a valid quantity.");
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


    ROISPtr roi{new ROI(bounds, pickedChannel->output()->spacing(), pickedChannel->position())};

    setResizable(true);
    activateEventHandler(false);

    emit roiDefined(roi);

    //  m_sliceSelector = new RectangularRegionSliceSelector(m_widget);
    //  m_sliceSelector->setLeftLabel ("From");
    //  m_sliceSelector->setRightLabel("To");
    //  m_viewManager->addSliceSelectors(m_sliceSelector, ViewManager::From|ViewManager::To);
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::updateBounds(Bounds bounds)
{
  Q_ASSERT(m_roi);

  m_undoStack->beginMacro("Resize Region of Interest");
  m_undoStack->push(new ModifyRectangularRegion(m_roi, bounds));
  m_undoStack->endMacro();
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::updateRectangularRegion()
{
  m_rrWidget->blockSignals(true);
  m_rrWidget->setBounds(m_roi->bounds());
  m_rrWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::createOrthogonalWidget()
{
  m_rrWidget = new RectangularRegion(m_roi->bounds());
  m_rrWidget->setResolution(m_roi->spacing());
  m_rrWidget->

  connect(m_rrWidget, SIGNAL(modified(Bounds)),
          this,       SLOT(updateBounds(Bounds)));
  connect(m_roi.get(), SIGNAL(dataChanged()),
          this,        SLOT(updateRectangularRegion()));

  m_widget = EspinaWidgetSPtr(m_rrWidget);
  Q_ASSERT(m_widget);


  m_viewManager->addWidget(m_widget);

  setResizeMode(m_mode);
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::setActionVisibility(bool visiblity)
{
  m_resizeROI->setVisible(visiblity);
  m_applyROI ->setVisible(visiblity);
}
