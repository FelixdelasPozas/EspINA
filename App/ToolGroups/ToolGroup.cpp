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

using namespace ESPINA;

//-----------------------------------------------------------------------------
ToolGroup::ToolGroup(const QIcon    &icon, const QString  &text, QObject *parent)
: QAction{icon, text, parent}
{
  setCheckable(true);

  connect(this, SIGNAL(toggled(bool)),
          this, SLOT(activate(bool)));
}

//-----------------------------------------------------------------------------
ToolGroup::~ToolGroup()
{
  for (auto tool : m_tools)
  {
    tool->abortOperation();
  }
}

//-----------------------------------------------------------------------------
void ToolGroup::addTool(ToolSPtr tool)
{
  onToolAdded(tool);

  m_tools << tool;
}

//-----------------------------------------------------------------------------
void ToolGroup::abortOperations()
{
  for (auto tool : m_tools)
  {
    tool->abortOperation();
  }
}

//-----------------------------------------------------------------------------
ToolSList ToolGroup::tools() const
{
  return m_tools;
}

//-----------------------------------------------------------------------------
void ToolGroup::activate(bool value)
{
  if (value)
  {
    emit activated(this);
  }
}