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

#ifndef ESPINA_SEGMENTATION_VOLUMETRIC_PIPELINE_H_
#define ESPINA_SEGMENTATION_VOLUMETRIC_PIPELINE_H_

// ESPINA
#include <GUI/Representations/RepresentationPipeline.h>
#include <GUI/ColorEngines/TransparencySelectionHighlighter.h>

namespace ESPINA
{
  class SegmentationVolumetricPipeline
  : public RepresentationPipeline
  {
    public:
      /** \brief SegmentationVolumetricPipeline class constructor.
       * \param[in] colorEngine segmentation color engine smart pointer.
       *
       */
      SegmentationVolumetricPipeline(ColorEngineSPtr colorEngine);

      /** \brief SegmentationVolumetricPipeline class virtual destructor.
       *
       */
      virtual ~SegmentationVolumetricPipeline()
      {};

      virtual RepresentationState representationState(const ViewItemAdapter     *item,
                                                      const RepresentationState &settings);

      virtual RepresentationPipeline::ActorList createActors(const ViewItemAdapter     *item,
                                                             const RepresentationState &state);

      virtual bool pick(ViewItemAdapter *item, const NmVector3 &point) const;

    private:
      ColorEngineSPtr m_colorEngine;

      static TransparencySelectionHighlighter s_highlighter;
  };

} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_VOLUMETRIC_PIPELINE_H_
