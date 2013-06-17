/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef MODELITEMEXTENSION_H
#define MODELITEMEXTENSION_H

#include "EspinaCore_Export.h"

#include "Core/Model/ModelItem.h"

#include <QDir>
#include <QMap>
#include <QStringList>
#include <QVariant>

#include <quazipfile.h>

namespace EspINA
{

  class EspinaCore_EXPORT ModelItem::Extension
  : public QObject
  {
  public:
    template<class D>
    class CacheEntry
    {
    public:
      explicit CacheEntry()
      : Dirty(false)
      {}

      bool Dirty;
      D    Data;
    };

    template <class K, class D>
    class EspinaCore_EXPORT Cache
    : public QMap<K, CacheEntry<D> >
    {
      typedef QMap<K, CacheEntry<D> > Base;

    public:
      explicit Cache()
      {}

      bool isCached(K key)
      {
        return Base::contains(key) && !Base::value(key).Dirty;
      }

      void markAsDirty(K key)
      {
        if (Base::contains(key))
          Base::operator[](key).Dirty = true;
      }

      void markAsClean(K key)
      {
        if (Base::contains(key))
          Base::operator[](key).Dirty = false;
      }

      /// Remove all dirty entries
      void purge(bool(*cond)(K) = NULL)
      {
        foreach(K key, this->keys())
        {
          if (this->value(key).Dirty 
          || key->m_model == NULL
          || (cond && cond(key)))
            Base::remove(key);
        }
      }
    };

  public:
    virtual ~Extension(){}

    virtual ModelItem::ExtId id() = 0;
    /// List of extension names which need to be loaded to use the extension
    virtual ModelItem::ExtIdList dependencies() const  = 0;

    virtual bool isCacheFile(const QString &file) const = 0;

    virtual void loadCache(QuaZipFile &file, const QDir &tmpDir, IEspinaModel *model) = 0;

    virtual bool saveCache(Snapshot &snapshot) = 0;

  protected:
    Extension() : m_init(false) {}
    mutable bool m_init;
  };

} // namespace EspINA

#endif // MODELITEMEXTENSION_H
