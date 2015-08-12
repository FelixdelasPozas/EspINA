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

#ifndef ESPINA_SUPPORT_WIDGETS_PANEL_SWITCH_H
#define ESPINA_SUPPORT_WIDGETS_PANEL_SWITCH_H

#include <Support/Widgets/ProgressTool.h>

namespace ESPINA
{
  namespace Support
  {
    namespace Widgets
    {
      class PanelSwitch
      : public ProgressTool
      {
        Q_OBJECT

      public:
        explicit PanelSwitch(const QString &id, DockWidget *dock, const QString &icon, const QString &tooltip, Context &context);

        virtual void restoreSettings(std::shared_ptr<QSettings> settings) override final;

        virtual void saveSettings(std::shared_ptr<QSettings> settings) override final;

      private slots:
        void showPanel(bool visible);

      private:
        virtual void abortOperation() override;

      private:
        DockWidget *m_dock;
      };
    }
  }
}

#endif // ESPINA_SUPPORT_WIDGETS_PANELSWITCH_H
