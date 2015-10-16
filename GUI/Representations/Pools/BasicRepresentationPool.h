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
    explicit BasicRepresentationPool(const ItemAdapter::Type &type, SchedulerSPtr scheduler, RepresentationPipelineSPtr pipeline);

    virtual ViewItemAdapterList pick(const NmVector3 &point, vtkProp *actor) const override;

  private:
    virtual void updatePipelinesImplementation(const GUI::Representations::FrameCSPtr frame) override;

    virtual void updateRepresentationsAtImlementation(const GUI::Representations::FrameCSPtr frame, ViewItemAdapterList modifiedItems) override;

    virtual void updateRepresentationColorsAtImlementation(const GUI::Representations::FrameCSPtr frame, ViewItemAdapterList modifiedItems) override;

    virtual void addRepresentationPipeline(ViewItemAdapterPtr source) override;

    virtual void removeRepresentationPipeline(ViewItemAdapterPtr source) override;

    virtual void applySettings(const RepresentationState &settings) override;

  private:
    void updateRepresentations();

  private:
    RepresentationUpdaterSPtr m_updater;
  };
}

#endif // ESPINA_BASIC_REPRESENTATION_POOL_H
