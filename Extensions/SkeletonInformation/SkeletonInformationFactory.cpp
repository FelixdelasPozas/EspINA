/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// Project
#include <Core/Analysis/Extensions.h>
#include <Core/Utils/EspinaException.h>
#include <Extensions/SkeletonInformation/SkeletonInformation.h>
#include <Extensions/SkeletonInformation/SkeletonInformationFactory.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Extensions;

//--------------------------------------------------------------------
SkeletonInformationFactory::SkeletonInformationFactory(CoreFactory* factory)
: SegmentationExtensionFactory{factory}
{
}

//--------------------------------------------------------------------
ESPINA::Extensions::SkeletonInformationFactory::~SkeletonInformationFactory()
{
}

//--------------------------------------------------------------------
Core::SegmentationExtensionSPtr SkeletonInformationFactory::createExtension(const Core::SegmentationExtension::Type& type,
    const Core::SegmentationExtension::InfoCache& cache, const State& state) const
{
  SegmentationExtensionSPtr extension = nullptr;

  if(type == SkeletonInformation::TYPE)
  {
    extension = SegmentationExtensionSPtr{new SkeletonInformation(cache)};
  }

  if(!extension || !extension.get())
  {
    auto message = QObject::tr("Unknown extension type: %1").arg(type);
    auto details = QObject::tr("MorphologicalInformationFactory::createExtension() -> ") + message;

    throw EspinaException(message, details);
  }

  return extension;
}

//--------------------------------------------------------------------
Core::SegmentationExtension::TypeList SkeletonInformationFactory::providedExtensions() const
{
  SegmentationExtension::TypeList list;
  list << SkeletonInformation::TYPE;

  return list;
}
