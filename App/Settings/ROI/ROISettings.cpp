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
  m_xSize   = 500;
  m_ySize   = 500;
  m_zSize   = 500;
}

//------------------------------------------------------------------------
void ROISettings::setXSize(int value)
{
  m_xSize = value;
}

//------------------------------------------------------------------------
void ROISettings::setYSize(int value)
{
  m_ySize = value;
}

//------------------------------------------------------------------------
void ROISettings::setZSize(int value)
{
  m_zSize = value;
}
