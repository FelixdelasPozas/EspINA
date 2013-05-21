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

#include "BasicGraphicalRepresentationFactory.h"
#include "SliceRepresentation.h"
#include "SimpleMeshRepresentation.h"
#include "SmoothedMeshRepresentation.h"
#include "VolumeRaycastRepresentation.h"

#include <Core/Outputs/MeshType.h>
#include <Core/Outputs/VolumeRepresentation.h>

using namespace EspINA;

void BasicGraphicalRepresentationFactory::createGraphicalRepresentations(ChannelOutputSPtr output)
{
  ChannelVolumeSPtr volumeData = channelVolume(output);
  output->addGraphicalRepresentation(GraphicalRepresentationSPtr(new ChannelSliceRepresentation(volumeData, NULL)));
}

void BasicGraphicalRepresentationFactory::createGraphicalRepresentations(SegmentationOutputSPtr output)
{
  SegmentationVolumeSPtr volumeRep = segmentationVolume(output);
  MeshRepresentationSPtr meshRep   = meshRepresentation(output);
  output->addGraphicalRepresentation(GraphicalRepresentationSPtr(new SegmentationSliceRepresentation(volumeRep, NULL)));
  output->addGraphicalRepresentation(GraphicalRepresentationSPtr(new SimpleMeshRepresentation(meshRep, NULL)));
  output->addGraphicalRepresentation(GraphicalRepresentationSPtr(new SmoothedMeshRepresentation(meshRep, NULL)));
  output->addGraphicalRepresentation(GraphicalRepresentationSPtr(new VolumeRaycastRepresentation(volumeRep, NULL)));
}

void EspINA::SetBasicGraphicalRepresentationFactory(Filter *filter)
{
  GraphicalRepresentationFactorySPtr factory(new BasicGraphicalRepresentationFactory);
  filter->setGraphicalRepresentationFactory(factory);
}

void EspINA::SetBasicGraphicalRepresentationFactory(FilterSPtr filter)
{
  SetBasicGraphicalRepresentationFactory(filter.data());
}
