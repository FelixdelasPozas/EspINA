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
#include "RestrictToolGroup.h"
#include <App/Settings/ROI/ROISettings.h>
#include <GUI/Selectors/PixelSelector.h>
#include <GUI/View/Widgets/OrthogonalRegion/OrthogonalRegion.h>
#include <GUI/View/Widgets/OrthogonalRegion/OrthogonalRegionSliceSelector.h>
#include <Undo/ROIUndoCommand.h>

// Qt
#include <QAction>
#include <QMessageBox>
#include <QMouseEvent>

using namespace ESPINA;

class ModifyOrthogonalRegion
: public QUndoCommand
{
public:
  ModifyOrthogonalRegion(ROISPtr roi, const Bounds &bounds)
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
OrthogonalROITool::OrthogonalROITool(ROISettings       *settings,
                                     Support::Context &context,
                                     RestrictToolGroup *toolGroup)
: m_context      (context)
, m_activeTool   {new QAction(QIcon(":/espina/roi_orthogonal.svg"), tr("Orthogonal Region of Interest"), this)}
, m_resizeROI    {new QAction(QIcon(":/espina/resize_roi.svg"), tr("Resize Orthogonal Region of Interest"), this)}
, m_applyROI     {new QAction(QIcon(":/espina/roi_go.svg"), tr("Define Orthogonal Region of Interest"), this)}
, m_enabled      {true}
, m_mode         {Mode::FIXED}
, m_color        {Qt::yellow}
, m_orWidget     {nullptr}
, m_resizeHandler{new EventHandler()}
, m_defineHandler{new PixelSelector()}
, m_settings     {settings}
{
  m_activeTool->setCheckable(true);
  m_resizeROI ->setCheckable(true);
  m_applyROI  ->setCheckable(true);

  m_resizeROI->setEnabled(false);

  m_defineHandler->setMultiSelection(false);
  m_defineHandler->setSelectionTag(Selector::CHANNEL, true);
  m_defineHandler->setCursor(QCursor(QPixmap(":/espina/roi_go.svg").scaled(32,32)));

  setActionVisibility(false);

  connect(m_activeTool, SIGNAL(toggled(bool)),
          this,         SLOT(setActive(bool)));

  connect(m_resizeROI, SIGNAL(triggered(bool)),
          this,        SLOT(setResizable(bool)));

  connect(m_applyROI, SIGNAL(triggered(bool)),
          this,       SLOT(setDefinitionMode(bool)));

// TODO URGENT   connect(context.viewState().get(), SIGNAL(eventHandlerChanged()),
//           this,                SLOT(onEventHandlerChanged()));

}

//-----------------------------------------------------------------------------
OrthogonalROITool::~OrthogonalROITool()
{
// TODO  disconnect(m_viewManager.get(), SIGNAL(eventHandlerChanged()),
//              this,                SLOT(onEventHandlerChanged()));

  disconnect(m_applyROI, SIGNAL(triggered(bool)),
             this,       SLOT(setDefinitionMode(bool)));

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
QList<QAction *> OrthogonalROITool::actions() const
{
  QList<QAction *> actions;

  actions << m_activeTool;
  actions << m_resizeROI;
  actions << m_applyROI;

  return actions;
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::abortOperation()
{
  setActive(false);
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
    setResizable(false);
  }

  m_resizeROI->setEnabled(validRectangularROI);
  // TODO m_viewManager->updateViews();
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::setVisible(bool visible)
{
  if (visible)
  {
    if (!m_orWidget) createOrthogonalWidget();
  }
  else
  {
    if (m_orWidget) destroyOrthogonalWidget();
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::setColor(const QColor& color)
{
  m_color = color;

  if (m_orWidget)
  {
    updateRepresentationColor();
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::onToolEnabled(bool enabled)
{
  setDefinitionMode(enabled);

  if (!enabled)
  {
    setResizable(false);
  }

  m_activeTool->setEnabled(enabled);
  m_resizeROI ->setEnabled(enabled && m_roi);
  m_applyROI  ->setEnabled(enabled);
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::createOrthogonalWidget()
{
  m_orWidget = new OrthogonalRegion(m_roi->bounds());
  m_orWidget->setResolution(m_roi->spacing());
  updateRepresentationColor();

  m_sliceSelector = std::make_shared<OrthogonalRegionSliceSelector>(m_orWidget);

  m_sliceSelector->setLabel("Current ROI");

  connect(m_orWidget, SIGNAL(modified(Bounds)),
          this,       SLOT(updateBounds(Bounds)));
  connect(m_roi.get(), SIGNAL(dataChanged()),
          this,        SLOT(updateOrthogonalRegion()));

  m_widget = EspinaWidgetSPtr(m_orWidget);
  Q_ASSERT(m_widget);


  // TODO URGENT m_viewManager->addWidget(m_widget);

  setResizeMode(m_mode);
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::destroyOrthogonalWidget()
{
  if (m_widget)
  {
    // TODO URGENT m_viewManager->removeWidget(m_widget);

    disconnect(m_orWidget, SIGNAL(modified(Bounds)),
               this,       SLOT(updateBounds(Bounds)));
    disconnect(m_roi.get(), SIGNAL(dataChanged()),
               this,        SLOT(updateOrthogonalRegion()));

    m_widget->setEnabled(false);

    m_widget        = nullptr;
    m_orWidget      = nullptr;
    m_sliceSelector = nullptr;
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::setDefinitionMode(bool value)
{
  m_defineHandler->setEnabled(value);

  m_applyROI->blockSignals(true);
  m_applyROI->setChecked(value);
  m_applyROI->blockSignals(false);

  auto &viewState = m_context.viewState();
  if (value)
  {
    if (viewState.eventHandler() != m_defineHandler)
    {
      viewState.setEventHandler(m_defineHandler);
      connect(m_defineHandler.get(), SIGNAL(itemsSelected(Selector::Selection)),
              this,                  SLOT(defineROI(Selector::Selection)));
    }
  }
  else
  {
    disconnect(m_defineHandler.get(), SIGNAL(itemsSelected(Selector::Selection)),
               this,                  SLOT(defineROI(Selector::Selection)));
    viewState.unsetEventHandler(m_defineHandler);
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::onEventHandlerChanged()
{
  auto handler = m_context.viewState().eventHandler();

  if (handler != m_resizeHandler)
  {
    setResizable(false);
  }
  if (handler != m_defineHandler)
  {
    setDefinitionMode(false);
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
      m_orWidget   ->setRepresentationPattern(0xFFF0);
      //TODO URGENT m_viewManager->addSliceSelectors(m_sliceSelector, View2D::From|View2D::To);
    }
    else
    {
      m_orWidget   ->setRepresentationPattern(0xFFFF);
      //TODO URGENT m_viewManager->removeSliceSelectors(m_sliceSelector);
    }

    m_orWidget   ->setEnabled(resizable);
    //TODO m_viewManager->updateViews();
  }

  auto viewState = &m_context.viewState();
  if (resizable)
  {
    viewState->setEventHandler(m_resizeHandler);
  }
  else
  {
    viewState->unsetEventHandler(m_resizeHandler);
  }

  m_mode = resizable?Mode::RESIZABLE:Mode::FIXED;

  m_resizeROI->blockSignals(true);
  m_resizeROI->setChecked(resizable);
  m_resizeROI->blockSignals(false);
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::defineROI(Selector::Selection channels)
{
  if (channels.isEmpty()) return;

  auto activeChannel = m_context.ActiveChannel;

  bool valid = false;
  Selector::SelectionItem selectedChannel;

  for(auto channel: channels)
  {
    if(channel.second == activeChannel)
    {
      valid = true;
      selectedChannel = channel;
      break;
    }
  }

  if(!valid) return;

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


    auto roi = std::make_shared<ROI>(bounds, pickedChannel->output()->spacing(), pickedChannel->position());

    setResizable(true);
    //setDefinitionMode(false);

    emit roiDefined(roi);
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::updateBounds(Bounds bounds)
{
  Q_ASSERT(m_roi);

  auto undoStack = m_context.undoStack();

  undoStack->beginMacro("Resize Region of Interest");
  undoStack->push(new ModifyOrthogonalRegion(m_roi, bounds));
  undoStack->endMacro();
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::updateOrthogonalRegion()
{
  m_orWidget->blockSignals(true);
  m_orWidget->setBounds(m_roi->bounds());
  m_orWidget->blockSignals(false);
}


//-----------------------------------------------------------------------------
void OrthogonalROITool::setActionVisibility(bool visiblity)
{
  m_resizeROI->setVisible(visiblity);
  m_applyROI ->setVisible(visiblity);
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::updateRepresentationColor()
{
  double color[3] = {m_color.redF(), m_color.greenF(), m_color.blueF()};

  m_orWidget->setRepresentationColor(color);
}
