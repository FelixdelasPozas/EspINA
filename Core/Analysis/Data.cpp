/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
