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


#ifndef ESPINA_SEGMENTATION_EXTENSION_FACTORY_H
#define ESPINA_SEGMENTATION_EXTENSION_FACTORY_H

#include "Core/EspinaCore_Export.h"
#include <Core/Analysis/Extension.h>

namespace EspINA
{
  class EspinaCore_EXPORT SegmentationExtensionFactory
  {
  public:
    struct Extension_Not_Provided_Exception {};

  public:
    virtual ~SegmentationExtensionFactory() {}

    virtual SegmentationExtensionSPtr createSegmentationExtension(const SegmentationExtension::Type      &type,
                                                                  const SegmentationExtension::InfoCache &cache = SegmentationExtension::InfoCache() ,
                                                                  const State                            &state = State()) const = 0;

    virtual SegmentationExtensionTypeList providedExtensions() const = 0 ;
  };

  using SegmentationExtensionFactoryPtr   = SegmentationExtensionFactory *;
  using SegmentationExtensionFactorySPtr  = std::shared_ptr<SegmentationExtensionFactory>;
  using SegmentationExtensionFactorySList = QList<SegmentationExtensionFactorySPtr>;

}// namespace EspINA

#endif // ESPINA_SEGMENTATION_EXTENSION_FACTORY_H