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

// EspINA
#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/Analysis/Data/MeshData.h>
#include "BasicRepresentationFactory.h"
#include "CrosshairRepresentation.h"
#include "SliceRepresentation.h"
#include "ContourRepresentation.h"
#include "MeshRepresentation.h"
#include "SmoothedMeshRepresentation.h"
#include "VolumetricRepresentation.h"
#include "VolumetricGPURepresentation.h"
#include "SliceCachedRepresentation.h"

using namespace EspINA;

//-----------------------------------------------------------------------------
RepresentationTypeList BasicChannelRepresentationFactory::representations() const
{
  RepresentationTypeList representations;

  representations << ChannelSliceRepresentation::TYPE;
  representations << ChannelSliceCachedRepresentation::TYPE;
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

  if (type == ChannelSliceCachedRepresentation::TYPE)
  {
    DefaultVolumetricDataSPtr volume = volumetricData(output);
    representation = RepresentationSPtr { new ChannelSliceCachedRepresentation(volume, nullptr) };
  }

  return representation;
}

//-----------------------------------------------------------------------------
RepresentationTypeList BasicSegmentationRepresentationFactory::representations() const
{
  RepresentationTypeList representations;

  representations << SegmentationSliceRepresentation::TYPE;
  representations << SegmentationSliceCachedRepresentation::TYPE;
  representations << ContourRepresentation::TYPE;
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

    if (volume)
    {
      representation = RepresentationSPtr{new SegmentationSliceRepresentation(volume, nullptr)};
    }
  }

  if (type == ContourRepresentation::TYPE)
  {
    DefaultVolumetricDataSPtr volume = volumetricData(output);

    if (volume)
    {
      representation = RepresentationSPtr{new ContourRepresentation(volume, nullptr)};
    }
  }

  if (type == MeshRepresentation::TYPE)
  {
    MeshDataSPtr mesh = meshData(output);

    if (mesh)
    {
      representation = RepresentationSPtr{new MeshRepresentation(mesh, nullptr)};
    }
  }

  if (type == SmoothedMeshRepresentation::TYPE)
  {
    MeshDataSPtr mesh = meshData(output);

    if (mesh)
    {
      representation = RepresentationSPtr{new SmoothedMeshRepresentation(mesh, nullptr)};
    }
  }

  if (type == VolumetricRepresentation<itkVolumeType>::TYPE)
  {
    DefaultVolumetricDataSPtr volume = volumetricData(output);

    if (volume)
    {
      representation = RepresentationSPtr{ new VolumetricRepresentation<itkVolumeType>(volume, nullptr)};
    }
  }

  if (type == VolumetricGPURepresentation<itkVolumeType>::TYPE)
  {
    DefaultVolumetricDataSPtr volume = volumetricData(output);

    if (volume)
    {
      representation = RepresentationSPtr{ new VolumetricGPURepresentation<itkVolumeType>(volume, nullptr)};
    }
  }

  if (type == SegmentationSliceCachedRepresentation::TYPE)
  {
    DefaultVolumetricDataSPtr volume = volumetricData(output);

    if (volume)
    {
      representation = RepresentationSPtr { new SegmentationSliceCachedRepresentation(volume, nullptr) };
    }
  }

  return representation;
}
