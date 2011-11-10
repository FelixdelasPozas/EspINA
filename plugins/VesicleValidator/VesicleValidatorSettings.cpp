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


#include "VesicleValidatorSettings.h"

#include <QSettings>

const QString DEFAULT_SVA_X("VesicleValidator::DefaultSVA::X");
const QString DEFAULT_SVA_Y("VesicleValidator::DefaultSVA::Y");
const QString DEFAULT_SVA_Z("VesicleValidator::DefaultSVA::Z");

//-----------------------------------------------------------------------------
VesicleValidatorSettings::VesicleValidatorSettings()
{
  QSettings settings;
  
  if (!settings.allKeys().contains(DEFAULT_SVA_X))
    settings.setValue(DEFAULT_SVA_X,60);
  if (!settings.allKeys().contains(DEFAULT_SVA_Y))
    settings.setValue(DEFAULT_SVA_Y,60);
  if (!settings.allKeys().contains(DEFAULT_SVA_Z))
    settings.setValue(DEFAULT_SVA_Z,1);

  m_xSize = settings.value(DEFAULT_SVA_X).toInt();
  m_ySize = settings.value(DEFAULT_SVA_Y).toInt();
  m_zSize = settings.value(DEFAULT_SVA_Z).toInt();
}

//-----------------------------------------------------------------------------
void VesicleValidatorSettings::setXSize(int value)
{
  QSettings settings;
  
  settings.setValue(DEFAULT_SVA_X,value);
  m_xSize = value;
}

//-----------------------------------------------------------------------------
void VesicleValidatorSettings::setYSize(int value)
{
  QSettings settings;
  
  settings.setValue(DEFAULT_SVA_Y,value);
  m_ySize = value;
}

//-----------------------------------------------------------------------------
void VesicleValidatorSettings::setZSize(int value)
{
  QSettings settings;
  
  settings.setValue(DEFAULT_SVA_Z,value);
  m_zSize = value;
}


//-----------------------------------------------------------------------------
QWidget* VesicleValidatorSettings::widget()
{
  return new QWidget();
}