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

#ifndef ESPINA_CHANNEL_SLICE_PIPELINE_H
#define ESPINA_CHANNEL_SLICE_PIPELINE_H

// ESPINA
#include <Core/Utils/Spatial.h>
#include <Core/Utils/Vector3.hxx>
#include <GUI/Model/ViewItemAdapter.h>
#include <GUI/Representations/RepresentationPipeline.h>
#include <GUI/Representations/RepresentationState.h>

namespace ESPINA
{
  class ChannelSlicePipeline
  : public RepresentationPipeline
  {
  public:
    explicit ChannelSlicePipeline(const Plane plane);

    virtual RepresentationState representationState(ConstViewItemAdapterPtr  item,
                                                    const RepresentationState &settings) override;


    virtual RepresentationPipeline::ActorList createActors(ConstViewItemAdapterPtr   item,
                                                           const RepresentationState &state) override;

    virtual void updateColors(ActorList                 &actors,
                              ConstViewItemAdapterPtr   item,
                              const RepresentationState &state) override;

    virtual bool pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const;

    void setPlane(const Plane plane);

  private:
    Plane m_plane;
  };
}

#endif // ESPINA_CHANNEL_SLICE_PIPELINE_H
