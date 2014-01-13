/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
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

#ifndef ESPINA_GENERAL_SETTINGS_PANEL_H
#define ESPINA_GENERAL_SETTINGS_PANEL_H

#include <Support/Settings/SettingsPanel.h>
#include <ui_GeneralSettingsPanel.h>

#include "Settings/GeneralSettings/GeneralSettings.h"

namespace EspINA {

  class GeneralSettingsPanel
  : public SettingsPanel
  , Ui::GeneralSettingsPanel
  {
  public:
    explicit GeneralSettingsPanel(GeneralSettingsSPtr settings);
    virtual ~GeneralSettingsPanel();

    virtual const QString shortDescription()
    {return "Session";}

    virtual const QString longDescription()
    {return "Session";}

    virtual const QIcon icon()
    {return QIcon(":/espina/editor.ico");}

    virtual void acceptChanges();

    virtual void rejectChanges();

    virtual bool modified() const;

    virtual SettingsPanelPtr clone();

  private:
    GeneralSettingsSPtr m_settings;
  };

} // namespace EspINA

#endif // ESPINA_GENERAL_SETTINGS_PANEL_H
