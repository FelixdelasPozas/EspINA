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

#include "cachedObjectBuilder.h"
#include "cache.h"

// ParaQ includes
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include <vtkSMProxy.h>
#include <vtkSMProperty.h>

CachedObjectBuilder * CachedObjectBuilder::m_singleton = NULL;

CachedObjectBuilder::CachedObjectBuilder()
{
  m_cache = new Cache();
}

CachedObjectBuilder* CachedObjectBuilder::instance()
{
  if (!m_singleton)
    m_singleton = new CachedObjectBuilder();
  return m_singleton;
}


EspinaProxy* CachedObjectBuilder::createFilter(
  std::string group
, std::string name
, ParamList args
  )
{
  // Create cache entry
  CacheIndex entryIndex(5);//(group,name,args);
  EspinaProxy * proxy = m_cache->getEntry(entryIndex);
  if (proxy)
    return proxy;
  
  proxy = createSMFilter(group,name,args);
  m_cache->insert(entryIndex,proxy);
}

pqPipelineSource *CachedObjectBuilder::createSMFilter(
  std::string group
, std::string name
, ParamList args)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  
  pqPipelineSource *filter; //= builder->createFilter(group, name,NULL);
  initFilter(filter,args);
}


void CachedObjectBuilder::initFilter(pqPipelineSource* filter, ParamList args)
{
}



