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

#include "BasicSegmentationFilter.h"

#include <Core/Outputs/VolumeRepresentation.h>
#include <Core/Outputs/VolumeProxy.h>
#include <Core/Outputs/MeshProxy.h>
#include <Core/Outputs/RawVolume.h>
#include <Core/Model/MarchingCubesMesh.h>

#include <GUI/Representations/SliceRepresentation.h>
#include <GUI/Representations/SimpleMeshRepresentation.h>
#include <GUI/Representations/SmoothedMeshRepresentation.h>
#include <GUI/Representations/VolumeRaycastRepresentation.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
BasicSegmentationFilter::BasicSegmentationFilter(NamedInputs namedInputs, ModelItem::Arguments args, FilterType type)
: SegmentationFilter(namedInputs, args, type)
{

}

//-----------------------------------------------------------------------------
void BasicSegmentationFilter::createDummyOutput(FilterOutputId id, const FilterOutput::OutputRepresentationName &type)
{
  if (SegmentationVolume::TYPE == type)
    createOutput(id, VolumeProxySPtr(new VolumeProxy()));
  else if (MeshType::TYPE == type)
    createOutput(id, MeshProxySPtr(new MeshProxy()));
  else
    Q_ASSERT(false);
}

//-----------------------------------------------------------------------------
void BasicSegmentationFilter::createOutputRepresentations(SegmentationOutputSPtr output)
{
  SegmentationVolumeSPtr volumeRep = segmentationVolume(output);
  MeshTypeSPtr           meshRep   = meshOutput(output);
  output->addGraphicalRepresentation(GraphicalRepresentationSPtr(new SegmentationSliceRepresentation(volumeRep, NULL)));
  output->addGraphicalRepresentation(GraphicalRepresentationSPtr(new SimpleMeshRepresentation(meshRep, NULL)));
  output->addGraphicalRepresentation(GraphicalRepresentationSPtr(new SmoothedMeshRepresentation(meshRep, NULL)));
  output->addGraphicalRepresentation(GraphicalRepresentationSPtr(new VolumeRaycastRepresentation(volumeRep, NULL)));
}

//-----------------------------------------------------------------------------
bool BasicSegmentationFilter::fetchSnapshot(FilterOutputId oId)
{
  bool fetched = false;

  if (!ignoreCurrentOutputs() && m_outputs.contains(oId))
  {
    QString filterPrefix = QString("%1_%2").arg(m_cacheId).arg(oId);

    RawSegmentationVolumeSPtr volume(new RawSegmentationVolume(m_outputs[0].get()));
    bool fetchVolume = volume->fetchSnapshot(this, filterPrefix);

    // TODO: RawMesh
    bool fetchMesh   = false;// meshOutput        (m_outputs[0])->fetchSnapshot(this, filterPrefix);

    if (fetchVolume && !fetchMesh)
    {
      SegmentationRepresentationSList repList;
      repList << volume;
      repList << MeshTypeSPtr(new MarchingCubesMesh(volume));//TODO: Pass the volume or the proxy?
      createOutput(oId, repList);
    } else if (fetchMesh && !fetchVolume)
    {
      // TODO: Review fetch snapshot
    }

    fetched = fetchVolume || fetchMesh;
  }
  return fetched;
}
