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
#include <GUI/Representations/Frame.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations;

//----------------------------------------------------------------------------
SliceManager::SliceManager(RepresentationPoolSPtr poolXY,
                           RepresentationPoolSPtr poolXZ,
                           RepresentationPoolSPtr poolYZ,
                           ManagerFlags           flags)
: PoolManager(ViewType::VIEW_2D, flags)
, m_plane{Plane::UNDEFINED}
, m_depth{0}
, m_XY{poolXY}
, m_XZ{poolXZ}
, m_YZ{poolYZ}
{
}

//----------------------------------------------------------------------------
ViewItemAdapterList SliceManager::pick(const NmVector3 &point, vtkProp *actor) const
{
  ViewItemAdapterList pickedItems;

  if (validPlane())
  {
    pickedItems = planePool()->pick(point, actor);
  }

  return pickedItems;
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
  return  normalCoordinate(currentCrosshair()) != normalCoordinate(crosshair);
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
void SliceManager::updateFrameRepresentations(const FrameCSPtr frame)
{
  Q_ASSERT(validPlane());

  planePool()->updatePipelines(frame);
}

// //----------------------------------------------------------------------------
// void SliceManager::changeCrosshair(const FrameCSPtr frame)
// {
//   Q_ASSERT(validPlane());
//
//   planePool()->setCrosshair(frame->crosshair, frame->time);
// }

// //----------------------------------------------------------------------------
// void SliceManager::changeSceneResolution(const FrameCSPtr frame)
// {
//   Q_ASSERT(validPlane());
//
//   planePool()->setSceneResolution(frame->resolution, frame->time);
// }

//----------------------------------------------------------------------------
RepresentationPipeline::Actors SliceManager::actors(TimeStamp t)
{
  Q_ASSERT(validPlane());

  return planePool()->actors(t);
}

//----------------------------------------------------------------------------
void SliceManager::invalidatePreviousActors(TimeStamp t)
{
  Q_ASSERT(validPlane());

  planePool()->invalidatePreviousActors(t);
}

//----------------------------------------------------------------------------
void SliceManager::onShow(const FrameCSPtr frame)
{
  connectPools();

  auto pool = planePool();

  RepresentationUtils::setPlane(pool, m_plane);
  RepresentationUtils::setSegmentationDepth(pool, m_depth);
}

//----------------------------------------------------------------------------
void SliceManager::onHide(const FrameCSPtr frame)
{
  disconnectPools();
}

//----------------------------------------------------------------------------
void SliceManager::connectPools()
{
  if (validPlane())
  {
    connect(planePool().get(), SIGNAL(actorsInvalidated(GUI::Representations::FrameCSPtr)),
            this,              SLOT(waitForDisplay(GUI::Representations::FrameCSPtr)));

    connect(planePool().get(), SIGNAL(actorsReady(GUI::Representations::FrameCSPtr)),
            this,              SLOT(emitRenderRequest(GUI::Representations::FrameCSPtr)));

    qDebug() << debugName() << "Activating representation pools";
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

    disconnect(planePool().get(), SIGNAL(actorsReady(GUI::Representations::FrameCSPtr)),
               this,              SLOT(emitRenderRequest(GUI::Representations::FrameCSPtr)));

    qDebug() << debugName() << "Dectivating representation pools";
    planePool()->decrementObservers();
  }
}

//----------------------------------------------------------------------------
RepresentationManagerSPtr SliceManager::cloneImplementation()
{
  auto clone = std::make_shared<SliceManager>(m_XY, m_XZ, m_YZ, flags());

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
RepresentationPoolSList SliceManager::pools() const
{
  RepresentationPoolSList result;
  result << m_XY << m_XZ << m_YZ;

  return result;
}

//----------------------------------------------------------------------------
Nm SliceManager::normalCoordinate(const NmVector3 &value) const
{
  return value[normalCoordinateIndex(m_plane)];
}
