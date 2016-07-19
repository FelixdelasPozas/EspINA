/*
 * FillHoles2DTool.cpp
 *
 *  Created on: 19 de jul. de 2016
 *      Author: heavy
 */

#include "FillHoles2DTool.h"
#include <Filters/FillHoles2DFilter.h>
#include <Undo/ReplaceOutputCommand.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

const Filter::Type FILL_HOLES_2D_FILTER  = "FillSegmentationHoles2D";

//------------------------------------------------------------------------
FillHoles2DTool::FillHoles2DTool(Support::Context &context)
: EditTool("FillHoles2D", ":/espina/fill_holes_2D.svg", tr("Fill Internal Holes in z direction"),context)
{
  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(fillHoles2D()));
}

//------------------------------------------------------------------------
void FillHoles2DTool::abortOperation()
{
}

//------------------------------------------------------------------------
bool FillHoles2DTool::acceptsNInputs(int n) const
{
  return n > 0;
}

//------------------------------------------------------------------------
bool FillHoles2DTool::acceptsSelection(SegmentationAdapterList segmentations)
{
  return acceptsVolumetricSegmentations(segmentations);
}

//------------------------------------------------------------------------
void FillHoles2DTool::fillHoles2D()
{
  auto segmentations = getSelectedSegmentations();

  for (auto segmentation : segmentations)
  {
    InputSList inputs;

    inputs << segmentation->asInput();

    auto filter = getFactory()->createFilter<FillHoles2DFilter>(inputs, FILL_HOLES_2D_FILTER);

    filter->setDescription(tr("Fill %1 Holes 2D").arg(segmentation->data(Qt::DisplayRole).toString()));

    TaskContext taskContext;

    taskContext.Task         = filter;
    taskContext.Operation    = tr("Fill Segmentation Holes 2D");
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
void FillHoles2DTool::onTaskFinished()
{
  auto filter = dynamic_cast<FillHoles2DFilterPtr>(sender());

  auto taskContext = m_executingTasks[filter];

  if (!filter->isAborted())
  {
    if (filter->numberOfOutputs() != 1)
    {
      auto what    = QObject::tr("Unable to process filter result.");
      auto details = QObject::tr("FillHolesTool::onTaskFinished() -> Invalid number of outputs: %1").arg(filter->numberOfOutputs());
      throw EspinaException(what, details);
    }

    auto undoStack = getUndoStack();

    undoStack->beginMacro(taskContext.Operation);
    undoStack->push(new ReplaceOutputCommand(taskContext.Segmentation, getInput(taskContext.Task, 0)));
    undoStack->endMacro();

  }

  markAsBeingModified(taskContext.Segmentation, false);

  m_executingTasks.remove(filter);
}
