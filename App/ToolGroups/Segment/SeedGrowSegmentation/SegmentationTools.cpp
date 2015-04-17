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

// Espina
#include "SegmentationTools.h"

using namespace ESPINA;

//-----------------------------------------------------------------------------
SegmentationTools::SegmentationTools(SeedGrowSegmentationSettings* settings,
                                     FilterDelegateFactorySPtr     filterDelegateFactory,
                                     Support::Context       &context)
: ToolGroup{":/espina/pixelSelector.svg", tr("Segmentation Tools")}
, m_sgsTool{new SeedGrowSegmentationTool(settings, filterDelegateFactory, context)}
{

}

//-----------------------------------------------------------------------------
SegmentationTools::~SegmentationTools()
{

}

//-----------------------------------------------------------------------------
void SegmentationTools::setEnabled(bool value)
{

}

//-----------------------------------------------------------------------------
bool SegmentationTools::enabled() const
{
  return true;
}

//-----------------------------------------------------------------------------
ToolSList SegmentationTools::tools()
{
  ToolSList availableTools;

  availableTools << m_sgsTool;

  return availableTools;
}
