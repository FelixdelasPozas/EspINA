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

#ifndef ESPINA_EXTENSIONS_H
#define ESPINA_EXTENSIONS_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/ItemExtension.hxx>

// Qt
#include <QVariant>
#include <QReadWriteLock>

namespace ESPINA
{
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

#endif // ESPINA_EXTENSIONS_H
