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

// EspINA
#include "SeedGrowSegmentationSettings.h"

// Qt
#include <QSettings>
#include <Core/EspinaSettings.h>
#include <GUI/Pickers/PixelSelector.h>
#include <QStringList>

using namespace EspINA;

const QString BEST_PIXEL     ("SeedGrowSegmentation::BestPixelValue");
const QString CLOSING        ("SeedGrowSegmentation::Closing");

// new tags
const QString DEFAULT_ROI_X_SIZE   ("SeedGrowSegmentation::DefaultROI::X_SIZE");
const QString DEFAULT_ROI_Y_SIZE   ("SeedGrowSegmentation::DefaultROI::Y_SIZE");
const QString DEFAULT_ROI_Z_SIZE   ("SeedGrowSegmentation::DefaultROI::Z_SIZE");
const QString TAXONOMICAL_ROI_USAGE("SeedGrowSegmentation::DefaultROI::USE_TAXONOMY_SIZE");

//------------------------------------------------------------------------
SeedGrowSegmentationSettings::SeedGrowSegmentationSettings(BestPixelSelector *selector)
: m_selector(selector)
{
  QSettings settings(CESVIMA, ESPINA);

  m_selector->setBestPixelValue(settings.value(BEST_PIXEL, 0).toInt());
  m_closing = settings.value(CLOSING, 0).toInt();

  m_xSize = settings.value(DEFAULT_ROI_X_SIZE, 500).toInt();
  m_ySize = settings.value(DEFAULT_ROI_Y_SIZE, 500).toInt();
  m_zSize = settings.value(DEFAULT_ROI_Z_SIZE, 500).toInt();
  m_taxonomicalROI = settings.value(TAXONOMICAL_ROI_USAGE, true).toBool();
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setXSize(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(DEFAULT_ROI_X_SIZE, value);
  settings.sync();
  m_xSize = value;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setYSize(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(DEFAULT_ROI_Y_SIZE, value);
  settings.sync();
  m_ySize = value;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setZSize(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(DEFAULT_ROI_Z_SIZE, value);
  settings.sync();
  m_zSize = value;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setTaxonomicalROI(bool value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(TAXONOMICAL_ROI_USAGE, value);
  settings.sync();
  m_taxonomicalROI = value;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setBestPixelValue(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(BEST_PIXEL, value);
  settings.sync();
  m_selector->setBestPixelValue(value);
}

//------------------------------------------------------------------------
int SeedGrowSegmentationSettings::bestPixelValue() const
{
  QSettings settings(CESVIMA, ESPINA);

  int bestValue = settings.value(BEST_PIXEL).toInt();
  Q_ASSERT(bestValue >= 0 && bestValue <= 255);
  return bestValue;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setClosing(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(CLOSING, value);
  settings.sync();
  m_closing = value;
}
