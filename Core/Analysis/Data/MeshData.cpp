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

#include "MeshData.h"

#include <Core/Analysis/Output.h>
#include <Core/Analysis/Data/Mesh/MeshProxy.h>

#include <vtkAlgorithmOutput.h>
#include <vtkPolyData.h>
#include <vtkAlgorithm.h>

using namespace EspINA;

//----------------------------------------------------------------------------
const Data::Type MeshData::TYPE = "MeshData";

//----------------------------------------------------------------------------
MeshData::MeshData()
: m_mesh(nullptr)
{
}

//----------------------------------------------------------------------------
Bounds MeshData::bounds()
{
  Nm bounds[6];
  m_mesh->GetBounds(bounds);

  return Bounds{ bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5] };
}

//----------------------------------------------------------------------------
DataProxySPtr MeshData::createProxy() const
{
  return DataProxySPtr proxy{ new MeshProxy(); };
}

//----------------------------------------------------------------------------
MeshDataSPtr meshRepresentation(OutputSPtr output)
{
  MeshDataSPtr meshData = std::dynamic_pointer_cast<MeshDataSPtr>(output->data(MeshData::Type));
  Q_ASSERT(meshData.get());
  return meshData;
}
