/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <GUI/Representations/Settings/SegmentationSkeletonPoolSettings.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations;

const QString SegmentationSkeletonPoolSettings::WIDTH   = "Width";
const QString SegmentationSkeletonPoolSettings::SHOWIDS = "Show Id";
const QString SegmentationSkeletonPoolSettings::OPACITY = "Opacity";

//--------------------------------------------------------------------
SegmentationSkeletonPoolSettings::SegmentationSkeletonPoolSettings()
{
  setOpacity(0.6);
  setWidth(2);
  setShowAnnotations(false);
}

//--------------------------------------------------------------------
void SegmentationSkeletonPoolSettings::setWidth(const int width)
{
  set<int>(WIDTH, width);
}

//--------------------------------------------------------------------
int SegmentationSkeletonPoolSettings::width() const
{
  return get<int>(WIDTH);
}

//--------------------------------------------------------------------
void SegmentationSkeletonPoolSettings::setShowAnnotations(bool value)
{
  set<bool>(SHOWIDS, value);
}

//--------------------------------------------------------------------
bool SegmentationSkeletonPoolSettings::showAnnotations() const
{
  return get<bool>(SHOWIDS);
}

//--------------------------------------------------------------------
void SegmentationSkeletonPoolSettings::setOpacity(const double opacity)
{
  set<double>(OPACITY, opacity);
}

//--------------------------------------------------------------------
double SegmentationSkeletonPoolSettings::opacity() const
{
  return get<double>(OPACITY);
}

//--------------------------------------------------------------------
int SegmentationSkeletonPoolSettings::getWidth(const RepresentationState& state)
{
  if(!state.hasValue(WIDTH)) return 1;

  return state.getValue<int>(WIDTH);
}

//--------------------------------------------------------------------
double SegmentationSkeletonPoolSettings::getOpacity(const RepresentationState& state)
{
  if(!state.hasValue(OPACITY)) return 0.6;

  return state.getValue<double>(OPACITY);
}

//--------------------------------------------------------------------
bool SegmentationSkeletonPoolSettings::getShowAnnotations(const RepresentationState& state)
{
  if(!state.hasValue(SHOWIDS)) return false;

  return state.getValue<bool>(SHOWIDS);
}
