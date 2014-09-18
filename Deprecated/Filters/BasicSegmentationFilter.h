/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_BASIC_SEGMENTATION_FILTER_H
#define ESPINA_BASIC_SEGMENTATION_FILTER_H

#include "Filters/EspinaFilters_Export.h"

#include <Core/Analysis/Filter.h>

namespace ESPINA
{
  // Provide common implementation for filters which generates volumetric outputs
  // with volumetric representations and marching cubes mesh representations
  // generated from it.
  // It also provide the following graphical representations for each output:
  //    SliceRepresentetion
  //    VolumetricRepresentation
  //    MeshRepresentation
  //    SmoothedMeshRepresentation
  class EspinaFilters_EXPORT BasicSegmentationFilter
  : public Filter
  {
  public:
    explicit BasicSegmentationFilter(InputSList inputs, Type type, SchedulerSPtr scheduler);

  protected:
    virtual DataSPtr createDataProxy(Output::Id id, const Data::Type& type);
  };
}

#endif // ESPINA_BASIC_SEGMENTATION_FILTER_H
