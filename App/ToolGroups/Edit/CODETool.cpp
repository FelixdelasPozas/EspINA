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
#include <Core/Utils/EspinaException.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Widgets/Styles.h>
#include <Undo/RemoveSegmentations.h>
#include <Undo/ReplaceOutputCommand.h>
#include <GUI/Widgets/ToolButton.h>

// Qt
#include <QHBoxLayout>
#include <QThread>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::Widgets::Styles;
using namespace ESPINA::Support::Widgets;

const QString RADIUS = "Radius";

//------------------------------------------------------------------------
CODEToolBase::CODEToolBase(const Filter::Type type,
                           const QString     &toolId,
                           const QString     &name,
                           const QString     &icon,
                           const QString     &tooltip,
                           Support::Context  &context)
: EditTool(toolId, icon, tooltip, context)
, m_type{type}
, m_name{name}
{
  setCheckable(true);
  setExclusive(true);

  initOptionWidgets();
}

//------------------------------------------------------------------------
CODEToolBase::~CODEToolBase()
{
  disconnect(m_apply, SIGNAL(clicked(bool)),
             this,    SLOT(onApplyClicked()));

  abortTasks();
}

//------------------------------------------------------------------------
void CODEToolBase::initOptionWidgets()
{
  m_radius = new NumericalInput();

  m_radius->setLabelText(tr("Radius"));
  m_radius->setMinimum(1);
  m_radius->setMaximum(99);
  m_radius->setSliderVisibility(false);
  m_radius->setToolTip(tr("Morphological %1 radius.").arg(m_name));
  Styles::setNestedStyle(m_radius);

  m_apply = Styles::createToolButton(":/espina/apply.svg", tr("Apply"));

  connect(m_apply, SIGNAL(clicked(bool)),
          this,    SLOT(onApplyClicked()));

  addSettingsWidget(m_radius);
  addSettingsWidget(m_apply);
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
    filter->setDescription(tr("%1 %2").arg(m_name).arg(segmentation->data(Qt::DisplayRole).toString()));

    TaskContext task;

    task.Task         = filter;
    task.Operation    = tr("%1 Segmentation").arg(m_name);
    task.Segmentation = segmentation;

    markAsBeingModified(segmentation, true);

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

  auto taskContext = m_executingTasks[filter];

  if (!filter->isAborted())
  {
    auto undoStack   = getUndoStack();

    if (!filter->validOutput(0))
    {
      auto name    = taskContext.Segmentation->data(Qt::DisplayRole).toString();
      auto title   = taskContext.Operation;
      auto buttons = QMessageBox::Yes|QMessageBox::Cancel;
      auto message = tr("%1 segmentation will be deleted by %2 operation.\n"
                        "Do you want to continue with the operation?").arg(name).arg(taskContext.Operation);

      if (DefaultDialogs::UserQuestion(message, buttons, title) == QMessageBox::Yes)
      {
        WaitingCursor cursor;

        auto macroText = tr("Remove segmentation '%1' by %2 operation with radius of %3.").arg(taskContext.Segmentation->data().toString()).arg(taskContext.Operation).arg(filter->radius());
        undoStack->beginMacro(macroText);
        undoStack->push(new RemoveSegmentations(taskContext.Segmentation, getModel()));
        undoStack->endMacro();
      }
    }
    else
    {
      if (filter->numberOfOutputs() != 1)
      {
        auto what    = QObject::tr("Unable to process filter result.");
        auto details = QObject::tr("CODEToolBase::onTaskFinished() -> Invalid number of outputs: %1").arg(filter->numberOfOutputs());
        throw EspinaException(what, details);
      }

      WaitingCursor cursor;

      auto macroText = tr("%1 '%2' with radius of %3.").arg(taskContext.Operation).arg(taskContext.Segmentation->data().toString()).arg(filter->radius());
      undoStack->beginMacro(macroText);
      undoStack->push(new ReplaceOutputCommand(taskContext.Segmentation, getInput(taskContext.Task, 0)));
      undoStack->endMacro();
    }

  }
  markAsBeingModified(taskContext.Segmentation, false);

  m_executingTasks.remove(filter);
}

//------------------------------------------------------------------------
bool CODEToolBase::acceptsSelection(SegmentationAdapterList segmentations)
{
  return acceptsVolumetricSegmentations(segmentations);
}

//------------------------------------------------------------------------
void CODEToolBase::abortTasks()
{
  for(auto task: m_executingTasks)
  {
    disconnect(task.Task.get(), SIGNAL(finished()),
               this,            SLOT(onTaskFinished()));

    task.Task->abort();

    markAsBeingModified(task.Segmentation, false);

    if(!task.Task->thread()->wait(100))
    {
      task.Task->thread()->terminate();
    }
  }

  m_executingTasks.clear();
}
