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
  namespace Core
  {
    /** \class StackExtension
     * \brief Implements Extension for stacks.
     *
     */
    class EspinaCore_EXPORT StackExtension
    : public Extension<Channel>
    {
        Q_OBJECT
      signals:
        void invalidated();

      public slots:
        virtual void invalidate()
        { Extension<Channel>::invalidate(); emit invalidated(); }

      protected:
        /** \brief StackExtension class constructor.
         * \param[in] infoCache cache object.
         *
         */
        StackExtension(const InfoCache &infoCache)
        : Extension<Channel>(infoCache)
        {}
    };

    using StackExtensionPtr      = StackExtension *;
    using StackExtensionSPtr     = std::shared_ptr<StackExtension>;
    using StackExtensionList     = QList<StackExtensionPtr>;
    using StackExtensionSList    = QList<StackExtensionSPtr>;
    using StackExtensionSMap     = QMap<QString, StackExtensionSPtr>;

    /** \class SegmentationExtension
     * \brief Implements Extension class for segmentations.
     *
     */
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

        /** \brief Returns true if the extension can get results with the given data.
         * \param[in] output output containing the data.
         *
         */
        virtual bool validData(const OutputSPtr output) const = 0;

      signals:
        void invalidated();

      public slots:
        virtual void invalidate()
        {
          Extension<Segmentation>::invalidate();

          invalidateImplementation();

          emit invalidated();
        }

      protected:
        /** \brief Particular invalidate actions of the extension. To be implemented on the extension that needs it, default empty.
         *
         */
        virtual void invalidateImplementation()
        {};

        /** \brief SegmentationExtension class constructor.
         * \param[in] infoCache cache object.
         *
         */
        SegmentationExtension(const InfoCache &infoCache)
        : Extension<Segmentation>(infoCache)
        {}
    };

    using SegmentationExtensionPtr      = SegmentationExtension *;
    using SegmentationExtensionSPtr     = std::shared_ptr<SegmentationExtension>;
    using SegmentationExtensionList     = QList<SegmentationExtensionPtr>;
    using SegmentationExtensionSList    = QList<SegmentationExtensionSPtr>;
    using SegmentationExtensionSMap     = QMap<QString,SegmentationExtensionSPtr>;

    template<typename E> typename E::InformationKey createKey(const std::shared_ptr<E> &extension, const typename E::Key &key)
    {
      return typename E::InformationKey(extension->type(), key);
    }
  } // namespace Core
} // namespace ESPINA

#endif // ESPINA_EXTENSIONS_H
