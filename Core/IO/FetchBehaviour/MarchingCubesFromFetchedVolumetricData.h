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
#include <Core/Analysis/FetchBehaviour.h>
#include <Core/Analysis/Data/VolumetricData.hxx>

namespace ESPINA
{
  class EspinaCore_EXPORT MarchingCubesFromFetchedVolumetricData
  : public FetchBehaviour
  {
  public:
  	/* \brief Implements FetchBehaviour::fetchOutputData().
  	 *
  	 */
    virtual void fetchOutputData(OutputSPtr output, TemporalStorageSPtr storage, QString prefix, QXmlStreamAttributes info);

  protected:
    /* \brief Helper method to fetch a volume from storage.
     *
     */
    virtual DefaultVolumetricDataSPtr fetchVolumetricData(OutputSPtr output, TemporalStorageSPtr storage, QString prefix);
  };
} // namespace ESPINA

#endif // ESPINA_MARCHING_CUBES_FROM_FETCHED_VOLUMETRIC_DATA_H
