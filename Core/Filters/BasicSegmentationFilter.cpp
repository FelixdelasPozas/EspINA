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

#include <Core/OutputRepresentations/VolumeRepresentation.h>
#include <Core/OutputRepresentations/VolumeProxy.h>
#include <Core/OutputRepresentations/MeshProxy.h>
#include <Core/OutputRepresentations/RawVolume.h>
#include <Core/OutputRepresentations/RawMesh.h>
#include <Core/OutputRepresentations/RasterizedVolume.h>
#include <Core/Model/MarchingCubesMesh.h>

#include <vtkMath.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
BasicSegmentationFilter::BasicSegmentationFilter(NamedInputs namedInputs, ModelItem::Arguments args, FilterType type)
: SegmentationFilter(namedInputs, args, type)
{

}

//-----------------------------------------------------------------------------
SegmentationRepresentationSPtr BasicSegmentationFilter::createRepresentationProxy(FilterOutputId id, const FilterOutput::OutputRepresentationName &type)
{
  SegmentationRepresentationSPtr proxy;

  Q_ASSERT(m_outputs.contains(id));
  Q_ASSERT( NULL == m_outputs[id]->representation(type));

  if (SegmentationVolume::TYPE == type)
    proxy = VolumeProxySPtr(new VolumeProxy());
  else if (MeshRepresentation::TYPE == type)
    proxy = MeshProxySPtr(new MeshProxy());
  else
    Q_ASSERT(false);

  m_outputs[id]->setRepresentation(type, proxy);

  return proxy;
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

    RawMeshSPtr mesh(new RawMesh(m_outputs[0].get()));
    bool fetchMesh   = mesh->fetchSnapshot(this, filterPrefix);

    SegmentationRepresentationSList repList;
    if (fetchVolume)
      repList << volume;
    else if (fetchMesh)
      repList << SegmentationVolumeSPtr(new RasterizedVolume(mesh));

    if (fetchMesh)
      repList << mesh;
    else if (fetchVolume)
      repList << MeshRepresentationSPtr(new MarchingCubesMesh(volume));//TODO: Pass the volume or the proxy?

    addOutputRepresentations(oId, repList);

    fetched = fetchVolume || fetchMesh;
  }

  return fetched;
}
