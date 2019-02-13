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
#include <Support/Settings/Settings.h>
#include "SeedGrowSegmentationSettings.h"

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
  bool okX, okY, okZ, okRadius, okValue;

  settings.beginGroup(SGS_GROUP);
  m_xSize            = settings.value(ROI_X_SIZE_KEY, 500).toLongLong(&okX);
  m_ySize            = settings.value(ROI_Y_SIZE_KEY, 500).toLongLong(&okY);
  m_zSize            = settings.value(ROI_Z_SIZE_KEY, 500).toLongLong(&okZ);
  m_applyClose       = settings.value(APPLY_CLOSE, false).toBool();
  m_radius           = settings.value(CLOSE_RADIUS, 1).toInt(&okRadius);
  m_bestValue        = settings.value(BEST_PIXEL_VALUE, 0).toInt(&okValue);
  m_applyCategoryROI = settings.value(APPLY_ROI, true).toBool();
  settings.endGroup();

  blockSignals(true);
  if(!okX)      setXSize(500);
  if(!okY)      setYSize(500);
  if(!okZ)      setZSize(500);
  if(!okRadius) setCloseRadius(1);
  if(!okValue)  setBestPixelValue(0);
  blockSignals(false);
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setXSize(long long value)
{
  m_xSize = value;

  set<long long>(ROI_X_SIZE_KEY, value);
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setYSize(long long value)
{
  m_ySize = value;

  set<long long>(ROI_Y_SIZE_KEY, value);
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setZSize(long long value)
{
  m_zSize = value;

  set<long long>(ROI_Z_SIZE_KEY, value);
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

  emit closeRadiusChanged(value);
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setApplyClose(bool enable)
{
  m_applyClose = enable;

  set<bool>(APPLY_CLOSE, enable);

  emit applyCloseChanged(enable);
}
