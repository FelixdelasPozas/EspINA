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

#ifndef EXTENSIONS_BASICSEGMENTATIONINFORMATION_H_
#define EXTENSIONS_BASICSEGMENTATIONINFORMATION_H_

#include <Extensions/EspinaExtensions_Export.h>

// ESPINA
#include <Core/Analysis/Extensions.h>

namespace ESPINA
{
  namespace Extensions
  {
    /** \class BasicSegmentationInformation
     * \brief Provides basic information about a segmentation.
     *
     */
    class EspinaExtensions_EXPORT BasicSegmentationInformationExtension
    : public Core::SegmentationExtension
    {
      public:
        static const Type TYPE;

      public:
        /** \brief BasicSegmentationInformation class virtual destructor.
         *
         */
        virtual ~BasicSegmentationInformationExtension()
        {};

        virtual Type type() const
        { return TYPE; }

        virtual bool invalidateOnChange() const
        { return true; }

        virtual State state() const
        { return State(); }

        virtual Snapshot snapshot() const
        { return Snapshot(); }

        virtual const TypeList dependencies() const
        { return TypeList(); }

        virtual const InformationKeyList availableInformation() const;

        virtual bool validCategory(const QString &classification) const
        { return true; }

        virtual bool validData(const OutputSPtr output) const
        { return true; }

      protected:
        virtual QVariant cacheFail(const InformationKey &key) const;

        virtual void onExtendedItemSet(Segmentation* item)
        {};

      private:
        /** \brief BasicSegmentationInformation class constructor.
         *
         */
        BasicSegmentationInformationExtension(const InfoCache &infoCache);

        friend class BasicSegmentationInformationExtensionFactory;
    };
  
  } // namespace Extensions
} // namespace ESPINA

#endif // EXTENSIONS_BASICSEGMENTATIONINFORMATION_H_
