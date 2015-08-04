/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_SUPPORT_TYPES_H_
#define ESPINA_SUPPORT_TYPES_H_

#include <memory>
#include <QList>

namespace ESPINA
{
  class RepresentationFactory;
  using RepresentationFactorySPtr  = std::shared_ptr<RepresentationFactory>;
  using RepresentationFactorySList = QList<RepresentationFactorySPtr>;
  
  namespace Support
  {
    class Context;
    class Report;

    using ReportSPtr  = std::shared_ptr<Report>;
    using ReportSList = QList<ReportSPtr>;

    namespace Settings
    {
      class SettingsPanel;

      using SettingsPanelPtr   = SettingsPanel *;
      using SettingsPanelList  = QList<SettingsPanelPtr>;
      using SettingsPanelSPtr  = std::shared_ptr<SettingsPanel>;
      using SettingsPanelSList = QList<SettingsPanelSPtr>;
    }
    namespace Widgets
    {
      class ColorEngineSwitch;
      using ColorEngineSwitchSPtr = std::shared_ptr<ColorEngineSwitch>;

      class ProgressTool;
      using ToolSPtr  = std::shared_ptr<ProgressTool>;
      using ToolSList = QList<ToolSPtr>;
    }
  }
}

#endif //ESPINA_SUPPORT_TYPES_H_