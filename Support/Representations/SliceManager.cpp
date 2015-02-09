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
#include <GUI/View/RenderView.h>

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
void SliceManager::onCrosshairChanged(NmVector3 crosshair, TimeStamp time)
{
  if (validPlane())
  {
    planePool()->setCrosshair(crosshair, time);
    planePool()->update();
  }
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
  if (plane != m_plane)
  {
    if (Plane::UNDEFINED != m_plane)
    {
      qWarning() << "The DAY has come";
      disconnect(planePool().get(), SIGNAL(representationsReady()),
                 this,              SIGNAL(renderRequested()));
    }

    m_plane = plane;

    connect(planePool().get(), SIGNAL(representationsReady()),
            this,              SIGNAL(renderRequested()));
  }
}

//----------------------------------------------------------------------------
RepresentationPipelineSList SliceManager::pipelines(TimeStamp time)
{
  RepresentationPipelineSList pipelines;

  if (validPlane())
  {
    pipelines = planePool()->pipelines(time);
  }

  return pipelines;
}

//----------------------------------------------------------------------------
void SliceManager::notifyPoolUsed()
{
  if (validPlane())
  {
    planePool()->incrementObservers();
  }
}

//----------------------------------------------------------------------------
void SliceManager::notifyPoolNotUsed()
{
  if (validPlane())
  {
    planePool()->decrementObservers();
  }
}

//----------------------------------------------------------------------------
void SliceManager::updatePipelines()
{
  if (validPlane())
  {
    planePool()->update();
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
