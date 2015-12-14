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

#include <Support/Settings/Settings.h>
#include "AutoSave.h"


using namespace ESPINA;

const QString AUTOSAVE_PATH     = "Autosave::Path";
const QString AUTOSAVE_INTERVAL = "Autosave::Interval";

//------------------------------------------------------------------------
AutoSave::AutoSave()
{
  ESPINA_SETTINGS(settings);

  setInterval(settings.value(AUTOSAVE_INTERVAL, 10).toUInt());
  setPath(settings.value(AUTOSAVE_PATH, QDir::homePath()+"/.espina").toString());

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
  settings.setValue(AUTOSAVE_PATH, path.absolutePath());
}

//------------------------------------------------------------------------
void AutoSave::setInterval(const unsigned int minutes)
{
  m_timer.setInterval(minutes*60*1000);

  ESPINA_SETTINGS(settings);
  settings.setValue(AUTOSAVE_INTERVAL, minutes);
}

//------------------------------------------------------------------------
int AutoSave::interval() const
{
  return m_timer.interval()/60/1000;
}

//------------------------------------------------------------------------
void AutoSave::resetCountDown()
{
  m_timer.start();
}

//------------------------------------------------------------------------
bool AutoSave::canRestore()
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
bool AutoSave::isAutoSaveFile(const QString& filename)
{
  return filename == autosaveFile();
}
