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

#include "MeshType.h"

#include <vtkAlgorithmOutput.h>
#include <vtkPolyData.h>
#include <vtkAlgorithm.h>

using namespace EspINA;

//----------------------------------------------------------------------------
const FilterOutput::OutputRepresentationName MeshRepresentation::TYPE = "MeshOutputType";

//----------------------------------------------------------------------------
EspinaRegion MeshRepresentation::representationBounds()
{
  vtkPolyData *polyData = dynamic_cast<vtkPolyData *>(mesh()->GetProducer()->GetOutputDataObject(0));

  return EspinaRegion(polyData->GetBounds());
}

//----------------------------------------------------------------------------
MeshRepresentationSPtr EspINA::meshRepresentation(OutputSPtr output)
{
  SegmentationOutputSPtr segmentationOutput = boost::dynamic_pointer_cast<SegmentationOutput>(output);
  Q_ASSERT(segmentationOutput.get());
  return boost::dynamic_pointer_cast<MeshRepresentation>(segmentationOutput->representation(MeshRepresentation::TYPE));
}

//----------------------------------------------------------------------------
MeshRepresentationSPtr EspINA::meshRepresentation(SegmentationOutputSPtr output)
{
  return boost::dynamic_pointer_cast<MeshRepresentation>(output->representation(MeshRepresentation::TYPE));
}