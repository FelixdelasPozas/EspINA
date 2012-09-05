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


#include "VolumeOfInterestPreferences.h"

#include <QSettings>


VolumeOfInterestPanel::VolumeOfInterestPanel(VolumeOfInterestPreferences* pref): QWidget(pref)
{
  setupUi(this);
  
  connect(xSize,SIGNAL(valueChanged(int)),
	  pref, SLOT(setXSize(int)));
  connect(ySize,SIGNAL(valueChanged(int)),
	  pref, SLOT(setYSize(int)));
  connect(zSize,SIGNAL(valueChanged(int)),
	  pref, SLOT(setZSize(int)));
  
  
  xSize->setValue(pref->xSize());
  ySize->setValue(pref->ySize());
  zSize->setValue(pref->zSize());
}

VolumeOfInterestPanel::~VolumeOfInterestPanel()
{

}

VolumeOfInterestPreferences::VolumeOfInterestPreferences()
{
  QSettings settings;
  
  if (!settings.allKeys().contains(DEFAULT_VOI_X))
    settings.setValue(DEFAULT_VOI_X,250);
  if (!settings.allKeys().contains(DEFAULT_VOI_Y))
    settings.setValue(DEFAULT_VOI_Y,250);
  if (!settings.allKeys().contains(DEFAULT_VOI_Z))
    settings.setValue(DEFAULT_VOI_Z,250);

  m_xSize = settings.value(DEFAULT_VOI_X).toInt();
  m_ySize = settings.value(DEFAULT_VOI_Y).toInt();
  m_zSize = settings.value(DEFAULT_VOI_Z).toInt();
}


QWidget* VolumeOfInterestPreferences::widget()
{
  VolumeOfInterestPanel *panel = new VolumeOfInterestPanel(this);
  return panel;
}

void VolumeOfInterestPreferences::setXSize(int value)
{
  QSettings settings;
  
  settings.setValue(DEFAULT_VOI_X,value);
  m_xSize = value;
}

void VolumeOfInterestPreferences::setYSize(int value)
{
  QSettings settings;
  
  settings.setValue(DEFAULT_VOI_Y,value);
  m_ySize = value;
}


void VolumeOfInterestPreferences::setZSize(int value)
{
  QSettings settings;
  
  settings.setValue(DEFAULT_VOI_Z,value);
  m_zSize = value;
}
