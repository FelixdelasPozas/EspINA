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
#include "SliceInterpolationTool.h"
#include "EditToolGroup.h"

using namespace ESPINA;

//------------------------------------------------------------------------
SliceInterpolationTool::SliceInterpolationTool(Support::Context &context)
    : EditTool("SliceInterpolation", ":/espina/slice_interpolation.svg", tr("Interpolate marked slices"), context)
{
  // TODO
  setCheckable(true);
}

//------------------------------------------------------------------------
SliceInterpolationTool::~SliceInterpolationTool()
{
  // TODO
}

//------------------------------------------------------------------------
void ESPINA::SliceInterpolationTool::abortOperation()
{
  for(auto task: m_executingTasks.keys())
  {
    task->abort();
  }
}

//------------------------------------------------------------------------
bool ESPINA::SliceInterpolationTool::acceptsNInputs(int n) const
{
  return n > 0;// TODO
}

//------------------------------------------------------------------------
bool ESPINA::SliceInterpolationTool::acceptsSelection(SegmentationAdapterList segmentations)
{
  return acceptsVolumetricSegmentations(segmentations);// TODO
}

//------------------------------------------------------------------------
void ESPINA::SliceInterpolationTool::executeFilter()
{

  auto segmentations = getSelectedSegmentations();

  for (auto segmentation : segmentations)
  {
    InputSList inputs;

    inputs << segmentation->asInput();

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
void ESPINA::SliceInterpolationTool::onTaskFinished()
{
  // TODO
}
