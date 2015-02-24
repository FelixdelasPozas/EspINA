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

#include "SliceManager.h"
#include "RepresentationUtils.h"
#include <GUI/View/RenderView.h>
#include <App/ToolGroups/View/Representations/RepresentationSettings.h>

using namespace ESPINA;

//----------------------------------------------------------------------------
SliceManager::SliceManager(RepresentationPoolSPtr xy,
                           RepresentationPoolSPtr xz,
                           RepresentationPoolSPtr yz)
: RepresentationManager(ViewType::VIEW_2D)
, m_xy{xy}
, m_xz{xz}
, m_yz{yz}
, m_plane{Plane::UNDEFINED}
{
}

//----------------------------------------------------------------------------
RepresentationManager::PipelineStatus SliceManager::pipelineStatus() const
{
  return PipelineStatus::RANGE_DEPENDENT;
}

//----------------------------------------------------------------------------
TimeRange SliceManager::readyRange() const
{
  TimeRange range;

  if(validPlane())
  {
    range = planePool()->readyRange();
  }

  return range;
}

//----------------------------------------------------------------------------
void SliceManager::setResolution(const NmVector3 &resolution)
{
  if (validPlane())
  {
    planePool()->setResolution(resolution);
  }
}

//----------------------------------------------------------------------------
void SliceManager::setPlane(Plane plane)
{
  Q_ASSERT(Plane::UNDEFINED == m_plane);

  m_plane = plane;

  if(validPlane())
  {
    RepresentationUtils::setPlane(planePool(), plane);
  }
}

//----------------------------------------------------------------------------
void SliceManager::setRepresentationDepth(Nm depth)
{
  if(validPlane())
  {
    RepresentationUtils::setSegmentationDepth(planePool(), depth);
  }
}

//----------------------------------------------------------------------------
ViewItemAdapterPtr SliceManager::pick(const NmVector3 &point, vtkProp *actor) const
{
  ViewItemAdapterPtr pickedItem = nullptr;

  if (validPlane())
  {
    pickedItem = planePool()->pick(point, actor);
  }

  return pickedItem;
}

//----------------------------------------------------------------------------
void SliceManager::setCrosshair(const NmVector3 &crosshair, TimeStamp time)
{
  if (validPlane())
  {
    planePool()->setCrosshair(crosshair, time);
  }
}

//----------------------------------------------------------------------------
RepresentationPipeline::Actors SliceManager::actors(TimeStamp time)
{
  RepresentationPipeline::Actors actors;

  if (validPlane())
  {
    actors = planePool()->actors(time);
  }

  return actors;
}

//----------------------------------------------------------------------------
void SliceManager::invalidatePreviousActors(TimeStamp time)
{
  if (validPlane())
  {
    planePool()->invalidatePreviousActors(time);
  }
}

//----------------------------------------------------------------------------
void SliceManager::connectPools()
{
  if (validPlane())
  {
    connect(planePool().get(), SIGNAL(actorsReady(TimeStamp)),
            this,              SLOT(emitRenderRequest(TimeStamp)));

    connect(planePool().get(), SIGNAL(actorsInvalidated()),
            this,              SLOT(invalidateActors()));

    planePool()->incrementObservers();
  }
}

//----------------------------------------------------------------------------
void SliceManager::disconnectPools()
{
  if (validPlane())
  {
    disconnect(planePool().get(), SIGNAL(actorsReady(TimeStamp)),
               this,              SLOT(emitRenderRequest(TimeStamp)));
    disconnect(planePool().get(), SIGNAL(actorsInvalidated()),
               this,              SLOT(invalidateActors()));

    planePool()->decrementObservers();
  }
}

//----------------------------------------------------------------------------
RepresentationManagerSPtr SliceManager::cloneImplementation()
{
  auto clone = std::make_shared<SliceManager>(m_xy, m_xz, m_yz);

  clone->m_name          = m_name;
  clone->m_description   = m_description;
  clone->m_plane         = m_plane;
  clone->m_showPipelines = m_showPipelines;

  return clone;
}

//----------------------------------------------------------------------------
RepresentationPoolSPtr SliceManager::planePool() const
{
  switch (m_plane)
  {
    case Plane::XY:
      return m_xy;
    case Plane::XZ:
      return m_xz;
    case Plane::YZ:
      return m_yz;
    case Plane::UNDEFINED:
      Q_ASSERT(false);
  };

  return RepresentationPoolSPtr();
}

//----------------------------------------------------------------------------
bool SliceManager::validPlane() const
{
  return Plane::UNDEFINED != m_plane;
}
