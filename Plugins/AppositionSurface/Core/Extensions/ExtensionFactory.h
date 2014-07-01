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

#include <Core/Factory/SegmentationExtensionFactory.h>

using namespace EspINA;

class ASExtensionFactory
: public SegmentationExtensionFactory
{
  public:
    explicit ASExtensionFactory();
    virtual ~ASExtensionFactory();

    virtual SegmentationExtensionSPtr createSegmentationExtension(const SegmentationExtension::Type      &type,
                                                                  const SegmentationExtension::InfoCache &cache = SegmentationExtension::InfoCache(),
                                                                  const State& state = State()) const;

    virtual SegmentationExtensionTypeList providedExtensions() const;
};

#endif // APPOSITION_SURFACE_EXTENSION_FACTORY_H_
