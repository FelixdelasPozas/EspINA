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

// ESPINA
#include "Utils.h"
#include <Core/Utils/EspinaException.h>

// Qt
#include <QStringList>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

//-----------------------------------------------------------------------------
void ESPINA::copySettings(std::shared_ptr<QSettings> from, std::shared_ptr<QSettings> to)
{
  for(auto key: from->childKeys())
  {
    to->setValue(key, from->value(key));
  }

  auto groups = from->childGroups();

  for(auto group: groups)
  {
    to->beginGroup(group);
    from->beginGroup(group);

    copySettings(from, to);

    from->endGroup();
    to->endGroup();
  }
}

//-----------------------------------------------------------------------------
QDebug ESPINA::operator<< (QDebug debug, std::shared_ptr<QSettings> settings)
{
  debug << "\n---- QSettings ----\n";

  for(auto key: settings->allKeys())
  {
    debug << "Key:" << key << "->" << settings->value(key) << "\n";
  }

  debug << "-------------------\n";

  return debug;
}

//-----------------------------------------------------------------------------
SettingsContainer::SettingsContainer()
{
  if(!m_file.open())
  {
    auto message = QObject::tr("Unable to open temporary file.");
    auto details = QObject::tr("SettingsContainer::constructor() -> ") + message;

    throw EspinaException(message, details);
  }
  m_file.close();

  m_settings = std::make_shared<QSettings>(m_file.fileName(), QSettings::IniFormat);
}

//-----------------------------------------------------------------------------
std::shared_ptr<QSettings> SettingsContainer::settings() const
{
  return m_settings;
}

//-----------------------------------------------------------------------------
void SettingsContainer::copyFrom(std::shared_ptr<QSettings> settings)
{
  copySettings(settings, m_settings);
}

//-----------------------------------------------------------------------------
void SettingsContainer::copyTo(std::shared_ptr<QSettings> settings)
{
  copySettings(m_settings, settings);
}
