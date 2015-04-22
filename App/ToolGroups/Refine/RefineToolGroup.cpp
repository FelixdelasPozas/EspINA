/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include "RefineToolGroup.h"

#include <Core/Analysis/Output.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <Undo/DrawUndoCommand.h>
#include <Undo/ModifyDataCommand.h>
#include <Undo/RemoveSegmentations.h>

// Qt
#include <QApplication>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Model::Utils;


//-----------------------------------------------------------------------------
RefineToolGroup::RefineToolGroup(FilterDelegateFactorySPtr filterDelegateFactory,
                                 Support::Context &context)
: ToolGroup      {":/espina/toolgroup_refine.svg", tr("Refine")}
, m_context      {context}
{
  // DESIGN: Consider using a base class for all refine tools to
  //         manage enabling tools depending on current selection

  m_manualEdition = std::make_shared<ManualEditionTool>(context);
  m_split         = std::make_shared<SplitTool>(context);
  m_morphological = std::make_shared<MorphologicalEditionTool>(filterDelegateFactory, context);

  addTool(m_manualEdition);
  addTool(m_split);
  addTool(m_morphological);

  connect(m_manualEdition.get(), SIGNAL(voxelsDeleted(ViewItemAdapterPtr)),
          this,                  SLOT(onVoxelDeletion(ViewItemAdapterPtr)));

  connect(context.selection().get(), SIGNAL(selectionChanged()),
          this,                      SLOT(enableCurrentSelectionActions()));

  enableCurrentSelectionActions();
}

//-----------------------------------------------------------------------------
RefineToolGroup::~RefineToolGroup()
{
  disconnect(this->parent());
}

//-----------------------------------------------------------------------------
void RefineToolGroup::enableCurrentSelectionActions()
{
  auto selection     = m_context.selection()->segmentations();
  auto selectionSize = selection.size();

  auto noSegmentation       = (selectionSize == 0);
  auto onlyOneSegmentation  = (selectionSize == 1);
  auto hasRequiredData      = false;

  if(onlyOneSegmentation)
  {
    auto selectedSegmentation = selection.first();
    hasRequiredData = hasVolumetricData(selectedSegmentation->output());
  }

  m_manualEdition->setEnabled(noSegmentation || onlyOneSegmentation);
  m_split        ->setEnabled(onlyOneSegmentation && hasRequiredData);
  // NOTE: morphological tools manage selection on their own, as it's tools
  // haven't a unique requirement.
}

//-----------------------------------------------------------------------------
void RefineToolGroup::onVoxelDeletion(ViewItemAdapterPtr item)
{
  Q_ASSERT(item && isSegmentation(item) && hasVolumetricData(item->output()));

  auto segmentation = segmentationPtr(item);

  auto volume = volumetricData(segmentation->output());

  auto undoStack = m_context.undoStack();

  if (volume->isEmpty())
  {
    undoStack->blockSignals(true);
    do
    {
      undoStack->undo();
    }
    while(volume->isEmpty());
    undoStack->blockSignals(false);

    if(segmentation->output()->numberOfDatas() == 1)
    {
      auto name = segmentation->data(Qt::DisplayRole).toString();
      DefaultDialogs::InformationMessage(tr("Deleting segmentation"),
                                         tr("%1 will be deleted because all its voxels were erased.").arg(name));

      undoStack->beginMacro("Remove Segmentation");
      undoStack->push(new RemoveSegmentations(segmentation, m_context.model()));
    }
    else
    {
      auto output = segmentation->output();
      undoStack->beginMacro("Remove Segmentation's volume");
      undoStack->push(new RemoveDataCommand(output, VolumetricData<itkVolumeType>::TYPE));
    }
    undoStack->endMacro();
  }
  else
  {
    fitToContents(volume, SEG_BG_VALUE);
  }
}