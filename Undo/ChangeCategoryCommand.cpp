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
                                             Support::Context       &context,
                                             QUndoCommand*           parent)
: QUndoCommand(parent)
, WithContext (context)
, m_category  {context.model()->smartPointer(category)}
{
  for(auto segmentation: segmentations)
  {
    m_oldCategories[context.model()->smartPointer(segmentation)] = segmentation->category();
  }
}

//------------------------------------------------------------------------
ChangeCategoryCommand::~ChangeCategoryCommand()
{
}

//------------------------------------------------------------------------
void ChangeCategoryCommand::redo()
{
  ViewItemAdapterList segmentations;

  for(auto segmentation: m_oldCategories.keys())
  {
    getModel()->setSegmentationCategory(segmentation, m_category);
    segmentations << segmentation.get();
  }

  updateSelection(segmentations);
}

//------------------------------------------------------------------------
void ChangeCategoryCommand::undo()
{
  ViewItemAdapterList segmentations;

  for(auto segmentation: m_oldCategories.keys())
  {
    getModel()->setSegmentationCategory(segmentation, m_oldCategories[segmentation]);
    segmentations << segmentation.get();
  }

  updateSelection(segmentations);
}

//------------------------------------------------------------------------
void ChangeCategoryCommand::updateSelection(ViewItemAdapterList segmentations)
{
  getViewState().selection()->clear();
  getViewState().selection()->set(segmentations);
  getViewState().representationInvalidator().invalidateRepresentationColors(segmentations);
}
