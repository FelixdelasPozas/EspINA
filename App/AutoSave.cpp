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

// ESPINA
#include "AutoSave.h"
#include <Support/Settings/Settings.h>

// Qt
#include <QDateTime>

using namespace ESPINA;

const QString AutoSave::PATH     = "Autosave::Path";
const QString AutoSave::INTERVAL = "Autosave::Interval";
const QString AutoSave::INTHREAD = "Autosave::InThread";

//------------------------------------------------------------------------
AutoSave::AutoSave()
{
  ESPINA_SETTINGS(settings);

  setInterval(settings.value(INTERVAL, 10).toUInt());
  setPath(settings.value(PATH, QDir::homePath()+"/.espina").toString());
  m_inThread = settings.value(INTHREAD, true).toBool();

  connect(&m_timer, SIGNAL(timeout()),
          this,     SLOT(autoSave()));
}

//------------------------------------------------------------------------
AutoSave::~AutoSave()
{
  disconnect(&m_timer, SIGNAL(timeout()),
             this,     SLOT(autoSave()));

  if(m_timer.isActive())
  {
    m_timer.stop();
  }

  clear();
}

//------------------------------------------------------------------------
void AutoSave::setPath(const QDir& path)
{
  if (!path.exists())
  {
    path.mkpath(path.absolutePath());
  }

  m_path = path;

  ESPINA_SETTINGS(settings);
  settings.setValue(PATH, path.absolutePath());
}

//------------------------------------------------------------------------
void AutoSave::setInterval(const unsigned int minutes)
{
  auto value = minutes*60*1000;
  if(interval() != value)
  {
    m_timer.setInterval(value);

    ESPINA_SETTINGS(settings);
    settings.setValue(INTERVAL, minutes);
  }
}

//------------------------------------------------------------------------
const int AutoSave::interval() const
{
  return m_timer.interval()/60/1000;
}

//------------------------------------------------------------------------
void AutoSave::resetCountDown()
{
  m_timer.start();
}

//------------------------------------------------------------------------
bool AutoSave::canRestore() const
{
  return m_path.exists(autosaveFile());
}

//------------------------------------------------------------------------
void AutoSave::restore()
{
  resetCountDown();

  emit restoreFromFile(autosaveFile());
}

//------------------------------------------------------------------------
void AutoSave::clear()
{
  m_path.remove(autosaveFile());
}

//------------------------------------------------------------------------
void AutoSave::autoSave()
{
  resetCountDown();

  emit saveToFile(autosaveFile());
}

//------------------------------------------------------------------------
QString AutoSave::autosaveFile() const
{
  return m_path.absoluteFilePath("espina-autosave.seg");
}

//------------------------------------------------------------------------
const bool AutoSave::isAutoSaveFile(const QString& filename)
{
  return filename == autosaveFile();
}

//------------------------------------------------------------------------
const QString AutoSave::autoSaveDate() const
{
  QString dateString;

  if(canRestore())
  {
    QFileInfo info{autosaveFile()};
    auto dateTime = info.lastModified();

    dateString = dateTime.date().toString("dddd MMMM d yyyy");
  }

  return dateString;
}

//------------------------------------------------------------------------
const QString AutoSave::autoSaveTime() const
{
  QString timeString;

  if(canRestore())
  {
    QFileInfo info{autosaveFile()};
    auto dateTime = info.lastModified();

    timeString = dateTime.time().toString("H:mm:ss");
  }

  return timeString;
}

//------------------------------------------------------------------------
void AutoSave::setSaveInThread(const bool value)
{
  if(m_inThread != value)
  {
    m_inThread = value;

    ESPINA_SETTINGS(settings);

    settings.setValue(INTHREAD, value);
  }
}
