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

      Bounds bounds() const override;

      virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) const override = 0;

      /** \brief Returns the vtkPolyData smart pointer object.
       *
       */
      virtual vtkSmartPointer<vtkPolyData> skeleton() const = 0;

      /** \brief Replace current skeleton data with a new skeleton. Is somewhat the equivalent
       *   of 'draw' methods in the volumetric data. But this one replaces the entire polydata.
       *
       */
      virtual void setSkeleton(vtkSmartPointer<vtkPolyData> skeleton) = 0;

    protected:
      virtual bool fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id) override = 0;

    private:
      QString snapshotFilename(const QString &path, const QString &id) const
      { return QString("%1/%2_%3.vtp").arg(path).arg(id).arg(type()); }

      QString editedRegionSnapshotFilename(const QString &path, const QString &id) const
      { return snapshotFilename(path, id); }
  };

  using SkeletonDataSPtr = std::shared_ptr<SkeletonData>;

  /** \brief Returns true if the output has a SkeletonData type data.
   *
   */
  bool EspinaCore_EXPORT hasSkeletonData(OutputSPtr output);

  /** \brief Obtains and returns the SkeletonData smart pointer of the specified Output.
   * \param[in] output Output object smart pointer
   */
  Output::ReadLockData<SkeletonData> EspinaCore_EXPORT readLockSkeleton(OutputSPtr       output,
                                                                        DataUpdatePolicy policy = DataUpdatePolicy::Request)
                                                                        throw (Unavailable_Output_Data_Exception);

  Output::ReadLockData<SkeletonData> EspinaCore_EXPORT readLockSkeleton(Output          *output,
                                                                        DataUpdatePolicy policy = DataUpdatePolicy::Request)
                                                                        throw (Unavailable_Output_Data_Exception);

  Output::WriteLockData<SkeletonData> EspinaCore_EXPORT writeLockSkeleton(Output          *output,
                                                                          DataUpdatePolicy policy = DataUpdatePolicy::Request)
                                                                          throw (Unavailable_Output_Data_Exception);

  Output::WriteLockData<SkeletonData> EspinaCore_EXPORT writeLockSkeleton(OutputSPtr       output,
                                                                          DataUpdatePolicy policy = DataUpdatePolicy::Request)
                                                                          throw (Unavailable_Output_Data_Exception);

} // namespace ESPINA

#endif // ESPINA_SKELETON_DATA_H_
