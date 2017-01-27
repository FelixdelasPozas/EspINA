/*
* Copyright (C) 2016, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
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
#include "FillHoles2DTool.h"
#include "EditToolGroup.h"
#include <Core/Utils/Spatial.h>
#include <Filters/FillHoles2DFilter.h>
#include <GUI/Widgets/Styles.h>
#include <GUI/Widgets/ToolButton.h>
#include <Undo/ReplaceOutputCommand.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets::Styles;

//------------------------------------------------------------------------
FillHoles2DTool::FillHoles2DTool(Support::Context &context)
: EditTool("FillHoles2D", ":/espina/fill_holes_2D.svg", tr("Fill internal holes of each 2D slice individually in an axis direction."),context)
{
  setCheckable(true);

  initOptionWidgets();
}

//------------------------------------------------------------------------
ESPINA::FillHoles2DTool::~FillHoles2DTool()
{
	delete m_directionLabel;
	delete m_directionComboBox;
	delete m_applyButton;
}

//------------------------------------------------------------------------
void FillHoles2DTool::abortOperation()
{
  for(auto task: m_executingTasks.keys())
  {
    task->abort();
  }
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
void FillHoles2DTool::applyFilter()
{
  auto segmentations = getSelectedSegmentations();
  auto direction     = toAxis(m_directionComboBox->currentIndex());

  for (auto segmentation : segmentations)
  {
    InputSList inputs;

    inputs << segmentation->asInput();

    auto filter = getFactory()->createFilter<FillHoles2DFilter>(inputs, MorphologicalFilterFactory::FILL_HOLES2D_FILTER);

    filter->setDescription(tr("Fill %1 Holes in %2 direction").arg(segmentation->data().toString()).arg(toText(direction)));
    filter->setDirection(direction);

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
void FillHoles2DTool::initOptionWidgets()
{
	m_directionLabel = new QLabel();
	m_directionLabel->setText(tr("Orthogonal Direction"));

	m_directionComboBox = new QComboBox();
	m_directionComboBox->addItem("X");
	m_directionComboBox->addItem("Y");
	m_directionComboBox->addItem("Z");
	m_directionComboBox->setCurrentIndex(2);

	m_applyButton = GUI::Widgets::Styles::createToolButton(":/espina/apply.svg", tr("Apply current state"));

	addSettingsWidget(m_directionLabel);
	addSettingsWidget(m_directionComboBox);
	addSettingsWidget(m_applyButton);

	connect(m_applyButton, SIGNAL(clicked(bool)),
			    this,          SLOT(applyFilter()));
}
//------------------------------------------------------------------------
void FillHoles2DTool::onTaskFinished()
{
  auto filter = dynamic_cast<FillHoles2DFilterPtr>(sender());

  if(filter && m_executingTasks.keys().contains(filter))
  {
    auto taskContext = m_executingTasks[filter];

    if (!filter->isAborted() && filter->validOutput(0))
    {
      if (filter->numberOfOutputs() != 1)
      {
        auto what = QObject::tr("Unable to process filter result.");
        auto details = QObject::tr("FillHoles2DTool::onTaskFinished() -> Invalid number of outputs: %1").arg(filter->numberOfOutputs());
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
    auto details = QObject::tr("FillHoles2DTool::onTaskFinished() -> ") + what;
    throw EspinaException(what, details);
  }
}
