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

#include <Support/EspinaSupport_Export.h>

// ESPINA
#include <Support/Widgets/ProgressTool.h>

namespace ESPINA
{
  namespace Support
  {
    namespace Widgets
    {
      /** \class PanelSwitch
       * \brief Implements a button to hide/show a panel in the Espina toolbar.
       *
       */
      class EspinaSupport_EXPORT PanelSwitch
      : public ProgressTool
      {
          Q_OBJECT

        public:
          /** \brief PanelSwitch class constructor.
           * \param[in] id switch id
           * \param[in] dock panel to hide/show
           * \param[in] icon button icon.
           * \param[in] tooltip button tooltip.
           * \param[in] context application context.
           *
           */
          explicit PanelSwitch(const QString &id, Panel *dock, const QString &icon, const QString &tooltip, Context &context);

          /** \brief PanelSwitch class virtual destructor.
           *
           */
          virtual ~PanelSwitch()
          {}

          virtual void restoreSettings(std::shared_ptr<QSettings> settings) override final;

          virtual void saveSettings(std::shared_ptr<QSettings> settings) override final;

        private slots:
          /** \brief Shows/Hides the panel.
           * \param[in] visible true to show the panel and false to hide it.
           *
           */
          void showPanel(bool visible);

        private:
          virtual void abortOperation() override;

        private:
          Panel *m_dock; /** dock of the tool button. */
      };
    }
  }
}

#endif // ESPINA_SUPPORT_WIDGETS_PANELSWITCH_H
