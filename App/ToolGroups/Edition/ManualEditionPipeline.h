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
#include <GUI/Representations/RepresentationPipeline.h>
#include <ToolGroups/View/Representations/SegmentationSlice/SegmentationSlicePipeline.h>

namespace ESPINA
{
  class ManualEditionPipeline
  : public RepresentationPipeline
  {
  public:
    explicit ManualEditionPipeline(ColorEngineSPtr colorEngine);

    virtual RepresentationState representationState(const ViewItemAdapter *item, const RepresentationState &settings);

    virtual ActorList createActors(const ViewItemAdapter *item, const RepresentationState &state);

    virtual bool pick(ViewItemAdapter *item, const NmVector3 &point) const;

    void setTemporalActor(VTKActor actor);

  private:
    VTKActor m_actor;
    SegmentationSlicePipeline m_slicePipeline;
  };

  using ManualEditionPipelineSPtr = std::shared_ptr<ManualEditionPipeline>;
}

#endif // ESPINA_MANUALEDITIONPIPELINE_H
