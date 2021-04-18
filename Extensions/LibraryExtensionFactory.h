/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef ESPINA_EXTENSION_FACTORY_H
#define ESPINA_EXTENSION_FACTORY_H

#include "Extensions/EspinaExtensions_Export.h"

// ESPINA
#include <Core/Analysis/Extensions.h>
#include <Core/Factory/CoreFactory.h>
#include <Core/Factory/ExtensionFactory.h>

namespace ESPINA
{
  /** \class LibraryStackExtensionsFactory
   * \brief Factory for stack extensions creation.
   *
   */
  class EspinaExtensions_EXPORT LibraryStackExtensionFactory
  : public Core::StackExtensionFactory
  {
    public:
      /** \brief LibraryStackExtensionFactory class constructor.
       * \param[in] factory pointer to the core factory.
       *
       */
      LibraryStackExtensionFactory(CoreFactory *factory);

      /** \brief StackExtensionFactory class destructor.
       *
       */
      virtual ~LibraryStackExtensionFactory()
      {};

      virtual Core::StackExtensionSPtr createExtension(const typename Core::StackExtension::Type      &type,
                                                       const typename Core::StackExtension::InfoCache &cache = Core::StackExtension::InfoCache(),
                                                       const State                                    &state = State()) const;

      virtual Core::StackExtension::TypeList providedExtensions() const;

    private:
      QMap<Core::StackExtension::Type, Core::StackExtensionFactorySPtr> m_factories;
  };

  /** \class LibrarySegmentationExtensionsFactory
   * \brief Factory for segmentation extensions creation.
   *
   */
  class EspinaExtensions_EXPORT LibrarySegmentationExtensionFactory
  : public Core::SegmentationExtensionFactory
  {
    public:
      /** \brief LibrarySegmentationExtensionFactory class constructor.
       * \param[in] factory pointer to the core factory.
       *
       */
      LibrarySegmentationExtensionFactory(CoreFactory *factory);

      /** \brief LibrarySegmentationExtensionFactory class virtual destructor.
       *
       */
      virtual ~LibrarySegmentationExtensionFactory()
      {};

      virtual Core::SegmentationExtensionSPtr createExtension(const typename Core::SegmentationExtension::Type      &type,
                                                              const typename Core::SegmentationExtension::InfoCache &cache = Core::SegmentationExtension::InfoCache(),
                                                              const State                                           &state = State()) const;


      virtual Core::SegmentationExtension::TypeList providedExtensions() const;

    private:
      QMap<Core::SegmentationExtension::Type, Core::SegmentationExtensionFactorySPtr> m_factories;
  };

}// namespace ESPINA

#endif // ESPINA_EXTENSION_FACTORY_H
