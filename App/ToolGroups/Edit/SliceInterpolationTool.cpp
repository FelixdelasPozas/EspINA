/*
 * Copyright (C) 2017, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
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
#include <GUI/Model/Utils/QueryAdapter.h>
#include "SliceInterpolationTool.h"
#include "EditToolGroup.h"
#include <Undo/ReplaceOutputCommand.h>

// QT
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

//------------------------------------------------------------------------
SliceInterpolationTool::SliceInterpolationTool(Support::Context &context)
    : EditTool( "SliceInterpolation", ":/espina/slice_interpolation.svg",
                tr("Interpolate marked slices in a specific direction"), context)
{
  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(applyFilter()));
}

//------------------------------------------------------------------------
SliceInterpolationTool::~SliceInterpolationTool()
{
  disconnect( this, SIGNAL(triggered(bool)),
              this, SLOT(applyFilter()));
  abortOperation();
}

//------------------------------------------------------------------------
void SliceInterpolationTool::abortOperation()
{
  for(auto task: m_executingTasks.keys())
  {
    disconnect(task, SIGNAL(finished()),
                this,    SLOT(onTaskFinished()));
    task->abort();
  }
  m_executingTasks.clear();
}

//------------------------------------------------------------------------
bool SliceInterpolationTool::acceptsNInputs(int n) const
{
  return n > 0;
}

//------------------------------------------------------------------------
bool SliceInterpolationTool::acceptsSelection(SegmentationAdapterList segmentations)
{
  return acceptsVolumetricSegmentations(segmentations);
}

//------------------------------------------------------------------------
void SliceInterpolationTool::applyFilter()
{
  auto segmentations = getSelectedSegmentations();

  for (auto segmentation : segmentations)
  {
    InputSList inputs;

    inputs << segmentation->asInput();

    auto channels = QueryAdapter::channels(segmentation);
    Q_ASSERT(!channels.empty());
    auto stack = channels.first();
    inputs << stack->asInput();

    auto filter = getFactory()->createFilter<SliceInterpolationFilter>(inputs, MorphologicalFilterFactory::SLICE_INTERPOLATION_FILTER);

    filter->setDescription(tr("Interpolate slices from %1").arg(segmentation->data().toString()));

    TaskContext taskContext;
    taskContext.Filter       = filter;
    taskContext.Segmentation = segmentation;

    markAsBeingModified(segmentation, true);

    m_executingTasks[filter.get()] = taskContext;

    showTaskProgress(filter);

    connect(filter.get(), SIGNAL(finished()),
            this,         SLOT(onTaskFinished()));

    Task::submit(filter);
  }
}

//------------------------------------------------------------------------
void SliceInterpolationTool::onTaskFinished()
{
  auto filter = dynamic_cast<SliceInterpolationFilterPtr>(sender());

  if(filter && m_executingTasks.keys().contains(filter))
  {
    auto taskContext = m_executingTasks[filter];

    if (!filter->isAborted() && filter->validOutput(0))
    {
      if (filter->numberOfOutputs() != 1)
      {
        auto what = QObject::tr("Unable to process filter result.");
        auto details = QObject::tr("SliceInterpolationTool::onTaskFinished() -> Invalid number of outputs: %1").arg(filter->numberOfOutputs());
        throw EspinaException(what, details);
      }

      auto undoStack = getUndoStack();

      undoStack->beginMacro(taskContext.Filter->description());
      undoStack->push(new ReplaceOutputCommand(taskContext.Segmentation, getInput(taskContext.Filter, 0)));
      undoStack->endMacro();
    }

    markAsBeingModified(taskContext.Segmentation, false);

    m_executingTasks.remove(filter);
  }
  else
  {
    auto what = QObject::tr("Unable to identify signal sender object as filter.");
    auto details = QObject::tr("SliceInterpolationTool::onTaskFinished() -> ") + what;
    throw EspinaException(what, details);
  }
}
