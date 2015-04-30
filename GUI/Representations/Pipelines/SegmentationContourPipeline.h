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

#ifndef ESPINA_SEGMENTATION_CONTOUR_PIPELINE_H_
#define ESPINA_SEGMENTATION_CONTOUR_PIPELINE_H_

// ESPINA
#include <Core/Utils/Spatial.h>
#include <GUI/ColorEngines/ColorEngine.h>
#include <GUI/ColorEngines/IntensitySelectionHighlighter.h>
#include <GUI/Representations/RepresentationPipeline.h>

class vtkImageCanvasSource2D;

namespace ESPINA
{
  class SegmentationContourPipeline
  : public RepresentationPipeline
  {
    public:
      enum class Width: std::int8_t { tiny = 0, small = 1, medium = 2, large = 3, huge = 4 };
      enum class Pattern: std::int8_t { normal = 0, dotted = 1, dashed = 2 } ;
      static QString WIDTH;
      static QString PATTERN;

    public:
      /** \brief SegmentationContourPipeline class constructor.
       * \param[in] colorEngine color engine smart pointer.
       *
       */
      explicit SegmentationContourPipeline(Plane plane, ColorEngineSPtr colorEngine);

      /** \brief SegmentationContourPipeline class virtual destructor.
       *
       */
      virtual ~SegmentationContourPipeline()
      {};

      virtual RepresentationState representationState(const ViewItemAdapter     *item,
                                                      const RepresentationState &settings);

      virtual RepresentationPipeline::ActorList createActors(const ViewItemAdapter     *item,
                                                             const RepresentationState &state);

      virtual bool pick(ViewItemAdapter *item, const NmVector3 &point) const;

      /** \brief Returns the integer value of the specified width value.
       * \param[in] value Width value;
       */
      static int widthToInteger(Width value);

      /** \brief Returns the integer value of the specified pattern value.
       * \param[in] value Pattern value;
       */
      static int patternToInteger(Pattern value);

    private:
      /** \brief Helper method get the width value from the settings.
       * \param[in] state RepresentationState object.
       *
       */
      Width representationWidth(RepresentationState state) const;

      /** \brief Helper method to get the pattern value from the settings.
       * \param[in] state RepresentationState object.
       */
      Pattern representationPattern(RepresentationState state) const;

      /** \brief Helper method to generate the actor's texture with the current state.
       *
       */
      void generateTexture(vtkImageCanvasSource2D *textureIcon, Pattern texturePattern) const;

      /** \brief Returns the hexadecimal value of the specified pattern
       * \param[in] value Pattern value.
       *
       */
      int hexPatternValue(Pattern value) const;

    private:
      ColorEngineSPtr m_colorEngine;
      Plane           m_plane;

      static ESPINA::GUI::ColorEngines::IntensitySelectionHighlighter s_highlighter;
  };

} // namespace ESPINA

#endif // GUI_REPRESENTATIONS_PIPELINES_SEGMENTATIONCONTOURPIPELINE_H_
