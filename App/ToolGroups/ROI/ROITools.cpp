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
#include <GUI/View/Widgets/ROI/ROIWidget.h>
#include "ROITools.h"
#include "CleanROITool.h"
#include "ManualROITool.h"
#include "OrthogonalROITool.h"

using namespace ESPINA;

//-----------------------------------------------------------------------------
ROIToolsGroup::ROIToolsGroup(ROISettings*     settings,
                             ModelAdapterSPtr model,
                             ModelFactorySPtr factory,
                             ViewManagerSPtr  viewManager,
                             QUndoStack      *undoStack,
                             QWidget         *parent)
: ToolGroup          {viewManager, QIcon(":/espina/voi.svg"), tr("Volume Of Interest Tools"), parent}
, m_viewManager      {viewManager}
, m_manualROITool    {new ManualROITool(model, viewManager, undoStack, this)}
, m_ortogonalROITool {new OrthogonalROITool(settings, model, viewManager, undoStack, this)}
, m_cleanROITool     {new CleanROITool(model, viewManager, undoStack, this)}
, m_enabled          {true}
, m_accumulator      {nullptr}
, m_accumulatorWidget{nullptr}
{
  connect(m_ortogonalROITool.get(), SIGNAL(roiDefined()),
          this,                     SIGNAL(roiChanged()));
}

//-----------------------------------------------------------------------------
ROIToolsGroup::~ROIToolsGroup()
{
  setCurrentROI(nullptr);
}

//-----------------------------------------------------------------------------
void ROIToolsGroup::setEnabled(bool value)
{
  if(m_enabled == value)
    return;

  m_manualROITool   ->setEnabled(value);
  m_ortogonalROITool->setEnabled(value);
  m_cleanROITool    ->setEnabled(value);

  m_enabled = value;
}

//-----------------------------------------------------------------------------
bool ROIToolsGroup::enabled() const
{
  return m_enabled;
}

//-----------------------------------------------------------------------------
ToolSList ROIToolsGroup::tools()
{
  ToolSList availableTools;

  availableTools << m_manualROITool;
  availableTools << m_ortogonalROITool;
  availableTools << m_cleanROITool;

  return availableTools;
}

//-----------------------------------------------------------------------------
void ROIToolsGroup::setCurrentROI(ROISPtr roi)
{
  if(m_accumulator != nullptr)
    m_viewManager->removeWidget(m_accumulatorWidget);

  if(roi != nullptr)
  {
    m_accumulatorWidget = EspinaWidgetSPtr{new ROIWidget(roi)};
    m_viewManager->addWidget(m_accumulatorWidget);
  }
  else
  {
    m_ortogonalROITool->cancelWidget();
  }

  m_accumulator = roi;
  m_viewManager->setCurrentROI(m_accumulator);

  emit roiChanged();
}

//-----------------------------------------------------------------------------
ROISPtr ROIToolsGroup::currentROI()
{
  return m_accumulator;
}

//-----------------------------------------------------------------------------
bool ROIToolsGroup::hasValidROI()
{
  return m_accumulator || m_ortogonalROITool->isDefined();
}

//-----------------------------------------------------------------------------
void ROIToolsGroup::updateROI()
{
  auto roi = m_viewManager->currentROI();

  if(m_accumulator != nullptr)
    m_viewManager->removeWidget(m_accumulatorWidget);

  if(roi != nullptr)
  {
    m_accumulatorWidget = EspinaWidgetSPtr{new ROIWidget(roi)};
    m_viewManager->addWidget(m_accumulatorWidget);
  }

  m_accumulator = roi;
}
