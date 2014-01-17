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

#include "BasicRepresentationFactory.h"
#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/Analysis/Data/MeshData.h>
#include "CrosshairRepresentation.h"
#include "SliceRepresentation.h"
#include "MeshRepresentation.h"
#include "SmoothedMeshRepresentation.h"
#include "VolumetricRepresentation.h"
#include "VolumetricGPURepresentation.h"

// #include "ContourRepresentation.h"
//
// #include <Core/OutputRepresentations/MeshType.h>
// #include <Core/OutputRepresentations/VolumeRepresentation.h>
// #include <Core/Model/Filter.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
RepresentationTypeList BasicChannelRepresentationFactory::representations() const
{
  RepresentationTypeList representations;

  representations << ChannelSliceRepresentation::TYPE;
  representations << CrosshairRepresentation::TYPE;

  return representations;
}


//-----------------------------------------------------------------------------
RepresentationSPtr BasicChannelRepresentationFactory::createRepresentation(OutputSPtr output, Representation::Type type)
{
  RepresentationSPtr representation;

  if (type == ChannelSliceRepresentation::TYPE)
  {
    DefaultVolumetricDataSPtr volume = volumetricData(output);
    representation = RepresentationSPtr{new ChannelSliceRepresentation(volume, nullptr)};
  }

  if (type == CrosshairRepresentation::TYPE)
  {
    DefaultVolumetricDataSPtr volume = volumetricData(output);
    representation = RepresentationSPtr { new CrosshairRepresentation(volume, nullptr) };
  }

  return representation;
}

//-----------------------------------------------------------------------------
RepresentationTypeList BasicSegmentationRepresentationFactory::representations() const
{
  RepresentationTypeList representations;

  representations << SegmentationSliceRepresentation::TYPE;
  representations << MeshRepresentation::TYPE;
  representations << SmoothedMeshRepresentation::TYPE;
  representations << VolumetricRepresentation<itkVolumeType>::TYPE;
  representations << VolumetricGPURepresentation<itkVolumeType>::TYPE;

  return representations;
}

//-----------------------------------------------------------------------------
RepresentationSPtr BasicSegmentationRepresentationFactory::createRepresentation(OutputSPtr output, Representation::Type type)
{
  RepresentationSPtr representation;

  if (type == SegmentationSliceRepresentation::TYPE)
  {
    DefaultVolumetricDataSPtr volume = volumetricData(output);

    representation = RepresentationSPtr{new SegmentationSliceRepresentation(volume, nullptr)};
  }

  if (type == MeshRepresentation::TYPE)
  {
    MeshDataSPtr mesh = meshData(output);

    representation = RepresentationSPtr{new MeshRepresentation(mesh, nullptr)};
  }

  if (type == SmoothedMeshRepresentation::TYPE)
  {
    MeshDataSPtr mesh = meshData(output);

    representation = RepresentationSPtr{new SmoothedMeshRepresentation(mesh, nullptr)};
  }

  if (type == VolumetricRepresentation<itkVolumeType>::TYPE)
  {
    DefaultVolumetricDataSPtr volume = volumetricData(output);

    representation = RepresentationSPtr{ new VolumetricRepresentation<itkVolumeType>(volume, nullptr)};
  }

  if (type == VolumetricGPURepresentation<itkVolumeType>::TYPE)
  {
    DefaultVolumetricDataSPtr volume = volumetricData(output);

    representation = RepresentationSPtr{ new VolumetricGPURepresentation<itkVolumeType>(volume, nullptr)};
  }

  return representation;
}

// void BasicGraphicalRepresentationFactory::createGraphicalRepresentations(ChannelOutputSPtr output)
// {
//   ChannelVolumeSPtr volumeData = channelVolume(output);
//   output->addGraphicalRepresentation(GraphicalRepresentationSPtr(new ChannelSliceRepresentation(volumeData, NULL)));
//   output->addGraphicalRepresentation(GraphicalRepresentationSPtr(new CrosshairRepresentation(volumeData, NULL)));
// }
// 
// void BasicGraphicalRepresentationFactory::createGraphicalRepresentations(SegmentationOutputSPtr output)
// {
//   if (output && output->isValid())
//   {
//     SegmentationVolumeSPtr volumeRep = segmentationVolume(output);
//     MeshRepresentationSPtr meshRep   = meshRepresentation(output);
// 
//     GraphicalRepresentationSPtr simpleMeshRepresentation      (new SimpleMeshRepresentation       (meshRep,   NULL));
//     GraphicalRepresentationSPtr smoothedMeshRepresentation    (new SmoothedMeshRepresentation     (meshRep,   NULL));
//     GraphicalRepresentationSPtr volumeRaycastRepresentation   (new VolumeRaycastRepresentation    (volumeRep, NULL));
//     GraphicalRepresentationSPtr volumeGPURayCastRepresentation(new VolumeGPURaycastRepresentation (volumeRep, NULL));
//     GraphicalRepresentationSPtr sliceRepresentation           (new SegmentationSliceRepresentation(volumeRep, NULL));
//     GraphicalRepresentationSPtr contourRepresentation         (new ContourRepresentation          (volumeRep, NULL));
// 
//     output->addGraphicalRepresentation(simpleMeshRepresentation      );
//     output->addGraphicalRepresentation(smoothedMeshRepresentation    );
//     output->addGraphicalRepresentation(volumeRaycastRepresentation   );
//     output->addGraphicalRepresentation(volumeGPURayCastRepresentation);
//     output->addGraphicalRepresentation(sliceRepresentation           );
//     output->addGraphicalRepresentation(contourRepresentation         );
//   }
// }
// 
// void EspINA::SetBasicGraphicalRepresentationFactory(Filter *filter)
// {
//   GraphicalRepresentationFactorySPtr factory(new BasicGraphicalRepresentationFactory);
//   filter->setGraphicalRepresentationFactory(factory);
// }
// 
// void EspINA::SetBasicGraphicalRepresentationFactory(FilterSPtr filter)
// {
//   SetBasicGraphicalRepresentationFactory(filter.get());
// }
// 
