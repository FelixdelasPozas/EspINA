/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#include <Core/Utils/EspinaException.h>
#include <Support/Settings/Settings.h>
#include <Support/Settings/Settings.h>

using namespace ESPINA;
using namespace ESPINA::Support;
using namespace ESPINA::Core::Utils;

const QString ApplicationSettings::LOAD_SEG_SETTINGS_KEY     = "Load SEG settings";
const QString ApplicationSettings::TEMPORAL_STORAGE_PATH_KEY = "Temporal Storage Path";
const QString ApplicationSettings::USER_NAME                 = "UserName";

//-----------------------------------------------------------------------------
ApplicationSettings::ApplicationSettings()
{
  ESPINA_SETTINGS(settings);
  m_userName            = settings.value(USER_NAME, "User").toString();
  m_loadSEGSettings     = settings.value(LOAD_SEG_SETTINGS_KEY, true).toBool();
  m_temporalStoragePath = settings.value(TEMPORAL_STORAGE_PATH_KEY, QDir::tempPath()).toString();

  if(!QDir{m_temporalStoragePath}.exists())
  {
    setTemporalPath(QDir::tempPath());
  }
}

//-----------------------------------------------------------------------------
void ApplicationSettings::setUserName(const QString &name)
{
  m_userName = name;

  ESPINA_SETTINGS(settings);
  settings.setValue(USER_NAME, m_userName);
}

//-----------------------------------------------------------------------------
void ApplicationSettings::setLoadSEGfileSettings(bool enable)
{
  m_loadSEGSettings = enable;

  ESPINA_SETTINGS(settings);
  settings.setValue(LOAD_SEG_SETTINGS_KEY, enable);
}

//-----------------------------------------------------------------------------
void ApplicationSettings::setTemporalPath(const QString& path)
{
  QFileInfo info(path);

  if(!info.isDir())
  {
    auto what = QObject::tr("Given path is not a directory. Path: %1").arg(path);
    auto details = QObject::tr("GeneralSettings::setTemporalPath() -> Given path is not a directory. Path: %1").arg(path);

    throw EspinaException(what, details);
  }

  if(!info.isWritable())
  {
    auto what = QObject::tr("Invalid permissions on given path, must be writable. Path: %1").arg(path);
    auto details = QObject::tr("GeneralSettings::setTemporalPath() -> Invalid permissions on given path, must be writable. Path: %1").arg(path);

    throw EspinaException(what, details);
  }

  m_temporalStoragePath = path;

  ESPINA_SETTINGS(settings);
  settings.setValue(TEMPORAL_STORAGE_PATH_KEY, path);
}