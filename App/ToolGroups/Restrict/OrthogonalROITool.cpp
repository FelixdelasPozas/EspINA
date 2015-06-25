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
#include <GUI/View/Widgets/OrthogonalRegion/OrthogonalWidget2D.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Widgets/Styles.h>
#include <Undo/ROIUndoCommand.h>

// Qt
#include <QAction>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>

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

using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::OrthogonalRegion;

//-----------------------------------------------------------------------------
OrthogonalROITool::OrthogonalROITool(ROISettings       *settings,
                                     Support::Context  &context,
                                     RestrictToolGroup *toolGroup)
: ProgressTool("OrthogonalROI", ":/espina/roi_orthogonal.svg", tr("Orthogonal Region of Interest"), context)
, m_prototypes   {new TemporalPrototypes(std::make_shared<OrthogonalWidget2D>(m_roiRepresentation), TemporalRepresentation3DSPtr())}
, m_resizeHandler{new EventHandler()}
, m_defineHandler{new PixelSelector()}
, m_settings     {settings}
{
  setCheckable(true);
  setExclusive(true);

  initControls();

  connect(this, SIGNAL(toggled(bool)),
          this, SLOT(setActive(bool)));

  connect(&getViewState(), SIGNAL(eventHandlerChanged()),
          this,            SLOT(onEventHandlerChanged()));

}

//-----------------------------------------------------------------------------
OrthogonalROITool::~OrthogonalROITool()
{
  delete m_applyROI;
  delete m_resizeROI;
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
    disableOrthogonalWidget();
  }

  m_resizeROI->setEnabled(validRectangularROI);
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::setVisible(bool visible)
{
  if (visible)
  {
    createOrthogonalWidget();
  }
  else
  {
    destroyOrthogonalWidget();
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::setColor(const QColor& color)
{
  m_roiRepresentation.setColor(color);
}

//-----------------------------------------------------------------------------
Bounds OrthogonalROITool::createRegion(const NmVector3 &centroid, const Nm xSize, const Nm ySize, const Nm zSize)
{
  return Bounds{centroid[0] - xSize/2.0, centroid[0] + xSize/2.0,
                centroid[1] - ySize/2.0, centroid[1] + ySize/2.0,
                centroid[2] - zSize/2.0, centroid[2] + zSize/2.0 };
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::onToolGroupActivated()
{
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::initControls()
{
  m_resizeROI = Styles::createToolButton(":/espina/resize_roi.svg", tr("Resize Orthogonal Region of Interest"));
  m_applyROI  = Styles::createToolButton(":/espina/roi_define.svg", tr("Define Orthogonal Region of Interest"));

  m_resizeROI ->setCheckable(true);
  m_applyROI  ->setCheckable(true);
  m_resizeROI->setEnabled(false);

  m_defineHandler->setMultiSelection(false);
  m_defineHandler->setSelectionTag(Selector::CHANNEL, true);
  m_defineHandler->setCursor(QCursor(QPixmap(":/espina/roi_define_cursor.svg").scaled(32,32)));

  connect(m_resizeROI, SIGNAL(clicked(bool)),
          this,        SLOT(setResizable(bool)));

  connect(m_applyROI, SIGNAL(clicked(bool)),
          this,       SLOT(setDefinitionMode(bool)));


  addSettingsWidget(m_resizeROI);
  addSettingsWidget(m_applyROI);
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::createOrthogonalWidget()
{
  updateRegionRepresentation();

  connect(&m_roiRepresentation, SIGNAL(boundsChanged(Bounds)),
          this,                 SLOT(updateBounds(Bounds)));

  connect(m_roi.get(), SIGNAL(dataChanged()),
          this,        SLOT(updateRegionRepresentation()));


  m_sliceSelector = std::make_shared<OrthogonalSelector>(m_roiRepresentation);
  m_sliceSelector->setLabel(tr("Current ROI"));

  showSliceSelectors();

  getViewState().addTemporalRepresentations(m_prototypes);
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::destroyOrthogonalWidget()
{
  hideSliceSelectors();

  getViewState().removeTemporalRepresentations(m_prototypes);

  m_sliceSelector = nullptr;

  disconnect(&m_roiRepresentation, SIGNAL(boundsChanged(Bounds)),
             this,                 SLOT(updateBounds(Bounds)));

  disconnect(m_roi.get(), SIGNAL(dataChanged()),
             this,        SLOT(updateRegionRepresentation()));
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::disableOrthogonalWidget()
{
  m_roi = nullptr;
  setResizable(false);
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::setDefinitionMode(bool value)
{
  m_defineHandler->setEnabled(value);

  m_applyROI->blockSignals(true);
  m_applyROI->setChecked(value);
  m_applyROI->blockSignals(false);

  auto &viewState = getViewState();

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
  auto handler = getViewState().eventHandler();

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
  setDefinitionMode(value);

  m_resizeROI->setEnabled(m_roi != nullptr);
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::setResizable(bool resizable)
{
  auto viewState = &getViewState();
  if (resizable)
  {
    m_roiRepresentation.setRepresentationPattern(0xFFF0);

    showSliceSelectors();

    viewState->setEventHandler(m_resizeHandler);
  }
  else
  {
    m_roiRepresentation.setRepresentationPattern(0xFFFF);

    hideSliceSelectors();

    viewState->unsetEventHandler(m_resizeHandler);
  }

  setRepresentationResizable(resizable);

  getViewState().refresh();

  m_resizeROI->blockSignals(true);
  m_resizeROI->setChecked(resizable);
  m_resizeROI->blockSignals(false);
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::defineROI(Selector::Selection channels)
{
  if (channels.isEmpty()) return;

  bool validSelection = false;
  Selector::SelectionItem selectedChannel;

  for(auto channel: channels)
  {
    if(channel.second == getActiveChannel())
    {
      validSelection  = true;
      selectedChannel = channel;
      break;
    }
  }

  if(invalidSettings())
  {
    auto title = tr("Invalid ROI size values");
    auto msg   = tr("At least one of the sizes for the ROI is 0, please\n"
                    "modify the ROI size values to a valid quantity.");

    DefaultDialogs::InformationMessage(title, msg);
  }
  else if(validSelection)
  {
    Q_ASSERT(selectedChannel.first->numberOfVoxels() == 1); //Only one pixel's selected

    auto pos   = centroid(selectedChannel.first->bounds());
    auto pItem = selectedChannel.second;

    if (isChannel(pItem))
    {
      auto pickedChannel = channelPtr(pItem);

      auto bounds = createRegion(pos, m_settings->xSize(), m_settings->ySize(), m_settings->zSize());
      auto roi    = std::make_shared<ROI>(bounds, pickedChannel->output()->spacing(), pickedChannel->position());

      setResizable(true);

      emit roiDefined(roi);
    }
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::updateBounds(Bounds bounds)
{
  Q_ASSERT(m_roi);

  auto undoStack = getUndoStack();

  undoStack->beginMacro(tr("Resize Region of Interest"));
  undoStack->push(new ModifyOrthogonalRegion(m_roi, bounds));
  undoStack->endMacro();
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::updateRegionRepresentation()
{
  m_roiRepresentation.blockSignals(true);
  m_roiRepresentation.setBounds(m_roi->bounds());
  m_roiRepresentation.blockSignals(false);
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::setRepresentationResizable(const bool value)
{
  auto mode = value?Representation::Mode::RESIZABLE:Representation::Mode::FIXED;

  m_roiRepresentation.setMode(mode);
}

//-----------------------------------------------------------------------------
bool OrthogonalROITool::isResizable() const
{
  return m_roiRepresentation.mode() == Representation::Mode::RESIZABLE;
}

//-----------------------------------------------------------------------------
bool OrthogonalROITool::invalidSettings() const
{
  return m_settings->xSize() == 0 || m_settings->ySize() == 0 || m_settings->zSize() == 0;
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::showSliceSelectors()
{
  if (m_sliceSelector && isResizable())
  {
    getViewState().addSliceSelectors(m_sliceSelector, From|To);
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::hideSliceSelectors()
{
  if (m_sliceSelector && isResizable())
  {
    getViewState().removeSliceSelectors(m_sliceSelector);
  }
}
