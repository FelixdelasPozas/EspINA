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

#ifndef CACHEDOBJECTBUILDER_H
#define CACHEDOBJECTBUILDER_H
#include "cache.h"

#include <filter.h>

class pqPipelineSource;
class Cache;

//! A class to provide ParaView pqPipelineSources, either using
//! ESPINA cache system or creating a new one if not available.
class CachedObjectBuilder
{
public:
  static CachedObjectBuilder *instance();
  
  vtkFilter *createFilter(const QString group, const QString name, const vtkFilter::Arguments args, bool persistent=false);
  void removeFilter(vtkFilter *filter);
  vtkFilter *getFilter(Cache::Index &id) { return m_cache->getEntry(id); }

  /**
   * Insert a stack in the Espina Cache which has been already created in the server
   *  The only difference with createStack is that the pqPipelineSource was already
   *  created by ParaView system
   */
  //! Only used to load samples by pqPipelineDataReaction
  vtkFilter* registerProductCreator(QString& id, pqPipelineSource* source);

private:
  CachedObjectBuilder();
  ~CachedObjectBuilder(){}
  
  CachedObjectBuilder(const CachedObjectBuilder&);//Not implemented
  void *operator=(const CachedObjectBuilder&);//Not implemented
  
  Cache::Index generateId(const QString group, const QString name, const vtkFilter::Arguments args);
  pqPipelineSource *createSMFilter(const QString group, const QString name, const vtkFilter::Arguments args);
  
  static CachedObjectBuilder *m_singleton;
  Cache *m_cache;
};

#endif // CACHEDOBJECTBUILDER_H
