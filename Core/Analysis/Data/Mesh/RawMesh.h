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

#ifndef ESPINA_RAW_MESH_H
#define ESPINA_RAW_MESH_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/Data/MeshData.h>
#include "MeshProxy.h"

// VTK
#include <vtkSmartPointer.h>

namespace ESPINA
{
  /** \class RawMesh
   * \brief Implements a raw mesh data.
   *
   */
  class EspinaCore_EXPORT RawMesh
  : public MeshData
  {
  public:
    /** \brief RawMesh class constructor.
     *
     */
    explicit RawMesh();

    /** \brief RawMesh class constructor.
     * \param[in] mesh vtkPolyData smart pointer.
     * \param[in] spacing spacing of origin volume.
     * \param[in] output smart pointer of associated output.
     *
     */
    explicit RawMesh(vtkSmartPointer<vtkPolyData> mesh,
                     const NmVector3 &spacing = NmVector3{1,1,1},
                     const NmVector3 &origin  = NmVector3{0,0,0});

    /** \brief RawMesh class virtual destructor.
     *
     */
    virtual ~RawMesh()
    {};

    virtual void setMesh(vtkSmartPointer<vtkPolyData> mesh, bool notify = true) override;

    virtual bool isValid() const override;

    virtual bool isEmpty() const override;

    virtual void restoreEditedRegions(TemporalStorageSPtr storage, const QString& path, const QString& id) override
    { fetchDataImplementation(storage, path, id, m_bounds); }

    virtual vtkSmartPointer<vtkPolyData> mesh() const override;

    void setSpacing(const NmVector3 &spacing) override;

    size_t memoryUsage() const override;

  private:
    virtual QList<Data::Type> updateDependencies() const override
    { return QList<Data::Type>(); }

  private:
    vtkSmartPointer<vtkPolyData> m_mesh; /** mesh data. */
    mutable QMutex               m_lock; /** data lock. */
  };
}

#endif // ESPINA_RAW_MESH_H
