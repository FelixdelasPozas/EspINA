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

#ifndef ESPINA_BASIC_REPRESENTATION_POOL_H
#define ESPINA_BASIC_REPRESENTATION_POOL_H

#include <GUI/Representations/RepresentationPool.h>
#include "RepresentationUpdater.h"

namespace ESPINA {

  template<typename P>
  class BasicRepresentationPool
  : public RepresentationPool
  {
  public:
    explicit BasicRepresentationPool(SchedulerSPtr scheduler);

    virtual void setCrosshair(const NmVector3 &point);

    virtual void setResolution ( const NmVector3 &resolution );

    virtual RepresentationPipelineSList pipelines();

  private:
    virtual void addRepresentationPipeline(ViewItemAdapterPtr source);

    virtual void updateImplementation();

    virtual bool isReadyImplementation() const;

  private:
    RepresentationUpdaterSPtr m_updater;
  };
}

#include "BasicRepresentationPool.cpp"

#endif // ESPINA_BASIC_REPRESENTATION_POOL_H
