/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "GenericTogglableTool.h"

using namespace ESPINA;
using namespace ESPINA::Support;

//-----------------------------------------------------------------------------
GenericTogglableTool::GenericTogglableTool(const QString &id, const QString &icon, const QString &tooltip, Context &context)
: ProgressTool(id, icon, tooltip, context)
{
  setCheckable(true);
}

//-----------------------------------------------------------------------------
GenericTogglableTool::GenericTogglableTool(const QString &id, const QIcon &icon, const QString &tooltip, Context &context)
: ProgressTool(id, icon, tooltip, context)
{
  setCheckable(true);
}

//-----------------------------------------------------------------------------
void GenericTogglableTool::saveSettings(std::shared_ptr<QSettings> settings)
{
  saveCheckSetting(settings);
}

//-----------------------------------------------------------------------------
void GenericTogglableTool::restoreSettings(std::shared_ptr<QSettings> settings)
{
  restoreCheckedState(settings);
}
