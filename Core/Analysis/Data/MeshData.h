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

// ESPINA
#include <Core/Analysis/Data.h>
#include <Core/Utils/Bounds.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

// C++
#include <memory>

namespace ESPINA
{
  class EspinaCore_EXPORT MeshData
  : public Data
  {
  public:
    static const Data::Type TYPE;

  public:
    /** \brief MeshData class constructor.
     *
     */
    explicit MeshData();

    /** \brief Implements Data::type() const.
     *
     */
    virtual Data::Type type() const
    { return TYPE; }

    /** \brief Implements Data::createProxy() const.
     *
     */
    virtual DataProxySPtr createProxy() const;

    /** \brief Implements Data::bounds() const.
     *
     */
    Bounds bounds() const;

    /** \brief Returns the vtkPolyData smart pointer object.
     *
     */
    virtual vtkSmartPointer<vtkPolyData> mesh() const = 0;

  };

  using MeshDataSPtr = std::shared_ptr<MeshData>;

  /** \brief Obtains and returns the MeshData smart pointer of the spacified Output.
   * \param[in] output, Output object smart pointer.
   */
  MeshDataSPtr EspinaCore_EXPORT meshData(OutputSPtr output);

} // namespace ESPINA

#endif // ESPINA_MESH_DATA_H
