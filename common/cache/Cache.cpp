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

#include "Cache.h"

#include "common/processing/pqFilter.h"

#include<QDebug>
#define DEBUG_CACHE 0

#define CACHE_DEBUG(exp) if (DEBUG_CACHE)      \
			  qDebug() << "Cache:" << exp;
			  

Cache *Cache::m_singleton = NULL;

Cache* Cache::instance()
{
  if (!m_singleton)
    m_singleton = new Cache();
  return m_singleton;
}

void Cache::insert(const Cache::Index& index, pqFilter* filter, bool persistent)
{
  Q_ASSERT(index == filter->id());

  //m_translator.insert(id,index);
  if (m_cachedProxies.contains(index))
  {
    m_cachedProxies[index].refCounter++;
  }else{
//     CACHE_DEBUG("Inserting" << index);
    Entry newEntry;
    newEntry.refCounter = 1 + persistent?1:0;
    newEntry.filter = filter;
    m_cachedProxies.insert(index, newEntry);
  }
}

void Cache::addReference(const Cache::Index& index)
{
  m_cachedProxies[index].refCounter++;
//   CACHE_DEBUG(index << "already has" << m_cachedProxies[index].refCounter << "references");
}

pqFilter *Cache::getEntry(const Cache::Index index)
{
  // Cache HIT
  if (m_cachedProxies.contains(index))
  {
    return m_cachedProxies[index].filter;
  }
  else
  {
    // Look for entry in disk cache
    //TODO: Change segmentation format
    QFileInfo cacheEntry = m_diskCachePath.filePath(index + ".pvd");
    if (cacheEntry.exists())
    {
      Q_ASSERT(false);
//       vtkAlgorithm *diskSource = reader::New();
//       diskSource->SetFileName(cacheEntry.absoluteFilePath());
//       // Disck Cache HIT
//       pqFilter* filter = new pqFilter(diskSource, index);
//       // update cache
//       insert(index, filter);
//       // The last vtkFilter inserted has incremented the refCounter in one
//       // because of intialization. But it is not used yet by anyone.
//       m_cachedProxies[index].refCounter--;
//       return filter;
    }else
    {
      //Cache MISS
      return NULL;
    }
  }
}

void Cache::removeReference(const Cache::Index& index)
{
  Q_ASSERT(false);
//   Q_ASSERT(m_cachedProxies.contains(index));
//   m_cachedProxies[index].refCounter--;
//   CACHE_DEBUG(index << "has" << m_cachedProxies[index].refCounter << "references");
//   if (m_cachedProxies[index].refCounter <= 0)
//   {
//     CACHE_DEBUG(index << "removed");
//     delete m_cachedProxies[index].filter;
//     m_cachedProxies.remove(index);
//     Q_ASSERT(!m_cachedProxies.contains(index));
//   }
}

//-----------------------------------------------------------------------------
void Cache::setWorkingDirectory(const QFileInfo& sample)
{
  m_diskCachePath = sample.filePath();
//   qDebug() <<  "Cache Directory" << m_diskCachePath;
}