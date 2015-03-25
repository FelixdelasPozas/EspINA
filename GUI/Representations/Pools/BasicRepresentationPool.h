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

// ESPINA
#include <GUI/Representations/RepresentationPool.h>
#include <GUI/Representations/RepresentationUpdater.h>

namespace ESPINA
{
  class BasicRepresentationPool
  : public RepresentationPool
  {
  public:
    explicit BasicRepresentationPool(SchedulerSPtr scheduler, RepresentationPipelineSPtr pipeline);

    virtual void setResolution(const NmVector3 &resolution);

    virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const;

  private:
    virtual void addRepresentationPipeline(ViewItemAdapterPtr source);

    virtual void removeRepresentationPipeline(ViewItemAdapterPtr source);

    virtual void setCrosshairImplementation(const NmVector3 &point, TimeStamp t);

    virtual void onSettingsChanged(const RepresentationState &settings);

    virtual bool actorsChanged() const;

    virtual void invalidateImplementation();

    virtual void invalidateRepresentations(ViewItemAdapterList items, TimeStamp t);

  private:
    RepresentationUpdaterSPtr m_updater;
    bool m_init;
    bool m_hasChanged;
  };
}

#endif // ESPINA_BASIC_REPRESENTATION_POOL_H
