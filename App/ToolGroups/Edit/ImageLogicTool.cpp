/*
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ESPINA
#include "ImageLogicTool.h"
#include "EditToolGroup.h"
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Model/Utils/ModelUtils.h>
#include <GUI/Widgets/Styles.h>
#include <Undo/AddSegmentations.h>
#include <Undo/RemoveSegmentations.h>

// Qt
#include <QThread>

using namespace ESPINA;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::Widgets::Styles;
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
ImageLogicTool::~ImageLogicTool()
{
  abortTasks();
}

//------------------------------------------------------------------------
void ImageLogicTool::setOperation(ImageLogicFilter::Operation operation)
{
  m_operation = operation;
}

//------------------------------------------------------------------------
bool ImageLogicTool::acceptsNInputs(int n) const
{
  return n > 1;
}

//------------------------------------------------------------------------
bool ImageLogicTool::acceptsSelection(SegmentationAdapterList segmentations)
{
  return EditTool::acceptsVolumetricSegmentations(segmentations);
}

//------------------------------------------------------------------------
void ImageLogicTool::applyFilter()
{
  auto segmentations = getSelectedSegmentations();

  Q_ASSERT(segmentations.size() > 1);

  auto type        = EditionFilterFactory::ADDITION_FILTER;
  auto description = tr("Segmentation addition.");
  auto remove      = true;

  if (ImageLogicFilter::Operation::SUBTRACTION == m_operation)
  {
    type        = EditionFilterFactory::SUBTRACTION_FILTER;
    description = tr("Segmentation subtraction.");
    remove      = m_removeOnSubtract;
  }

  auto selection = getSelection();
  selection->clear();

  InputSList inputs;
  for(auto segmentation: segmentations)
  {
    markAsBeingModified(segmentation, true);
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
    segmentation->setNumber(firstUnusedSegmentationNumber(getModel()));

    auto samples = QueryAdapter::samples(taskContext.Segmentations.first());

    SegmentationAdapterList segmentationList;
    QString opSegmentations;

    for(auto item: taskContext.Segmentations)
    {
      item->setBeingModified(false);

      if(item != taskContext.Segmentations.first())
      {
        opSegmentations += (item == taskContext.Segmentations.last() ? " and ": ", ");
      }
      opSegmentations += "'" + item->data().toString() + "'";

      if(!taskContext.Remove && (item != taskContext.Segmentations.first()))
      {
        continue;
      }

      segmentationList << item;
    }

    auto macroText = tr("Add segmentation '%1' from the %2 of %3.").arg(segmentation->data().toString())
                     .arg(taskContext.Operation == ImageLogicFilter::Operation::SUBTRACTION ? "subtraction" : "addition")
                     .arg(opSegmentations);

    {
      WaitingCursor cursor;

      undoStack->beginMacro(macroText);
      undoStack->push(new AddSegmentations(segmentation, samples, getModel()));
      undoStack->push(new RemoveSegmentations(segmentationList, getModel()));
      undoStack->endMacro();
    }

    getSelection()->clear();
    getSelection()->set(toViewItemList(segmentation.get()));
  }
  else
  {
    for(auto item: m_executingTasks[filter].Segmentations)
    {
      markAsBeingModified(item, false);
    }
  }

  m_executingTasks.remove(filter);
}

//------------------------------------------------------------------------
void ImageLogicTool::abortTasks()
{
  for(auto data: m_executingTasks)
  {
    disconnect(data.Task.get(), SIGNAL(finished()),
               this,            SLOT(onTaskFinished()));

    data.Task->abort();

    for(auto segmentation: data.Segmentations)
    {
      markAsBeingModified(segmentation, false);
    }

    if(!data.Task->thread()->wait(500))
    {
      data.Task->thread()->terminate();
    }
  }

  m_executingTasks.clear();
}
