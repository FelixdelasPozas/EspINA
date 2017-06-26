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

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Representations/Pipelines/SegmentationSkeletonPipelineBase.h>

namespace ESPINA
{
  namespace GUI
  {
    namespace Representations
    {
      /** \class SegmentationSkeleton2DPipeline
       * \brief Two dimensional representation for Skeletons.
       *
       */
      class EspinaGUI_EXPORT SegmentationSkeleton2DPipeline
      : public SegmentationSkeletonPipelineBase
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

          virtual RepresentationPipeline::ActorList createActors(ConstViewItemAdapterPtr    item,
                                                                 const RepresentationState &state) override final;

        private:
          Plane m_plane; /** orientation of the representation. */
      };

    } // namespace Representations
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_REPRESENTATIONS_PIPELINES_SEGMENTATIONSKELETON2DPIPELINE_H_
