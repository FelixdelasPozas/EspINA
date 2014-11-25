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

#ifndef ESPINA_RAW_SKELETON_H_
#define ESPINA_RAW_SKELETON_H_

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/Output.h>
#include <Core/Analysis/Data/SkeletonData.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

namespace ESPINA
{
  class EspinaCore_EXPORT RawSkeleton
  : public SkeletonData
  {
    public:
      /** \brief RawSkeleton class constructor.
       * \param[in] output, smart pointer of associated output.
       *
       */
      explicit RawSkeleton(OutputSPtr output = nullptr);

      /** \brief RawSkeleton class constructor.
       * \param[in] skeleton vtkPolyData smart pointer.
       * \param[in] spacing spacing of origin volume.
       * \param[in] output smart pointer of associated output.
       *
       */
      explicit RawSkeleton(vtkSmartPointer<vtkPolyData> skeleton,
                           const NmVector3 &spacing,
                           OutputSPtr output = nullptr);

      /** \brief RawSkeleton class virtual destructor.
       *
       */
      virtual ~RawSkeleton()
      {};

      virtual bool isValid() const
      { return (m_skeleton.Get() != nullptr); }

      virtual bool isEmpty() const
      { return !isValid(); }

      virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) const override
      { return SkeletonData::snapshot(storage, path, id); }

      virtual Snapshot editedRegionsSnapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) const override
      { return snapshot(storage, path, id); }

      virtual void restoreEditedRegions(TemporalStorageSPtr storage, const QString &path, const QString &id) override
      { fetchDataImplementation(storage, path, id); }

      virtual vtkSmartPointer<vtkPolyData> skeleton() const
      { return m_skeleton; }

      virtual void setSkeleton(vtkSmartPointer<vtkPolyData> skeleton) override;

      void setSpacing(const NmVector3 &spacing);

      NmVector3 spacing() const;

      void undo()
      { /* TODO: not allowed */ };

      size_t memoryUsage() const;

    protected:
      virtual bool fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id) override
      { return SkeletonData::fetchDataImplementation(storage, path, id); }

    private:
      virtual QList<Data::Type> updateDependencies() const override
      { return QList<Data::Type>(); }

    private:
      vtkSmartPointer<vtkPolyData> m_skeleton;
  };

  using RawSkeletonPtr  = RawSkeleton *;
  using RawSkeletonSPtr = std::shared_ptr<RawSkeleton>;

  /** \brief Obtains and returns the RawSkeleton smart pointer of the specified Output.
   * \param[in] output, Output object smart pointer.
   */
  RawSkeletonSPtr rawSkeleton(OutputSPtr output);

} // namespace ESPINA

#endif // ESPINA_RAW_SKELETON_H_
