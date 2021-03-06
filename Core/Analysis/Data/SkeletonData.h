/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_SKELETON_DATA_H_
#define ESPINA_SKELETON_DATA_H_

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/Data.h>
#include <Core/Utils/Bounds.h>
#include <Core/Analysis/Output.h>

// VTK
#include <vtkSmartPointer.h>

class vtkPolyData;

namespace ESPINA
{
  /** \class SkeletonData
   * \brief Implements a data to hold skeletons.
   *
   */
  class EspinaCore_EXPORT SkeletonData
  : public Data
  {
    public:
      static const Data::Type TYPE;

    public:
      /** \brief SkeletonData class constructor.
       *
       */
      explicit SkeletonData();

      virtual Data::Type type() const override final
      { return TYPE; }

      virtual DataSPtr createProxy() const override final;

      /** \brief Returns the vtkPolyData smart pointer object.
       *
       */
      virtual vtkSmartPointer<vtkPolyData> skeleton() const = 0;

      /** \brief Replace current skeleton data with a new skeleton. Is somewhat the equivalent
       *   of 'draw' methods in the volumetric data. But this one replaces the entire polydata.
       *
       */
      virtual void setSkeleton(vtkSmartPointer<vtkPolyData> skeleton) = 0;

      virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString& path, const QString& id);

    protected:
      virtual bool fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds) override = 0;

    private:
      const QString snapshotFilename(const QString &path, const QString &id) const
      { return path + QString("%1_%2.vtp").arg(id).arg(type()); }

      const QString editedRegionSnapshotFilename(const QString &path, const QString &id) const
      { return snapshotFilename(path, id); }
  };

  using SkeletonDataSPtr = std::shared_ptr<SkeletonData>;

  /** \brief Returns true if the output has a SkeletonData type data.
   *
   */
  bool EspinaCore_EXPORT hasSkeletonData(OutputSPtr output);

  /** \brief Obtains and returns the SkeletonData smart pointer of the specified Output for read only operations.
   * \param[in] output Output object smart pointer
   * \param[in] policy marks if the data need to be updated before retrieval. Request to update, Ignore for not.
   *
   */
  Output::ReadLockData<SkeletonData> EspinaCore_EXPORT readLockSkeleton(OutputSPtr       output,
                                                                        DataUpdatePolicy policy = DataUpdatePolicy::Request);

  /** \brief Obtains and returns the SkeletonData smart pointer of the specified Output for read only operations.
   * \param[in] output Output object pointer.
   * \param[in] policy marks if the data need to be updated before retrieval.
   *
   */
  Output::ReadLockData<SkeletonData> EspinaCore_EXPORT readLockSkeleton(Output          *output,
                                                                        DataUpdatePolicy policy = DataUpdatePolicy::Request);

  /** \brief Obtains and returns the SkeletonData smart pointer of the specified Output for read-write operations.
   * \param[in] output Output object smart pointer
   * \param[in] policy marks if the data need to be updated before retrieval. Request to update, Ignore for not.
   *
   */
  Output::WriteLockData<SkeletonData> EspinaCore_EXPORT writeLockSkeleton(Output          *output,
                                                                          DataUpdatePolicy policy = DataUpdatePolicy::Request);

  /** \brief Obtains and returns the SkeletonData smart pointer of the specified Output for read-write operations.
   * \param[in] output Output object pointer
   * \param[in] policy marks if the data need to be updated before retrieval. Request to update, Ignore for not.
   *
   */
  Output::WriteLockData<SkeletonData> EspinaCore_EXPORT writeLockSkeleton(OutputSPtr       output,
                                                                          DataUpdatePolicy policy = DataUpdatePolicy::Request);

} // namespace ESPINA

#endif // ESPINA_SKELETON_DATA_H_
