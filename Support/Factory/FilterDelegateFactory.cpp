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

#include "FilterDelegateFactory.h"

using namespace ESPINA;

//------------------------------------------------------------------------
void FilterDelegateFactory::registerFilterDelegateFactory(SpecificFilterDelegateFactorySPtr factory)
{
  for (auto filterType : factory->availableFilterDelegates())
  {
    m_factories[filterType] = factory;
  }
}

//------------------------------------------------------------------------
FilterDelegateSPtr FilterDelegateFactory::createDelegate(SegmentationAdapterPtr segmentation)
throw (Unknown_Filter_Type_Exception)
{
  auto filter = segmentation->filter();
  auto type   = filter->type();

  if (!m_instances.contains(segmentation)
    || m_instances[segmentation].second != type)
  {

    if (!m_factories.contains(type)) throw Unknown_Filter_Type_Exception();

    m_instances.insert(segmentation, Factory(m_factories[type]->createDelegate(segmentation, filter), type));
  }

  return m_instances[segmentation].first;
}

//------------------------------------------------------------------------
void FilterDelegateFactory::resetDelegates()
{
  m_instances.clear();
}
