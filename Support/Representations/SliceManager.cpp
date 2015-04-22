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
: PoolManager(ViewType::VIEW_2D)
, m_plane{Plane::UNDEFINED}
, m_depth{0}
, m_XY{poolXY}
, m_XZ{poolXZ}
, m_YZ{poolYZ}
{
}

//----------------------------------------------------------------------------
TimeRange SliceManager::readyRangeImplementation() const
{
  TimeRange range;

  if(validPlane())
  {
    range = planePool()->readyRange();
  }

  return range;
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
void SliceManager::setPlane(Plane plane)
{
  Q_ASSERT(Plane::UNDEFINED == m_plane);

  m_plane = plane;
}

//----------------------------------------------------------------------------
void SliceManager::setRepresentationDepth(Nm depth)
{
  m_depth = depth;
}

//----------------------------------------------------------------------------
bool SliceManager::acceptCrosshairChange(const NmVector3 &crosshair) const
{
  return normalCoordinate(currentCrosshair()) != normalCoordinate(crosshair);
}

//----------------------------------------------------------------------------
bool SliceManager::acceptSceneResolutionChange(const NmVector3 &resolution) const
{
  return normalCoordinate(currentSceneResolution()) != normalCoordinate(resolution);
}

//----------------------------------------------------------------------------
bool SliceManager::acceptSceneBoundsChange(const Bounds &bounds) const
{
  return false;
}


//----------------------------------------------------------------------------
bool SliceManager::hasRepresentations() const
{
  Q_ASSERT(validPlane());

  return planePool()->hasSources();
}

//----------------------------------------------------------------------------
void SliceManager::updateRepresentations(const NmVector3 &crosshair, const NmVector3 &resolution, const Bounds &bounds, TimeStamp t)
{
  Q_ASSERT(validPlane());

  planePool()->updatePipelines(crosshair, resolution, t);
}

//----------------------------------------------------------------------------
void SliceManager::changeCrosshair(const NmVector3 &crosshair, TimeStamp time)
{
  Q_ASSERT(validPlane());

  planePool()->setCrosshair(crosshair, time);
}

//----------------------------------------------------------------------------
void SliceManager::changeSceneResolution(const NmVector3 &resolution, TimeStamp t)
{
  Q_ASSERT(validPlane());

  planePool()->setSceneResolution(resolution, t);
}

//----------------------------------------------------------------------------
RepresentationPipeline::Actors SliceManager::actors(TimeStamp t)
{
  Q_ASSERT(validPlane());

  return  planePool()->actors(t);
}

//----------------------------------------------------------------------------
void SliceManager::invalidatePreviousActors(TimeStamp t)
{
  Q_ASSERT(validPlane());

  planePool()->invalidatePreviousActors(t);
}

//----------------------------------------------------------------------------
void SliceManager::onShow(TimeStamp t)
{
  connectPools();

  auto pool = planePool();

  RepresentationUtils::setPlane(pool, m_plane);
  RepresentationUtils::setSegmentationDepth(pool, m_depth);
}

//----------------------------------------------------------------------------
void SliceManager::onHide(TimeStamp t)
{
  disconnectPools();
}

//----------------------------------------------------------------------------
void SliceManager::connectPools()
{
  if (validPlane())
  {
    connect(planePool().get(), SIGNAL(actorsInvalidated()),
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
    disconnect(planePool().get(), SIGNAL(actorsInvalidated()),
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

//----------------------------------------------------------------------------
Nm SliceManager::normalCoordinate(const NmVector3 &value) const
{
  return value[normalCoordinateIndex(m_plane)];
}
