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

#ifndef ESPINA_SEGMENTATION_SKELETON_2D_PIPELINE_H_
#define ESPINA_SEGMENTATION_SKELETON_2D_PIPELINE_H_

// ESPINA
#include <Core/Utils/Spatial.h>
#include <GUI/Types.h>
#include <GUI/ColorEngines/IntensitySelectionHighlighter.h>
#include <GUI/Representations/RepresentationPipeline.h>

namespace ESPINA
{
  class SegmentationSkeleton2DPipeline
  : public RepresentationPipeline
  {
    public:
      /** \brief SegmentationSkeleton2DPipeline class constructor.
       * \param[in] plane Representation orientation plane.
       * \param[in] colorEngine color engine smart pointer.
       *
       */
      explicit SegmentationSkeleton2DPipeline(Plane plane, GUI::ColorEngines::ColorEngineSPtr colorEngine);

      /** \brief SegmentationSkeleton2DPipeline class virtual destructor.
       *
       */
      virtual ~SegmentationSkeleton2DPipeline()
      {};

      virtual RepresentationState representationState(ConstViewItemAdapterPtr    item,
                                                      const RepresentationState &settings);

      virtual RepresentationPipeline::ActorList createActors(ConstViewItemAdapterPtr    item,
                                                             const RepresentationState &state);

    virtual void updateColors(ActorList& actors, ConstViewItemAdapterPtr    item, const RepresentationState& state) {}
      virtual bool pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const;

    private:
      Plane           m_plane;
      GUI::ColorEngines::ColorEngineSPtr m_colorEngine;

      static GUI::ColorEngines::IntensitySelectionHighlighter s_highlighter;
  };

} // namespace ESPINA

#endif // GUI_REPRESENTATIONS_PIPELINES_SEGMENTATIONSKELETON2DPIPELINE_H_
