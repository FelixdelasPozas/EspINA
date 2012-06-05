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


#ifndef VOLUMEOFINTERESTPREFERENCES_H
#define VOLUMEOFINTERESTPREFERENCES_H

#include <../frontend/IPreferencePanel.h>
#include "ui_VOIPreferences.h"

const QString DEFAULT_VOI_X("VolumeOfInterest::DefaultVOI::X");
const QString DEFAULT_VOI_Y("VolumeOfInterest::DefaultVOI::Y");
const QString DEFAULT_VOI_Z("VolumeOfInterest::DefaultVOI::Z");

class VolumeOfInterestPreferences;

class VolumeOfInterestPanel : public QWidget, public Ui::VOIPreferences
{
  Q_OBJECT
public:
  explicit VolumeOfInterestPanel(VolumeOfInterestPreferences* pref);
  virtual ~VolumeOfInterestPanel();
};

class VolumeOfInterestPreferences : public IPreferencePanel
{
    Q_OBJECT
public:
  VolumeOfInterestPreferences();
  virtual const QString shortDescription() {return "VOI";}
  virtual const QString longDescription() {return "Volume Of Interest Preferences";}
  virtual const QIcon icon() {return QIcon(":/roi.svg");}

  virtual QWidget* widget();
  
  int xSize(){return m_xSize;}
  int ySize(){return m_ySize;}
  int zSize(){return m_zSize;}
public slots:
  void setXSize(int value);
  void setYSize(int value);
  void setZSize(int value);
    
protected:
  int m_xSize, m_ySize, m_zSize;
};

#endif // VOLUMEOFINTERESTPREFERENCES_H
