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
#include <Core/Analysis/Output.h>

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

    virtual Data::Type type() const override final
    { return TYPE; }

    virtual DataSPtr createProxy() const override final;

    virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) override;

    // Because meshes store the whole mesh polydata when their edited regions
    // are requested, we can use the same name which will cause fetch method to
    // succeed when restoring from edited regions (this will also will avoid
    // executing the filter itself if no other data is required)
    virtual Snapshot editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) override
    { return snapshot(storage, path, id); };

    /** \brief Returns the vtkPolyData smart pointer object.
     *
     */
    virtual vtkSmartPointer<vtkPolyData> mesh() const = 0;

    /** \brief Replace current mesh data with mesh
     *
     */
    virtual void  setMesh(vtkSmartPointer<vtkPolyData> mesh) = 0;

  protected:
    virtual bool fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds) override;

  private:
    QString snapshotFilename(const QString &path, const QString &id) const
    { return path + QString("%1_%2.vtp").arg(id).arg(type()); }

    QString oldSnapshotFilename(const QString &path, const QString &id) const
    { return path + QString("%1_%2.vtp").arg(type()).arg(id); }

    QString editedRegionSnapshotFilename(const QString &path, const QString &id) const
    { return snapshotFilename(path, id); }
  };

  using MeshDataPtr  = MeshData *;
  using MeshDataSPtr = std::shared_ptr<MeshData>;

  /** \brief Obtains and returns the MeshData smart pointer of the spacified Output.
   * \param[in] output Output object smart pointer.
   * 
   *  This function ensures the output is up to date by callig mesh data update first
   *  If the output doesn't contain the requested data type an expection will be thrown
   */
  Output::ReadLockData<MeshData> EspinaCore_EXPORT readLockMesh(OutputSPtr       output,
                                                                DataUpdatePolicy policy = DataUpdatePolicy::Request);

  Output::ReadLockData<MeshData> EspinaCore_EXPORT readLockMesh(Output          *output,
                                                                DataUpdatePolicy policy = DataUpdatePolicy::Request);

  Output::WriteLockData<MeshData> EspinaCore_EXPORT writeLockMesh(Output          *output,
                                                                  DataUpdatePolicy policy = DataUpdatePolicy::Request);

  Output::WriteLockData<MeshData> EspinaCore_EXPORT writeLockMesh(OutputSPtr       output,
                                                                  DataUpdatePolicy policy = DataUpdatePolicy::Request);

  /** \brief Returns whether output has any mesh data or not
   *
   */
  bool EspinaCore_EXPORT hasMeshData(OutputSPtr output);


} // namespace ESPINA

#endif // ESPINA_MESH_DATA_H
