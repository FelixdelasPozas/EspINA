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

#ifndef ESPINA_SEGMENTATION_SMOOTHED_MESH_PIPELINE_H_
#define ESPINA_SEGMENTATION_SMOOTHED_MESH_PIPELINE_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Types.h>
#include <GUI/ColorEngines/IntensitySelectionHighlighter.h>
#include <GUI/Representations/RepresentationPipeline.h>

namespace ESPINA
{
  /** \class SegmentationSmoothedMeshPipeline
   * \brief Pipeline for the creation of smoothed meshes representations of segmentations.
   *
   */
  class EspinaGUI_EXPORT SegmentationSmoothedMeshPipeline
  : public RepresentationPipeline
  {
    public:
      /** \brief SegmentationSmoothedMesh class constructor.
       * \param[in] colorEngine color engine smart pointer.
       *
       */
      explicit SegmentationSmoothedMeshPipeline(GUI::ColorEngines::ColorEngineSPtr colorEngine);

      /** \brief SegmentationSmoothedMesh class virtual destructor.
       *
       */
      virtual ~SegmentationSmoothedMeshPipeline()
      {};

      virtual RepresentationState representationState(ConstViewItemAdapterPtr     item,
                                                      const RepresentationState &settings);

      virtual RepresentationPipeline::ActorList createActors(ConstViewItemAdapterPtr  item,
                                                             const RepresentationState &state);

      virtual void updateColors(ActorList                 &actors,
                                ConstViewItemAdapterPtr   item,
                                const RepresentationState &state);

      virtual bool pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const;

    private:
      GUI::ColorEngines::ColorEngineSPtr m_colorEngine; /** representation's color engine. */

      static ESPINA::GUI::ColorEngines::IntensitySelectionHighlighter s_highlighter; /** intensity color engine. */
  };

} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_SMOOTHED_MESH_PIPELINE_H_
