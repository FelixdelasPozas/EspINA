/*
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include <Core/Utils/EspinaException.h>
#include <Support/Factory/FilterRefinerFactory.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Support;

//------------------------------------------------------------------------
void FilterRefinerFactory::registerFilterRefiner(const FilterRefinerSPtr refiner, const Filter::Type type)
{
  m_register[type] = refiner;
}

//------------------------------------------------------------------------
QWidget *FilterRefinerFactory::createRefineWidget(SegmentationAdapterPtr segmentation, Context& context)
{
  auto filter = segmentation->filter();
  auto type   = filter->type();

  if (!m_register.contains(type))
  {
    auto what    = QObject::tr("Failed to create refiner, unknown filter register type, type: %1").arg(type);
    auto details = QObject::tr("FilterRefinerFactory::createRefineWidget() -> Unknown filter register type, type: %1").arg(type);

    throw EspinaException(what, details);
  }

  return m_register[type]->createWidget(segmentation, context);
}

//------------------------------------------------------------------------
void FilterRefinerFactory::unregisterFilterRefiners()
{
  m_register.clear();
}
