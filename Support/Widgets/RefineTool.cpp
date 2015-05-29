/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include "RefineTool.h"


// Qt
#include <QAction>
#include <QSettings>
#include <QUndoStack>
#include <QApplication>

using namespace ESPINA;
using namespace ESPINA::Support::Widgets;

//------------------------------------------------------------------------
RefineTool::RefineTool(const QString& icon, const QString& tooltip, Support::Context& context)
: ProgressTool(icon, tooltip, context)
{
  connect(getSelection().get(), SIGNAL(selectionChanged()),
          this,                 SLOT(updateStatus()));
}

//------------------------------------------------------------------------
RefineTool::~RefineTool()
{
}

//------------------------------------------------------------------------
bool RefineTool::acceptsVolumetricSegmenations(SegmentationAdapterList segmentations)
{
  bool hasRequiredData = true;

  for(auto segmentation : segmentations)
  {
    hasRequiredData &= hasVolumetricData(segmentation->output());
  }

  return hasRequiredData;
}

//------------------------------------------------------------------------
void RefineTool::updateStatus()
{
  auto selection = getSelectedSegmentations();

  setEnabled(acceptsNInputs(selection.size()) && acceptsSelection(selection));
}

//------------------------------------------------------------------------
void RefineTool::onToolEnabled(bool enabled)
{
  updateStatus();
}

//------------------------------------------------------------------------
bool RefineTool::acceptsSelection(SegmentationAdapterList segmentations)
{
  return true;
}