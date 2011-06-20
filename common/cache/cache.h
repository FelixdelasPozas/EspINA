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
#include <QDir>

class vtkFilter;
//typedef QString EspinaId;

class Cache
{
  struct Entry
  {
    int refCounter;
    vtkFilter *filter;
  };
public:
  typedef QString Index;
  static Cache *instance();
  void insert(const Index& index, vtkFilter *filter, bool persistent=false);
  void reference(const Index& index);
  vtkFilter *getEntry(const Index index);
  void remove(const Index& index);
  /** Set the working directory of to the current Sample. It is used to find posible
   *  files for the cache disk
   */
  void setWorkingDirectory(QFileInfo& sample);
  //CacheEntry *getEspinaEntry(const EspinaId &id) const;
  
protected:
  Cache(const QDir &path=QDir::tempPath()) : m_diskCachePath(path) {};
  
private:
  static Cache *m_singleton;
  //QMap<EspinaId, CacheIndex> m_translator;// Relation between EspinaId and CacheIndex
  QMap<Index, Entry> m_cachedProxies;
  QDir m_diskCachePath;
};

#endif // CACHE_H
