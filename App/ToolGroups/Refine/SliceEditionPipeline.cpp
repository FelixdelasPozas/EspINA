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

#include "SliceEditionPipeline.h"

#include <GUI/View/View2D.h>
#include "Support/Representations/RepresentationUtils.h"

using namespace ESPINA;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::RepresentationUtils;

//------------------------------------------------------------------------
SliceEditionPipeline::SliceEditionPipeline(ColorEngineSPtr colorEngine)
: RepresentationPipeline("")
, m_slicePipeline(Plane::XY, colorEngine)
{
  setType(m_slicePipeline.type());
}

//------------------------------------------------------------------------
RepresentationState SliceEditionPipeline::representationState(const ViewItemAdapter *item, const RepresentationState &settings)
{
  return m_slicePipeline.representationState(item, settings);
}

//------------------------------------------------------------------------
RepresentationPipeline::ActorList SliceEditionPipeline::createActors(const ViewItemAdapter *item, const RepresentationState &state)
{
  ActorList actors;
  //qDebug() << "plane" << normalCoordinateIndex(m_plane) << "stateplane" << normalCoordinateIndex(plane(state))<< "state ch" << crosshairPoint(state) << "param ch" << m_crosshair << "evaluate" << ((crosshairPoint(state) == m_crosshair) && (plane(state) == m_plane));

  if ((crosshairPoint(state) == m_crosshair) && (plane(state) == m_plane))
  {
    actors << m_actor;
  }
  else
  {
    actors << m_slicePipeline.createActors(item, state);
  }

  return actors;
}

//------------------------------------------------------------------------
bool SliceEditionPipeline::pick(ViewItemAdapter *item, const NmVector3 &point) const
{
  return m_slicePipeline.pick(item, point);
}

//------------------------------------------------------------------------
void SliceEditionPipeline::setTemporalActor(RepresentationPipeline::VTKActor actor, RenderView *view)
{
  auto view2D = view2D_cast(view);
  m_plane     = view2D->plane();
  m_crosshair = view2D->crosshair();

  m_actor = actor;
}
