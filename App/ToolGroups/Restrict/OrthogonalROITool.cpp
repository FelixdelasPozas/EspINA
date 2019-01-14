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
#include <GUI/Widgets/ToolButton.h>
#include <Undo/ROIUndoCommand.h>

// Qt
#include <QAction>
#include <QMessageBox>
#include <QMouseEvent>

namespace ESPINA
{
  /** \class ModifyOrthogonalRegion
   * \brief QUndoCommand for orthogonal ROI modifications.
   *
   */
  class ModifyOrthogonalRegion
  : public QUndoCommand
  {
    public:
      /** \brief ModifyOrthogonalRegion class constructor.
       * \param[in] roi ROI being modified.
       * \param[in] bounds new bounds.
       *
       */
      ModifyOrthogonalRegion(ROISPtr roi, const Bounds &bounds)
      : m_roi {roi}
      , m_swap{bounds}
      {}

      virtual void redo()
      { swapBounds(); }

      virtual void undo()
      { swapBounds(); }

    private:
      /** \brief Swaps the old and new bounds.
       *
       */
      void swapBounds()
      {
        Bounds tmp = m_roi->bounds();
        m_roi->resize(m_swap);
        m_swap = tmp;
      }

    private:
      ROISPtr m_roi;  /** ROI being modified. */
      Bounds  m_swap; /** old/new bounds      */
  };
}

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::Widgets::Styles;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::OrthogonalRegion;

//-----------------------------------------------------------------------------
OrthogonalROITool::OrthogonalROITool(ROISettings       *settings,
                                     Support::Context  &context,
                                     RestrictToolGroup *toolGroup)
: ProgressTool("OrthogonalROI", ":/espina/roi_orthogonal_roi.svg", tr("Orthogonal 3D ROI"), context)
, m_visible          {true}
, m_roi              {nullptr}
, m_roiRepresentation{new OrthogonalRepresentation()}
, m_resizeHandler    {new EventHandler()}
, m_defineHandler    {new PixelSelector()}
, m_settings         {settings}
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
  m_resizeROI->disconnect();
  m_applyROI->disconnect();

  if(m_defineHandler->isEnabled())
  {
    m_defineHandler->disconnect();
    getViewState().unsetEventHandler(m_defineHandler);
  }

  delete m_applyROI;
  delete m_resizeROI;
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::setROI(ROISPtr roi)
{
  if (m_roi && m_visible)
  {
    destroyOrthogonalWidget();
  }

  m_roi = roi;

  bool validRectangularROI = m_roi && m_roi.get() && m_roi->isOrthogonal();
  if (validRectangularROI)
  {
    if(m_visible)
    {
      createOrthogonalWidget();
    }
  }
  else
  {
    if(m_visible)
    {
      disableOrthogonalWidget();
    }
  }

  m_resizeROI->setEnabled(validRectangularROI);
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::setVisible(bool visible)
{
  if(m_visible != visible)
  {
    m_visible = visible;

    if (visible)
    {
      createOrthogonalWidget();
    }
    else
    {
      destroyOrthogonalWidget();
    }
  }
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::setColor(const QColor& color)
{
  m_roiRepresentation->setColor(color);
}

//-----------------------------------------------------------------------------
Bounds OrthogonalROITool::createRegion(const NmVector3 &centroid, const Nm xSize, const Nm ySize, const Nm zSize)
{
  return Bounds{centroid[0] - xSize/2.0, centroid[0] + xSize/2.0,
                centroid[1] - ySize/2.0, centroid[1] + ySize/2.0,
                centroid[2] - zSize/2.0, centroid[2] + zSize/2.0 };
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::initControls()
{
  m_resizeROI = Styles::createToolButton(":/espina/resize_roi.svg", tr("Resize Orthogonal ROI"));
  m_applyROI  = Styles::createToolButton(":/espina/roi_define.svg", tr("Define Orthogonal ROI"));

  m_resizeROI->setCheckable(true);
  m_applyROI ->setCheckable(true);
  m_resizeROI->setEnabled(false);

  m_defineHandler->setMultiSelection(true);
  m_defineHandler->setSelectionTag(Selector::CHANNEL, true);
  m_defineHandler->setCursor(QCursor(QPixmap(":/espina/roi_define_cursor.svg").scaled(32,32)));

  connect(m_defineHandler.get(), SIGNAL(itemsSelected(Selector::Selection)),
          this,                  SLOT(defineROI(Selector::Selection)));

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
  Q_ASSERT(!m_prototype && m_visible);

  updateRegionRepresentation();

  connect(m_roiRepresentation.get(), SIGNAL(boundsChanged(Bounds)),
          this,                      SLOT(updateBounds(Bounds)));

  connect(m_roi.get(), SIGNAL(dataChanged()),
          this,        SLOT(updateRegionRepresentation()));


  m_sliceSelector = std::make_shared<OrthogonalSelector>(m_roiRepresentation);
  m_sliceSelector->setLabel(tr("Current ROI"));

  showSliceSelectors();

  m_prototype = createTemporalRepresentationPrototype();

  getViewState().addTemporalRepresentations(m_prototype);
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::destroyOrthogonalWidget()
{
  Q_ASSERT(m_visible);

  hideSliceSelectors();

  getViewState().removeTemporalRepresentations(m_prototype);

  m_prototype.reset();

  m_sliceSelector = nullptr;

  disconnect(m_roiRepresentation.get(), SIGNAL(boundsChanged(Bounds)),
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
    }
  }
  else
  {
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
    m_roiRepresentation->setRepresentationPattern(0xFFF0);
    m_roiRepresentation->setMode(OrthogonalRepresentation::Mode::RESIZABLE);

    showSliceSelectors();

    viewState->setEventHandler(m_resizeHandler);
  }
  else
  {
    viewState->unsetEventHandler(m_resizeHandler);

    hideSliceSelectors();

    m_roiRepresentation->setRepresentationPattern(0xFFFF);
    m_roiRepresentation->setMode(OrthogonalRepresentation::Mode::FIXED);
  }

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

  auto it = std::find_if(channels.begin(), channels.end(), [this](const ESPINA::Selector::SelectionItem &item){ return (item.second == getActiveChannel()); });

  if(it != channels.end())
  {
    validSelection = true;
    selectedChannel = (*it);
  }

  if(invalidSettings())
  {
    auto title = tr("Invalid ROI size values");
    auto msg   = tr("At least one of the sizes for the ROI is 0, please\n"
                    "modify the ROI size values to a valid quantity.");

    DefaultDialogs::InformationMessage(msg, title);
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

  {
    WaitingCursor cursor;

    auto undoStack = getUndoStack();

    undoStack->beginMacro(tr("Resize ROI."));
    undoStack->push(new ModifyOrthogonalRegion(m_roi, bounds));
    undoStack->endMacro();
  }

  emit roiModified(m_roi);
}

//-----------------------------------------------------------------------------
void OrthogonalROITool::updateRegionRepresentation()
{
  m_roiRepresentation->blockSignals(true);
  m_roiRepresentation->setBounds(m_roi->bounds());
  m_roiRepresentation->blockSignals(false);
}

//-----------------------------------------------------------------------------
bool OrthogonalROITool::isResizable() const
{
  return m_roiRepresentation->mode() == Representation::Mode::RESIZABLE;
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

//-----------------------------------------------------------------------------
OrthogonalROITool::TemporalPrototypesSPtr OrthogonalROITool::createTemporalRepresentationPrototype() const
{
  return std::make_shared<TemporalPrototypes>(std::make_shared<OrthogonalWidget2D>(m_roiRepresentation), TemporalRepresentation3DSPtr(), id());
}
