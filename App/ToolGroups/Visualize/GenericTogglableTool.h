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

#ifndef ESPINA_GENERIC_TOGGLABE_TOOL_H
#define ESPINA_GENERIC_TOGGLABE_TOOL_H

#include <Support/Widgets/ProgressTool.h>

namespace ESPINA
{
  class GenericTogglableTool
  : public Support::Widgets::ProgressTool
  {
  public:
    explicit GenericTogglableTool(const QString &id, const QString &icon, const QString &tooltip, Support::Context &context);

    explicit GenericTogglableTool(const QString &id, const QIcon &icon, const QString &tooltip, Support::Context &context);

    virtual void saveSettings(std::shared_ptr<QSettings> settings);

    virtual void restoreSettings(std::shared_ptr< QSettings > settings);
  };
}

#endif // ESPINA_GENERIC_TOGGLABE_TOOL_H
