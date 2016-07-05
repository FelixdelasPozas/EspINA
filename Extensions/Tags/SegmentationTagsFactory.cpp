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

// ESPINA
#include "SegmentationTags.h"
#include "SegmentationTagsFactory.h"
#include <Core/Utils/EspinaException.h>

using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Extensions;

//---------------------------------------------------------------------
SegmentationTagsFactory::SegmentationTagsFactory(CoreFactory* factory)
: SegmentationExtensionFactory{factory}
{
}

//---------------------------------------------------------------------
SegmentationTagsFactory::~SegmentationTagsFactory()
{
}

//---------------------------------------------------------------------
SegmentationExtensionSPtr SegmentationTagsFactory::createExtension(const SegmentationExtension::Type      &type,
                                                                   const SegmentationExtension::InfoCache &cache,
                                                                   const State                            &state) const
{
  SegmentationExtensionSPtr extension = nullptr;

  if(type == SegmentationTags::TYPE)
  {
    extension = SegmentationExtensionSPtr{new SegmentationTags(cache)};
  }

  if(!extension || !extension.get())
  {
    auto message = QObject::tr("Unknown extension type: %1").arg(type);
    auto details = QObject::tr("SegmentationTagsFactory::createExtension() -> ") + message;
    
    throw EspinaException(message, details);
  }

  return extension;
}

//---------------------------------------------------------------------
SegmentationExtension::TypeList SegmentationTagsFactory::providedExtensions() const
{
  SegmentationExtension::TypeList list;
  list << SegmentationTags::TYPE;
  
  return list;
}

