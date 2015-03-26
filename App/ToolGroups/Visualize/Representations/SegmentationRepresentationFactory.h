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

#ifndef ESPINA_SEGMENTATION_REPRESENTATION_FACTORY_H
#define ESPINA_SEGMENTATION_REPRESENTATION_FACTORY_H

#include <Support/Representations/RepresentationFactory.h>

namespace ESPINA
{
  class SegmentationRepresentationFactory
  : public RepresentationFactory
  {
  public:
    explicit SegmentationRepresentationFactory(SchedulerSPtr scheduler);

    virtual Representation createRepresentation(ColorEngineSPtr colorEngine) const;

  private:
    void configurePool(RepresentationPoolSPtr           pool,
                       ColorEngineSPtr                  colorEngine,
                       RepresentationPool::SettingsSPtr settings) const;

    void createSliceRepresentation(Representation &rep, ColorEngineSPtr colorEngine, const unsigned int windowSize) const;
    void createContourRepresentation(Representation &rep, ColorEngineSPtr colorEngine, const unsigned int windowSize) const;
    void createSkeletonRepresentation(Representation &rep, ColorEngineSPtr colorEngine, const unsigned int windowSize) const;
    void createVolumetricRepresentation(Representation &rep, ColorEngineSPtr colorEngine) const;
    void createMeshRepresentation(Representation &rep, ColorEngineSPtr colorEngine) const;

  private:
    SchedulerSPtr m_scheduler;
  };
}

#endif // ESPINA_SEGMENTATION_REPRESENTATION_FACTORY_H
