/*

 Copyright (C) 2014 Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_RAW_DATA_FACTORY_H
#define ESPINA_RAW_DATA_FACTORY_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/DataFactory.h>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Data/SkeletonData.h>
#include <Core/Analysis/Data/VolumetricData.hxx>

namespace ESPINA
{
  /** \class RawDataFactory
   * \brief Implements a data factory that returns non-dependent datas (raw types).
   *
   */
  class EspinaCore_EXPORT RawDataFactory
  : public DataFactory
  {
    public:
      virtual DataSPtr createData(OutputSPtr           output,
                                  TemporalStorageSPtr  storage,
                                  const QString       &path,
                                  QXmlStreamAttributes info) override;

    protected:
      /** \brief Helper method to fetch a volume from storage.
       * \param[in] output output that has the data.
       * \param[in] storage temporal storage with the data files.
       * \param[in] path path to the data files.
       * \param[in] bounds data bounds.
       *
       */
      virtual DefaultVolumetricDataSPtr createVolumetricData(OutputSPtr          output,
                                                             TemporalStorageSPtr storage,
                                                             const QString      &path,
                                                             const VolumeBounds &bounds);

      /** \brief Helper method to fetch a mesh from storage.
       * \param[in] output output that has the data.
       * \param[in] storage temporal storage with the data files.
       * \param[in] path path to the data files.
       * \param[in] bounds data bounds.
       *
       */
      virtual MeshDataSPtr createMeshData(OutputSPtr          output,
                                          TemporalStorageSPtr storage,
                                          const QString      &path,
                                          const VolumeBounds &bounds);

      /** \brief Helper method to fetch a skeleton from storage.
       * \param[in] output output that has the data.
       * \param[in] storage temporal storage with the data files.
       * \param[in] path path to the data files.
       * \param[in] bounds data bounds.
       *
       */
      virtual SkeletonDataSPtr createSkeletonData(OutputSPtr          output,
                                                  TemporalStorageSPtr storage,
                                                  const QString      &path,
                                                  const VolumeBounds &bounds);
  };
} // namespace ESPINA

#endif // ESPINA_FETCH_RAW_DATA_H
