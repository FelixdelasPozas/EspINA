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


#include "AddSegmentations.h"

#include <Core/Analysis/Query.h>
#include <GUI/Model/Utils/QueryAdapter.h>

using namespace EspINA;

//----------------------------------------------------------------------------
AddSegmentations::AddSegmentations(SegmentationAdapterSPtr segmentation,
                                   ModelAdapterSPtr        model,
                                   QUndoCommand           *parent)
: QUndoCommand(parent)
, m_model(model)
{
  m_segmentations << segmentation;

  for(auto segmentation : m_segmentations)
  {
    m_samples[segmentation] = findSamplesUsingInputChannels(segmentation);
  }
}

//----------------------------------------------------------------------------
AddSegmentations::AddSegmentations(SegmentationAdapterSList segmentations,
                                   ModelAdapterSPtr         model,
                                   QUndoCommand            *parent)
: QUndoCommand(parent)
, m_model(model)
{
  m_segmentations << segmentations;

  for(auto segmentation : m_segmentations)
  {
    m_samples[segmentation] = findSamplesUsingInputChannels(segmentation);
  }
}

//----------------------------------------------------------------------------
void AddSegmentations::redo()
{
  m_model->add(m_segmentations);

  for(auto samples : m_samples)
  {
    Q_ASSERT(samples.size() == 1); // Tiling not supported yet

    auto sample = samples[0];
    for(auto segmentation : m_segmentations)
    {
      m_model->addRelation(sample, segmentation, Query::CONTAINS);
    }
  }
}

//----------------------------------------------------------------------------
void AddSegmentations::undo()
{
  m_model->remove(m_segmentations);
}

//----------------------------------------------------------------------------
SampleAdapterSList AddSegmentations::findSamplesUsingInputChannels(SegmentationAdapterSPtr segmentation)
{
  SampleAdapterSList samples;

  auto channels = QueryAdapter::channels(segmentation);
  Q_ASSERT(channels.size() == 1); // Tiling not supported yet

  for(auto channel : channels)
  {
    samples << QueryAdapter::samples(channel);
  }

  return samples;
}

