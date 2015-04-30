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
  : m_width  {SegmentationContourPipeline::Width::medium}
  , m_pattern{SegmentationContourPipeline::Pattern::normal}
  , m_opacity{0.6}
  {
  }

  //----------------------------------------------------------------------------
  RepresentationState SegmentationContourPoolSettings::poolSettingsImplementation() const
  {
    RepresentationState state;

    state.setValue<int>(SegmentationContourPipeline::WIDTH, SegmentationContourPipeline::widthToInteger(m_width));
    state.setValue<int>(SegmentationContourPipeline::PATTERN, SegmentationContourPipeline::patternToInteger(m_pattern));
    state.setValue<double>(OPACITY, m_opacity);

    return state;
  }
  
  //----------------------------------------------------------------------------
  void SegmentationContourPoolSettings::setWidth(SegmentationContourPipeline::Width value)
  {
    m_width = value;
  }

  //----------------------------------------------------------------------------
  SegmentationContourPipeline::Width SegmentationContourPoolSettings::width() const
  {
    return m_width;
  }

  //----------------------------------------------------------------------------
  void SegmentationContourPoolSettings::setPattern(SegmentationContourPipeline::Pattern value)
  {
    m_pattern = value;
  }

  //----------------------------------------------------------------------------
  SegmentationContourPipeline::Pattern SegmentationContourPoolSettings::pattern() const
  {
    return m_pattern;
  }

  //----------------------------------------------------------------------------
  void SegmentationContourPoolSettings::setOpacity(double value)
  {
    m_opacity = value;
  }

  //----------------------------------------------------------------------------
  double SegmentationContourPoolSettings::opacity() const
  {
    return m_opacity;
  }

} // namespace ESPINA
