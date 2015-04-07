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
#include "ChangeCategoryCommand.h"
#include <GUI/Model/ModelAdapter.h>

using namespace ESPINA;

//------------------------------------------------------------------------
ChangeCategoryCommand::ChangeCategoryCommand(SegmentationAdapterList segmentations,
                                             CategoryAdapterPtr      category,
                                             ModelAdapterSPtr        model,
                                             QUndoCommand*           parent)
: QUndoCommand {parent}
, m_model      {model}
, m_category   {m_model->smartPointer(category)}
{
  for(auto segmentation: segmentations)
  {
    m_oldCategories[m_model->smartPointer(segmentation)] = segmentation->category();
  }
}

//------------------------------------------------------------------------
ChangeCategoryCommand::~ChangeCategoryCommand()
{
}

//------------------------------------------------------------------------
void ChangeCategoryCommand::redo()
{
  ViewItemAdapterSList segmentations;

  for(auto segmentation: m_oldCategories.keys())
  {
    m_model->setSegmentationCategory(segmentation, m_category);
    segmentations << segmentation;
  }

  m_model->notifyRepresentationsModified(segmentations);
}

//------------------------------------------------------------------------
void ChangeCategoryCommand::undo()
{
  ViewItemAdapterSList segmentations;

  for(auto segmentation: m_oldCategories.keys())
  {
    m_model->setSegmentationCategory(segmentation, m_oldCategories[segmentation]);
    segmentations << segmentation;
  }

  m_model->notifyRepresentationsModified(segmentations);
}
