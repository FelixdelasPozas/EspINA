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
#include "data/hash.h"

#include "filter.h"

// Qt
#include <QString>
#include <QStringList>

// ParaQ includes
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqLoadDataReaction.h"
#include <vtkSMProxy.h>
#include <vtkSMProperty.h>
#include <vtkSMIntVectorProperty.h>
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


EspinaProxy* CachedObjectBuilder::createFilter(Filter *filter)
{
  // Create cache entry
  QStringList namesToHash;
  namesToHash.push_back(QString(filter->name));
  namesToHash.append( reduceVtkArgs(filter->vtkArgs()) );
  CacheIndex entryIndex = generateSha1( namesToHash );//createIndex(group,name,args);
  
  EspinaProxy * proxy = m_cache->getEntry(entryIndex);
  if (proxy)
    return proxy;
  
  proxy = createSMFilter(filter);
  m_cache->insert(filter->id(),entryIndex,proxy);
  return proxy;
}

pqPipelineSource *CachedObjectBuilder::createSMFilter(Filter *filter)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* ob = core->getObjectBuilder();
  
  qDebug() << "CachedObjectBuilder: Create Filter " << filter->name;
  pqPipelineSource *proxy; //= builder->createFilter(group, name,NULL);
  VtkParamList args = filter->vtkArgs();
  for (int p = 0; p < args.size(); p++)
  {
    VtkArg vtkArg = args[p].first;
    switch (vtkArg.type)
    {
      case INPUT:
      {
	QStringList input = args[p].second.split(":");
	assert(input.size()==2);//(id,portNumber)
	pqPipelineSource *inputProxy = m_cache->getEspinaEntry(input[0]);
	assert(inputProxy);
	proxy = ob->createFilter(filter->group(), filter->name, inputProxy, input[1].toInt());
      }
      break;
      case INTVECT:
	assert(proxy);
	{
	  vtkSMIntVectorProperty * prop = vtkSMIntVectorProperty::SafeDownCast(
	    proxy->getProxy()->GetProperty(vtkArg.name.toStdString().c_str())
	  );
	  QStringList values = args[p].second.split(",");//   QString(args[p].second.c_str()).split(",");
	  qDebug() << "Values" <<  values;
	  for (int i = 0; i < values.size(); i++)
	    prop->SetElement(i, values[i].toInt());
	}
	break;
      case DOUBLEVECT:
	assert(proxy);
	{
	  vtkSMDoubleVectorProperty * prop = vtkSMDoubleVectorProperty::SafeDownCast(
	    proxy->getProxy()->GetProperty(vtkArg.name.toStdString().c_str())
	  );
	  QStringList values = args[p].second.split(","); //QString(args[p].second.c_str()).split(",");
	  qDebug() << "Values" <<  values;
	  for (int i = 0; i < values.size(); i++)
	    prop->SetElement(i, values[i].toDouble());
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
  assert(proxy);
  proxy->getProxy()->UpdateVTKObjects();
  proxy->updatePipeline();
 // initFilter(filter,args);
 return proxy;
}

//-----------------------------------------------------------------------------
/**
 * For stacks loaded from the client-side
 */
EspinaProxy* CachedObjectBuilder::createStack(QString& filePath)
{
  QStringList v;
  v.push_back( filePath );
  CacheIndex sampleHash = generateSha1(v);
  CacheEntry* proxy = m_cache->getEntry(sampleHash);
  if( proxy )
    return proxy;
  
  proxy = pqLoadDataReaction::loadData( QStringList(filePath) );
  m_cache->insert(filePath, sampleHash, proxy);
  return proxy;
}

/*
TODO there are two ways to load sample files.
  1.- ParaView way: it uses the default method of paraView to load files. It loads
          data from the server's filesystem. It has it's own ParaView GUI
          This uses EspinaMainWindow::loadData, insertLoadedStack, 
  2.- Our way: our own GUI. Internaly it uses the ParaView load data system
          This uses EspinaMainWindow::loadFile, insertLoadedStack,
*/
/**
 * Insert a stack in the Espina Cache which has been already created in the server
 * The only difference with createStack is that the pqPipelineSource was already
 * created by ParaView system
 * If it returns something different to NULL the element has been already registered
 * in the cache
 */
EspinaProxy* CachedObjectBuilder::registerLoadedStack(QString& filePath, EspinaProxy* source)
{
  QStringList v;
  v.push_back( filePath );
  CacheIndex sampleHash = generateSha1(v);
  CacheEntry* proxy = m_cache->getEntry(sampleHash);
  if( proxy )
    return proxy;
  m_cache->insert(filePath, sampleHash, source);
  return NULL;
}


// void CachedObjectBuilder::initFilter(pqPipelineSource* filter, ParamList args)
// {
// }



