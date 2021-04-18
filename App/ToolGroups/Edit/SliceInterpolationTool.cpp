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
#include <App/ToolGroups/Edit/EditToolGroup.h>
#include <Extensions/SLIC/StackSLIC.h>
#include <Core/Analysis/Channel.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Widgets/NumericalInput.h>
#include <GUI/Widgets/ToolButton.h>
#include <GUI/Widgets/Styles.h>
#include <Undo/ReplaceOutputCommand.h>

// Qt
#include <QPushButton>
#include <QThread>

using namespace ESPINA;
using namespace ESPINA::Extensions;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets;

const QString SLIC_ENABLED    {"SLIC Enabled"};
const QString THRESHOLD_VALUE {"Threshold value"};

//------------------------------------------------------------------------
SliceInterpolationTool::SliceInterpolationTool(Support::Context &context)
: EditTool{"SliceInterpolation", ":/espina/slice_interpolation.svg", tr("Interpolate marked slices in a specific direction"), context}
{
  setCheckable(true);

  initSettingsWidgets();
}

//------------------------------------------------------------------------
SliceInterpolationTool::~SliceInterpolationTool()
{
  abortTasks();
}

//------------------------------------------------------------------------
void SliceInterpolationTool::abortOperation()
{
  abortTasks();

  setChecked(false);
}

//------------------------------------------------------------------------
bool SliceInterpolationTool::acceptsNInputs(int n) const
{
  return (n > 0);
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

  auto useSLIC = m_slicButton->isChecked();
  double thresholdValue = m_threshold->value() / 100.;

  for(auto segmentation: segmentations)
  {
    InputSList inputs;
    inputs << segmentation->asInput();

    auto channels = QueryAdapter::channels(segmentation);
    Q_ASSERT(!channels.empty());
    auto stack = channels.first();
    inputs << stack->asInput();
    auto extension = retrieveOrCreateStackExtension<StackSLIC>(stack, getFactory());

    auto filter = getFactory()->createFilter<SliceInterpolationFilter>(inputs, EditionFilterFactory::SLICE_INTERPOLATION_FILTER);
    filter->setDescription(tr("Interpolate slices of %1").arg(segmentation->data().toString()));
    filter->setUseSLIC(useSLIC);
    filter->setSLICExtension(extension.get());
    filter->setThreshold(thresholdValue);

    TaskContext taskContext;
    taskContext.filter       = filter;
    taskContext.segmentation = segmentation;

    markAsBeingModified(segmentation, true);

    m_executingTasks.insert(filter.get(), taskContext);

    showTaskProgress(filter);

    connect(filter.get(), SIGNAL(finished()),
            this,         SLOT(onTaskFinished()));

    Task::submit(filter);
  }
}

//------------------------------------------------------------------------
void SliceInterpolationTool::restoreSettings(std::shared_ptr<QSettings> settings)
{
  m_slicButton->setChecked(settings->value(SLIC_ENABLED, false).toBool());
  m_threshold->setValue(settings->value(THRESHOLD_VALUE, 50).toInt());
}

//------------------------------------------------------------------------
void SliceInterpolationTool::saveSettings(std::shared_ptr<QSettings> settings)
{
  settings->setValue(SLIC_ENABLED, m_slicButton->isChecked());
  settings->setValue(THRESHOLD_VALUE, m_threshold->value());
}

//------------------------------------------------------------------------
void SliceInterpolationTool::initSettingsWidgets()
{
  m_slicButton = Styles::createToolButton(":espina/slic.svg", tr("Use SLIC clustering"));
  m_slicButton->setCheckable(true);
  m_slicButton->setChecked(false);
  m_slicButton->setToolTip(tr("Use supervoxel clustering"));
  addSettingsWidget(m_slicButton);

  m_threshold = new NumericalInput{};
  m_threshold->setLabelText("Threshold");
  m_threshold->setWidgetsToolTip(tr("Voxel inclusion threshold pertentage"));
  m_threshold->setLabelVisibility(true);
  m_threshold->setSliderVisibility(true);
  m_threshold->setSpinBoxVisibility(true);
  m_threshold->setSpinBoxSuffix("%");
  m_threshold->setMinimum(10);
  m_threshold->setMaximum(90);
  m_threshold->setValue(50);

  Styles::setNestedStyle(m_threshold);
  addSettingsWidget(m_threshold);

  auto apply = Styles::createToolButton(":/espina/apply.svg", tr("Apply"));
  apply->setToolTip(tr("Apply slice interpolation to selected segmentations."));

  connect(apply, SIGNAL(clicked(bool)),
          this,  SLOT(applyFilter()));

  addSettingsWidget(apply);
}

//------------------------------------------------------------------------
void SliceInterpolationTool::onTaskFinished()
{
  auto filter = dynamic_cast<SliceInterpolationFilterPtr>(sender());

  if(filter && m_executingTasks.keys().contains(filter))
  {
    const auto &taskContext = m_executingTasks[filter];

    if (!filter->isAborted())
    {
      if (filter->numberOfOutputs() != 1 || !filter->validOutput(0))
      {
        auto what = QObject::tr("Unable to process filter result.");
        auto details = QObject::tr("SliceInterpolationTool::onTaskFinished() -> Invalid output or number of outputs: %1").arg(filter->numberOfOutputs());

        throw EspinaException(what, details);
      }

      auto undoStack = getUndoStack();

      undoStack->beginMacro(taskContext.filter->description());
      undoStack->push(new ReplaceOutputCommand(taskContext.segmentation, getInput(taskContext.filter, 0)));
      undoStack->endMacro();
    }
    else
    {
      if(!filter->errors().isEmpty())
      {
        auto title   = tr("Slice interpolation");
        auto message = tr("Couldn't complete operation due to errors.");
        auto details = filter->errors().first();

        GUI::DefaultDialogs::ErrorMessage(message, title, details);
      }
    }

    markAsBeingModified(taskContext.segmentation, false);

    m_executingTasks.remove(filter);
  }
  else
  {
    auto what    = QObject::tr("Unable to identify signal sender object as filter.");
    auto details = QObject::tr("SliceInterpolationTool::onTaskFinished() -> ") + what;

    throw EspinaException(what, details);
  }
}

//------------------------------------------------------------------------
void SliceInterpolationTool::abortTasks()
{
  for(auto task: m_executingTasks.keys())
  {
    disconnect(task, SIGNAL(finished()),
               this, SLOT(onTaskFinished()));

    if(task && task->isRunning()) task->abort();

    const auto &taskContext = m_executingTasks[task];

    markAsBeingModified(taskContext.segmentation, false);

    if (!task->thread()->wait(100))
    {
      task->thread()->terminate();
    }
  }

  m_executingTasks.clear();
}
