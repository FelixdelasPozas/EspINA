/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "GeneralSettings.h"
#include <Support/Settings/EspinaSettings.h>

// Qt
#include <QDir>
#include <QSettings>

using namespace ESPINA;

//-----------------------------------------------------------------------------
GeneralSettings::GeneralSettings()
{
  ESPINA_SETTINGS(settings);
  m_userName         = settings.value(USER_NAME, "User").toString();
  m_autosaveInterval = settings.value(AUTOSAVE_INTERVAL, 10).toInt();
  m_autosavePath     = settings.value(AUTOSAVE_PATH, QDir::homePath()+"/.espina").toString();
}

//-----------------------------------------------------------------------------
GeneralSettings::~GeneralSettings()
{
}

//-----------------------------------------------------------------------------
void GeneralSettings::setUserName(const QString &name)
{
  m_userName = name;

  ESPINA_SETTINGS(settings);
  settings.setValue(USER_NAME, m_userName);
}

//-----------------------------------------------------------------------------
void GeneralSettings::setAutosaveInterval(int min)
{
  m_autosaveInterval = min;

  ESPINA_SETTINGS(settings);
  settings.setValue(AUTOSAVE_INTERVAL, min);
}

//-----------------------------------------------------------------------------
void GeneralSettings::setAutosavePath(const QString &path)
{
  m_autosavePath = QDir(path);

  ESPINA_SETTINGS(settings);
  settings.setValue(AUTOSAVE_PATH, path);
}
