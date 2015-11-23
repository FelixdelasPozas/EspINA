/*

    Copyright (C) 2014
    Jorge Pe�a Pastor<jpena@cesvima.upm.es>,
    Felix de las Pozas<fpozas@cesvima.upm.es>

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
#include "ModelUtils.h"

#include <Core/Analysis/Segmentation.h>
#include <GUI/Model/Utils/SegmentationUtils.h>

// Qt
#include <QString>

using namespace ESPINA;
using namespace ESPINA::GUI::Model;

//------------------------------------------------------------------------
unsigned int Utils::firstUnusedSegmentationNumber(const ModelAdapterSPtr model)
{
  unsigned int number = 0;

  for (auto segmentation: model->segmentations())
  {
    if (segmentation->number() > number)
    {
      number = segmentation->number();
    }
  }

  return ++number;
}

//------------------------------------------------------------------------
Utils::Items Utils::classifyViewItems(const ViewItemAdapterList &items)
{
  Items result;

  for (auto item : items)
  {
    if (isChannel(item))
    {
      result.stacks << item;
    }
    else
    {
      result.segmentations << item;
    }
  }

  return result;
}

//------------------------------------------------------------------------
Utils::Items Utils::classifyViewItems(const ViewItemAdapterList &items, const Items &group)
{
  Items result;

  for (auto item : items)
  {
    if (isChannel(item) && group.stacks.contains(item))
    {
      result.stacks << item;
    }
    else
    {
      if(group.segmentations.contains(item))
      {
        result.segmentations << item;
      }
    }
  }

  return result;
}
