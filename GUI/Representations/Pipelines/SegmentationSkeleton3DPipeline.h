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

#ifndef ESPINA_SEGMENTATION_SKELETON_3D_PIPELINE_H_
#define ESPINA_SEGMENTATION_SKELETON_3D_PIPELINE_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Representations/Pipelines/SegmentationSkeletonPipelineBase.h>
#include <GUI/Representations/RepresentationState.h>
#include <GUI/Types.h>

class vtkFollower;
class vtkGlyphSource2D;

namespace ESPINA
{
  namespace GUI
  {
    namespace Representations
    {
      /** \class SegmentationSkeleton3DPipeline
       * \brief Three dimensional representation for Skeletons.
       *
       */
      class EspinaGUI_EXPORT SegmentationSkeleton3DPipeline
      : public SegmentationSkeletonPipelineBase
      {
        public:
          /** \brief SegmentationSkeleton3DPipeline class constructor.
           * \param[in] colorEngine color engine smart pointer.
           *
           */
          explicit SegmentationSkeleton3DPipeline(ColorEngines::ColorEngineSPtr colorEngine);

          /** brief SegmentationSkeleton3DPipeline class virtual destructor.
           *
           */
          virtual ~SegmentationSkeleton3DPipeline()
          {};

          virtual RepresentationPipeline::ActorList createActors(ConstViewItemAdapterPtr    item,
                                                                 const RepresentationState &state) override;

        private:
          /** \brief Returns a truncated point actor that will follow the camera.
           * \param[in] point Point coordinates vector pointer.
           *
           */
          vtkSmartPointer<vtkFollower> createTruncatedPointActor(const double *point) const;

          vtkSmartPointer<vtkGlyphSource2D> m_truncatedGlyph;
      };

    } // namespace Representations
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_SKELETON_3D_PIPELINE_H_
