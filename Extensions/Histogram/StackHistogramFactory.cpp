/*

 Copyright (C) 2019 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Extensions/Histogram/StackHistogram.h>
#include <Extensions/Histogram/StackHistogramFactory.h>

using namespace ESPINA;
using namespace ESPINA::Extensions;

//--------------------------------------------------------------------
ESPINA::Extensions::StackHistogramFactory::StackHistogramFactory(CoreFactory *factory)
: Core::StackExtensionFactory{factory}
{
}

//--------------------------------------------------------------------
Core::StackExtensionSPtr StackHistogramFactory::createExtension(const Core::SegmentationExtension::Type& type,
                                                                       const Core::SegmentationExtension::InfoCache& cache,
                                                                       const State& state) const
{
  Core::StackExtensionSPtr extension = nullptr;

  if(!providedExtensions().contains(type))
  {
    auto message = QObject::tr("Unknown extension: %1").arg(type);
    auto details = QObject::tr("StackHistogramFactory::createExtension() -> ") + message;

    throw Core::Utils::EspinaException(message, details);
  }

  extension = Core::StackExtensionSPtr{new StackHistogram(m_factory)};

  return extension;
}

//--------------------------------------------------------------------
Core::SegmentationExtension::TypeList StackHistogramFactory::providedExtensions() const
{
  Core::SegmentationExtension::TypeList extensions;

  extensions << StackHistogram::TYPE;

  return extensions;
}
