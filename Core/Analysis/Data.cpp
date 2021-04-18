/*

 Copyright (C) 2013 Jorge Peña Pastor <jpena@cesvima.upm.es>

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

// ESPINA
#include "Data.h"
#include "Output.h"
#include "Filter.h"

using namespace ESPINA;

//----------------------------------------------------------------------------
TimeStamp Data::s_tick = 0;

//----------------------------------------------------------------------------
void Data::setFetchContext(const TemporalStorageSPtr storage,
                           const QString &path,
                           const QString &id,
                           const VolumeBounds &bounds)
{
  QMutexLocker lock(&m_mutex);
  m_path        = path;
  m_id          = id;
  m_bounds      = bounds;
  m_storage     = storage;
  m_needFetch   = true;

  applyFixes();
}


//----------------------------------------------------------------------------
void Data::copyFetchContext(DataSPtr data)
{
  QMutexLocker lock(&m_mutex);
  m_path        = data->m_path;
  m_id          = data->m_id;
  m_storage     = data->m_storage;

  if(m_needFetch)
  {
    m_bounds = data->bounds();
  }

  applyFixes();
}

//----------------------------------------------------------------------------
bool Data::fetchData()
{
  QMutexLocker lock(&m_mutex);

  auto dataFetched = fetchDataImplementation(m_storage, m_path, m_id, m_bounds);

  m_needFetch = !dataFetched;

  return dataFetched;
}

//----------------------------------------------------------------------------
QList<Data::Type> Data::dependencies() const
{
  return updateDependencies();
}
