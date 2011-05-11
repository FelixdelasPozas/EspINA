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

#ifndef CACHE_H
#define CACHE_H

#include <QMap>
#include <QString>

class vtkFilter;
//typedef QString EspinaId;

class Cache
{
public:
  typedef QString Index;
  static Cache *instance();
  void insert(const Index& index, vtkFilter *filter);
  vtkFilter *getEntry(const Index index) const;
  //CacheEntry *getEspinaEntry(const EspinaId &id) const;
  
protected:
  Cache(const QString &path="cache") : m_diskCachePath(path) {};
  
private:
  static Cache *m_singleton;
  //QMap<EspinaId, CacheIndex> m_translator;// Relation between EspinaId and CacheIndex
  QMap<Index, vtkFilter *> m_cachedProxies;
  QString m_diskCachePath; 
};

#endif // CACHE_H
