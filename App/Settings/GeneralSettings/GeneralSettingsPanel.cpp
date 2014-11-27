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

// ESPINA
#include "GeneralSettingsPanel.h"

using namespace ESPINA;

//------------------------------------------------------------------------
GeneralSettingsPanel::GeneralSettingsPanel(GeneralSettingsSPtr settings)
: m_settings{settings}
{
  setupUi(this);

  userName        ->setText (m_settings->userName());
  autosavePath    ->setText (m_settings->autosavePath().absolutePath());
  autosaveInterval->setValue(m_settings->autosaveInterval());
}

//------------------------------------------------------------------------
GeneralSettingsPanel::~GeneralSettingsPanel()
{
  //qDebug() << "Destroying General Settings Panel";
}

//------------------------------------------------------------------------
void GeneralSettingsPanel::acceptChanges()
{
  m_settings->setUserName(userName->text());
  m_settings->setAutosavePath(autosavePath->text());
  m_settings->setAutosaveInterval(autosaveInterval->value());
}

//------------------------------------------------------------------------
void GeneralSettingsPanel::rejectChanges()
{
}

//------------------------------------------------------------------------
bool GeneralSettingsPanel::modified() const
{
  return userName->text()          != m_settings->userName()
      || autosavePath->text()      != m_settings->autosavePath().absolutePath()
      || autosaveInterval->value() != m_settings->autosaveInterval();
}

//------------------------------------------------------------------------
SettingsPanelPtr GeneralSettingsPanel::clone()
{
  return new GeneralSettingsPanel(m_settings);
}
