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
#include "EditTool.h"


// Qt
#include <QAction>
#include <QSettings>
#include <QUndoStack>
#include <QApplication>

using namespace ESPINA;
using namespace ESPINA::Support::Widgets;

//------------------------------------------------------------------------
EditTool::EditTool(const QString &id, const QString& icon, const QString& tooltip, Support::Context& context)
: ProgressTool(id, icon, tooltip, context)
{
  auto selection = getSelection().get();

//   connect(selection, SIGNAL(selectionChanged()),
//           this,      SLOT(updateStatus()));

  connect(selection, SIGNAL(selectionStateChanged()),
          this,      SLOT(updateStatus()));
}

//------------------------------------------------------------------------
EditTool::~EditTool()
{
}

//------------------------------------------------------------------------
bool EditTool::acceptsVolumetricSegmentations(SegmentationAdapterList segmentations)
{
  bool hasRequiredData = true;

  for(auto segmentation : segmentations)
  {
    hasRequiredData &= hasVolumetricData(segmentation->output());
  }

  return hasRequiredData;
}

//------------------------------------------------------------------------
void EditTool::markAsBeingModified(SegmentationAdapterPtr segmentation, bool value)
{
  segmentation->setBeingModified(value);
  getSelection()->modified();
}

//------------------------------------------------------------------------
void EditTool::updateStatus()
{
  auto selection = getSelectedSegmentations();

  setEnabled(acceptsNInputs(selection.size())
          && selectionIsNotBeingModified(selection)
          && acceptsSelection(selection));
}

//------------------------------------------------------------------------
void EditTool::onToolGroupActivated()
{
  updateStatus();
}

//------------------------------------------------------------------------
bool EditTool::acceptsSelection(SegmentationAdapterList segmentations)
{
  return true;
}

//------------------------------------------------------------------------
bool EditTool::selectionIsNotBeingModified(SegmentationAdapterList segmentations)
{
  bool beingModified = false;

  int i = 0;

  while (!beingModified && i < segmentations.size())
  {
    beingModified = segmentations[i]->isBeingModified();
    ++i;
  }

  return !beingModified;
}
