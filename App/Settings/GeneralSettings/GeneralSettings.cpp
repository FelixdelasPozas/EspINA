/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

#include "GeneralSettings.h"
#include <Support/Settings/EspinaSettings.h>


#include <QDir>
#include <QSettings>
#include <QDebug>

using namespace EspINA;

//-----------------------------------------------------------------------------
GeneralSettings::GeneralSettings()
: m_settings(new QSettings(CESVIMA, ESPINA))
{
  m_userName         = m_settings->value(USER_NAME, "User").toString();
  m_autosaveInterval = m_settings->value(AUTOSAVE_INTERVAL, 10).toInt();
  m_autosavePath     = m_settings->value(AUTOSAVE_PATH, QDir::homePath()+"/.espina").toString();
}

//-----------------------------------------------------------------------------
GeneralSettings::~GeneralSettings()
{
  //qDebug() << "Destroying General Settings";
}

//-----------------------------------------------------------------------------
void GeneralSettings::setUserName(QString name)
{
  m_userName = name;
  m_settings->setValue(USER_NAME, m_userName);
}

//-----------------------------------------------------------------------------
void GeneralSettings::setAutosaveInterval(int min)
{
  m_autosaveInterval = min;
  m_settings->setValue(AUTOSAVE_INTERVAL, min);
}

//-----------------------------------------------------------------------------
void GeneralSettings::setAutosavePath(const QString path)
{
  m_autosavePath = QDir(path);
  m_settings->setValue(AUTOSAVE_PATH, path);
}
