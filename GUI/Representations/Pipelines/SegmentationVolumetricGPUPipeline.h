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

#ifndef ESPINA_SEGMENTATION_VOLUMETRIC_GPU_PIPELINE_H_
#define ESPINA_SEGMENTATION_VOLUMETRIC_GPU_PIPELINE_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Types.h>
#include <GUI/Representations/RepresentationPipeline.h>
#include <GUI/ColorEngines/IntensitySelectionHighlighter.h>

namespace ESPINA
{
  /** \class SegmentationVolumetricGPUPipeline
   * \brief Pipeline for the creation of volumetric representations of segmentations using the GPU.
   *
   */
  class EspinaGUI_EXPORT SegmentationVolumetricGPUPipeline
  : public RepresentationPipeline
  {
    public:
      /** \brief SegmentationVolumetricGPUPipeline class constructor.
       * \param[in] colorEngine segmentation color engine smart pointer.
       *
       */
      explicit SegmentationVolumetricGPUPipeline(GUI::ColorEngines::ColorEngineSPtr colorEngine);

      /** \brief SegmentationVolumetricGPUPipeline class virtual destructor.
       *
       */
      virtual ~SegmentationVolumetricGPUPipeline()
      {};

      virtual RepresentationState representationState(ConstViewItemAdapterPtr    item,
                                                      const RepresentationState &settings) override;

      virtual RepresentationPipeline::ActorList createActors(ConstViewItemAdapterPtr    item,
                                                             const RepresentationState &state) override;

      virtual void updateColors(ActorList                 &actors,
                                ConstViewItemAdapterPtr    item,
                                const RepresentationState &state) override;

      virtual bool pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const override;

    private:
      GUI::ColorEngines::ColorEngineSPtr m_colorEngine; /** representation's color engine. */

      static GUI::ColorEngines::IntensitySelectionHighlighter s_highlighter; /** selection color engine. */
  };

} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_VOLUMETRIC_GPU_PIPELINE_H_
