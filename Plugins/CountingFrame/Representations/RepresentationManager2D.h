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

#ifndef ESPINA_CF_REPRESENTATION_MANAGER_2D_H
#define ESPINA_CF_REPRESENTATION_MANAGER_2D_H

#include <GUI/Representations/RepresentationManager.h>
#include "CountingFrameManager.h"

namespace ESPINA
{
  namespace CF
  {
    class RepresentationManager2D
    : public RepresentationManager
    , public ESPINA::RepresentationManager2D
    {
    public:
      explicit RepresentationManager2D(CountingFrameManager &manager, ViewTypeFlags supportedViews);

      virtual void setResolution(const NmVector3 &resolution);

      virtual PipelineStatus pipelineStatus() const;

      virtual TimeRange readyRange() const;

      virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const;

      virtual void setPlane(Plane plane) {}

      virtual void setRepresentationDepth(Nm depth) {}

    private:
      virtual bool hasSources() const { true; }

      virtual void setCrosshair(const NmVector3 &crosshair, TimeStamp time) {}

      virtual RepresentationPipeline::Actors actors(TimeStamp time)
      { return RepresentationPipeline::Actors(); }

      virtual void invalidatePreviousActors(TimeStamp time){}

      virtual void connectPools(){}

      virtual void disconnectPools(){}

      virtual RepresentationManagerSPtr cloneImplementation();

    private:
      CountingFrameManager &m_manager;
    };
  }
}

#endif // ESPINA_CF_REPRESENTATION_MANAGER_2D_H
