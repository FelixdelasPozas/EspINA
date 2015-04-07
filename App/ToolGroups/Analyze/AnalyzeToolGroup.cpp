/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <ToolGroups/Analyze/AnalyzeToolGroup.h>

namespace ESPINA
{
  //----------------------------------------------------------------------------
  AnalyzeToolGroup::AnalyzeToolGroup(const Support::Context &context)
  : ToolGroup{QIcon(":/espina/toolgroup_analyze.svg"), tr("Analyze")}
  {
    addTool(std::make_shared<MeasureTool>(context.viewState()));
    addTool(std::make_shared<RulerTool>(context.viewState()));
  }

  //----------------------------------------------------------------------------
  AnalyzeToolGroup::~AnalyzeToolGroup()
  {
  }
} /* namespace ESPINA */
