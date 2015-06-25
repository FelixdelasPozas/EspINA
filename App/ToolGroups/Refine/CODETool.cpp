/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
#include "CODETool.h"
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Widgets/Styles.h>
#include <Undo/RemoveSegmentations.h>
#include <Undo/ReplaceOutputCommand.h>

#include <QHBoxLayout>
#include <QPushButton>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::Support::Widgets;

const QString RADIUS = "Radius";

//------------------------------------------------------------------------
CODEToolBase::CODEToolBase(const QString    &toolId,
                           const QString    &name,
                           const QString    &icon,
                           const QString    &tooltip,
                           Support::Context &context)
: RefineTool(toolId, icon, tooltip, context)
, m_name    {name}
{
  setCheckable(true);
  setExclusive(true);

  setGroupWith("CODE");

  initOptionWidgets();
}

//------------------------------------------------------------------------
void CODEToolBase::abortOperation()
{
}

//------------------------------------------------------------------------
void CODEToolBase::initOptionWidgets()
{
  m_radius = new NumericalInput();

  m_radius->setLabelText(tr("Radius"));
  m_radius->setMinimum(1);
  m_radius->setMaximum(99);
  m_radius->setSliderVisibility(false);

  auto apply = Styles::createToolButton(":/espina/tick.png", tr("Apply"));

  connect(apply, SIGNAL(clicked(bool)),
          this,  SLOT(onApplyClicked()));

  addSettingsWidget(m_radius);
  addSettingsWidget(apply);
}

//------------------------------------------------------------------------
bool CODEToolBase::acceptsNInputs(int n) const
{
  return n > 0;
}

//------------------------------------------------------------------------
void CODEToolBase::onApplyClicked()
{
  setEventHandler(nullptr);

  auto selection = getSelectedSegmentations();

  Q_ASSERT(!selection.isEmpty());

  for (auto segmentation :  selection)
  {
    InputSList inputs;

    inputs << segmentation->asInput();

    auto filter = createFilter(inputs, m_type);

    filter->setRadius(m_radius->value());
    filter->setDescription(tr("%1 %2").arg(m_name)
    .arg(segmentation->data(Qt::DisplayRole)
    .toString()));

    TaskContext task;

    task.Task         = filter;
    task.Operation    = tr("%1 Segmentation").arg(m_name);
    task.Segmentation = segmentation;

    segmentation->setBeingModified(true);

    m_executingTasks[filter.get()] = task;

    showTaskProgress(filter);

    connect(filter.get(), SIGNAL(finished()),
            this,         SLOT(onTaskFinished()));

    Task::submit(filter);
  }
}

//------------------------------------------------------------------------
void CODEToolBase::restoreSettings(std::shared_ptr<QSettings> settings)
{
  m_radius->setValue(settings->value(RADIUS, 1).toInt());
}

//------------------------------------------------------------------------
void CODEToolBase::saveSettings(std::shared_ptr<QSettings> settings)
{
  settings->setValue(RADIUS, m_radius->value());
}

//------------------------------------------------------------------------
void CODEToolBase::onTaskFinished()
{
  auto filter = dynamic_cast<MorphologicalEditionFilterPtr>(sender());

  if (!filter->isAborted())
  {
    auto taskContext = m_executingTasks[filter];
    auto undoStack   = getUndoStack();

    if (filter->isOutputEmpty())
    {
      auto name    = taskContext.Segmentation->data(Qt::DisplayRole).toString();
      auto title   = taskContext.Operation;
      auto message = tr("%1 segmentation will be deleted by %2 operation.\n"
                        "Do you want to continue with the operation?").arg(name).arg(taskContext.Operation);

      if (DefaultDialogs::UserConfirmation(title, message))
      {
        undoStack->beginMacro(taskContext.Operation);
        undoStack->push(new RemoveSegmentations(taskContext.Segmentation, context().model()));
        undoStack->endMacro();
      }
    }
    else
    {
      if (filter->numberOfOutputs() != 1) throw Filter::Undefined_Output_Exception();

      undoStack->beginMacro(taskContext.Operation);
      undoStack->push(new ReplaceOutputCommand(taskContext.Segmentation, getInput(taskContext.Task, 0)));
      undoStack->endMacro();
    }

    taskContext.Segmentation->setBeingModified(false);
  }

  m_executingTasks.remove(filter);
}
