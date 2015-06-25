/*

 Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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
#include <GUI/Representations/Settings/SegmentationContourPoolSettings.h>
#include <GUI/Representations/Settings/PipelineStateUtils.h>

using namespace ESPINA::Representations;

namespace ESPINA
{
  //----------------------------------------------------------------------------
  SegmentationContourPoolSettings::SegmentationContourPoolSettings()
  {
    setWidth(SegmentationContourPipeline::Width::MEDIUM);
    setPattern(SegmentationContourPipeline::Pattern::NORMAL);
    setOpacity(0.6);
  }

  //----------------------------------------------------------------------------
  void SegmentationContourPoolSettings::setWidth(SegmentationContourPipeline::Width value)
  {
    set<int>(SegmentationContourPipeline::WIDTH, static_cast<int>(value));
  }

  //----------------------------------------------------------------------------
  SegmentationContourPipeline::Width SegmentationContourPoolSettings::width() const
  {
    auto value = get<int>(SegmentationContourPipeline::WIDTH);

    return static_cast<SegmentationContourPipeline::Width>(value);
  }

  //----------------------------------------------------------------------------
  void SegmentationContourPoolSettings::setPattern(SegmentationContourPipeline::Pattern value)
  {
    set<int>(SegmentationContourPipeline::PATTERN, static_cast<int>(value));
  }

  //----------------------------------------------------------------------------
  SegmentationContourPipeline::Pattern SegmentationContourPoolSettings::pattern() const
  {
    auto value = get<int>(SegmentationContourPipeline::PATTERN);

    return static_cast<SegmentationContourPipeline::Pattern>(value);
  }

  //----------------------------------------------------------------------------
  void SegmentationContourPoolSettings::setOpacity(double value)
  {
    set<double>(OPACITY, value);
  }

  //----------------------------------------------------------------------------
  double SegmentationContourPoolSettings::opacity() const
  {
    return get<double>(OPACITY);
  }

} // namespace ESPINA
