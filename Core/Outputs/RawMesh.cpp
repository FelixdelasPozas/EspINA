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

#include "RawMesh.h"
#include <vtkPolyData.h>

using namespace EspINA;

//----------------------------------------------------------------------------
RawMesh::RawMesh(vtkSmartPointer<vtkPolyData> mesh,
                 FilterOutput *output): MeshType(output)
{

}

//----------------------------------------------------------------------------
bool RawMesh::dumpSnapshot(const QString &prefix, Snapshot &snapshot) const
{
  return false;
}

//----------------------------------------------------------------------------
bool RawMesh::fetchSnapshot(Filter *filter, const QString &prefix)
{
  return false;
}

//----------------------------------------------------------------------------
bool RawMesh::isValid() const
{
  return true;
}

//----------------------------------------------------------------------------
bool RawMesh::setInternalData(SegmentationRepresentationSPtr rhs)
{
  return true;
}

//----------------------------------------------------------------------------
bool RawMesh::isEdited() const
{
  return false;
}

//----------------------------------------------------------------------------
void RawMesh::clearEditedRegions()
{

}

//----------------------------------------------------------------------------
void RawMesh::commitEditedRegions(bool withData) const
{

}

//----------------------------------------------------------------------------
void RawMesh::restoreEditedRegions(const QDir &cacheDir, const QString &outputId)
{

}


//----------------------------------------------------------------------------
vtkAlgorithmOutput *RawMesh::mesh()
{
  return m_mesh->GetProducerPort();
}

//----------------------------------------------------------------------------
RawMeshSPtr EspINA::rawMesh(OutputSPtr output)
{
  SegmentationOutputSPtr segmentationOutput = boost::dynamic_pointer_cast<SegmentationOutput>(output);
  Q_ASSERT(segmentationOutput.get());
  return boost::dynamic_pointer_cast<RawMesh>(segmentationOutput->representation(MeshType::TYPE));
}
