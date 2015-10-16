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

#include "FilterRefinerRegister.h"

using namespace ESPINA;
using namespace ESPINA::Support;

//------------------------------------------------------------------------
void FilterRefinerRegister::registerFilterRefiner(const FilterRefinerSPtr refiner, const Filter::Type type)
{
  m_register[type] = refiner;
}

//------------------------------------------------------------------------
QWidget *FilterRefinerRegister::createRefineWidget(SegmentationAdapterPtr segmentation, Context& context)
throw (Unknown_Filter_Type_Exception)
{
  auto filter = segmentation->filter();
  auto type   = filter->type();

  if (!m_register.contains(type)) throw Unknown_Filter_Type_Exception();

  return m_register[type]->createWidget(segmentation, context);
}

//------------------------------------------------------------------------
void FilterRefinerRegister::unregisterFilterRefiners()
{
  m_register.clear();
}