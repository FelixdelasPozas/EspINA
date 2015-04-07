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

using namespace ESPINA;

//----------------------------------------------------------------------------
SliceManager::SliceManager(RepresentationPoolSPtr poolXY,
                           RepresentationPoolSPtr poolXZ,
                           RepresentationPoolSPtr poolYZ)
: ActorManager(ViewType::VIEW_2D)
, m_plane{Plane::UNDEFINED}
, m_XY{poolXY}
, m_XZ{poolXZ}
, m_YZ{poolYZ}
{
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
void SliceManager::onSceneResolutionChanged(const NmVector3 &resolution, TimeStamp t)
{
  if (validPlane())
  {
    planePool()->setResolution(resolution, t);
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
    connect(planePool().get(), SIGNAL(poolUpdated(TimeStamp)),
            this,              SLOT(waitForDisplay()));

    connect(planePool().get(), SIGNAL(actorsReady(TimeStamp)),
            this,              SLOT(emitRenderRequest(TimeStamp)));

    connect(planePool().get(), SIGNAL(actorsInvalidated()),
            this,              SLOT(invalidateRepresentations()));

    planePool()->incrementObservers();
  }
}

//----------------------------------------------------------------------------
void SliceManager::disconnectPools()
{
  if (validPlane())
  {
    disconnect(planePool().get(), SIGNAL(poolUpdated(TimeStamp)),
               this,              SLOT(waitForDisplay()));

    disconnect(planePool().get(), SIGNAL(actorsReady(TimeStamp)),
               this,              SLOT(emitRenderRequest(TimeStamp)));

    disconnect(planePool().get(), SIGNAL(actorsInvalidated()),
               this,              SLOT(invalidateRepresentations()));

    planePool()->decrementObservers();
  }
}

//----------------------------------------------------------------------------
RepresentationManagerSPtr SliceManager::cloneImplementation()
{
  auto clone = std::make_shared<SliceManager>(m_XY, m_XZ, m_YZ);

  clone->m_plane = m_plane;

  return clone;
}

//----------------------------------------------------------------------------
RepresentationPoolSPtr SliceManager::planePool() const
{
  switch (m_plane)
  {
    case Plane::XY:
      return m_XY;
    case Plane::XZ:
      return m_XZ;
    case Plane::YZ:
      return m_YZ;
    case Plane::UNDEFINED:
      Q_ASSERT(false);
      break;
  };

  return RepresentationPoolSPtr();
}

//----------------------------------------------------------------------------
bool SliceManager::validPlane() const
{
  return Plane::UNDEFINED != m_plane;
}
