/*

 Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ESPINA
#include <ToolGroups/View/Representations/SegmentationVolumetric/VolumetricManager.h>

namespace ESPINA
{
  //----------------------------------------------------------------------------
  VolumetricManager::VolumetricManager()
  : ActorManager{ViewType::VIEW_3D}
  {
  }

  //----------------------------------------------------------------------------
  RepresentationManager::PipelineStatus VolumetricManager::pipelineStatus() const
  {
    return PipelineStatus::READY;
  }

  //----------------------------------------------------------------------------
  TimeRange VolumetricManager::readyRange() const
  {
    TimeRange range;

    // TODO

    return range;
  }

  //----------------------------------------------------------------------------
  void VolumetricManager::setResolution(const NmVector3 &resolution)
  {
  }

  //----------------------------------------------------------------------------
  ViewItemAdapterPtr VolumetricManager::pick(const NmVector3 &point, vtkProp *actor) const
  {
    ViewItemAdapterPtr pickedItem = nullptr;

    // TODO

    return pickedItem;

  }

  //----------------------------------------------------------------------------
  bool VolumetricManager::hasSources() const
  {
    return true;
  }

  //----------------------------------------------------------------------------
  void VolumetricManager::setCrosshair(const NmVector3 &crosshair, TimeStamp time)
  {
    // TODO: use
  }

  //----------------------------------------------------------------------------
  RepresentationPipeline::Actors VolumetricManager::actors(TimeStamp time)
  {
    RepresentationPipeline::Actors actors;

    // TODO

    return actors;
  }

  //----------------------------------------------------------------------------
  void VolumetricManager::invalidatePreviousActors(TimeStamp time)
  {
    // TODO
  }

  //----------------------------------------------------------------------------
  void VolumetricManager::connectPools()
  {
  }

  //----------------------------------------------------------------------------
  void VolumetricManager::disconnectPools()
  {
  }

  //----------------------------------------------------------------------------
  RepresentationManagerSPtr VolumetricManager::cloneImplementation()
  {
    auto clone = std::make_shared<VolumetricManager>();
    clone->m_name          = m_name;
    clone->m_description   = m_description;
    clone->m_showRepresentations = m_showRepresentations;

    return clone;
  }

} // namespace ESPINA
