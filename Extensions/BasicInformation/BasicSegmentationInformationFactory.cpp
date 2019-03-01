/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Core/Utils/EspinaException.h>
#include <Extensions/BasicInformation/BasicSegmentationInformation.h>
#include <Extensions/BasicInformation/BasicSegmentationInformationFactory.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Extensions;

//--------------------------------------------------------------------
BasicSegmentationInformationExtensionFactory::BasicSegmentationInformationExtensionFactory()
: Core::SegmentationExtensionFactory{nullptr}
{
}

//--------------------------------------------------------------------
Core::SegmentationExtensionSPtr BasicSegmentationInformationExtensionFactory::createExtension(const Core::SegmentationExtension::Type& type,
                                                                                              const Core::SegmentationExtension::InfoCache& cache,
                                                                                              const State& state) const
{
  if(type != BasicSegmentationInformationExtension::TYPE)
  {
    auto message = QString("Unknown extension type '%1'.").arg(type);
    auto details = QString("BasicSegmentationInformationExtensionFactory::createExtension() -> ") + message;

    throw Utils::EspinaException(message, details);
  }

  return std::make_shared<BasicSegmentationInformationExtension>(cache);
}

//--------------------------------------------------------------------
Core::SegmentationExtension::TypeList BasicSegmentationInformationExtensionFactory::providedExtensions() const
{
  SegmentationExtension::TypeList types;

  types << BasicSegmentationInformationExtension::TYPE;

  return types;
}
