/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#include "EspinaPluginManager.h"

#include <espina_debug.h>

#include <EspinaPlugin.h>


EspinaPluginManager *EspinaPluginManager::m_singleton = NULL;

//-----------------------------------------------------------------------------
void EspinaPluginManager::registerFilter(const QString& filter, IFilterFactory* factory)
{
  assert( m_filters.contains(filter) == false );
  m_filters.insert(filter, factory);
}

//-----------------------------------------------------------------------------
EspinaFilter* EspinaPluginManager::createFilter(const QString& filter, ITraceNode::Arguments& args)
{
  if (!m_filters.contains(filter))
    return NULL;
  
  IFilterFactory *factory = m_filters[filter];
  return factory->createFilter(filter,args);
}

//-----------------------------------------------------------------------------
void EspinaPluginManager::registerReader(const QString& extension, IFileReader* reader)
{
  assert( m_readers.contains(extension) == false );
  m_readers.insert(extension, reader);
}

//-----------------------------------------------------------------------------
void EspinaPluginManager::readFile(pqPipelineSource* proxy, const QString& filePath)
{
  const QString extension = filePath.section('.',-1);
  
  if (!m_readers.contains(extension))
    return;
  
  IFileReader *reader = m_readers[extension];
  reader->readFile(proxy,filePath);
}

//-----------------------------------------------------------------------------
void EspinaPluginManager::registerPreferencePanel(IPreferencePanel* panel)
{
  m_panels.push_back(panel);
}




