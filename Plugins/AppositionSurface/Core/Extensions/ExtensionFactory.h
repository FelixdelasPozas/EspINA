/*
 
 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef APPOSITION_SURFACE_EXTENSION_FACTORY_H_
#define APPOSITION_SURFACE_EXTENSION_FACTORY_H_

#include "AppositionSurfacePlugin_Export.h"

// ESPINA
#include <Core/Factory/ExtensionFactory.h>

namespace ESPINA
{
  class CoreFactory;

  /** \class ASExtensionFactory
   * \brief Factory for the creation of SAS Extensions.
   *
   */
  class AppositionSurfacePlugin_EXPORT ASExtensionFactory
  : public Core::SegmentationExtensionFactory
  {
    public:
      /** \brief ASExtensionFactory class constructor.
       *
       */
      explicit ASExtensionFactory()
      : Core::SegmentationExtensionFactory{nullptr}
      {}

      /** \brief ASExtensionFactory class virtual destructor.
       *
       */
      virtual ~ASExtensionFactory()
      {}

      virtual Core::SegmentationExtensionSPtr createExtension(const Core::SegmentationExtension::Type      &type,
                                                              const Core::SegmentationExtension::InfoCache &cache = Core::SegmentationExtension::InfoCache(),
                                                              const State& state = State()) const;

      virtual Core::SegmentationExtension::TypeList providedExtensions() const;
  };
} // namespace ESPINA

#endif // APPOSITION_SURFACE_EXTENSION_FACTORY_H_
