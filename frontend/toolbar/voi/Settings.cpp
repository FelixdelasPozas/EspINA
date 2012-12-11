/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#include "Settings.h"

#include <QSettings>
#include <settings/EspinaSettings.h>

const QString DEFAULT_VOI_X("RectangularVOI::DefaultVOI::X");
const QString DEFAULT_VOI_Y("RectangularVOI::DefaultVOI::Y");
const QString DEFAULT_VOI_Z("RectangularVOI::DefaultVOI::Z");

//------------------------------------------------------------------------
RectangularVOI::Settings::Settings()
{
  QSettings settings(CESVIMA, ESPINA);

  m_xSize   = settings.value(DEFAULT_VOI_X, 250).toInt();
  m_ySize   = settings.value(DEFAULT_VOI_Y, 250).toInt();
  m_zSize   = settings.value(DEFAULT_VOI_Z, 250).toInt();
}

//------------------------------------------------------------------------
void RectangularVOI::Settings::setXSize(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(DEFAULT_VOI_X, value);
  m_xSize = value;
}

//------------------------------------------------------------------------
void RectangularVOI::Settings::setYSize(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(DEFAULT_VOI_Y, value);
  m_ySize = value;
}

//------------------------------------------------------------------------
void RectangularVOI::Settings::setZSize(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(DEFAULT_VOI_Z, value);
  m_zSize = value;
}