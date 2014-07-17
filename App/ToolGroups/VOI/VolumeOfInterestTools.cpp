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
#include <GUI/View/Widgets/ROI/ROIWidget.h>
#include "VolumeOfInterestTools.h"
#include "CleanVOITool.h"
#include "ManualVOITool.h"
#include "OrthogonalVOITool.h"

using namespace EspINA;

//-----------------------------------------------------------------------------
VOIToolsGroup::VOIToolsGroup(ModelAdapterSPtr model,
                                             ModelFactorySPtr factory,
                                             ViewManagerSPtr  viewManager,
                                             QUndoStack      *undoStack,
                                             QWidget         *parent)
: ToolGroup(viewManager, QIcon(":/espina/voi.svg"), tr("Volume Of Interest Tools"), parent)
, m_viewManager      {viewManager}
, m_enabled          {true}
, m_accumulator      {nullptr}
, m_accumulatorWidget{nullptr}
{
  connect(m_viewManager.get(), SIGNAL(ROIChanged()),
          this,                SLOT(updateROI()), Qt::QueuedConnection);

  m_manualVOITool    = ManualVOIToolSPtr{new ManualVOITool(model, viewManager, undoStack, this)};
  m_ortogonalVOITool = OrthogonalVOIToolSPtr{new OrthogonalVOITool(model, viewManager, undoStack, this)};
  m_cleanVOITool     = CleanVOIToolSPtr{new CleanVOITool(model, viewManager, undoStack, this)};
}

//-----------------------------------------------------------------------------
VOIToolsGroup::~VOIToolsGroup()
{
}

//-----------------------------------------------------------------------------
void VOIToolsGroup::setEnabled(bool value)
{
  if(m_enabled == value)
    return;

  m_manualVOITool->setEnabled(value);
  m_ortogonalVOITool->setEnabled(value);
  m_cleanVOITool->setEnabled(value);
  m_enabled = value;
}

//-----------------------------------------------------------------------------
bool VOIToolsGroup::enabled() const
{
  return m_enabled;
}

//-----------------------------------------------------------------------------
ToolSList VOIToolsGroup::tools()
{
  ToolSList availableTools;

  availableTools << m_manualVOITool;
  availableTools << m_ortogonalVOITool;
  availableTools << m_cleanVOITool;

  return availableTools;
}

//-----------------------------------------------------------------------------
void VOIToolsGroup::setCurrentROI(ROISPtr roi)
{
  if(m_accumulator != nullptr)
    m_viewManager->removeWidget(m_accumulatorWidget);

  if(roi != nullptr)
  {
    m_accumulatorWidget = EspinaWidgetSPtr{new ROIWidget(roi)};
    m_viewManager->addWidget(m_accumulatorWidget);
  }

  m_accumulator = roi;
  m_viewManager->setCurrentROI(m_accumulator);
}

//-----------------------------------------------------------------------------
ROISPtr VOIToolsGroup::currentROI()
{
  if(m_viewManager->currentROI() != m_accumulator)
    m_accumulator = m_viewManager->currentROI();

  return m_accumulator;
}

//-----------------------------------------------------------------------------
void VOIToolsGroup::updateROI()
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
