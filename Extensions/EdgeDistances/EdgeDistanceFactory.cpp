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
#include <Extensions/EdgeDistances/EdgeDistanceFactory.h>
#include <Extensions/EdgeDistances/EdgeDistance.h>
#include <Core/Utils/EspinaException.h>

using namespace ESPINA;
using namespace ESPINA::Extensions;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;

//----------------------------------------------------------------------
EdgeDistanceFactory::EdgeDistanceFactory(CoreFactory* factory)
: SegmentationExtensionFactory{factory}
{
}

//----------------------------------------------------------------------
EdgeDistanceFactory::~EdgeDistanceFactory()
{
}

//----------------------------------------------------------------------
SegmentationExtensionSPtr EdgeDistanceFactory::createExtension(const SegmentationExtension::Type& type,
                                                               const SegmentationExtension::InfoCache& cache,
                                                               const State& state) const
{
  SegmentationExtensionSPtr extension = nullptr;

  if(type == EdgeDistance::TYPE)
  {
    extension = SegmentationExtensionSPtr{new EdgeDistance(m_factory, cache, state)};
  }

  if(!extension || !extension.get())
  {
    auto message = QObject::tr("Unknown extension type: %1").arg(type);
    auto details = QObject::tr("EdgeDistanceFactory::createExtension() -> ") + message;

    throw EspinaException(message, details);
  }

  return extension;
}

//----------------------------------------------------------------------
SegmentationExtension::TypeList EdgeDistanceFactory::providedExtensions() const
{
  SegmentationExtension::TypeList list;
  list << EdgeDistance::TYPE;

  return list;
}
