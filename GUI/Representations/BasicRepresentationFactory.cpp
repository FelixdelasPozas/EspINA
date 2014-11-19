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

// ESPINA
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Data/SkeletonData.h>
#include "BasicRepresentationFactory.h"
#include "CrosshairRepresentation.h"
#include "SliceRepresentation.h"
#include "ContourRepresentation.h"
#include "MeshRepresentation.h"
#include "SmoothedMeshRepresentation.h"
#include "VolumetricRepresentation.hxx"
#include "VolumetricGPURepresentation.hxx"
#include "SliceCachedRepresentation.h"
#include "SkeletonRepresentation.h"

using namespace ESPINA;

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

  if (hasVolumetricData(output))
  {
    auto volume = volumetricData(output);

    if (ChannelSliceRepresentation::TYPE == type)
    {
      representation = std::make_shared<ChannelSliceRepresentation>(volume, nullptr);
    }
    else if (CrosshairRepresentation::TYPE == type)
    {
      representation = std::make_shared<CrosshairRepresentation>(volume, nullptr);
    }
    else if (ChannelSliceCachedRepresentation::TYPE == type)
    {
      representation = std::make_shared<ChannelSliceCachedRepresentation>(volume, nullptr);
    }
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
  representations << SkeletonRepresentation::TYPE;

  return representations;
}

//-----------------------------------------------------------------------------
RepresentationSPtr BasicSegmentationRepresentationFactory::createRepresentation(OutputSPtr output, Representation::Type type)
{
  RepresentationSPtr representation;

  if (hasVolumetricData(output))
  {
    auto volume = volumetricData(output);

    if (SegmentationSliceRepresentation::TYPE == type)
    {
      representation = std::make_shared<SegmentationSliceRepresentation>(volume, nullptr);
    }
    else if (ContourRepresentation::TYPE == type)
    {
      representation = std::make_shared<ContourRepresentation>(volume, nullptr);
    }
    else if (VolumetricRepresentation<itkVolumeType>::TYPE == type)
    {
      representation = std::make_shared<VolumetricRepresentation<itkVolumeType>>(volume, nullptr);
    }
    else if (VolumetricGPURepresentation<itkVolumeType>::TYPE == type)
    {
      representation = std::make_shared<VolumetricGPURepresentation<itkVolumeType>>(volume, nullptr);
    }
    else if (SegmentationSliceCachedRepresentation::TYPE == type)
    {
      representation = std::make_shared<SegmentationSliceCachedRepresentation>(volume, nullptr);
    }
  }

  if (hasMeshData(output))
  {
    auto mesh = meshData(output);

    if (MeshRepresentation::TYPE == type)
    {
      representation = std::make_shared<MeshRepresentation>(mesh, nullptr);
    }
    else if (SmoothedMeshRepresentation::TYPE == type)
    {
      representation = std::make_shared<SmoothedMeshRepresentation>(mesh, nullptr);
    }
  }

  if(hasSkeletonData(output))
  {
    auto skeleton = skeletonData(output);

    if(SkeletonRepresentation::TYPE == type)
    {
      representation = std::make_shared<SkeletonRepresentation>(skeleton, nullptr);
    }
  }

  return representation;
}
