/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

 This program is free software: you can redistribute it and/or modify
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

#ifndef SAS_FETCH_BEHAVIOUR_H_
#define SAS_FETCH_BEHAVIOUR_H_

// EspINA
#include <Core/IO/FetchBehaviour/RasterizedVolumeFromFetchedMeshData.h>

namespace EspINA
{
  
  class SASFetchBehaviour
  : public RasterizedVolumeFromFetchedMeshData
  {
    public:
      /* \brief SASFetchBehaviour class virtual destructor.
       *
       */
      virtual ~SASFetchBehaviour()
      {};

    protected:
      virtual MeshDataSPtr fetchMeshData(OutputSPtr output, TemporalStorageSPtr storage, QString prefix);
  };

} // namespace EspINA

#endif // SAS_FETCH_BEHAVIOUR_H_
