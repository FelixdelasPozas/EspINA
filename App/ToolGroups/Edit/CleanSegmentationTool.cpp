/*
 File: CleanSegmentationTool.cpp
 Created on: 25/07/2019
 Author: Felix de las Pozas Alvarez

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

// Project
#include "CleanSegmentationTool.h"
#include "EditToolGroup.h"
#include <GUI/Widgets/Styles.h>
#include <GUI/ModelFactory.h>
#include <Undo/ReplaceOutputCommand.h>
#include <GUI/Dialogs/DefaultDialogs.h>

// Qt
#include <QMutex>
#include <QWriteLocker>
#include <QThread>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets::Styles;

//-----------------------------------------------------------------------------
CleanSegmentationTool::CleanSegmentationTool(Support::Context& context)
: EditTool("CleanSegmentation", ":/espina/clean_segmentation.svg", tr("Remove groups of isolated voxels"),context)
{
  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(applyFilter()));
}

//-----------------------------------------------------------------------------
bool CleanSegmentationTool::acceptsNInputs(int n) const
{
  return n == 1;
}

//-----------------------------------------------------------------------------
bool CleanSegmentationTool::acceptsSelection(SegmentationAdapterList segmentations)
{
  return acceptsVolumetricSegmentations(segmentations);
}

//-----------------------------------------------------------------------------
void CleanSegmentationTool::applyFilter()
{
  auto segmentations = getSelectedSegmentations();
  if(segmentations.size() > 1) return;

  auto segmentation = segmentations.takeFirst();

  InputSList inputs;
  inputs << segmentation->asInput();

  auto filter = getFactory()->createFilter<CleanSegmentationVoxelsFilter>(inputs, EditionFilterFactory::CLEAN_SEGMENTATION_FILTER);

  filter->setDescription(tr("Remove %1 isolated voxels").arg(segmentation->data(Qt::DisplayRole).toString()));

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

//-----------------------------------------------------------------------------
void CleanSegmentationTool::onTaskFinished()
{
  auto filter = dynamic_cast<CleanSegmentationVoxelsFilter *>(sender());

  if(filter)
  {
    auto taskContext = m_executingTasks[filter];
    const auto segName = taskContext.Segmentation->data().toString();
    const auto removedNumber = filter->removedVoxelsNumber();

    if (removedNumber > 0 && !filter->isAborted() && filter->validOutput(0))
    {
      if (filter->numberOfOutputs() != 1)
      {
        auto what    = QObject::tr("Unable to process filter result.");
        auto details = QObject::tr("CleanSegmentationTool::onTaskFinished() -> Invalid number of outputs: %1").arg(filter->numberOfOutputs());
        throw EspinaException(what, details);
      }

      if(filter->errors().isEmpty())
      {
        WaitingCursor cursor;

        auto undoStack = getUndoStack();
        undoStack->beginMacro(tr("Remove isolated voxel groups of '%1'. Removed %2 voxel%3.").arg(segName).arg(removedNumber).arg(removedNumber > 1 ? "s":""));
        undoStack->push(new ReplaceOutputCommand(taskContext.Segmentation, getInput(taskContext.Filter, 0)));
        undoStack->endMacro();
      }
    }

    markAsBeingModified(taskContext.Segmentation, false);

    m_executingTasks.remove(filter);

    const auto title = tr("Remove isolated voxels.");
    if(filter->errors().isEmpty())
    {
      const auto message = (removedNumber == 0 ? tr("'%1' is already clean.").arg(segName) : tr("Removed %1 voxels from segmentation '%2'.").arg(removedNumber).arg(segName));
      DefaultDialogs::InformationMessage(message, title);
    }
    else
    {
      DefaultDialogs::ErrorMessage(filter->errors().join("\n"), title);
    }
  }
  else
  {
    auto what    = QObject::tr("Unable to identify signal sender object as filter.");
    auto details = QObject::tr("FillHolesTool::onTaskFinished() -> ") + what;
    throw EspinaException(what, details);
  }
}

//-----------------------------------------------------------------------------
void CleanSegmentationTool::abortTasks()
{
  for(auto data: m_executingTasks)
  {
    disconnect(data.Filter.get(), SIGNAL(finished()),
               this,              SLOT(onTaskFinished()));

    data.Filter->abort();

    markAsBeingModified(data.Segmentation, false);

    if(!data.Filter->thread()->wait(500))
    {
      data.Filter->thread()->terminate();
    }
  }

  m_executingTasks.clear();
}
