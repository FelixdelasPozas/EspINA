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

#include "ManualEditionTool.h"
#include "SplitTool.h"
#include "CODETool.h"
#include "FillHolesTool.h"
#include "ImageLogicTool.h"

#include <Core/Analysis/Output.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <Undo/DrawUndoCommand.h>
#include <Undo/ModifyDataCommand.h>
#include <Undo/RemoveSegmentations.h>
#include <Support/Widgets/RefineTool.h>
#include <Filters/CloseFilter.h>
#include <Filters/OpenFilter.h>
#include <Filters/DilateFilter.h>
#include <Filters/ErodeFilter.h>

// Qt
#include <QApplication>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Model::Utils;

//-----------------------------------------------------------------------------
RefineToolGroup::RefineToolGroup(FilterDelegateFactorySPtr filterDelegateFactory,
                                 Support::Context &context)
: ToolGroup{":/espina/toolgroup_refine.svg", tr("Refine")}
, m_context(context)
{
  auto morphologicalFactory = std::make_shared<MorphologicalFilterFactory>();
  context.factory()->registerFilterFactory(morphologicalFactory);
  filterDelegateFactory->registerFilterDelegateFactory(morphologicalFactory);

  initManualEditionTool();
  initSplitTool();
  initCODETools();
  initFillHolesTool();
  initImageLogicTools();
}

//-----------------------------------------------------------------------------
RefineToolGroup::~RefineToolGroup()
{
}

//-----------------------------------------------------------------------------
void RefineToolGroup::initManualEditionTool()
{
  auto manualEdition = std::make_shared<ManualEditionTool>(m_context);

  connect(manualEdition.get(), SIGNAL(voxelsDeleted(ViewItemAdapterPtr)),
          this,                SLOT(onVoxelDeletion(ViewItemAdapterPtr)));

  addTool(manualEdition);
}

//-----------------------------------------------------------------------------
void RefineToolGroup::initSplitTool()
{
  addTool(std::make_shared<SplitTool>(m_context));
}

//-----------------------------------------------------------------------------
void RefineToolGroup::initCODETools()
{
  addTool(std::make_shared<CODETool<CloseFilter>> (tr("Close"), ":/espina/close.png",  tr("Close selected segmentations") , m_context));
  addTool(std::make_shared<CODETool<OpenFilter>>  (tr("Open"),  ":/espina/open.png",   tr("Open selected segmentations")  , m_context));
  addTool(std::make_shared<CODETool<DilateFilter>>(tr("Dilate"),":/espina/dilate.png", tr("Dilate selected segmentations"), m_context));
  addTool(std::make_shared<CODETool<ErodeFilter>> (tr("Erode"), ":/espina/erode.png",  tr("Erode selected segmentations") , m_context));
}

//-----------------------------------------------------------------------------
void RefineToolGroup::initFillHolesTool()
{
  addTool(std::make_shared<FillHolesTool>(m_context));
}

//-----------------------------------------------------------------------------
void RefineToolGroup::initImageLogicTools()
{
  auto addition  = std::make_shared<ImageLogicTool>(":/espina/add.svg",    tr("Merge selected segmentations"),     m_context);
  addition->setOperation(ImageLogicFilter::Operation::ADDITION);

  auto substract = std::make_shared<ImageLogicTool>(":/espina/remove.svg", tr("Substract selected segmentations"), m_context);
  substract->setOperation(ImageLogicFilter::Operation::SUBTRACTION);

  addTool(addition);
  addTool(substract);
}

//-----------------------------------------------------------------------------
void RefineToolGroup::onVoxelDeletion(ViewItemAdapterPtr item)
{
  Q_ASSERT(item && isSegmentation(item) && hasVolumetricData(item->output()));

  auto segmentation = segmentationPtr(item);

  auto volume = writeLockVolume(segmentation->output());

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