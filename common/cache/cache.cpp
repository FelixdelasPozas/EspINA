/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#include "cache.h"

// Debug
#include "espina_debug.h"

#include <filter.h>

// Qt
#include <QStringList>

#include <boost/filesystem.hpp>

// ParaView
#include "pqPipelineSource.h"
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqActiveObjects.h"
#include "pqServer.h"

#include "vtkSMProxy.h"
#include <pqLoadDataReaction.h>

Cache *Cache::m_singleton = NULL;

Cache* Cache::instance()
{
  if (!m_singleton)
    m_singleton = new Cache();
  return m_singleton;
}

void Cache::insert(const Index& index, vtkFilter* filter, bool persistent)
{
  if ( index != filter->id())
    CACHE_DEBUG("Cache: Inserting ... different id");
  //m_translator.insert(id,index);
  if (m_cachedProxies.contains(index))
  {
    m_cachedProxies[index].refCounter++;
  }else{
    CACHE_DEBUG("Inserting" << index);
    Entry newEntry;
    newEntry.refCounter = 1 + persistent?1:0;
    newEntry.filter = filter;
    m_cachedProxies.insert(index,newEntry);
  }
}

void Cache::reference(const Cache::Index& index)
{
  m_cachedProxies[index].refCounter++;
  CACHE_DEBUG(index << "already has" << m_cachedProxies[index].refCounter << "references");
}

vtkFilter *Cache::getEntry(const Cache::Index index)
{
  // First we try to recover the proxy from cache
  if (m_cachedProxies.contains(index))
  {
    CACHE_DEBUG(index << " HIT");
    return m_cachedProxies[index].filter;
  }
  else
  {
    // Try to load from cache disk
    QStringList fileName(m_diskCachePath.filePath(index + ".pvd"));
    pqPipelineSource *diskSource = pqLoadDataReaction::loadData(fileName);
    if( diskSource )
    {
      CACHE_DEBUG(index << " HIT Disk Cache");
      // insert it in the cache
      vtkFilter* diskEntryFilter = new vtkFilter(diskSource, index);
      insert(index, diskEntryFilter);
      // The last vtkFilter inserted has increments the refCounter in one
      // because intialization. But it is not used yet by anyone.
      m_cachedProxies[index].refCounter--;
      return diskEntryFilter;
    }else{
      CACHE_DEBUG("Cache: " << index << "Failed to found entry");
      return NULL;
    }
  }
}

void Cache::remove(const Cache::Index& index)
{
  assert(m_cachedProxies.contains(index));
  m_cachedProxies[index].refCounter--;
  CACHE_DEBUG(index << "already has" << m_cachedProxies[index].refCounter << "references");
  if (m_cachedProxies[index].refCounter <= 0)
  {
    CACHE_DEBUG(index << "removed");
    delete m_cachedProxies[index].filter;
    m_cachedProxies.remove(index);
    assert(!m_cachedProxies.contains(index));
  }
}

// CacheEntry* Cache::getEspinaEntry(const EspinaId& id) const
// {
//   CacheIndex index = m_translator.value(id,"");
//   assert(index != "");
//   return getEntry(index);
// }

//-----------------------------------------------------------------------------
void Cache::setWorkingDirectory(QFileInfo& sample)
{
  m_diskCachePath = sample.filePath();
}




