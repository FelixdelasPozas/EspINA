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
#include <boost/graph/graph_concepts.hpp>


#include <QDebug>

CachedObjectBuilder * CachedObjectBuilder::m_singleton = NULL;

CachedObjectBuilder::CachedObjectBuilder()
{
  m_cache = Cache::instance();
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
  CacheIndex entryIndex(5);//createIndex(group,name,args);
  
  EspinaProxy * proxy = m_cache->getEntry(entryIndex);
  if (proxy)
    return proxy;
  
  proxy = createSMFilter(group,name,args);
  m_cache->insert(entryIndex,proxy);
  return proxy;
}

pqPipelineSource *CachedObjectBuilder::createSMFilter(
  std::string group
, std::string name
, ParamList args)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* ob = core->getObjectBuilder();
  
  qDebug() << "Create Filter: " << group.c_str() << "::" << name.c_str();
  pqPipelineSource *filter; //= builder->createFilter(group, name,NULL);
  for (int p = 0; p < args.size(); p++)
  {
    if (args[p].first == "input")
    {
      pqPipelineSource *proxy = m_cache->getEntry(args[p].second.c_str());
      filter = ob->createFilter(group.c_str(),name.c_str(),proxy);
    }
    else
      qDebug() << "Unkown parameter";
  }
 // initFilter(filter,args);
 return filter;
}


void CachedObjectBuilder::initFilter(pqPipelineSource* filter, ParamList args)
{
}



