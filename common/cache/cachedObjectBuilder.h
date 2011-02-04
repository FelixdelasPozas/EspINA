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

#include "translatorTable.h"

class pqPipelineSource;
class Cache;

//! A class to provide ParaView proxies, either using
//! ESPINA cache system or creating a new one if not
//! available.
class CachedObjectBuilder
{
public:
  static CachedObjectBuilder *instance();
  EspinaProxy *createFilter(QString group, QString name, VtkParamList args);
  
private:
  CachedObjectBuilder();
  ~CachedObjectBuilder(){}
  
  CachedObjectBuilder(const CachedObjectBuilder&);//Not implemented
  void *operator=(const CachedObjectBuilder&);//Not implemented
  pqPipelineSource *createSMFilter(QString group, QString name, VtkParamList args);
 // void initFilter(pqPipelineSource* filter, ParamList args);
  
  static CachedObjectBuilder *m_singleton;
  Cache *m_cache;
};

#endif // CACHEDOBJECTBUILDER_H
