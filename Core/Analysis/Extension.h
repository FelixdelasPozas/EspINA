/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef ESPINA_EXTENSION_H
#define ESPINA_EXTENSION_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Core/EspinaTypes.h"
#include <Core/Analysis/ViewItem.h>

// Qt
#include <QVariant>
#include <QReadWriteLock>

namespace ESPINA
{

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

      void operator=(const InformationKey &rhs)
      {
        m_extension = rhs.extension();
        m_key       = rhs.value();
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

    private:
      Type m_extension;
      Key  m_key;
    };

    using InformationKeyList = QList<InformationKey>;
    using InfoCache          = QMap<Key, QVariant>;

    struct Existing_Extension{};
    struct Extension_Not_Found{};

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
                this,           SLOT(invalidate()));
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
    virtual KeyList availableInformation() const = 0;

    bool hasInformation(const Key &key) const
    { return availableInformation().contains(key); }

    /** \brief Returns the list of keys whose information is ready
     *
     */
    KeyList readyInformation() const
    {
      QReadLocker locker(&m_lock);

      return m_infoCache.keys();
    }

    bool isReady(const Key &key) const
    { return readyInformation().contains(key); }

    /** \brief Returns the value of the information key provided.
     * \param[in] key informaton key
     *
     */
    QVariant information(const Key &key) const
    {
      Q_ASSERT(hasInformation(key));

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
    virtual QVariant cacheFail(const Key &key) const = 0;

    /** \brief Returns the information from the cache.
     * \param[in] key information key.
     *
     */
    QVariant cachedInfo(const Key &key) const
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

  protected:
    /** \brief Invalidates the cache values.
     *
     */
    virtual void invalidate()
    {
      QWriteLocker locker(&m_lock);

      m_infoCache.clear();
    }

  protected:
    T *m_extendedItem;
    mutable InfoCache m_infoCache;

  private:
    mutable QReadWriteLock m_lock;
  };

  class EspinaCore_EXPORT ChannelExtension
  : public Extension<Channel>
  {
    Q_OBJECT

  public slots:
    virtual void invalidate()
    {
      Extension<Channel>::invalidate();
    }

  protected:
    /** \brief ChannelExtension class constructor.
     * \param[in] infoCache cache object.
     *
     */
    ChannelExtension(const InfoCache &infoCache)
    : Extension<Channel>(infoCache)
    {}
  };

  using ChannelExtensionPtr      = ChannelExtension *;
  using ChannelExtensionList     = QList<ChannelExtensionPtr>;
  using ChannelExtensionSPtr     = std::shared_ptr<ChannelExtension>;
  using ChannelExtensionSList    = QList<ChannelExtensionSPtr>;
  using ChannelExtensionSMap     = QMap<QString, ChannelExtensionSPtr>;
  using ChannelExtensionTypeList = QList<ChannelExtension::Type>;

  class EspinaCore_EXPORT SegmentationExtension
  : public Extension<Segmentation>
  {
    Q_OBJECT
  public:
    /** \brief Returns true if the extension is applicable to given category.
     * \param[in] classification name.
     *
     */
    virtual bool validCategory(const QString &classification) const = 0;

  public slots:
    virtual void invalidate()
    {
      Extension<Segmentation>::invalidate();
    }

  protected:
  /** \brief SegmentationExtension class constructor.
   * \param[in] infoCache cache object.
   *
   */
    SegmentationExtension(const InfoCache &infoCache)
    : Extension<Segmentation>(infoCache)
    {}
  };

  using SegmentationExtensionPtr      = SegmentationExtension *;
  using SegmentationExtensionList     = QList<SegmentationExtensionPtr>;
  using SegmentationExtensionSPtr     = std::shared_ptr<SegmentationExtension>;
  using SegmentationExtensionSList    = QList<SegmentationExtensionSPtr>;
  using SegmentationExtensionSMap     = QMap<QString,SegmentationExtensionSPtr>;
  using SegmentationExtensionTypeList = QList<SegmentationExtension::Type>;

  template<typename E> typename E::InformationKey createKey(const std::shared_ptr<E> &extension, const typename E::Key &key)
  {
    return typename E::InformationKey(extension->type(), key);
  }

} // namespace ESPINA

#endif // ESPINA_EXTENSION_H
