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

#include "SeedGrowSegmentationPreferences.h"


#include <QSettings>
#include <QString>
#include <pixelSelector.h>

// const QString BEST_PIXEL("SeedGrowSegmentation::BestPixel");

//------------------------------------------------------------------------
SeedGrowSegmentationPanel::SeedGrowSegmentationPanel(SeedGrowSegmentationSettings* pref)
{
  setupUi(this);
  
  connect(pixelValue,SIGNAL(valueChanged(int)),
	  pref, SLOT(setBestPixelValue(int)));
  connect(xSize,SIGNAL(valueChanged(int)),
	  pref, SLOT(setXSize(int)));
  connect(ySize,SIGNAL(valueChanged(int)),
	  pref, SLOT(setYSize(int)));
  connect(zSize,SIGNAL(valueChanged(int)),
	  pref, SLOT(setZSize(int)));
  connect(pixelValue,SIGNAL(valueChanged(int)),
	  this, SLOT(setBestPixelValue(int)));

  QSettings settings;
  
  int bestPixel = settings.value(BEST_PIXEL).toInt();
  pixelValue->setValue(bestPixel);
  setBestPixelValue(bestPixel); //To prevent non initialization of black
  
  xSize->setValue(pref->xSize());
  ySize->setValue(pref->ySize());
  zSize->setValue(pref->zSize());
}

//------------------------------------------------------------------------
SeedGrowSegmentationPanel::~SeedGrowSegmentationPanel()
{
}

//------------------------------------------------------------------------
void SeedGrowSegmentationPanel::setBestPixelValue(int value)
{
  QSettings settings;
  
  settings.setValue(BEST_PIXEL, value);  
  QPixmap pic(32,32);
  pic.fill(QColor(value,value,value));
  colorSample->setPixmap(pic);
  colorSample->setToolTip(QString("Pixel Value: %1").arg(value));
  pixelValue->setToolTip(QString("Pixel Value: %1").arg(value));
  
}

SeedGrowSegmentationSettings::SeedGrowSegmentationSettings(BestPixelSelector* sel)
: m_selector(sel)
{
  QSettings settings;
  
  if (!settings.allKeys().contains(BEST_PIXEL))
    settings.setValue(BEST_PIXEL,0);
  if (!settings.allKeys().contains(DEFAULT_VOI_X))
    settings.setValue(DEFAULT_VOI_X,60);
  if (!settings.allKeys().contains(DEFAULT_VOI_Y))
    settings.setValue(DEFAULT_VOI_Y,60);
  if (!settings.allKeys().contains(DEFAULT_VOI_Z))
    settings.setValue(DEFAULT_VOI_Z,60);

  m_selector->setBestPixelValue(settings.value(BEST_PIXEL).toInt());
  m_xSize = settings.value(DEFAULT_VOI_X).toInt();
  m_ySize = settings.value(DEFAULT_VOI_Y).toInt();
  m_zSize = settings.value(DEFAULT_VOI_Z).toInt();
}


void SeedGrowSegmentationSettings::setXSize(int value)
{
  QSettings settings;
  
  settings.setValue(DEFAULT_VOI_X,value);
  m_xSize = value;
  
}

void SeedGrowSegmentationSettings::setYSize(int value)
{
  QSettings settings;
  
  settings.setValue(DEFAULT_VOI_Y,value);
  m_ySize = value;
}

void SeedGrowSegmentationSettings::setZSize(int value)
{
  QSettings settings;
  
  settings.setValue(DEFAULT_VOI_Z,value);
  m_zSize = value;
}


void SeedGrowSegmentationSettings::setBestPixelValue(int value)
{
  m_selector->setBestPixelValue(value);
}


QWidget* SeedGrowSegmentationSettings::widget()
{
  SeedGrowSegmentationPanel *panel = new SeedGrowSegmentationPanel(this);
  return panel;
}

