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

#include "SettingsPanel.h"

#include <common/selection/PixelSelector.h>
#include "Settings.h"

#include <QSettings>
#include <QString>
#include <QDebug>


//------------------------------------------------------------------------
SeedGrowSegmentation::SettingsPanel::SettingsPanel(SeedGrowSegmentation::Settings* settings)
: m_settings(settings)
{
  setupUi(this);

  connect(pixelValue,SIGNAL(valueChanged(int)),
	  this, SLOT(displayColor(int)));

  pixelValue->setValue(settings->bestPixelValue());
  displayColor(pixelValue->value());
  xSize->setValue(settings->xSize());
  ySize->setValue(settings->ySize());
  zSize->setValue(settings->zSize());
}

//------------------------------------------------------------------------
void SeedGrowSegmentation::SettingsPanel::acceptChanges()
{
  m_settings->setBestPixelValue(pixelValue->value());
  m_settings->setXSize(xSize->value());
  m_settings->setYSize(ySize->value());
  m_settings->setZSize(zSize->value());
}

//------------------------------------------------------------------------
bool SeedGrowSegmentation::SettingsPanel::modified() const
{
  return xSize->value() != m_settings->xSize()
  || ySize->value() != m_settings->ySize()
  || zSize->value() != m_settings->zSize()
  || pixelValue->value() != m_settings->bestPixelValue();
}


//------------------------------------------------------------------------
ISettingsPanel *SeedGrowSegmentation::SettingsPanel::widget()
{
  SettingsPanel *panel = new SettingsPanel(m_settings);
  return panel;
}

//------------------------------------------------------------------------
void SeedGrowSegmentation::SettingsPanel::displayColor(int value)
{
  QPixmap pic(32,32);
  pic.fill(QColor(value,value,value));
  colorSample->setPixmap(pic);
  colorSample->setToolTip(QString("Pixel Value: %1").arg(value));
  pixelValue->setToolTip(QString("Pixel Value: %1").arg(value));
}
