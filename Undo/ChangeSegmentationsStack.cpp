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
#include <Core/Utils/ListUtils.hxx>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <Undo/ChangeSegmentationsStack.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Undo;

//--------------------------------------------------------------------
ChangeSegmentationsStack::ChangeSegmentationsStack(const SegmentationAdapterList& segmentations, const ChannelAdapterPtr stack, QUndoCommand* parent)
: QUndoCommand{parent}
{
  for(auto segmentation: segmentations)
  {
    m_map.insert(segmentation, stack);
  }
}

//--------------------------------------------------------------------
ChangeSegmentationsStack::ChangeSegmentationsStack(const SegmentationAdapterPtr segmentation, const ChannelAdapterPtr stack, QUndoCommand* parent)
: QUndoCommand{parent}
{
  m_map.insert(segmentation, stack);
}

//--------------------------------------------------------------------
void ChangeSegmentationsStack::redo()
{
  for(auto segmentation: m_map.keys())
  {
    auto oldStacks = QueryAdapter::channels(segmentation);
    if(oldStacks.size() != 1)
    {
      qWarning() << "ChangeSegmentationStack::redo() -> more than one stack for segmentation" << segmentation->data().toString();
    }

    auto model  = segmentation->model();
    auto stack  = m_map[segmentation];

    try
    {
      model->changeSegmentationStack(segmentation, stack);
    }
    catch(...)
    {
      qWarning() << "ChangeSegmentationStack::redo() -> exception caught changing segmentation" << segmentation->data().toString();
    }

    m_map[segmentation] = oldStacks.first().get();
  }
}

//--------------------------------------------------------------------
void ChangeSegmentationsStack::undo()
{
  redo();
}
