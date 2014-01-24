/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

    This program is free software: you can redistribute it and/or modify
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

#include "EspinaCore_Export.h"
#include <Core/Analysis/Extensions/SegmentationExtension.h>

namespace EspINA
{
  class EspinaCore_EXPORT SegmentationExtensionFactory
  {
  public:
    virtual ~SegmentationExtensionFactory() {}

    virtual SegmentationExtensionSPtr createSegmentationExtension(const SegmentationExtension::Type type, const State &state = State()) const = 0;

    virtual SegmentationExtensionTypeList providedExtensions() const = 0 ;
  };

  using SegmentationExtensionFactoryPtr   = SegmentationExtensionFactory *;
  using SegmentationExtensionFactorySPtr  = std::shared_ptr<SegmentationExtensionFactory>;
  using SegmentationExtensionFactorySList = QList<SegmentationExtensionFactorySPtr>;

}// namespace EspINA

#endif // ESPINA_SEGMENTATION_EXTENSION_FACTORY_H