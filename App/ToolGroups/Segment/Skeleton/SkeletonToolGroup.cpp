/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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
#include "SkeletonToolGroup.h"

using namespace ESPINA;
using namespace ESPINA::Support::Widgets;

//-----------------------------------------------------------------------------
SkeletonToolGroup::SkeletonToolGroup(Support::Context &context)
: ToolGroup  {":/espina/tubular.svg", tr("Skeleton tools.")}
, m_tool     {new SkeletonTool{context}}
, m_enabled  {false}
{
}

//-----------------------------------------------------------------------------
void SkeletonToolGroup::setEnabled(bool value)
{
  m_enabled = value;
  m_tool->setEnabled(value);
}

//-----------------------------------------------------------------------------
ToolSList SkeletonToolGroup::tools()
{
  ToolSList list;
  list << m_tool;
  return list;
}