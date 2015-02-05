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

#ifndef ESPINA_BUFFERED_REPRESENTATION_POOL_H
#define ESPINA_BUFFERED_REPRESENTATION_POOL_H

#include <GUI/Representations/RepresentationPool.h>
#include "RepresentationUpdater.h"
#include "RepresentationWindow.h"

#include <vtkMath.h>

namespace ESPINA
{
  template<typename P>
  class BufferedRepresentationPool
  : public RepresentationPool
  {
  public:
    explicit BufferedRepresentationPool(const Plane   plane,
                                        SchedulerSPtr scheduler,
                                        unsigned      windowSize);

    virtual void setCrosshair(const NmVector3 &point);

    virtual void setResolution(const NmVector3 &resolution);

    virtual RepresentationPipelineSList pipelines();

  private:
    virtual void addRepresentationPipeline(ViewItemAdapterPtr source);

    virtual bool isReadyImplementation() const;

    virtual void updateImplementation();

    void updateWindowPosition(RepresentationUpdaterSPtr updater, Priority priority);

    int distanceFromLastCrosshair(const NmVector3 &crosshair);

    int normal(const NmVector3 &point) const;

    NmVector3 representationCrosshair(const NmVector3 &point, int shift) const;

    void invalidateBuffer();

    void updateBuffer(const NmVector3 &point, int shift);

  private:
    const int m_normalIdx;

    RepresentationWindow m_updateWindow;

    Nm m_normalRes;
    Nm m_lastCoordinate;
  };

#include "BufferedRepresentationPool.cpp"
}

#endif // ESPINA_BUFFERED_REPRESENTATION_POOL_H
