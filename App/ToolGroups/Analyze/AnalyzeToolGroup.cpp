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

// ESPINA
#include "AnalyzeToolGroup.h"
#include "ConnectionCount/ConnectionCountTool.h"
#include "MeasureLength/MeasureLengthTool.h"
#include "SelectionMeasure/SelectionMeasureTool.h"
#include "Reports/ReportsTool.h"

using namespace ESPINA;

//----------------------------------------------------------------------------
AnalyzeToolGroup::AnalyzeToolGroup(Support::Context &context, QWidget *parent)
: ToolGroup{":/espina/toolgroup_analyze.svg", tr("Analyze")}
, m_reports(new ReportsTool(context))
{
  m_reports->setOrder("1-0", "1-REPORTS");
  addTool(m_reports);

  auto measure = std::make_shared<MeasureLengthTool>(context);
  measure->setOrder("1-0", "3-MEASURE");
  addTool(measure);

  auto selectionMeasure = std::make_shared<SelectionMeasureTool>(context);
  selectionMeasure->setOrder("1-1", "3-MEASURE");
  addTool(selectionMeasure);

  auto connectionCount = std::make_shared<ConnectionCountTool>(context);
  connectionCount->setOrder("1-0", "2-SKELETON");
  addTool(connectionCount);
}

//----------------------------------------------------------------------------
AnalyzeToolGroup::~AnalyzeToolGroup()
{
}

//----------------------------------------------------------------------------
void AnalyzeToolGroup::registerReport(Support::ReportSPtr report)
{
  m_reports->registerReport(report);
}
