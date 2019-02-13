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
#include <Core/Analysis/Data/SkeletonData.h>
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
}

//------------------------------------------------------------------------
const bool EditTool::acceptsVolumetricSegmentations(const SegmentationAdapterList &segmentations) const
{
  auto operation = [](const SegmentationAdapterPtr segmentation) { return hasVolumetricData(segmentation->output()); };
  auto valid = std::all_of(segmentations.constBegin(), segmentations.constEnd(), operation);

  return valid;
}

//------------------------------------------------------------------------
const bool EditTool::acceptsSkeletonSegmentations(const SegmentationAdapterList &segmentations) const
{
  auto operation = [](const SegmentationAdapterPtr segmentation) { return hasSkeletonData(segmentation->output()); };
  auto valid = std::all_of(segmentations.constBegin(), segmentations.constEnd(), operation);

  return valid;
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
  for(auto segmentation: segmentations)
  {
    if(segmentation->isBeingModified()) return false;
  }

  return true;
}
