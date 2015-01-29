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

namespace ESPINA
{
  template<typename P>
  class BufferedRepresentationPool
  : public RepresentationPool
  {
  public:
    explicit BufferedRepresentationPool(SchedulerSPtr scheduler, unsigned windowSize);

    virtual void setCrosshair(const NmVector3 &point);

    virtual RepresentationPipelineSList pipelines();

  private:
    virtual void addRepresentationPipeline(ViewItemAdapterPtr source);

    virtual bool isReadyImplementation() const;

    virtual void updateImplementation();

    void updateWindowPosition(RepresentationUpdaterSPtr updater, Priority priority);

  private:
    RepresentationWindow m_updateWindow;
  };

#include "BufferedRepresentationPool.cpp"
}

#endif // ESPINA_BUFFERED_REPRESENTATION_POOL_H
