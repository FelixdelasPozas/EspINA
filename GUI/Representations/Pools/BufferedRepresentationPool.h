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

// ESPINA
#include <GUI/Representations/RepresentationPool.h>
#include <GUI/Representations/RepresentationUpdater.h>
#include <GUI/Representations/RepresentationWindow.h>

// VTK
#include <vtkMath.h>

namespace ESPINA
{
  class BufferedRepresentationPool
  : public RepresentationPool
  {
  public:
    explicit BufferedRepresentationPool(const Plane                plane,
                                        RepresentationPipelineSPtr pipeline,
                                        SchedulerSPtr              scheduler,
                                        unsigned                   windowSize);

    virtual void setResolution(const NmVector3 &resolution, TimeStamp t) override;

    virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const override;

  private:
    virtual void addRepresentationPipeline(ViewItemAdapterPtr source) override;

    virtual void removeRepresentationPipeline(ViewItemAdapterPtr source) override;

    virtual void setCrosshairImplementation(const NmVector3 &point, TimeStamp t) override;

    virtual void onSettingsChanged(const RepresentationState &settings) override;

    virtual bool actorsChanged() const override;

    virtual void invalidateImplementation() override;

    virtual void invalidateRepresentations(ViewItemAdapterList items, TimeStamp t) override;

    void updatePriorities();

    int distanceFromLastCrosshair(const NmVector3 &crosshair);

    Nm normal(const NmVector3 &point) const;

    NmVector3 representationCrosshair(const NmVector3 &point, int shift) const;

    /** \brief Configures and return a list of invalid updaters ready to be executed
     *
     */
    RepresentationUpdaterSList updateBuffer(const NmVector3 &point, int shift, const TimeStamp t);

    void updatePipelines(RepresentationUpdaterSList updaters);

    void checkCurrentActors();

    int invalidationShift() const;

  private:
    const int m_normalIdx;

    RepresentationWindow m_updateWindow;

    bool      m_init;
    Nm        m_normalRes;
    NmVector3 m_crosshair;

    bool      m_hasChanged;
  };
}

#endif // ESPINA_BUFFERED_REPRESENTATION_POOL_H
