/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef EXTENSIONS_SKELETONINFORMATION_SKELETONINFORMATION_H_
#define EXTENSIONS_SKELETONINFORMATION_SKELETONINFORMATION_H_

#include <Extensions/EspinaExtensions_Export.h>

// ESPINA
#include <Core/Analysis/Extensions.h>
#include <Core/Analysis/Data/SkeletonData.h>

namespace ESPINA
{
  namespace Extensions
  {
    class SkeletonInformationFactory;

    /** \class SkeletonInformation
     * \brief Extension that provides information about a skeleton.
     *
     */
    class EspinaExtensions_EXPORT SkeletonInformation
    : public Core::SegmentationExtension
    {
        Q_OBJECT
      public:
        static const Type TYPE;

        /** \brief SkeletonInformation class virtual destructor.
         *
         */
        virtual ~SkeletonInformation();

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
        { return true;}

        virtual bool validData(const OutputSPtr output) const
        { return hasSkeletonData(output); }

      protected:
        virtual QVariant cacheFail(const InformationKey& tag) const;

        virtual void onExtendedItemSet(Segmentation* item);

        virtual void invalidateImplementation() override;

      private slots:
        /** \brief Generates the keys for this segmentation.
         *
         */
        void updateKeys();

      private:
        /** \brief Computes information values.
         *
         */
        void updateInformation() const;

        /** \brief SkeletonInformation class constructor.
         * \param[in] infoCache cache object.
         *
         */
        explicit SkeletonInformation(const InfoCache& infoCache = InfoCache());

        mutable QReadWriteLock     m_mutex; /** data protection mutex for concurrent access. */
        mutable InformationKeyList m_keys;  /** informatio keys in this extension.           */

        friend class SkeletonInformationFactory;
    };
  
  } // namespace Extensions
} // namespace ESPINA

#endif // EXTENSIONS_SKELETONINFORMATION_SKELETONINFORMATION_H_
