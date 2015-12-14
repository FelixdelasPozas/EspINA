/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_GENERAL_SETTINGS_PANEL_H
#define ESPINA_GENERAL_SETTINGS_PANEL_H

// ESPINA
#include <Support/Settings/SettingsPanel.h>
#include <Support/Settings/Settings.h>

// Qt
#include <ui_GeneralSettingsPanel.h>

namespace ESPINA
{
  class AutoSave;

  /** \class GeneralSettingsPanel
   * \brief Settings panel for the application general settings.
   *
   */
  class GeneralSettingsPanel
  : public Support::Settings::SettingsPanel
  , Ui::GeneralSettingsPanel
  {
      Q_OBJECT
    public:
      /** \brief GeneralSettinsPanel class constructor.
       * \param[in] settings, GeneralSettings object smart pointer.
       *
       */
      explicit GeneralSettingsPanel(AutoSave &autoSave, Support::GeneralSettingsSPtr settings);

      /** \brief GeneralSettingsPanel class virtual destructor.
       *
       */
      virtual ~GeneralSettingsPanel();

      virtual const QString shortDescription() override
      {return "Session";}

      virtual const QString longDescription() override
      {return "Session";}

      virtual const QIcon icon() override
      {return QIcon(":/espina/editor.ico");}

      virtual void acceptChanges() override;

      virtual void rejectChanges() override;

      virtual bool modified() const override;

      virtual Support::Settings::SettingsPanelPtr clone() override;

    private slots:
      /** \brief Disaplays the directory selection dialog an updates the internal values once the dialog has been closed.
       *
       */
      void onBrowseDirClicked();

      /** \brief Updates the values when the "use system temporal directory" checkbox changes state.
       * \param[in] state new checkbox state.
       *
       */
      void onTempDirCheckboxChangedState(int state);

    private:
      AutoSave                    &m_autoSave;
      Support::GeneralSettingsSPtr m_settings;
  };

} // namespace ESPINA

#endif // ESPINA_GENERAL_SETTINGS_PANEL_H
