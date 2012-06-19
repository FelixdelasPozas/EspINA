/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>

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

#include "common/cache/CachedObjectBuilder.h"

// Debug
// #include "espina_debug.h"

#include "common/cache/CacheHash.h"
#include "common/File.h"

// Qt
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
#include <vtkSIProxy.h>


//------------------------------------------------------------------------
CachedObjectBuilder * CachedObjectBuilder::m_singleton = NULL;

//------------------------------------------------------------------------
CachedObjectBuilder::CachedObjectBuilder()
{
  m_cache = Cache::instance();
}

//------------------------------------------------------------------------
CachedObjectBuilder* CachedObjectBuilder::instance()
{
  if (!m_singleton)
    m_singleton = new CachedObjectBuilder();
  return m_singleton;
}

//------------------------------------------------------------------------
pqFilter* CachedObjectBuilder::loadFile(const QString file)
{
  Cache::Index id = File::extendedName(file);
  pqFilter *reader = getFilter(id);
  if (NULL == reader)
  {
    pqPipelineSource *source = pqLoadDataReaction::loadData(QStringList(file));
    if (!source)
      return NULL;

    reader = new pqFilter(source, id);
    m_cache->insert(id, reader, false);
  }else{
    m_cache->addReference(id);
  }
  Q_ASSERT(reader);

  return reader;
}

//------------------------------------------------------------------------
pqFilter* CachedObjectBuilder::createFilter(const QString group,
					    const QString name,
					    const pqFilter::Arguments args,
					    bool persistent,
					    bool ignoreCache)
{
  // Create cache entry
  Cache::Index id = generateId(group, name, args);

  pqFilter *filter = NULL;
  if (!ignoreCache)
  {
    filter = getFilter(id);
    if (filter)
    {
      m_cache->addReference(id);
      return filter;
    }
  }

  pqPipelineSource *proxy = createSMFilter(group, name, args);
  filter = new pqFilter(proxy, id);
  m_cache->insert(id, filter, persistent);
  return filter;
}

void CachedObjectBuilder::removeFilter(pqFilter* filter)
{
  m_cache->removeReference(filter->id());
}

Cache::Index CachedObjectBuilder::generateId(const QString group, const QString name, const pqFilter::Arguments args)
{
  QStringList namesToHash;
  namesToHash.push_back( QString(name) );

  foreach(pqFilter::Argument arg, args)
  {
    namesToHash.push_back( QString(arg.type));
    namesToHash.push_back(arg.name);
    namesToHash.push_back(arg.value);
  }
  
  return generateSha1(namesToHash);
}


pqPipelineSource *CachedObjectBuilder::createSMFilter(const QString group, const QString name, const pqFilter::Arguments args)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* ob     = core->getObjectBuilder();
  pqServer *activeServer  = core->getActiveServer();

  //   CACHE_DEBUG("CachedObjectBuilder: Create Filter " << name);
  pqPipelineSource *filter = NULL;
  vtkSMProperty *p;

  QMap<QString, QList<pqOutputPort*> > namedInputs;
  foreach (pqFilter::Argument arg, args)
  {
    if (arg.type == pqFilter::Argument::INPUT)
    {
      // Repeatable input
      QStringList inputs = arg.value.split(",");
      foreach (QString input, inputs)
      {
	QStringList inputData = input.split(":");
	Q_ASSERT(inputData.size()==2);
	pqFilter *inputCreator = m_cache->getEntry(inputData[0]);
	Q_ASSERT(inputCreator);
	pqOutputPort *port = inputCreator->pipelineSource()->getOutputPort(inputData[1].toInt());
	Q_ASSERT(port);
	// Recover already connected output ports for this input
	if (namedInputs.contains(arg.name))
	  namedInputs[arg.name] << port;
	else
	  namedInputs[arg.name] = (QList<pqOutputPort*>() << port);
      }
    }
  }

  // Filter is a source
  if (namedInputs.isEmpty())
    filter = ob->createSource(group, name, activeServer);
  else
    filter = ob->createFilter(group, name, namedInputs, activeServer);

  Q_ASSERT(filter != NULL);

  foreach (pqFilter::Argument arg, args)
  {
    switch (arg.type)
    {
      case pqFilter::Argument::INPUT:
	break;
      case pqFilter::Argument::INTVECT:
      {
	p = filter->getProxy()->GetProperty( arg.name.toStdString().c_str() );
	vtkSMIntVectorProperty * prop = vtkSMIntVectorProperty::SafeDownCast(p);
	QStringList values = arg.value.split(",");
	// 	  CACHE_DEBUG("CachedObjectBuilder:" << arg.name << "Values" <<  values);
	for (int i = 0; i < values.size(); i++)
	  prop->SetElement(i, values[i].toInt());

	break;
      }
      case pqFilter::Argument::DOUBLEVECT:
      {
	p = filter->getProxy()->GetProperty( arg.name.toStdString().c_str() );
	vtkSMDoubleVectorProperty * prop = vtkSMDoubleVectorProperty::SafeDownCast(p);
	QStringList values = arg.value.split(",");
	// 	  qDebug() << "CachedObjectBuilder:" << arg.name << "Values" <<  values;
	for (int i = 0; i < values.size(); i++)
	  prop->SetElement(i, values[i].toDouble());

	break;
      }
      default:
	qDebug() << "Unkown parameter type. Ignored";
    };
  }

  filter->getProxy()->UpdateVTKObjects();
  return filter;
}

pqFilter* CachedObjectBuilder::registerFilter(const QString id, pqPipelineSource* source)
{
  pqFilter* filter = m_cache->getEntry(id);
  if( filter )
  {
    m_cache->removeReference(id);
    Q_ASSERT(!m_cache->getEntry(id));
  }

  filter = new pqFilter(source, id);
  m_cache->insert(id, filter);
  return filter;
}


