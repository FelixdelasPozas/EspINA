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
#include <Support/Settings/Settings.h>
#include "ROISettings.h"
#include <QSettings>

using namespace ESPINA;

//------------------------------------------------------------------------
ROISettings::ROISettings()
{
  ESPINA_SETTINGS(settings);

  settings.beginGroup(ROI_SETTINGS_GROUP);
  m_xSize = settings.value(DEFAULT_ROI_X_SIZE_KEY, 500).toLongLong();
  m_ySize = settings.value(DEFAULT_ROI_Y_SIZE_KEY, 500).toLongLong();
  m_zSize = settings.value(DEFAULT_ROI_Z_SIZE_KEY, 500).toLongLong();
  settings.endGroup();
}

//------------------------------------------------------------------------
void ROISettings::setXSize(long long value)
{
  m_xSize = value;

  set(DEFAULT_ROI_X_SIZE_KEY, value);
}

//------------------------------------------------------------------------
void ROISettings::setYSize(long long value)
{
  m_ySize = value;

  set(DEFAULT_ROI_Y_SIZE_KEY, value);
}

//------------------------------------------------------------------------
void ROISettings::setZSize(long long value)
{
  m_zSize = value;

  set(DEFAULT_ROI_Z_SIZE_KEY, value);
}

//------------------------------------------------------------------------
void ROISettings::set(const QString& key, long long value)
{
  ESPINA_SETTINGS(settings);

  settings.beginGroup(ROI_SETTINGS_GROUP);
  settings.setValue(key, value);
  settings.endGroup();

  settings.sync();
}
