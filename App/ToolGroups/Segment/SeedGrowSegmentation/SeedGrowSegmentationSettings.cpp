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

using namespace ESPINA;

const QString SeedGrowSegmentationSettings::SGS_GROUP = "SeedGrowSegmentationSettings";

const QString ROI_X_SIZE_KEY   = "ROI X size";
const QString ROI_Y_SIZE_KEY   = "ROI Y size";
const QString ROI_Z_SIZE_KEY   = "ROI Z size";
const QString APPLY_CLOSE      = "Apply Close";
const QString CLOSE_RADIUS     = "Close Radius";
const QString BEST_PIXEL_VALUE = "Best Pixel Value";
const QString APPLY_ROI        = "Apply ROI";

//------------------------------------------------------------------------
SeedGrowSegmentationSettings::SeedGrowSegmentationSettings()
{
  ESPINA_SETTINGS(settings);

  settings.beginGroup(SGS_GROUP);
  m_xSize            = settings.value(ROI_X_SIZE_KEY, 500).toInt();
  m_ySize            = settings.value(ROI_Y_SIZE_KEY, 500).toInt();
  m_zSize            = settings.value(ROI_Z_SIZE_KEY, 500).toInt();
  m_applyClose       = settings.value(APPLY_CLOSE, false).toBool();
  m_radius           = settings.value(CLOSE_RADIUS, 0).toInt();
  m_bestValue        = settings.value(BEST_PIXEL_VALUE, 0).toInt();
  m_applyCategoryROI = settings.value(APPLY_ROI, true).toBool();
  settings.endGroup();
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setXSize(int value)
{
  m_xSize = value;

  set<int>(ROI_X_SIZE_KEY, value);
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setYSize(int value)
{
  m_ySize = value;

  set<int>(ROI_Y_SIZE_KEY, value);
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setZSize(int value)
{
  m_zSize = value;

  set<int>(ROI_Z_SIZE_KEY, value);
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setApplyCategoryROI(bool value)
{
  m_applyCategoryROI = value;

  set<bool>(APPLY_ROI, value);

  emit applyCategoryROIChanged(value);
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setBestPixelValue(int value)
{
  m_bestValue = value;

  set<int>(BEST_PIXEL_VALUE, value);

  emit bestValueChanged(value);
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setCloseRadius(int value)
{
  m_radius = value;

  set<int>(CLOSE_RADIUS, value);
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setApplyClose(bool enable)
{
  m_applyClose = enable;

  set<bool>(APPLY_CLOSE, enable);
}
