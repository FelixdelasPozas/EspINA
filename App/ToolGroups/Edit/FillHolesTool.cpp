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
#include "FillHolesTool.h"
#include <Core/Analysis/Segmentation.h>
#include <Core/Utils/EspinaException.h>
#include <Filters/FillHolesFilter.h>
#include <Undo/ReplaceOutputCommand.h>
#include <App/ToolGroups/Edit/EditToolGroup.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

//------------------------------------------------------------------------
FillHolesTool::FillHolesTool(Support::Context &context)
: EditTool("FillHoles", ":/espina/fill_holes.svg", tr("Fill Internal Holes"),context)
{
  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(applyFilter()));
}

//------------------------------------------------------------------------
void FillHolesTool::abortOperation()
{
  for(auto filter: m_executingTasks.keys())
  {
    filter->abort();
  }
}

//------------------------------------------------------------------------
bool FillHolesTool::acceptsNInputs(int n) const
{
  return n > 0;
}

//------------------------------------------------------------------------
bool FillHolesTool::acceptsSelection(SegmentationAdapterList segmentations)
{
  return acceptsVolumetricSegmentations(segmentations);
}

//------------------------------------------------------------------------
void FillHolesTool::applyFilter()
{
  auto segmentations = getSelectedSegmentations();

  for (auto segmentation : segmentations)
  {
    InputSList inputs;

    inputs << segmentation->asInput();

    auto filter = getFactory()->createFilter<FillHolesFilter>(inputs, EditionFilterFactory::FILL_HOLES_FILTER);

    filter->setDescription(tr("Fill %1 Holes").arg(segmentation->data(Qt::DisplayRole).toString()));

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
void FillHolesTool::onTaskFinished()
{
  auto filter = dynamic_cast<FillHolesFilterPtr>(sender());

  if(filter)
  {
    auto taskContext = m_executingTasks[filter];

    if (!filter->isAborted() && filter->validOutput(0))
    {
      if (filter->numberOfOutputs() != 1)
      {
        auto what    = QObject::tr("Unable to process filter result.");
        auto details = QObject::tr("FillHolesTool::onTaskFinished() -> Invalid number of outputs: %1").arg(filter->numberOfOutputs());
        throw EspinaException(what, details);
      }

      auto undoStack = getUndoStack();

      auto operation = QString("Fill " + taskContext.Segmentation->data().toString() + " Holes");
      undoStack->beginMacro(operation);
      undoStack->push(new ReplaceOutputCommand(taskContext.Segmentation, getInput(taskContext.Filter, 0)));
      undoStack->endMacro();
    }

    markAsBeingModified(taskContext.Segmentation, false);

    m_executingTasks.remove(filter);
  }
  else
  {
    auto what    = QObject::tr("Unable to identify signal sender object as filter.");
    auto details = QObject::tr("FillHolesTool::onTaskFinished() -> ") + what;
    throw EspinaException(what, details);
  }
}
