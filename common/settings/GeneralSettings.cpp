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

#include <QDir>
#include <QSettings>

//-----------------------------------------------------------------------------
GeneralSettings::GeneralSettings()
{
  QSettings settings;

  if (!settings.allKeys().contains(STACK_DIR))
    settings.setValue(STACK_DIR, QDir::homePath()+"/Stacks");
  if (!settings.allKeys().contains(USER_NAME))
    settings.setValue(USER_NAME, "");

  m_stackDir = settings.value(STACK_DIR).toString();
  m_userName = settings.value(USER_NAME).toString();
}

//-----------------------------------------------------------------------------
void GeneralSettings::setStackDirectory(QString path)
{
  QSettings settings;

  m_stackDir = path;
  settings.setValue(STACK_DIR, m_stackDir);
}

//-----------------------------------------------------------------------------
void GeneralSettings::setUserName(QString name)
{
  QSettings settings;

  m_userName = name;
  settings.setValue(USER_NAME, m_userName);
}
