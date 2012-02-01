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

class pqFilter;
//typedef QString EspinaId;

class Cache
{
  struct Entry
  {
    int refCounter;
    pqFilter *filter;
  };
public:
  typedef QString Index;

  static Cache *instance();

  /// Insert cache entry
  void insert(const Index& index, pqFilter *filter, bool persistent=false);
  /// Increase entry's reference count
  void addReference(const Index& index);
  /// Retrieve cache entry given its index
  pqFilter *getEntry(const Index index);
  /// Remove 
  void removeReference(const Index& index);

  /// Set the working directory of to the current Sample. It is used to find posible
  /// files for the cache disk
  void setWorkingDirectory(QFileInfo& sample);
  QDir workingDirectory() const {return m_diskCachePath;}
  //CacheEntry *getEspinaEntry(const EspinaId &id) const;

protected:
  Cache(const QDir &path=QDir::tempPath()) : m_diskCachePath(path) {};

private:
  static Cache *m_singleton;
  QMap<Index, Entry> m_cachedProxies;
  QDir m_diskCachePath;
};

#endif // CACHE_H
