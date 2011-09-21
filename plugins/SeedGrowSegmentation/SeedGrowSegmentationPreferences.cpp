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
SeedGrowSegmentationPanel::SeedGrowSegmentationPanel(BestPixelSelector *selector)
: m_selector(selector)
{
  setupUi(this);
  
  connect(pixelValue,SIGNAL(valueChanged(int)),
	  this, SLOT(setBestPixelValue(int)));

  QSettings settings;
  
  if (!settings.allKeys().contains(BEST_PIXEL))
  {
    settings.setValue(BEST_PIXEL,0);
  }
  
  int bestPixel = settings.value(BEST_PIXEL).toInt();
  pixelValue->setValue(bestPixel);
  setBestPixelValue(bestPixel); //To prevent non initialization of black
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
  m_selector->setBestPixelValue(value);
  colorSample->setToolTip(QString("Pixel Value: %1").arg(value));
  pixelValue->setToolTip(QString("Pixel Value: %1").arg(value));
}


QWidget* SeedGrowSegmentationPreferences::widget()
{
  SeedGrowSegmentationPanel *panel = new SeedGrowSegmentationPanel(m_selector);
  return panel;
}

