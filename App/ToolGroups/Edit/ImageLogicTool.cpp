/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include "ImageLogicTool.h"
#include <Undo/RemoveSegmentations.h>
#include <Undo/AddSegmentations.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include "EditToolGroup.h"

using namespace ESPINA;
using namespace ESPINA::Support::Widgets;

//------------------------------------------------------------------------
ImageLogicTool::ImageLogicTool(const QString &id, const QString &icon, const QString &tooltip, Support::Context &context)
: EditTool          (id, icon, tooltip, context)
, m_operation       {ImageLogicFilter::Operation::ADDITION}
, m_removeOnSubtract{true}
{
  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(applyFilter()));
}

//------------------------------------------------------------------------
void ImageLogicTool::setOperation(ImageLogicFilter::Operation operation)
{
  m_operation = operation;
}

//------------------------------------------------------------------------
void ImageLogicTool::abortOperation()
{
}

//------------------------------------------------------------------------
bool ImageLogicTool::acceptsNInputs(int n) const
{
  return n > 1;
}

//------------------------------------------------------------------------
bool ImageLogicTool::acceptsSelection(SegmentationAdapterList segmentations)
{
  return EditTool::acceptsVolumetricSegmenations(segmentations);
}

//------------------------------------------------------------------------
void ImageLogicTool::applyFilter()
{
  auto segmentations = getSelectedSegmentations();

  Q_ASSERT(segmentations.size() > 1);

  auto type        = MorphologicalFilterFactory::ADDITION_FILTER;
  auto description = tr("Segmentation addition");
  auto remove      = true;

  if (ImageLogicFilter::Operation::SUBTRACTION == m_operation)
  {
    type        = MorphologicalFilterFactory::SUBTRACTION_FILTER;
    description = tr("Segmentation subtraction");
    remove      = m_removeOnSubtract;
  }

  auto selection = getSelection();
  selection->clear();

  InputSList inputs;
  for(auto segmentation: segmentations)
  {
    segmentation->setBeingModified(true);
    inputs << segmentation->asInput();
  }

  selection->set(segmentations);

  auto filter = getFactory()->createFilter<ImageLogicFilter>(inputs, type);
  filter->setOperation(m_operation);
  filter->setDescription(description);

  TaskContext taskContext;

  taskContext.Task          = filter;
  taskContext.Operation     = m_operation;
  taskContext.Segmentations = segmentations;
  taskContext.Remove        = remove;

  m_executingTasks.insert(filter.get(), taskContext);

  showTaskProgress(filter);

  connect(filter.get(), SIGNAL(finished()),
          this,         SLOT(onTaskFinished()));

  Task::submit(filter);
}

//------------------------------------------------------------------------
void ImageLogicTool::onTaskFinished()
{
  auto filter = dynamic_cast<ImageLogicFilterPtr>(sender());

  if (!filter->isAborted())
  {
    Q_ASSERT(m_executingTasks.keys().contains(filter));

    auto taskContext = m_executingTasks[filter];
    auto undoStack   = getUndoStack();

    auto segmentation = getFactory()->createSegmentation(taskContext.Task, 0);

    segmentation->setCategory(taskContext.Segmentations.first()->category());

    auto samples = QueryAdapter::samples(taskContext.Segmentations.first());

    undoStack->beginMacro(filter->description());
    undoStack->push(new AddSegmentations(segmentation, samples, getModel()));

    SegmentationAdapterList segmentationList;

    for(auto item: taskContext.Segmentations)
    {
      item->setBeingModified(false);

      if(!taskContext.Remove && (item != taskContext.Segmentations.first()))
      {
        continue;
      }

      segmentationList << item;
    }

    undoStack->push(new RemoveSegmentations(segmentationList, getModel()));
    undoStack->endMacro();

    segmentationList.clear();
    segmentationList << segmentation.get();

    auto selection = getSelection();
    selection->set(segmentationList);
  }

  m_executingTasks.remove(filter);
}
