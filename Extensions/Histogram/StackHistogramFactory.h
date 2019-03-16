/*

 Copyright (C) 2019 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef EXTENSIONS_HISTOGRAM_STACKHISTOGRAMFACTORY_H_
#define EXTENSIONS_HISTOGRAM_STACKHISTOGRAMFACTORY_H_

// ESPINA
#include <Extensions/EspinaExtensions_Export.h>
#include <Core/Analysis/Extensions.h>
#include <Core/Factory/ExtensionFactory.h>

namespace ESPINA
{
  namespace Extensions
  {
    /** \class StackHistogramFactory
     *  \brief Factory for StackHistogram extensions.
     *
     */
    class EspinaExtensions_EXPORT StackHistogramFactory
    : public Core::StackExtensionFactory
    {
      public:
        /** \brief StackHistogramFactory class constructor.
         *
         */
        explicit StackHistogramFactory(CoreFactory *factory);

        /** \brief StackHistogramFactory class virtual destructor.
         *
         */
        virtual ~StackHistogramFactory()
        {};

        virtual Core::StackExtensionSPtr createExtension(const Core::SegmentationExtension::Type      &type,
                                                         const Core::SegmentationExtension::InfoCache &cache = Core::SegmentationExtension::InfoCache() ,
                                                         const State                                  &state = State()) const;

        virtual Core::SegmentationExtension::TypeList providedExtensions() const;

      private:
        CoreFactory *m_factory; /** Core factory pointer. */
    };
  
  } // namespace Extensions
} // namespace ESPINA

#endif // EXTENSIONS_HISTOGRAM_STACKHISTOGRAMFACTORY_H_
