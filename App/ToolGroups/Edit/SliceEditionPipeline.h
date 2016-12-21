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

#ifndef ESPINA_MANUALEDITIONPIPELINE_H
#define ESPINA_MANUALEDITIONPIPELINE_H

#include <GUI/Representations/Pipelines/SegmentationSlicePipeline.h>
#include <GUI/Representations/RepresentationPipeline.h>

namespace ESPINA
{
  class RenderView;

  /** \class SliceEditionPipeline
   * \brief Temporal representation pipeline for edition tools.
   *
   */
  class SliceEditionPipeline
  : public RepresentationPipeline
  {
  public:
      /** \brief SliceEditionPipeline class constructor.
       * \param[in] colorEngine application color engine.
       *
       */
      explicit SliceEditionPipeline(GUI::ColorEngines::ColorEngineSPtr colorEngine);

      virtual RepresentationState representationState(ConstViewItemAdapterPtr item, const RepresentationState &settings) override;

      virtual ActorList createActors(ConstViewItemAdapterPtr item, const RepresentationState &state) override;

      virtual void updateColors(ActorList& actors, ConstViewItemAdapterPtr item, const RepresentationState& state) override;

      virtual bool pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const override;

      /** \brief Set the temporal actor to show in the given view in the current crosshair point.
       * \param[in] actor vtk actor.
       * \param[in] view view to show the actor.
       *
       */
      void setTemporalActor(VTKActor actor, RenderView *view);

    private:
      Plane                              m_plane;         /** orthogonal plane.     */
      Nm                                 m_slice;         /** plane slice position. */
      VTKActor                           m_actor;         /** actor.                */
      GUI::ColorEngines::ColorEngineSPtr m_colorEngine;   /** slice color engine.   */
      SegmentationSlicePipeline          m_slicePipeline; /** slice pipeline.       */
  };

  using SliceEditionPipelineSPtr = std::shared_ptr<SliceEditionPipeline>;
}

#endif // ESPINA_MANUALEDITIONPIPELINE_H
