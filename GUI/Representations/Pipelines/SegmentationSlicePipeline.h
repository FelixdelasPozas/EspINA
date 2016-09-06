/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_SEGMENTATION_SLICE_PIPELINE_H
#define ESPINA_SEGMENTATION_SLICE_PIPELINE_H

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <GUI/Types.h>
#include <GUI/ColorEngines/IntensitySelectionHighlighter.h>
#include <GUI/Representations/RepresentationPipeline.h>

namespace ESPINA
{
  /** \class SegmentationSlicePipeline
   * \brief Pipeline for the creation of slice actors for segmentations.
   *
   */
  class EspinaGUI_EXPORT SegmentationSlicePipeline
  : public RepresentationPipeline
  {
    public:
      /** \brief SegmentationSlicePipeline class constructor.
       * \param[in] plane orthogonal plane of the representation.
       * \param[in] colorEngine application color engine.
       *
       */
      explicit SegmentationSlicePipeline(const Plane plane, GUI::ColorEngines::ColorEngineSPtr colorEngine);

      virtual RepresentationState representationState(ConstViewItemAdapterPtr   item,
                                                      const RepresentationState &settings) override;

      virtual RepresentationPipeline::ActorList createActors(ConstViewItemAdapterPtr   item,
                                                             const RepresentationState &state) override;

      virtual void updateColors(ActorList                 &actors,
                                ConstViewItemAdapterPtr   item,
                                const RepresentationState &state) override;

      virtual bool pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const override;

      /** \brief Sets the orthogonal plane of the representation.
       * \param[in] plane orthogonal plane.
       *
       */
      void setPlane(const Plane plane);

    private:
      Plane                              m_plane;       /** orthogonal plane of the representation. */
      GUI::ColorEngines::ColorEngineSPtr m_colorEngine; /** application's color engine.             */

      static GUI::ColorEngines::IntensitySelectionHighlighter s_highlighter; /** selection highlighter for this class. */
  };
}

#endif // ESPINA_SEGMENTATION_SLICE_PIPELINE_H
