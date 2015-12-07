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
#include "MeshData.h"
#include <Core/Analysis/Output.h>
#include <Core/Analysis/Data/Mesh/MeshProxy.h>
#include <Core/Utils/vtkPolyDataUtils.h>

using namespace ESPINA;

const Data::Type MeshData::TYPE = "MeshData";

//----------------------------------------------------------------------------
MeshData::MeshData()
{
}

//----------------------------------------------------------------------------
bool MeshData::fetchDataImplementation(TemporalStorageSPtr storage, const QString& path, const QString& id, const VolumeBounds &bounds)
{
  bool dataFetched = false;

  for (auto filename : {snapshotFilename   (path, id),
                        oldSnapshotFilename(path, id)})
  {
    QFileInfo meshFile(storage->absoluteFilePath(filename));

    if(meshFile.exists())
    {
      setMesh(PolyDataUtils::readPolyDataFromFile(meshFile.absoluteFilePath()));
      dataFetched = true;
      break;
    }
  }

  Q_ASSERT((!m_bounds.areValid() && !bounds.areValid()) ||  m_bounds == bounds);
  m_bounds = bounds;

  return dataFetched;
}

//----------------------------------------------------------------------------
VolumeBounds MeshData::meshBounds(vtkSmartPointer<vtkPolyData> mesh, const NmVector3 &spacing, const NmVector3 &origin) const
{
  Bounds result;

  if (mesh && mesh->GetNumberOfCells() > 0)
  {
    Nm bounds[6];

    mesh->GetBounds(bounds);

    result = Bounds(bounds);
  }

  return VolumeBounds(result, spacing, origin);
}


//----------------------------------------------------------------------------
Snapshot MeshData::snapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) const
{
  Snapshot snapshot;

  auto currentMesh = mesh();

  if (currentMesh)
  {
    QString fileName = snapshotFilename(path, id);
    storage->makePath(path);

    snapshot << SnapshotData(fileName, PolyDataUtils::savePolyDataToBuffer(currentMesh));
  }

  return snapshot;
}

//----------------------------------------------------------------------------
DataSPtr MeshData::createProxy() const
{
  return std::make_shared<MeshProxy>();
}

//----------------------------------------------------------------------------
Output::ReadLockData<MeshData> ESPINA::readLockMesh(OutputSPtr output, DataUpdatePolicy policy)
{
  return outputReadLockData<MeshData>(output.get(), policy);
}

//----------------------------------------------------------------------------
Output::ReadLockData<MeshData> ESPINA::readLockMesh(Output *output, DataUpdatePolicy policy)
{
  return outputReadLockData<MeshData>(output, policy);
}

//----------------------------------------------------------------------------
Output::WriteLockData<MeshData> ESPINA::writeLockMesh(Output *output, DataUpdatePolicy policy)
{
  return outputWriteLockData<MeshData>(output, policy);
}

//----------------------------------------------------------------------------
Output::WriteLockData<MeshData> ESPINA::writeLockMesh(OutputSPtr output, DataUpdatePolicy policy)
{
  return outputWriteLockData<MeshData>(output.get(), policy);
}

//----------------------------------------------------------------------------
bool ESPINA::hasMeshData(OutputSPtr output)
{
  return output->hasData(MeshData::TYPE);
}
