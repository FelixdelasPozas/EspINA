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


#include "Settings.h"

#include <QSettings>
#include <Core/EspinaSettings.h>
#include <GUI/Pickers/PixelPicker.h>

const QString BEST_PIXEL     ("SeedGrowSegmentation::BestPixelValue");
const QString DEFAULT_VOI_X  ("SeedGrowSegmentation::DafaultVOI::X");
const QString DEFAULT_VOI_Y  ("SeedGrowSegmentation::DafaultVOI::Y");
const QString DEFAULT_VOI_Z  ("SeedGrowSegmentation::DafaultVOI::Z");
const QString TAXONOMICAL_VOI("SeedGrowSegmentation::DafaultVOI::USE_TAXONOMY_SIZE");
const QString CLOSING        ("SeedGrowSegmentation::Closing");


//------------------------------------------------------------------------
SeedGrowSegmentation::Settings::Settings(BestPixelSelector *selector)
: m_selector(selector) 
{
  QSettings settings(CESVIMA, ESPINA);

  m_selector->setBestPixelValue(settings.value(BEST_PIXEL, 0).toInt());
  m_xSize   = settings.value(DEFAULT_VOI_X, 250).toInt();
  m_ySize   = settings.value(DEFAULT_VOI_Y, 250).toInt();
  m_zSize   = settings.value(DEFAULT_VOI_Z, 250).toInt();
  m_taxonomicalVOI = settings.value(TAXONOMICAL_VOI, true).toBool();
  m_closing = settings.value(CLOSING, 0).toInt();
}

//------------------------------------------------------------------------
void SeedGrowSegmentation::Settings::setXSize(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(DEFAULT_VOI_X, value);
  m_xSize = value;
}

//------------------------------------------------------------------------
void SeedGrowSegmentation::Settings::setYSize(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(DEFAULT_VOI_Y, value);
  m_ySize = value;
}

//------------------------------------------------------------------------
void SeedGrowSegmentation::Settings::setZSize(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(DEFAULT_VOI_Z, value);
  m_zSize = value;
}

//------------------------------------------------------------------------
void SeedGrowSegmentation::Settings::setTaxonomicalVOI(bool value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(TAXONOMICAL_VOI, value);
  m_taxonomicalVOI = value;
}

//------------------------------------------------------------------------
void SeedGrowSegmentation::Settings::setBestPixelValue(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(BEST_PIXEL, value);
  m_selector->setBestPixelValue(value);
}

//------------------------------------------------------------------------
int SeedGrowSegmentation::Settings::bestPixelValue() const
{
  QSettings settings(CESVIMA, ESPINA);

  int bestValue = settings.value(BEST_PIXEL).toInt();
  Q_ASSERT(bestValue >= 0 && bestValue <= 255);
  return bestValue;
}

//------------------------------------------------------------------------
void SeedGrowSegmentation::Settings::setClosing(int value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(CLOSING, value);
  m_closing = value;
}