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

using namespace ESPINA;

const Data::Type MeshData::TYPE = "MeshData";

//----------------------------------------------------------------------------
MeshData::MeshData()
{
}

//----------------------------------------------------------------------------
Bounds MeshData::bounds() const
{
  Bounds result;

  auto meshPolyData = mesh();

  if (meshPolyData)
  {
    Nm bounds[6];

    meshPolyData->GetBounds(bounds);

    result = Bounds{bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]};
  }

  return result;
}

//----------------------------------------------------------------------------
DataProxySPtr MeshData::createProxy() const
{
  return DataProxySPtr{ new MeshProxy() };
}

//----------------------------------------------------------------------------
ESPINA::MeshDataSPtr ESPINA::meshData(OutputSPtr output)
{
  output->update();

  auto data = output->data(MeshData::TYPE);

  return std::dynamic_pointer_cast<MeshData>(data);
}
