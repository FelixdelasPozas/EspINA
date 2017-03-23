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
#include "ToolGroup.h"

// Qt
#include <QToolBar>

using namespace ESPINA;
using namespace ESPINA::Support::Widgets;

//-----------------------------------------------------------------------------
ToolGroup::ToolGroup(const QString &icon, const QString  &text, QObject *parent)
: QAction{QIcon(icon), text, parent}
{
  setCheckable(true);

  connect(this, SIGNAL(toggled(bool)),
          this, SLOT(activate(bool)));
}

//-----------------------------------------------------------------------------
void ToolGroup::addTool(ToolSPtr tool)
{
  connect(tool.get(), SIGNAL(exclusiveToolInUse(Support::Widgets::ProgressTool*)),
          this,       SIGNAL(exclusiveToolInUse(Support::Widgets::ProgressTool*)));

  onToolAdded(tool);

  auto group = tool->groupWith();

  if(group.isEmpty()) group = tr("Ungrouped");

  m_tools[group] << tool;

  auto &tools = m_tools[group];
  std::sort(tools.begin(), tools.end(), [](const ToolSPtr &lhs, const ToolSPtr &rhs){ return lhs->positionName() < rhs->positionName(); });
}

//-----------------------------------------------------------------------------
void ToolGroup::abortOperations()
{
  for (auto groupTools : m_tools)
  {
    for (auto groupTool : groupTools)
    {
      groupTool->abortOperation();
    }
  }
}

//-----------------------------------------------------------------------------
void ToolGroup::onExclusiveToolInUse(Support::Widgets::ProgressTool* tool)
{
  for (auto groupTools : m_tools)
  {
    for (auto groupTool : groupTools)
    {
      groupTool->onExclusiveToolInUse(tool);
    }
  }
}

//-----------------------------------------------------------------------------
ToolGroup::GroupedTools ToolGroup::groupedTools() const
{
  GroupedTools tools;

  auto groups = m_tools.keys();

  qSort(groups);

  for (auto group : groups)
  {
    tools.append(m_tools[group]);
  }

  return tools;
}

//-----------------------------------------------------------------------------
void ToolGroup::activate(bool value)
{
  if (value)
  {
    emit activated(this);
  }
}

//-----------------------------------------------------------------------------
void ESPINA::populateToolBar(QToolBar *bar, ToolGroup::GroupedTools tools)
{
  for(auto list: tools)
  {
    for(auto tool: list)
    {
      for(auto action: tool->actions())
      {
        bar->addAction(action);
      }
    }
  }
}
