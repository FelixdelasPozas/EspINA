/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "AddSegmentations.h"
#include <Core/Analysis/Query.h>
#include <Core/Analysis/Sample.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Model/Utils/ModelUtils.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Model::Utils;

//----------------------------------------------------------------------------
AddSegmentations::AddSegmentations(SegmentationAdapterSPtr segmentation,
                                   SampleAdapterSList      samples,
                                   ModelAdapterSPtr        model,
                                   ConnectionList          connections,
                                   QUndoCommand           *parent)
: QUndoCommand {parent}
, m_samples    {samples}
, m_model      {model}
, m_connections{connections}
{
  m_segmentations << segmentation;
}

//----------------------------------------------------------------------------
AddSegmentations::AddSegmentations(SegmentationAdapterSList segmentations,
                                   SampleAdapterSList       samples,
                                   ModelAdapterSPtr         model,
                                   ConnectionList           connections,
                                   QUndoCommand            *parent)
: QUndoCommand {parent}
, m_samples    {samples}
, m_model      {model}
, m_connections{connections}
{
  m_segmentations << segmentations;
}

//----------------------------------------------------------------------------
void AddSegmentations::redo()
{
  unsigned int number = firstUnusedSegmentationNumber(m_model);

  for(auto segmentation : m_segmentations)
  {
    segmentation->setNumber(number++);
  }

  m_model->beginBatchMode();
  m_model->add(m_segmentations);

  for(auto segmentation : m_segmentations)
  {
    for(auto sample : m_samples)
    {
      m_model->addRelation(sample, segmentation, Sample::CONTAINS);
    }
  }

  if(!m_connections.empty()) m_model->addConnections(m_connections);

  m_model->endBatchMode();
}

//----------------------------------------------------------------------------
void AddSegmentations::undo()
{
  m_model->remove(m_segmentations);
}
