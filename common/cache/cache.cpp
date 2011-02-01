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
#include "pqPipelineSource.h"

Cache *Cache::m_singleton = NULL;

Cache* Cache::instance()
{
  if (!m_singleton)
    m_singleton = new Cache();
  return m_singleton;
}

void Cache::insert(const CacheIndex& index, CacheEntry* entry)
{
  m_cachedProxies.insert(index,entry);
}


CacheEntry* Cache::getEntry(const CacheIndex index) const
{
  //if (m_cachedProxies.find(index)
  // TODO: Check for null results
  return NULL; // Force cache fail
  return m_cachedProxies[index];
}




