/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_ITEM_EXTENSION_H_
#define ESPINA_ITEM_EXTENSION_H_

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Types.h>
#include <Core/Analysis/ViewItem.h>
#include <Core/Utils/EspinaException.h>

// Qt
#include <QList>
#include <QReadWriteLock>
#include <QString>
#include <QStringList>

namespace ESPINA
{
  /** \class Extension
   * \brief Adds additional information to an item, thus extending it.
   *
   */
  template<typename T>
  class EspinaCore_EXPORT Extension
  : public QObject
  {
  public:
    using Type     = QString;
    using TypeList = QList<Type>;
    using Key      = QString;
    using KeyList  = QList<Key>;

    class InformationKey
    {
    public:
      InformationKey(const Type &extension, const QString &property)
      : m_extension(extension)
      , m_key(property)
      {}

      InformationKey(const InformationKey &key)
      : m_extension(key.extension())
      , m_key(key.value())
      {}

      InformationKey &operator=(const InformationKey &rhs)
      {
        m_extension = rhs.extension();
        m_key       = rhs.value();

        return *this;
      }

      bool operator==(const InformationKey &rhs) const
      {
        return extension() == rhs.extension() && value() == rhs.value();
      }

      bool operator!=(const InformationKey &rhs) const
      { return !operator==(rhs); }

      bool operator<(const InformationKey &rhs) const
      {
        return extension() <  rhs.extension()
            ||(extension() == rhs.extension() && value() < rhs.value());
      }

      const Type extension() const
      { return m_extension; }

      const Key value() const
      { return m_key; }

      operator Key() const
      { return m_key; }

    private:
      Type m_extension;
      Key  m_key;
    };

    using InformationKeyList = QList<InformationKey>;
    using InfoCache          = QMap<Key, QVariant>;

    static QString ExtensionFilePath(T *item)
    {
      return QString("%1/%2.xml").arg(Path())
                                 .arg(item->uuid());
    }

  public:
    /** \brief Extension class destructor.
     *
     */
    virtual ~Extension() {}

    /** \brief Returns the type the extension.
     *
     */
    virtual Type type() const = 0;

    /** \brief Returns true if the extension need to invalidate its data when the extended item changes.
     *
     */
    virtual bool invalidateOnChange() const = 0;

    /** \brief Returns a state object with the actual state of the extension.
     *
     */
    virtual State state() const = 0;

    /** \brief Returns a snapshot of the data of the extension.
     *
     */
    virtual Snapshot snapshot() const = 0;

    /** \brief Returns a list of extension types this extension depends on.
     *
     */
    virtual TypeList dependencies() const = 0;

    /** \brief Sets the item this extension extends.
     *
     */
    void setExtendedItem(T *item)
    {
      m_extendedItem = item;

      if (invalidateOnChange())
      {
        connect(m_extendedItem, SIGNAL(outputModified()),
                this,           SLOT(invalidate()), Qt::DirectConnection);
      }

      onExtendedItemSet(item);
    }

    /** \brief Returns the extended item.
     *
     */
    T *extendedItem() {return m_extendedItem;}

    /** \brief Returns a list of keys this extension have information of.
     *
     */
    virtual InformationKeyList availableInformation() const = 0;

    bool hasInformation(const InformationKey &key) const
    {
      return key.extension() == type() && availableInformation().contains(key);
    }

    /** \brief Returns the list of keys whose information is ready
     *
     */
    InformationKeyList readyInformation() const
    {
      QReadLocker locker(&m_lock);

      InformationKeyList keys;

      for (auto key : m_infoCache.keys())
      {
        keys << createKey(key);
      }

      return keys;
    }

    bool isReady(const InformationKey &key) const
    {
      if (key.extension() != type())
      {
        auto what = QObject::tr("Invalid extension key, key: %1.").arg(key);
        auto details = QObject::tr("Extension::isReady(key) -> Invalid extension key, key: %1.").arg(key);

        throw Core::Utils::EspinaException(what, details);
      }

      return readyInformation().contains(key);
    }

    /** \brief Returns the value of the information key provided.
     * \param[in] key informaton key
     *
     */
    QVariant information(const InformationKey &key) const
    {
      if (!hasInformation(key))
      {
        auto what = QObject::tr("Unknown extension key, key: %1.").arg(key);
        auto details = QObject::tr("Extension::information(key) -> Unknown extension key, key: %1.").arg(key);

        throw Core::Utils::EspinaException(what, details);
      }

      QVariant info = cachedInfo(key);

      if (!info.isValid())
      {
        info = cacheFail(key);
      }

      return info;
    }

    /** \brief Returns the tooltip text of the extension.
     *
     */
    virtual QString toolTipText() const
    { return QString(); }

  protected:
    /** \brief Extension class constructor.
     * \param[in] infoCache extension cache object.
     *
     */
    Extension(const InfoCache &infoCache)
    : m_extendedItem{nullptr}
    , m_infoCache   {infoCache}
    {}

    /** \brief Returns the extension path for saving data.
     *
     */
    static QString Path()
    { return "Extensions"; }

    /** \brief Returns the snapshot file name.
     *
     */
    QString snapshotName(const QString &file) const
    {
      return QString("%1/%2/%3_%4").arg(Path())
                                   .arg(type())
                                   .arg(m_extendedItem->uuid())
                                   .arg(file);
    };

    /** \brief Performs custom operations when the extended item is set in derived classes.
     *
     */
    virtual void onExtendedItemSet(T *item) = 0;

    /** \brief Recomputes or reloads the information when the cache fails.
     * \param[in] key information key.
     *
     */
    virtual QVariant cacheFail(const InformationKey &key) const = 0;

    /** \brief Returns the information from the cache.
     * \param[in] key information key.
     *
     */
    QVariant cachedInfo(const InformationKey &key) const
    {
      QReadLocker locker(&m_lock);

      return m_infoCache.value(key, QVariant());
    }

    /** \brief Updates the cache of the key with a value.
     * \param[in] key   information key.
     * \param[in] value information value.
     *
     */
    void updateInfoCache(const Key &key, const QVariant &value) const
    {
      QWriteLocker locker(&m_lock);

      m_infoCache[key] = value;
    }

    /** \brief Invalidates the cache values.
     *
     */
    virtual void invalidate()
    {
      QWriteLocker locker(&m_lock);

      m_infoCache.clear();
    }

    InformationKey createKey(const Key &value) const
    { return InformationKey(type(), value); }

  protected:
    T *m_extendedItem;
    mutable InfoCache m_infoCache;

  private:
    mutable QReadWriteLock m_lock;
  };

}

// Qt

#endif // ESPINA_ITEM_EXTENSION_H_
