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

// ESPINA
#include "SeedGrowSegmentationSettings.h"
#include <Support/Settings/EspinaSettings.h>

// Qt
#include <QSettings>
#include <QStringList>

using namespace ESPINA;

const QString BEST_PIXEL     ("SeedGrowSegmentation::BestPixelValue");
const QString CLOSING        ("SeedGrowSegmentation::Closing");

// new tags
const QString DEFAULT_ROI_X_SIZE("SeedGrowSegmentation::DefaultROI::X_SIZE");
const QString DEFAULT_ROI_Y_SIZE("SeedGrowSegmentation::DefaultROI::Y_SIZE");
const QString DEFAULT_ROI_Z_SIZE("SeedGrowSegmentation::DefaultROI::Z_SIZE");
const QString APPLY_CATEGORY_ROI("SeedGrowSegmentation::DefaultROI::APPLY_CATEGORY_ROI");

//------------------------------------------------------------------------
SeedGrowSegmentationSettings::SeedGrowSegmentationSettings()
{
  ESPINA_SETTINGS(settings);

  m_xSize            = settings.value(DEFAULT_ROI_X_SIZE,  500).toInt();
  m_ySize            = settings.value(DEFAULT_ROI_Y_SIZE,  500).toInt();
  m_zSize            = settings.value(DEFAULT_ROI_Z_SIZE,  500).toInt();
  m_closing          = settings.value(CLOSING           ,    0).toInt();
  m_bestValue        = settings.value(BEST_PIXEL        ,    0).toInt();
  m_applyCategoryROI = settings.value(APPLY_CATEGORY_ROI, true).toBool();
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setXSize(int value)
{
  ESPINA_SETTINGS(settings);

  settings.setValue(DEFAULT_ROI_X_SIZE, value);
  settings.sync();
  m_xSize = value;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setYSize(int value)
{
  ESPINA_SETTINGS(settings);

  settings.setValue(DEFAULT_ROI_Y_SIZE, value);
  settings.sync();
  m_ySize = value;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setZSize(int value)
{
  ESPINA_SETTINGS(settings);

  settings.setValue(DEFAULT_ROI_Z_SIZE, value);
  settings.sync();

  m_zSize = value;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setApplyCategoryROI(bool value)
{
  ESPINA_SETTINGS(settings);

  settings.setValue(APPLY_CATEGORY_ROI, value);
  settings.sync();

  m_applyCategoryROI = value;

  emit applyCategoryROIChanged(value);
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setBestPixelValue(int value)
{
  ESPINA_SETTINGS(settings);

  settings.setValue(BEST_PIXEL, value);
  settings.sync();

  m_bestValue = value;

  emit bestValueChanged(value);
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setClosing(int value)
{
  ESPINA_SETTINGS(settings);

  settings.setValue(CLOSING, value);
  settings.sync();

  m_closing = value;
}
