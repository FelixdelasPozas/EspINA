/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef EXTENSIONS_NOTES_SEGMENTATIONNOTESFACTORY_H_
#define EXTENSIONS_NOTES_SEGMENTATIONNOTESFACTORY_H_

#include <Extensions/EspinaExtensions_Export.h>

// ESPINA
#include <Core/Factory/ExtensionFactory.h>

namespace ESPINA
{
  class CoreFactory;

  namespace Extensions
  {
    /** \class SegmentationNotesFactory
     * \brief Factory for SegmentationNotes extensions.
     *
     */
    class EspinaExtensions_EXPORT SegmentationNotesFactory
    : public Core::SegmentationExtensionFactory
    {
      public:
        /** \brief SegmentationNotesFactory class constructor.
         *
         */
        explicit SegmentationNotesFactory();

        /** \brief SegmentationNotesFactory class virtual destructor.
         *
         */
        virtual ~SegmentationNotesFactory();

        virtual Core::SegmentationExtensionSPtr createExtension(const Core::SegmentationExtension::Type      &type,
                                                                const Core::SegmentationExtension::InfoCache &cache = Core::SegmentationExtension::InfoCache() ,
                                                                const State                                  &state = State()) const;

        virtual Core::SegmentationExtension::TypeList providedExtensions() const;
    };

    using SegmentationNotesFactoryPtr  = SegmentationNotesFactory *;
    using SegmentationNotesFactorySPtr = std::shared_ptr<SegmentationNotesFactory>;
  
  } // namespace Extensions
} // namespace ESPINA

#endif // EXTENSIONS_NOTES_SEGMENTATIONNOTESFACTORY_H_
