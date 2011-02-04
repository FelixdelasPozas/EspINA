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

// Debug
#include <QDebug>

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
  CacheEntry *proxy;
  // First we try to recover the proxy from cache
  if (proxy = m_cachedProxies.value(index,NULL))
    return proxy;
  // If not available, try to read from disk/disk cache
  pqApplicationCore *core = pqApplicationCore::instance();
  pqObjectBuilder *ob = core->getObjectBuilder();
  pqServer * server= pqActiveObjects::instance().activeServer();
  QStringList file;
  file << index;
  // TODO: Only works in local mode!!!!
  if (boost::filesystem::exists(index.toStdString().c_str()))
    proxy = ob->createReader("sources","MetaImageReader",file,server);
  
  return proxy;
}




