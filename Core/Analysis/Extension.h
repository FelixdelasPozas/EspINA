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

#include "EspinaCore_Export.h"

#include "Core/EspinaTypes.h"
#include <Core/Analysis/Persistent.h>
#include <QVariant>
#include <QReadWriteLock>

namespace EspINA
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
    virtual ~Extension() {}

    virtual Type type() const = 0;

    virtual bool invalidateOnChange() const = 0;

    virtual State state() const = 0;

    virtual Snapshot snapshot() const = 0;

    virtual TypeList dependencies() const = 0;

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

    T *extendedItem() {return m_extendedItem;}

    virtual InfoTagList availableInformations() const = 0;

    InfoTagList readyInformation() const
    {
      QReadLocker locker(&m_lock);

      return m_infoCache.keys();
    }

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

    virtual QString toolTipText() const
    { return QString(); }

  protected:
    Extension(const InfoCache &infoCache)
    : m_extendedItem{nullptr}
    , m_infoCache{infoCache}
    {}

    static QString Path()
    { return "Extensions"; }

    QString snapshotName(const QString &file) const
    {
      return QString("%1/%2/%3_%4").arg(Path())
                                   .arg(type())
                                   .arg(m_extendedItem->uuid())
                                   .arg(file);
    };

    virtual void onExtendedItemSet(T *item) = 0;

    virtual QVariant cacheFail(const InfoTag &tag) const = 0;

    QVariant cachedInfo(const InfoTag &tag) const
    {
      QReadLocker locker(&m_lock);

      return m_infoCache.value(tag, QVariant());
    }

    void updateInfoCache(const InfoTag &tag, const QVariant &value) const
    {
      QWriteLocker locker(&m_lock);

      m_infoCache[tag] = value;
    }

  protected:
    virtual void invalidate()
    {
      QWriteLocker locker(&m_lock);

      m_infoCache.clear();
    }

  protected:
    T *m_extendedItem;

  private:
    mutable QReadWriteLock m_lock;
    mutable InfoCache m_infoCache;
  };

  class ChannelExtension
  : public Extension<Channel>
  {
    Q_OBJECT

  public slots:
    virtual void invalidate()
    {
      Extension<Channel>::invalidate();
    }

  protected:
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

  class SegmentationExtension
  : public Extension<Segmentation>
  {
    Q_OBJECT
  public:
    virtual bool validCategory(const QString &classificationName) const = 0;

  public slots:
    virtual void invalidate()
    {
      Extension<Segmentation>::invalidate();
    }

  protected:
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

} // namespace EspINA

#endif // ESPINA_EXTENSION_H
