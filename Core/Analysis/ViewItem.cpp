/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#include "ViewItem.h"
#include "Core/Analysis/Filter.h"

using namespace EspINA;

ViewItem::ViewItem(FilterSPtr filter, Output::Id output)
: m_filter{filter}
, m_outputId{output}
{
}

OutputSPtr ViewItem::output()
{
  return m_filter->output(m_outputId);
}

const OutputSPtr ViewItem::output() const
{
  return m_filter->output(m_outputId);
}

DataSPtr ViewItem::data(Data::Type type)
{
  return m_filter->output(m_outputId)->data(type);
}

const DataSPtr ViewItem::data(Data::Type type) const
{
  return m_filter->output(m_outputId)->data(type);
}

void ViewItem::changeOutput(FilterSPtr filter, Output::Id output)
{
  m_filter = filter;
  m_outputId = output;
}
