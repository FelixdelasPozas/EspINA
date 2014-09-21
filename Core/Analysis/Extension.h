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
#include <Core/Analysis/Persistent.h>

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
    using Type        = QString;
    using TypeList    = QList<Type>;
    using InfoTag     = QString;
    using InfoTagList = QList<InfoTag>;
    using InfoCache   = QMap<InfoTag, QVariant>;

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
                this, SLOT(invalidate()));
      }

      onExtendedItemSet(item);
    }

    /** \brief Returns the extended item.
     *
     */
    T *extendedItem() {return m_extendedItem;}

    /** \brief Returns a list of tags this extension have information of.
     *
     */
    virtual InfoTagList availableInformations() const = 0;

    /** \brief Returns true if the information has been computed or loaded.
     *
     */
    InfoTagList readyInformation() const
    {
      QReadLocker locker(&m_lock);

      return m_infoCache.keys();
    }

    /** \brief Returns the value of the information tag provided.
     * \param[in] tag, requested information tag.
     *
     */
    QVariant information(const InfoTag &tag) const
    {
      Q_ASSERT(availableInformations().contains(tag));

      QVariant info = cachedInfo(tag);

      if (!info.isValid())
      {
        info = cacheFail(tag);
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
     * \param[in] infoCache, extension cache object.
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
     * \param[in] tag, information key.
     *
     */
    virtual QVariant cacheFail(const InfoTag &tag) const = 0;

    /** \brief Returns the information from the cache.
     * \param[in] tag, information key.
     *
     */
    QVariant cachedInfo(const InfoTag &tag) const
    {
      QReadLocker locker(&m_lock);

      return m_infoCache.value(tag, QVariant());
    }

    /** \brief Updates the cache of the key with a value.
     * \param[in] tag, information key.
     * \param[value] value, information value.
     *
     */
    void updateInfoCache(const InfoTag &tag, const QVariant &value) const
    {
      QWriteLocker locker(&m_lock);

      m_infoCache[tag] = value;
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
    mutable InfoCache m_infoCache; // TODO: make private, fix SegmentationNotes first.

  private:
    mutable QReadWriteLock m_lock;
  };

  class EspinaCore_EXPORT ChannelExtension
  : public Extension<Channel>
  {
    Q_OBJECT

  public slots:
		/** \brief Implements Extension::invalidate().
		 *
		 */
    virtual void invalidate()
    {
      Extension<Channel>::invalidate();
    }

  protected:
    /** \brief ChannelExtension class constructor.
     * \param[in] infoCache, cache object.
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
     * \param[in] classificationName, classification name.
     *
     */
    virtual bool validCategory(const QString &classificationName) const = 0;

  public slots:
		/** \brief Implements Extension::invalidate().
		 *
		 */
    virtual void invalidate()
    {
      Extension<Segmentation>::invalidate();
    }

  protected:
  /** \brief SegmentationExtension class constructor.
   * \param[in] infoCache, cache object.
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

} // namespace ESPINA

#endif // ESPINA_EXTENSION_H
