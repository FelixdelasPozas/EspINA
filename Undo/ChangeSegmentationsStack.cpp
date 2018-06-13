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
#include <GUI/Model/Utils/QueryAdapter.h>
#include <Undo/ChangeSegmentationsStack.h>

using namespace ESPINA;
using namespace ESPINA::Undo;

//--------------------------------------------------------------------
ChangeSegmentationsStack::ChangeSegmentationsStack(const SegmentationAdapterList& segmentations, const ChannelAdapterPtr stack, QUndoCommand* parent)
: QUndoCommand{parent}
, m_newStack  {stack}
, m_newSample (QueryAdapter::sample(m_newStack))
{
  for(const auto segmentation: segmentations)
  {
    struct Data data;
    data.segmentation = segmentation;
    data.stacks       = QueryAdapter::channels(segmentation);
    data.samples      = QueryAdapter::samples(segmentation);

    m_segmentations << data;
  }
}

//--------------------------------------------------------------------
ChangeSegmentationsStack::ChangeSegmentationsStack(const SegmentationAdapterPtr segmentation, const ChannelAdapterPtr stack, QUndoCommand* parent)
: QUndoCommand{parent}
, m_newStack  {stack}
, m_newSample (QueryAdapter::sample(m_newStack))
{
  struct Data data;
  data.segmentation = segmentation;
  data.stacks       = QueryAdapter::channels(segmentation);
  data.samples      = QueryAdapter::samples(segmentation);

  m_segmentations << data;
}

//--------------------------------------------------------------------
void ChangeSegmentationsStack::redo()
{
  for(auto &data: m_segmentations)
  {
    auto model = data.segmentation->model();

    // TODO: need to access model contents and relations.
  }
}

//--------------------------------------------------------------------
void ChangeSegmentationsStack::undo()
{
  for(auto &data: m_segmentations)
  {
    auto model = data.segmentation->model();

    // TODO: need to access model contents and relations.
  }
}
