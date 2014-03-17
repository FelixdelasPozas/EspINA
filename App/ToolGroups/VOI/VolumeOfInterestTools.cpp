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
#include "VolumeOfInterestTools.h"

// Espina

using namespace EspINA;

//-----------------------------------------------------------------------------
VolumeOfInterestTools::VolumeOfInterestTools(ModelAdapterSPtr model,
                   ModelFactorySPtr factory,
                   ViewManagerSPtr  viewManager,
                   QUndoStack      *undoStack,
                   QWidget         *parent)
: ToolGroup(viewManager, QIcon(":/espina/voi.svg"), tr("Volume Of Interest Tools"), parent)
, m_brushVOITool    (new BrushVOITool(model, viewManager))
, m_ortogonalVOITool(new OrtogonalVOITool(model, viewManager))
, m_cleanVOITool    (new CleanVOITool(model, viewManager))
{

}

//-----------------------------------------------------------------------------
VolumeOfInterestTools::~VolumeOfInterestTools()
{

}

//-----------------------------------------------------------------------------
void VolumeOfInterestTools::setEnabled(bool value)
{

}

//-----------------------------------------------------------------------------
bool VolumeOfInterestTools::enabled() const
{
  return true;
}

//-----------------------------------------------------------------------------
ToolSList VolumeOfInterestTools::tools()
{
  ToolSList availableTools;

  availableTools << m_brushVOITool;
  availableTools << m_ortogonalVOITool;
  availableTools << m_cleanVOITool;

  return availableTools;
}