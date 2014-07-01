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

#include "ROISettings.h"
#include <QSettings>
#include <Support/Settings/EspinaSettings.h>

using namespace EspINA;

//------------------------------------------------------------------------
ROISettings::ROISettings()
{
  QSettings settings(CESVIMA, ESPINA);
  settings.beginGroup(ROI_SETTINGS_GROUP);

  m_xSize   = settings.value(DEFAULT_VOI_X, 500).toInt();
  m_ySize   = settings.value(DEFAULT_VOI_Y, 500).toInt();
  m_zSize   = settings.value(DEFAULT_VOI_Z, 500).toInt();
}

//------------------------------------------------------------------------
void ROISettings::setXSize(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(DEFAULT_VOI_X, value);
  m_xSize = value;
}

//------------------------------------------------------------------------
void ROISettings::setYSize(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(DEFAULT_VOI_Y, value);
  m_ySize = value;
}

//------------------------------------------------------------------------
void ROISettings::setZSize(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(DEFAULT_VOI_Z, value);
  m_zSize = value;
}
