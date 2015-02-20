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

#include "EditionTools.h"

#include <App/Undo/BrushUndoCommand.h>
#include <Core/Analysis/Output.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <Undo/ModifyDataCommand.h>
#include <Undo/RemoveSegmentations.h>

using namespace ESPINA;
using namespace ESPINA::GUI;


//-----------------------------------------------------------------------------
EditionTools::EditionTools(ModelAdapterSPtr          model,
                           ModelFactorySPtr          factory,
                           FilterDelegateFactorySPtr filterDelegateFactory,
                           ViewManagerSPtr           viewManager,
                           QUndoStack               *undoStack,
                           QWidget                  *parent)
: ToolGroup      {viewManager, QIcon(":/espina/pencil.png"), tr("Edition Tools"), parent}
, m_factory      {factory}
, m_undoStack    {undoStack}
, m_model        {model}
{

  m_manualEdition = std::make_shared<ManualEditionTool>(model, factory, undoStack, viewManager);
  m_split         = std::make_shared<SplitTool>(model, factory, viewManager, undoStack);
  m_morphological = std::make_shared<MorphologicalEditionTool>(model, factory, filterDelegateFactory, viewManager, undoStack);

  connect(m_manualEdition.get(), SIGNAL(voxelsDeleted(ViewItemAdapterPtr)),
          this,                  SLOT(onVoxelDeletion(ViewItemAdapterPtr)));

  connect(m_viewManager->selection().get(), SIGNAL(selectionChanged()),
          this,                             SLOT(selectionChanged()));

  connect(parent, SIGNAL(abortOperation()),
          this,   SLOT(abortOperation()));
  connect(parent, SIGNAL(analysisClosed()),
          this,   SLOT(abortOperation()));
}

//-----------------------------------------------------------------------------
EditionTools::~EditionTools()
{
  abortOperation();

  disconnect(m_viewManager.get());
  disconnect(this->parent());
}

//-----------------------------------------------------------------------------
void EditionTools::setEnabled(bool value)
{
  m_enabled = value;

  m_manualEdition->setEnabled(value);
  m_split        ->setEnabled(value);
  m_morphological->setEnabled(value);
}

//-----------------------------------------------------------------------------
bool EditionTools::enabled() const
{
  return m_enabled;
}

//-----------------------------------------------------------------------------
ToolSList EditionTools::tools()
{
  m_manualEdition->setEnabled(true);
  m_split        ->setEnabled(true);
  m_morphological->setEnabled(true);

  selectionChanged();

  ToolSList availableTools;
  availableTools << m_manualEdition;
  availableTools << m_split;
  availableTools << m_morphological;

  return availableTools;
}

//-----------------------------------------------------------------------------
void EditionTools::selectionChanged()
{
  auto selection     = m_viewManager->selection()->segmentations();
  auto selectionSize = selection.size();

  SegmentationAdapterSPtr selectedSeg;
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
void EditionTools::abortOperation()
{
  m_manualEdition->abortOperation();
  m_split        ->abortOperation();
}

// //-----------------------------------------------------------------------------
// void EditionTools::drawStroke(CategoryAdapterSPtr category, BinaryMaskSPtr<unsigned char> mask)
// {
//   auto tool      = qobject_cast<ManualEditionToolPtr>(sender());
//   auto selection = m_viewManager->selection();
//
//   if(selection->items().empty())
//   {
//     ChannelAdapterList primaryChannel;
//     primaryChannel << m_viewManager->activeChannel();
//     selection->set(primaryChannel);
//   }
//
//   SegmentationAdapterSPtr segmentation;
//   if(!selection->segmentations().empty())
//   {
//   }
//   else if(!selection->channels().empty())
//   {
//   }
//   else
//   {
//     Q_ASSERT(false);
//   }
//
//   m_viewManager->updateSegmentationRepresentations(segmentation.get());
//   m_viewManager->updateViews();
// }

//-----------------------------------------------------------------------------
void EditionTools::onVoxelDeletion(ViewItemAdapterPtr item)
{
  Q_ASSERT(item && isSegmentation(item) && hasVolumetricData(item->output()));

  auto segmentation = segmentationPtr(item);

  auto volume = volumetricData(segmentation->output());

  if (volume->isEmpty())
  {
    m_undoStack->blockSignals(true);
    do
    {
      m_undoStack->undo();
    }
    while(volume->isEmpty());
    m_undoStack->blockSignals(false);

    if(segmentation->output()->numberOfDatas() == 1)
    {
      auto name = segmentation->data(Qt::DisplayRole).toString();
      DefaultDialogs::InformationMessage(tr("Deleting segmentation"),
                                         tr("%1 will be deleted because all its voxels were erased.").arg(name));

      m_undoStack->beginMacro("Remove Segmentation");
      m_undoStack->push(new RemoveSegmentations(segmentation, m_model));
    }
    else
    {
      auto output = segmentation->output();
      m_undoStack->beginMacro("Remove Segmentation's volume");
      m_undoStack->push(new RemoveDataCommand(output, VolumetricData<itkVolumeType>::TYPE));
    }
    m_undoStack->endMacro();
  }
  else
  {
    fitToContents(volume, SEG_BG_VALUE);
  }
}
