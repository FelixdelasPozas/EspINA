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

#ifndef ESPINA_SEGMENTATION_MESH_PIPELINE_H_
#define ESPINA_SEGMENTATION_MESH_PIPELINE_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Types.h>
#include <GUI/ColorEngines/ColorEngine.h>
#include <GUI/ColorEngines/IntensitySelectionHighlighter.h>
#include <GUI/Representations/RepresentationPipeline.h>

namespace ESPINA
{
  /** \class SegmentationMeshPipeline
   * \brief Pipeline for mesh representations of segmentations.
   *
   */
  class EspinaGUI_EXPORT SegmentationMeshPipeline
  : public RepresentationPipeline
  {
    public:
      /** \brief SegmentationMeshPipeline class constructor.
       * \param[in] colorEngine color engine smart pointer.
       *
       */
      explicit SegmentationMeshPipeline(GUI::ColorEngines::ColorEngineSPtr colorEngine);

      /** \brief SegmentationMeshPipeline class virtual destructor.
       *
       */
      virtual ~SegmentationMeshPipeline()
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
      GUI::ColorEngines::ColorEngineSPtr m_colorEngine; /** color engine for representations. */

      static GUI::ColorEngines::IntensitySelectionHighlighter s_highlighter; /** selection highlighter color engine. */
  };

} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_MESH_PIPELINE_H_
