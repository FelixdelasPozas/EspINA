/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef EXTENSIONS_AXON_INFORMATION_H_
#define EXTENSIONS_AXON_INFORMATION_H_

#include <Extensions/EspinaExtensions_Export.h>

// ESPINA
#include <Core/Analysis/Extensions.h>
#include <Core/Analysis/Data/SkeletonData.h>

namespace ESPINA
{
  namespace Extensions
  {
    class SkeletonInformationFactory;

    /** \class AxonSkeletonInformation
     * \brief Extension that provides information about skeletal axons.
     *
     */
    class EspinaExtensions_EXPORT AxonSkeletonInformation
    : public Core::SegmentationExtension
    {
      public:
        static const Type TYPE;

        /** \brief AxonSkeletonInformation class virtual destructor.
         *
         */
        virtual ~AxonSkeletonInformation();

        virtual QString type() const
        { return TYPE; }

        virtual State state() const;

        virtual Snapshot snapshot() const;

        virtual TypeList dependencies() const
        { return TypeList(); }

        virtual bool invalidateOnChange() const
        { return true; }

        virtual InformationKeyList availableInformation() const;

        virtual bool validCategory(const QString& classificationName) const
        { return classificationName.startsWith("Axon"); }

        virtual bool validData(const OutputSPtr output) const
        { return hasSkeletonData(output); }

      protected:
        virtual QVariant cacheFail(const InformationKey& tag) const;

        virtual void onExtendedItemSet(Segmentation* item)
        {};

      private:
        /** \brief Computes information values.
         *
         */
        void updateInformation() const;

        /** \brief AxonSkeletonInformation class constructor.
         * \param[in] infoCache cache object.
         *
         */
        explicit AxonSkeletonInformation(const InfoCache& infoCache = InfoCache());

        mutable QReadWriteLock     m_mutex; /** data protection mutex for concurrent access. */
        mutable InformationKeyList m_keys;  /** information keys in this extension.           */

        friend class SkeletonInformationFactory;
    };
  
  } // namespace Extensions
} // namespace ESPINA

#endif // EXTENSIONS_AXON_INFORMATION_H_
