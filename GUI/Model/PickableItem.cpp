/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

#include "PickableItem.h"
#include "Filter.h"

using namespace EspINA;

//----------------------------------------------------------------------------
PickableItem::PickableItem()
: m_isSelected(false)
, m_outputIsModified(false)
{
}

//----------------------------------------------------------------------------
FilterSPtr PickableItem::filter()
{
  const FilterSPtr cp = static_cast<const PickableItem *>(this)->filter();
  return cp;
}

//----------------------------------------------------------------------------
OutputSPtr PickableItem::output()
{
  return filter()->output(outputId());
}

//----------------------------------------------------------------------------
const OutputSPtr PickableItem::output() const
{
  return filter()->output(outputId());
}

//----------------------------------------------------------------------------
PickableItemPtr EspINA::pickableItemPtr(ModelItemPtr item)
{
  Q_ASSERT(EspINA::SEGMENTATION == item->type() || EspINA::SAMPLE == item->type());
  PickableItemPtr ptr = dynamic_cast<PickableItemPtr>(item);
  Q_ASSERT(ptr);

  return ptr;
}

//----------------------------------------------------------------------------
PickableItemSPtr EspINA::pickableItemPtr(ModelItemSPtr &item)
{
  Q_ASSERT(EspINA::SEGMENTATION == item->type() || EspINA::SAMPLE == item->type());
  PickableItemSPtr ptr = boost::dynamic_pointer_cast<PickableItem>(item);
  Q_ASSERT(ptr != NULL);

  return ptr;
}
