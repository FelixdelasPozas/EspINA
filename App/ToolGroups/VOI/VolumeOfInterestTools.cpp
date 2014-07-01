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
#include "VolumeOfInterestTools.h"

using namespace EspINA;

//-----------------------------------------------------------------------------
VolumeOfInterestTools::VolumeOfInterestTools(ModelAdapterSPtr model,
                                             ModelFactorySPtr factory,
                                             ViewManagerSPtr  viewManager,
                                             QUndoStack      *undoStack,
                                             QWidget         *parent)
: ToolGroup(viewManager, QIcon(":/espina/voi.svg"), tr("Volume Of Interest Tools"), parent)
, m_manualVOITool   {new ManualVOITool(model, viewManager, undoStack)}
, m_ortogonalVOITool{new OrthogonalVOITool(model, viewManager, undoStack)}
, m_cleanVOITool    {new CleanVOITool(model, viewManager, undoStack)}
, m_enabled         {true}
{
}

//-----------------------------------------------------------------------------
VolumeOfInterestTools::~VolumeOfInterestTools()
{
}

//-----------------------------------------------------------------------------
void VolumeOfInterestTools::setEnabled(bool value)
{
  if(m_enabled == value)
    return;

  m_manualVOITool->setEnabled(value);
  m_ortogonalVOITool->setEnabled(value);
  m_cleanVOITool->setEnabled(value);
  m_enabled = value;
}

//-----------------------------------------------------------------------------
bool VolumeOfInterestTools::enabled() const
{
  return m_enabled;
}

//-----------------------------------------------------------------------------
ToolSList VolumeOfInterestTools::tools()
{
  ToolSList availableTools;

  availableTools << m_manualVOITool;
  availableTools << m_ortogonalVOITool;
  availableTools << m_cleanVOITool;

  return availableTools;
}
