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

using namespace ESPINA;

const unsigned int DEFAULT_ROI_SIZE      = 500;
const unsigned int DEFAULT_CLOSING_VALUE = 0;
const unsigned int DEFAULT_BEST_PIXEL    = 0;
const bool         DEFAULT_APPLY_ROI     = true;

//------------------------------------------------------------------------
SeedGrowSegmentationSettings::SeedGrowSegmentationSettings()
{
  m_xSize            = DEFAULT_ROI_SIZE;
  m_ySize            = DEFAULT_ROI_SIZE;
  m_zSize            = DEFAULT_ROI_SIZE;
  m_closing          = DEFAULT_CLOSING_VALUE;
  m_bestValue        = DEFAULT_BEST_PIXEL;
  m_applyCategoryROI = DEFAULT_APPLY_ROI;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setXSize(int value)
{
  m_xSize = value;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setYSize(int value)
{
  m_ySize = value;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setZSize(int value)
{
  m_zSize = value;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setApplyCategoryROI(bool value)
{
  m_applyCategoryROI = value;

  emit applyCategoryROIChanged(value);
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setBestPixelValue(int value)
{
  m_bestValue = value;

  emit bestValueChanged(value);
}

//------------------------------------------------------------------------
void SeedGrowSegmentationSettings::setClosing(int value)
{
  m_closing = value;
}
