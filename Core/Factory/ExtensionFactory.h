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

#ifndef CORE_FACTORY_EXTENSIONFACTORY_H_
#define CORE_FACTORY_EXTENSIONFACTORY_H_

#include <Core/Analysis/Extensions.h>

namespace ESPINA
{
  namespace Core
  {
    /** \class ExtensionFactory
     * \brief Factory for extension objects.
     *
     */
    class StackExtensionFactory
    {
      public:
        StackExtensionFactory(CoreFactory *factory)
        : m_factory{factory}
        {};

        /** \brief ExtensionFactory class virtual destructor.
         *
         */
        virtual ~StackExtensionFactory()
        {};

        /** \brief Creates a extension of the given type with the given state and cache object.
         * \param[in] type, extension type.
         * \param[in] cache, information cache object.
         * \param[in] state, state object.
         *
         */
        virtual StackExtensionSPtr createExtension(const StackExtension::Type      &type,
                                                   const StackExtension::InfoCache &cache = StackExtension::InfoCache() ,
                                                   const State                     &state = State()) const = 0;

        /** \brief Returns the list of types of channel extensions this filter can create.
         *
         */
        virtual StackExtension::TypeList providedExtensions() const = 0;

      protected:
        CoreFactory *m_factory;
    };

    using StackExtensionFactoryPtr   = StackExtensionFactory *;
    using StackExtensionFactorySPtr  = std::shared_ptr<StackExtensionFactory>;
    using StackExtensionFactorySList = QList<StackExtensionFactorySPtr>;
    using StackExtensionFactoryList  = QList<StackExtensionFactoryPtr>;

    /** \class ExtensionFactory
     * \brief Factory for extension objects.
     *
     */
    class SegmentationExtensionFactory
    {
      public:
        SegmentationExtensionFactory(CoreFactory *factory)
        : m_factory{factory}
        {};

        /** \brief ExtensionFactory class virtual destructor.
         *
         */
        virtual ~SegmentationExtensionFactory()
        {};

        /** \brief Creates a extension of the given type with the given state and cache object.
         * \param[in] type, extension type.
         * \param[in] cache, information cache object.
         * \param[in] state, state object.
         *
         */
        virtual SegmentationExtensionSPtr createExtension(const SegmentationExtension::Type      &type,
                                                          const SegmentationExtension::InfoCache &cache = SegmentationExtension::InfoCache() ,
                                                          const State                            &state = State()) const = 0;

        /** \brief Returns the list of types of channel extensions this filter can create.
         *
         */
        virtual SegmentationExtension::TypeList providedExtensions() const = 0;

      protected:
        CoreFactory *m_factory;
    };

    using SegmentationExtensionFactoryPtr   = SegmentationExtensionFactory *;
    using SegmentationExtensionFactorySPtr  = std::shared_ptr<SegmentationExtensionFactory>;
    using SegmentationExtensionFactorySList = QList<SegmentationExtensionFactorySPtr>;
    using SegmentationExtensionFactoryList  = QList<SegmentationExtensionFactoryPtr>;

  } // namespace Core
} // namespace ESPINA

#endif // CORE_FACTORY_EXTENSIONFACTORY_H_
