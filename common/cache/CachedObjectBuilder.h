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

#include <common/cache/Cache.h>

#include <common/processing/pqFilter.h>

class pqPipelineSource;

/// A class to provide ParaView pqPipelineSources, either using
/// ESPINA cache system or creating a new one if not available.
class CachedObjectBuilder
{
public:
  static CachedObjectBuilder *instance();

  pqFilter *loadFile(const QString file);
  pqFilter *createFilter(const QString group, const QString name, const pqFilter::Arguments args, bool persistent=false, bool ignoreCache=false);
  pqFilter *registerFilter(const QString id, pqPipelineSource* source);
  pqFilter *getFilter(Cache::Index &id) { return m_cache->getEntry(id); }
  void removeFilter(pqFilter *filter);

  static Cache::Index generateId(const QString group, const QString name, const pqFilter::Arguments args);
  
private:
  CachedObjectBuilder();
  ~CachedObjectBuilder(){}

  CachedObjectBuilder(const CachedObjectBuilder&);//Not implemented
  void *operator=(const CachedObjectBuilder&);//Not implemented

  pqPipelineSource *createSMFilter(const QString group, const QString name, const pqFilter::Arguments args);

  static CachedObjectBuilder *m_singleton;
  Cache *m_cache;
};

#endif // CACHEDOBJECTBUILDER_H
