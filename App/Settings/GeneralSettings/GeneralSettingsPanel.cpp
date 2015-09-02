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
#include <AutoSave.h>

using namespace ESPINA;
using namespace ESPINA::Support::Settings;

//------------------------------------------------------------------------
GeneralSettingsPanel::GeneralSettingsPanel(AutoSave &autoSave, GeneralSettingsSPtr settings)
: m_autoSave(autoSave)
, m_settings{settings}
{
  setupUi(this);

  m_userName        ->setText(m_settings->userName());
  m_autosavePath    ->setText(m_autoSave.path().absolutePath());
  m_autosaveInterval->setValue(m_autoSave.interval());
  m_loadSEGSettings ->setChecked(m_settings->loadSEGfileSettings());
}

//------------------------------------------------------------------------
GeneralSettingsPanel::~GeneralSettingsPanel()
{
}

//------------------------------------------------------------------------
void GeneralSettingsPanel::acceptChanges()
{
  m_settings->setUserName(m_userName->text());
  m_settings->setLoadSEGfileSettings(m_loadSEGSettings->isChecked());
  m_autoSave.setPath(m_autosavePath->text());
  m_autoSave.setInterval(m_autosaveInterval->value());
}

//------------------------------------------------------------------------
void GeneralSettingsPanel::rejectChanges()
{
}

//------------------------------------------------------------------------
bool GeneralSettingsPanel::modified() const
{
  return m_userName->text()             != m_settings->userName()
      || m_loadSEGSettings->isChecked() != m_settings->loadSEGfileSettings()
      || m_autosavePath->text()         != m_autoSave.path().absolutePath()
      || m_autosaveInterval->value()    != m_autoSave.interval();
}

//------------------------------------------------------------------------
SettingsPanelPtr GeneralSettingsPanel::clone()
{
  return new GeneralSettingsPanel(m_autoSave,m_settings);
}
