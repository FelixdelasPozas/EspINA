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


#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#include <QObject>
#include <QString>
#include <QDir>

class QSettings;
const QString AUTOSAVE_PATH("Autosave::Path");
const QString AUTOSAVE_INTERVAL("Autosave::Interval");
const QString STACK_DIR("StackDirectory");
const QString USER_NAME("UserName");

class GeneralSettings
{
public:
  explicit GeneralSettings();

  //QString stackDirectory() const {return m_stackDir;}
  //void setStackDirectory(QString path);

  QString userName() const {return m_userName;}
  void setUserName(QString name);

  int autosaveInterval() const {return m_autosaveInterval;}
  void setAutosaveInterval(int min);

  QDir autosavePath() const {return m_autosavePath;}
  void setAutosavePath(const QString path);

private:
  QSettings *m_settings;

  QString m_stackDir;
  QString m_userName;
  int     m_autosaveInterval;
  QDir    m_autosavePath;
};

#endif // GENERALSETTINGS_H
