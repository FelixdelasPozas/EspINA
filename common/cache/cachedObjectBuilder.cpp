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

// Debug
#include "espina_debug.h"

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
#include <pqOutputPort.h>


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

vtkFilter* CachedObjectBuilder::createFilter(const QString group, const QString name, const vtkFilter::Arguments args, bool persistent)
{
  // Create cache entry
  Cache::Index id = generateId(group, name, args);
  
  vtkFilter *filter = getFilter(id);
  if (filter)
  {
    m_cache->reference(filter->id());
    return filter;
  }
  
  pqPipelineSource *proxy = createSMFilter(group, name, args);
  filter = new vtkFilter(proxy, id);
  m_cache->insert(id,filter, persistent);
  return filter;
}

void CachedObjectBuilder::removeFilter(vtkFilter* filter)
{
  m_cache->remove(filter->id());
}


Cache::Index CachedObjectBuilder::generateId(const QString group, const QString name, const vtkFilter::Arguments args)
{
  QStringList namesToHash;
  namesToHash.push_back( QString(name) );
  
  foreach(vtkFilter::Argument arg, args)
  {
    namesToHash.push_back( QString(arg.type));
    namesToHash.push_back(arg.name);
    namesToHash.push_back(arg.value);
  }
  
  return generateSha1(namesToHash);
}


pqPipelineSource *CachedObjectBuilder::createSMFilter(const QString group, const QString name, const vtkFilter::Arguments args)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* ob = core->getObjectBuilder();
  
  CACHE_DEBUG("CachedObjectBuilder: Create Filter " << name);
  pqPipelineSource *filter; //= builder->createFilter(group, name,NULL);
  vtkSMProperty *p;
  foreach (vtkFilter::Argument arg, args)
  {
    switch (arg.type)
    {
      case INPUT:
      {
	// Filter is a source
	if (name == "CountingRegion")
	{
	  QStringList input = arg.value.split(":");
	  assert(input.size()==2);
	  vtkFilter *inputCreator = m_cache->getEntry(input[0]);
	  assert(inputCreator);
	  QMap<QString, QList<pqOutputPort *> > namedInputs;
	  QList<pqOutputPort *> inputs;
	  inputs.push_back(inputCreator->pipelineSource()->getOutputPort(0));
	  namedInputs["Input"] = inputs;
// 	  QList<pqOutputPort *> regions;
// 	  regions.push_back(inputCreator->pipelineSource()->getOutputPort(0));
// 	  namedInputs["Regions"] = inputs;
	  filter = ob->createFilter(group, name, namedInputs, pqApplicationCore::instance()->getActiveServer());
	}else if (arg.value == "")
	{
	  filter = ob->createSource(group, name, pqApplicationCore::instance()->getActiveServer());
	}
	else
	{
	  QStringList input = arg.value.split(":");
	  assert(input.size()==2);
	  vtkFilter *inputCreator = m_cache->getEntry(input[0]);
	  assert(inputCreator);
	  filter = ob->createFilter(group, name, inputCreator->pipelineSource(), input[1].toInt());
	}
      }
      break;
      case INTVECT:
	assert(filter);
	{
	  p = filter->getProxy()->GetProperty( arg.name.toStdString().c_str() );
	  vtkSMIntVectorProperty * prop = vtkSMIntVectorProperty::SafeDownCast(p);
	  QStringList values = arg.value.split(",");
	  CACHE_DEBUG("CachedObjectBuilder:" << arg.name << "Values" <<  values);
	  for (int i = 0; i < values.size(); i++)
	    prop->SetElement(i, values[i].toInt());
	}
	break;
      case DOUBLEVECT:
	assert(filter);
	{
	  p = filter->getProxy()->GetProperty( arg.name.toStdString().c_str() );
	  vtkSMDoubleVectorProperty * prop = vtkSMDoubleVectorProperty::SafeDownCast(p);
	  QStringList values = arg.value.split(",");
	  qDebug() << "CachedObjectBuilder:" << arg.name << "Values" <<  values;
	  for (int i = 0; i < values.size(); i++)
	    prop->SetElement(i, values[i].toDouble());
	}
	break;
      default:
	qDebug() << "Unkown parameter type";
	assert(false);
    };
  }
  assert(filter);
  //TODO: Review if needed here
  filter->getProxy()->UpdateVTKObjects();
//   filter->updatePipeline();
 return filter;
}

//-----------------------------------------------------------------------------
/**
 * For stacks loaded from the client-side
 */
// EspinaProxy* CachedObjectBuilder::createStack(QString& filePath)
// {
//   QStringList v;
//   v.push_back( filePath );
//   CacheIndex sampleHash = generateSha1(v);
//   CacheEntry* proxy = m_cache->getEntry(sampleHash);
//   if( proxy )
//     return proxy;
//   
//   proxy = pqLoadDataReaction::loadData( QStringList(filePath) );
//   m_cache->insert(filePath, sampleHash, proxy);
//   return proxy;
// }

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
 * If it exists it will be overwrited by the nwe pqPipelineSource
 */
vtkFilter* CachedObjectBuilder::registerProductCreator(QString& sampleFile, pqPipelineSource* source)
{
  vtkFilter* filter = m_cache->getEntry(sampleFile);
  if( filter )
  {
    m_cache->remove(sampleFile);
    assert(!m_cache->getEntry(sampleFile));
  }
  
  filter = new vtkFilter(source, sampleFile);
  m_cache->insert(sampleFile, filter);
  return filter;
}


