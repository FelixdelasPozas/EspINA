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


#ifndef ESPINA_EXTENSION_FACTORY_H
#define ESPINA_EXTENSION_FACTORY_H

#include "Extensions/EspinaExtensions_Export.h"
#include <Core/Analysis/Extension.h>

namespace EspINA
{
  class EspinaExtensions_EXPORT ExtensionFactory
  {
  public:
    virtual ~ExtensionFactory() {}

    virtual ChannelExtensionSPtr createChannelExtension(ChannelExtension::Type type) = 0;

    virtual SegmentationExtensionSPtr createSegmentationExtension(SegmentationExtension::Type type) = 0;
  };

  using ExtensionFactorySPtr  = std::shared_ptr<ExtensionFactory>;
  using ExtensionFactorySList = QList<ExtensionFactorySPtr>;

}// namespace EspINA

#endif // ESPINA_EXTENSION_FACTORY_H
