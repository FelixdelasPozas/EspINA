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
#include "CrosshairRepresentation.h"
#include "VolumeGPURepresentation.h"
#include "ContourRepresentation.h"

#include <Core/OutputRepresentations/MeshType.h>
#include <Core/OutputRepresentations/VolumeRepresentation.h>
#include <Core/Model/Filter.h>

using namespace EspINA;

void BasicGraphicalRepresentationFactory::createGraphicalRepresentations(ChannelOutputSPtr output)
{
  ChannelVolumeSPtr volumeData = channelVolume(output);
  output->addGraphicalRepresentation(GraphicalRepresentationSPtr(new ChannelSliceRepresentation(volumeData, NULL)));
  output->addGraphicalRepresentation(GraphicalRepresentationSPtr(new CrosshairRepresentation(volumeData, NULL)));
}

void BasicGraphicalRepresentationFactory::createGraphicalRepresentations(SegmentationOutputSPtr output)
{
  if (output && output->isValid())
  {
    SegmentationVolumeSPtr volumeRep = segmentationVolume(output);
    MeshRepresentationSPtr meshRep   = meshRepresentation(output);

    GraphicalRepresentationSPtr simpleMeshRepresentation      (new SimpleMeshRepresentation       (meshRep,   NULL));
    GraphicalRepresentationSPtr smoothedMeshRepresentation    (new SmoothedMeshRepresentation     (meshRep,   NULL));
    GraphicalRepresentationSPtr volumeRaycastRepresentation   (new VolumeRaycastRepresentation    (volumeRep, NULL));
    GraphicalRepresentationSPtr volumeGPURayCastRepresentation(new VolumeGPURaycastRepresentation (volumeRep, NULL));
    GraphicalRepresentationSPtr sliceRepresentation           (new SegmentationSliceRepresentation(volumeRep, NULL));
    GraphicalRepresentationSPtr contourRepresentation         (new ContourRepresentation          (volumeRep, NULL));

    output->addGraphicalRepresentation(simpleMeshRepresentation      );
    output->addGraphicalRepresentation(smoothedMeshRepresentation    );
    output->addGraphicalRepresentation(volumeRaycastRepresentation   );
    output->addGraphicalRepresentation(volumeGPURayCastRepresentation);
    output->addGraphicalRepresentation(sliceRepresentation           );
    output->addGraphicalRepresentation(contourRepresentation         );
  }
}

void EspINA::SetBasicGraphicalRepresentationFactory(Filter *filter)
{
  GraphicalRepresentationFactorySPtr factory(new BasicGraphicalRepresentationFactory);
  filter->setGraphicalRepresentationFactory(factory);
}

void EspINA::SetBasicGraphicalRepresentationFactory(FilterSPtr filter)
{
  SetBasicGraphicalRepresentationFactory(filter.get());
}