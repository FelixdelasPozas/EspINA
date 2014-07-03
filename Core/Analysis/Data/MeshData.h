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

#ifndef ESPINA_MESH_DATA_H
#define ESPINA_MESH_DATA_H

#include "Core/EspinaCore_Export.h"

#include <Core/Analysis/Data.h>
#include <Core/Utils/Bounds.h>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

#include <memory>

namespace EspINA
{
  class EspinaCore_EXPORT MeshData
  : public Data
  {
  public:
    static const Data::Type TYPE;

  public:
    explicit MeshData();

    virtual Data::Type type() const
    { return TYPE; }

    virtual DataProxySPtr createProxy() const;

    Bounds bounds() const;

    virtual vtkSmartPointer<vtkPolyData> mesh() const = 0;

  };

  using MeshDataSPtr = std::shared_ptr<MeshData>;

  MeshDataSPtr meshData(OutputSPtr output);

} // namespace EspINA

#endif // ESPINA_MESH_DATA_H
