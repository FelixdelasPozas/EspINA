/*
 * Copyright 2014 Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef ESPINA_MARCHING_CUBES_FROM_FETCHED_VOLUMETRIC_DATA_H
#define ESPINA_MARCHING_CUBES_FROM_FETCHED_VOLUMETRIC_DATA_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/DataFactory.h>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Data/VolumetricData.hxx>

namespace ESPINA
{
  class EspinaCore_EXPORT MarchingCubesFromFetchedVolumetricData
  : public DataFactory
  {
  public:
    virtual DataSPtr createData(OutputSPtr output, TemporalStorageSPtr storage, const QString &path, QXmlStreamAttributes info) override;

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
  };
} // namespace ESPINA

#endif // ESPINA_MARCHING_CUBES_FROM_FETCHED_VOLUMETRIC_DATA_H
