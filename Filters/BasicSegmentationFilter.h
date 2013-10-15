/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_BASICSEGMENTATIONFILTER_H
#define ESPINA_BASICSEGMENTATIONFILTER_H

#include "EspinaCore_Export.h"

#include <Core/Model/Filter.h>

namespace EspINA
{
  // Provide common implementation for filters which generates
  // volumetric outputs with volumetric representations and 
  // marching cubes mesh representations generated from it
  // It also provide the following graphical representations for each output:
  // SliceGraphicalRepresentetion
  // VolumetricGraphicalRepresentation
  // MeshGraphicalRepresentation
  // SmoothedMeshGraphicalRepresentation
  class EspinaCore_EXPORT BasicSegmentationFilter
  : public SegmentationFilter
  {
  public:
    explicit BasicSegmentationFilter(NamedInputs namedInputs, Arguments args, FilterType type);

  protected:
  virtual SegmentationRepresentationSPtr createRepresentationProxy(FilterOutputId id, const FilterOutput::OutputRepresentationName &type);

    virtual bool fetchSnapshot(FilterOutputId oId);
  };
}

#endif // ESPINA_BASICSEGMENTATIONFILTER_H
