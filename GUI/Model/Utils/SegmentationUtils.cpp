/*

    Copyright (C) 2016 Felix de las Pozas Alvarez<fpozas@cesvima.upm.es>

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
#include "SegmentationUtils.h"
#include <Core/Utils/ListUtils.hxx>
#include <GUI/Model/CategoryAdapter.h>

using namespace ESPINA;

//------------------------------------------------------------------------
SegmentationAdapterPtr ESPINA::GUI::Model::Utils::segmentationPtr(ItemAdapterPtr item)
{
  return dynamic_cast<SegmentationAdapterPtr>(item);
}

//------------------------------------------------------------------------
ConstSegmentationAdapterPtr ESPINA::GUI::Model::Utils::segmentationPtr(ConstItemAdapterPtr item)
{
  return dynamic_cast<ConstSegmentationAdapterPtr>(item);
}

//------------------------------------------------------------------------
bool ESPINA::GUI::Model::Utils::isSegmentation(ItemAdapterPtr item)
{
  return ItemAdapter::Type::SEGMENTATION == item->type();
}

//------------------------------------------------------------------------
const QString ESPINA::GUI::Model::Utils::categoricalName(SegmentationAdapterSPtr segmentation)
{
  return categoricalName(segmentation.get());
}

//------------------------------------------------------------------------
const QString ESPINA::GUI::Model::Utils::categoricalName(SegmentationAdapterPtr segmentation)
{
  auto name = QString("%1 %2").arg(segmentation->category() ? segmentation->category()->name() : "Unknown Category")
                              .arg(segmentation->number());

  return name;
}
