/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "FreeFormSource.h"

// EspINA
#include "Core/Model/EspinaFactory.h"
#include <Core/Model/MarchingCubesMesh.h>
#include <Core/Model/EspinaModel.h>
#include <Core/OutputRepresentations/RawVolume.h>

// ITK
#include <itkImageRegionIteratorWithIndex.h>
#include <vtkMath.h>

// Qt
#include <QDebug>

using namespace EspINA;

typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId FreeFormSource::SPACING = "SPACING";

//-----------------------------------------------------------------------------
FreeFormSource::FreeFormSource(const EspinaRegion        &bounds,
                               itkVolumeType::SpacingType spacing,
                               Filter::FilterType        type)
: BasicSegmentationFilter(NamedInputs(), Arguments(), type)
, m_param(m_args)
{
  RawSegmentationVolumeSPtr volumeRepresentation(new RawSegmentationVolume(bounds, spacing));

  SegmentationRepresentationSList representations;
  representations << volumeRepresentation;
  representations << MeshRepresentationSPtr(new MarchingCubesMesh(volumeRepresentation));

  addOutputRepresentations(0, representations);
}

//-----------------------------------------------------------------------------
FreeFormSource::FreeFormSource(NamedInputs inputs,
                               Arguments   args,
                               FilterType  type)
: BasicSegmentationFilter(inputs, args, type)
, m_param(m_args)
{
  Q_ASSERT(inputs.isEmpty());
}

//-----------------------------------------------------------------------------
FreeFormSource::~FreeFormSource()
{
}

//-----------------------------------------------------------------------------
void FreeFormSource::setGraphicalRepresentationFactory(GraphicalRepresentationFactorySPtr factory)
{
  EspINA::Filter::setGraphicalRepresentationFactory(factory);
  if (factory && validOutput(0))
    factory->createGraphicalRepresentations(m_outputs[0]);
}

//-----------------------------------------------------------------------------
bool FreeFormSource::dumpSnapshot(Snapshot &snapshot)
{
  OutputSPtr filterOutput = output(0);
  if (filterOutput) 
  {
    filterOutput->setCached(filterOutput->isCached() || m_model->isTraceable());

    SegmentationVolumeSPtr editedVolume = segmentationVolume(filterOutput);
    editedVolume->clearEditedRegions();
  }

  return EspINA::SegmentationFilter::dumpSnapshot(snapshot);
}

//-----------------------------------------------------------------------------
bool FreeFormSource::needUpdate(FilterOutputId oId) const
{
  return SegmentationFilter::needUpdate(oId);
}