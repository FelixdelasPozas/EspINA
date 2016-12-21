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

// ESPINA
#include "SliceEditionPipeline.h"
#include <GUI/View/View2D.h>
#include <GUI/Utils/RepresentationUtils.h>
#include <Support/Representations/RepresentationUtils.h>
#include <GUI/Utils/RepresentationUtils.h>

// VTK
#include <vtkProp.h>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::ColorEngines;

//------------------------------------------------------------------------
SliceEditionPipeline::SliceEditionPipeline(ColorEngineSPtr colorEngine)
: RepresentationPipeline{"TemporalSlicePipeline"}
, m_plane               {Plane::UNDEFINED}
, m_slice               {-VTK_DOUBLE_MAX}
, m_colorEngine         {colorEngine}
, m_slicePipeline       {Plane::UNDEFINED, colorEngine}
{
  setType(m_slicePipeline.type());
}

//------------------------------------------------------------------------
RepresentationState SliceEditionPipeline::representationState(ConstViewItemAdapterPtr item, const RepresentationState &settings)
{
  return m_slicePipeline.representationState(item, settings);
}

//------------------------------------------------------------------------
RepresentationPipeline::ActorList SliceEditionPipeline::createActors(ConstViewItemAdapterPtr item, const RepresentationState &state)
{
  ActorList actors;

  if (m_actor.GetPointer() && (RepresentationUtils::plane(state) == m_plane) && (crosshairPoint(state)[normalCoordinateIndex(m_plane)] == m_slice))
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
void SliceEditionPipeline::updateColors(RepresentationPipeline::ActorList& actors, ConstViewItemAdapterPtr item, const RepresentationState& state)
{
  if((RepresentationUtils::plane(state) != m_plane) || (crosshairPoint(state)[normalCoordinateIndex(m_plane)] != m_slice))
  {
    m_slicePipeline.updateColors(actors, item, state);
  }
}

//------------------------------------------------------------------------
bool SliceEditionPipeline::pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const
{
  return m_slicePipeline.pick(item, point);
}

//------------------------------------------------------------------------
void SliceEditionPipeline::setTemporalActor(RepresentationPipeline::VTKActor actor, RenderView *view)
{
  auto view2D = view2D_cast(view);

  if(view2D)
  {
    m_plane         = view2D->plane();
    m_slice         = view2D->crosshair()[normalCoordinateIndex(m_plane)];
    m_actor         = actor;
    m_slicePipeline = SegmentationSlicePipeline{m_plane, m_colorEngine};

    m_slicePipeline.setPlane(m_plane);
  }
}
