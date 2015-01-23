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

#include "ChannelSliceManager.h"
#include <GUI/View/RenderView.h>

using namespace ESPINA;

//----------------------------------------------------------------------------
ChannelSliceManager::ChannelSliceManager(RepresentationPoolSPtr xy, RepresentationPoolSPtr xz, RepresentationPoolSPtr yz)
: m_xy{xy}
, m_xz{xz}
, m_yz{yz}
, m_plane{Plane::UNDEFINED}
{

}

//----------------------------------------------------------------------------
bool ChannelSliceManager::isReady() const
{
  return planePool()->isReady();
}

//----------------------------------------------------------------------------
void ChannelSliceManager::onCrosshairChanged(NmVector3 crosshair)
{
  planePool()->setCrosshair(crosshair);
}

//----------------------------------------------------------------------------
void ChannelSliceManager::setPlane(Plane plane)
{
  if (plane != m_plane)
  {
    if (Plane::UNDEFINED != plane)
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
RepresentationPipelineSList ChannelSliceManager::pipelines()
{
  return planePool()->pipelines();
}

//----------------------------------------------------------------------------
RepresentationManagerSPtr ChannelSliceManager::cloneImpelementation()
{
  auto clone = std::make_shared<ChannelSliceManager>(m_xy, m_xz, m_yz);

  clone->m_name          = m_name;
  clone->m_description   = m_description;
  clone->m_plane         = m_plane;
  clone->m_showPipelines = m_showPipelines;

  return clone;
}

//----------------------------------------------------------------------------
RepresentationPoolSPtr ChannelSliceManager::planePool() const
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

