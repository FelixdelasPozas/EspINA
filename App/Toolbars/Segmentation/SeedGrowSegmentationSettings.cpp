/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

// EspINA
#include "SeedGrowSegmentationSettings.h"

// Qt
#include <QSettings>
#include <Core/EspinaSettings.h>
#include <GUI/Pickers/PixelPicker.h>
#include <QStringList>

using namespace EspINA;

const QString BEST_PIXEL     ("SeedGrowSegmentation::BestPixelValue");
const QString CLOSING        ("SeedGrowSegmentation::Closing");

// TODO: 2013-01-10 set to default final tags after one release
// old tags, last one included to fix a typography error
const QString DEFAULT_VOI_X  ("SeedGrowSegmentation::DafaultVOI::X");
const QString DEFAULT_VOI_Y  ("SeedGrowSegmentation::DafaultVOI::Y");
const QString DEFAULT_VOI_Z  ("SeedGrowSegmentation::DafaultVOI::Z");
const QString TAXONOMICAL_VOI("SeedGrowSegmentation::DafaultVOI::USE_TAXONOMY_SIZE");

// new tags
const QString DEFAULT_VOI_X_SIZE   ("SeedGrowSegmentation::DefaultVOI::X_SIZE");
const QString DEFAULT_VOI_Y_SIZE   ("SeedGrowSegmentation::DefaultVOI::Y_SIZE");
const QString DEFAULT_VOI_Z_SIZE   ("SeedGrowSegmentation::DefaultVOI::Z_SIZE");
const QString TAXONOMICAL_VOI_USAGE("SeedGrowSegmentation::DefaultVOI::USE_TAXONOMY_SIZE");

//------------------------------------------------------------------------
SeedGrowSegmentationSettings::SeedGrowSegmentationSettings(BestPixelPicker *selector)
: m_selector(selector)
{
  QSettings settings(CESVIMA, ESPINA);

  m_selector->setBestPixelValue(settings.value(BEST_PIXEL, 0).toInt());
  m_closing = settings.value(CLOSING, 0).toInt();

  // TODO: 2013-01-10 set to default final tags after one release
  if (settings.allKeys().contains(DEFAULT_VOI_X))
  {
    m_xSize = 2 * settings.value(DEFAULT_VOI_X, 250).toInt();
    m_ySize = 2 * settings.value(DEFAULT_VOI_Y, 250).toInt();
    m_zSize = 2 * settings.value(DEFAULT_VOI_Z, 250).toInt();
    m_taxonomicalVOI = settings.value(TAXONOMICAL_VOI, true).toBool();

    settings.remove(DEFAULT_VOI_X);
    settings.remove(DEFAULT_VOI_Y);
    settings.remove(DEFAULT_VOI_Z);
    settings.remove(TAXONOMICAL_VOI);

    settings.setValue(DEFAULT_VOI_X_SIZE, m_xSize);
    settings.setValue(DEFAULT_VOI_Y_SIZE, m_ySize);
    settings.setValue(DEFAULT_VOI_Z_SIZE, m_zSize);
    settings.setValue(TAXONOMICAL_VOI_USAGE, m_taxonomicalVOI);

    settings.sync();
  }
  else
  {
    m_xSize = settings.value(DEFAULT_VOI_X_SIZE, 500).toInt();
    m_ySize = settings.value(DEFAULT_VOI_Y_SIZE, 500).toInt();
    m_zSize = settings.value(DEFAULT_VOI_Z_SIZE, 500).toInt();
    m_taxonomicalVOI = settings.value(TAXONOMICAL_VOI_USAGE, true).toBool();
  }
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setXSize(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(DEFAULT_VOI_X_SIZE, value);
  m_xSize = value;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setYSize(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(DEFAULT_VOI_Y_SIZE, value);
  m_ySize = value;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setZSize(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(DEFAULT_VOI_Z_SIZE, value);
  m_zSize = value;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setTaxonomicalVOI(bool value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(TAXONOMICAL_VOI_USAGE, value);
  m_taxonomicalVOI = value;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setBestPixelValue(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(BEST_PIXEL, value);
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
  m_closing = value;
}
