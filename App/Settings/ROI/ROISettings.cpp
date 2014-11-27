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
#include "ROISettings.h"
#include <Support/Settings/EspinaSettings.h>

// Qt
#include <QSettings>

using namespace ESPINA;

//------------------------------------------------------------------------
ROISettings::ROISettings()
{
  ESPINA_SETTINGS(settings);
  settings.beginGroup(ROI_SETTINGS_GROUP);

  m_xSize   = settings.value(DEFAULT_ROI_X, 500).toInt();
  m_ySize   = settings.value(DEFAULT_ROI_Y, 500).toInt();
  m_zSize   = settings.value(DEFAULT_ROI_Z, 500).toInt();
}

//------------------------------------------------------------------------
void ROISettings::setXSize(int value)
{
  ESPINA_SETTINGS(settings);
  settings.beginGroup(ROI_SETTINGS_GROUP);

  settings.setValue(DEFAULT_ROI_X, value);
  settings.sync();

  m_xSize = value;
}

//------------------------------------------------------------------------
void ROISettings::setYSize(int value)
{
  ESPINA_SETTINGS(settings);
  settings.beginGroup(ROI_SETTINGS_GROUP);

  settings.setValue(DEFAULT_ROI_Y, value);
  settings.sync();

  m_ySize = value;
}

//------------------------------------------------------------------------
void ROISettings::setZSize(int value)
{
  ESPINA_SETTINGS(settings);
  settings.beginGroup(ROI_SETTINGS_GROUP);

  settings.setValue(DEFAULT_ROI_Z, value);
  settings.sync();

  m_zSize = value;
}
