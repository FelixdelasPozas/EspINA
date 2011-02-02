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

#include <QString>

// ParaQ includes
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include <vtkSMProxy.h>
#include <vtkSMProperty.h>
#include <vtkSMDoubleVectorProperty.h>

// TODO: Boost??
#include <boost/graph/graph_concepts.hpp>

// Debug
#include <QDebug>
#include <assert.h>



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
, VtkParamList args
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
, VtkParamList args)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* ob = core->getObjectBuilder();
  
  qDebug() << "Create Filter: " << group.c_str() << "::" << name.c_str();
  pqPipelineSource *filter; //= builder->createFilter(group, name,NULL);
  for (int p = 0; p < args.size(); p++)
  {
    VtkArg vtkArg = args[p].first;
    switch (vtkArg.type)
    {
      case INPUT:
      {
	pqPipelineSource *inputProxy = m_cache->getEntry(args[p].second.c_str());
	filter = ob->createFilter(group.c_str(),name.c_str(),inputProxy);
      }
      break;
      case DOUBLEVECT:
	assert(filter);
	{
	  vtkSMDoubleVectorProperty * prop = vtkSMDoubleVectorProperty::SafeDownCast(
	    filter->getProxy()->GetProperty(vtkArg.name.c_str())
	  );
	  QString value(args[p].second.c_str());
	  double th = value.toDouble();
	  qDebug() << "Value" <<  th;
	  prop->SetElements1(th);
	}
	break;
      default:
	qDebug() << "Unkown parameter type";
	assert(false);
    };
    /*
    if (args[p] == "input")
    {
      pqPipelineSource *proxy = m_cache->getEntry(args[p].second.c_str());
      filter = ob->createFilter(group.c_str(),name.c_str(),proxy);
    }
    else
      qDebug() << "Unkown parameter";
    */
  }
  assert(filter);
  filter->getProxy()->UpdateVTKObjects();
  filter->updatePipeline();
 // initFilter(filter,args);
 return filter;
}


// void CachedObjectBuilder::initFilter(pqPipelineSource* filter, ParamList args)
// {
// }



