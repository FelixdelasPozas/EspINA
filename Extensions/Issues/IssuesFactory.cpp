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

#include <Core/Utils/EspinaException.h>
#include <Extensions/Issues/IssuesFactory.h>
#include <Extensions/Issues/ItemIssues.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Extensions;

//--------------------------------------------------------------------
SegmentationIssuesFactory::SegmentationIssuesFactory()
: SegmentationExtensionFactory{nullptr}
{
}

//--------------------------------------------------------------------
SegmentationExtensionSPtr SegmentationIssuesFactory::createExtension(const SegmentationExtension::Type      &type,
                                                                     const SegmentationExtension::InfoCache &cache,
                                                                     const State                            &state) const
{
  SegmentationExtensionSPtr extension = nullptr;

  if(type == SegmentationIssues::TYPE)
  {
    extension = SegmentationExtensionSPtr{new SegmentationIssues(cache)};
  }

  if(!extension || !extension.get())
  {
    auto message = QObject::tr("Unknown extension type: %1").arg(type);
    auto details = QObject::tr("SegmentationIssuesFactory::createExtension() -> ") + message;

    throw EspinaException(message, details);
  }

  return extension;
}

//--------------------------------------------------------------------
SegmentationExtension::TypeList ESPINA::Extensions::SegmentationIssuesFactory::providedExtensions() const
{
  SegmentationExtension::TypeList list;
  list << SegmentationIssues::TYPE;

  return list;
}

//--------------------------------------------------------------------
ESPINA::Extensions::StackIssuesFactory::StackIssuesFactory()
: StackExtensionFactory{nullptr}
{
}

//--------------------------------------------------------------------
Core::StackExtensionSPtr ESPINA::Extensions::StackIssuesFactory::createExtension(const Core::StackExtension::Type& type,
    const Core::StackExtension::InfoCache& cache, const State& state) const
{
  StackExtensionSPtr extension = nullptr;

  if(type == StackIssues::TYPE)
  {
    extension = StackExtensionSPtr{new StackIssues(cache)};
  }

  if(!extension || !extension.get())
  {
    auto message = QObject::tr("Unknown extension type: %1").arg(type);
    auto details = QObject::tr("StackIssuesFactory::createExtension() -> ") + message;

    throw EspinaException(message, details);
  }

  return extension;
}

//--------------------------------------------------------------------
Core::StackExtension::TypeList ESPINA::Extensions::StackIssuesFactory::providedExtensions() const
{
  StackExtension::TypeList list;
  list << StackIssues::TYPE;

  return list;
}
