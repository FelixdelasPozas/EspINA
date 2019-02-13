/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <App/Dialogs/SkeletonInspector/SkeletonInspector.h>
#include <App/ToolGroups/Analyze/SkeletonInspector/SkeletonInspectorTool.h>
#include <Core/Analysis/Data/SkeletonData.h>
#include <GUI/Dialogs/DefaultDialogs.h>


using namespace ESPINA;
using namespace ESPINA::GUI;

//--------------------------------------------------------------------
SkeletonInspectorTool::SkeletonInspectorTool(Support::Context& context)
: ProgressTool{"SkeletonInspector", ":/espina/skeletonInspector.svg", tr("This tool requires a skeleton segmentation to be selected."), context}
{
  connectSignals();

  setEnabled(false);
}

//--------------------------------------------------------------------
void SkeletonInspectorTool::onPressed(bool unused)
{
  auto segmentations = getSelection()->segmentations();
  if(segmentations.size() != 1) return;

  if(segmentations.first()->isBeingModified())
  {
    auto title   = tr("Skeleton Inspector");
    auto message = tr("Cannot inspect '%1' because is being edited by another tool.").arg(segmentations.first()->data().toString());
    DefaultDialogs::InformationMessage(message, title);

    return;
  }

  auto dialog = new SkeletonInspector(getContext());
  dialog->setAttribute(Qt::WA_DeleteOnClose, true);
  dialog->exec();
}

//--------------------------------------------------------------------
void SkeletonInspectorTool::onSelectionChanged(SegmentationAdapterList selectedSegs)
{
  auto enabled = (selectedSegs.size() == 1) && hasSkeletonData(selectedSegs.first()->output());

  if(enabled)
  {
    if(selectedSegs.first()->isBeingModified())
    {
      setToolTip(tr("Cannot inspect '%1' because is being edited by another tool.").arg(selectedSegs.first()->data().toString()));
      enabled = false;
    }
    else
    {
      setToolTip(tr("Inspect '%1'").arg(selectedSegs.first()->data().toString()));
    }
  }
  else
  {
    setToolTip(tr("This tool requires a skeleton segmentation to be selected."));
  }

  setEnabled(enabled);
}

//--------------------------------------------------------------------
void SkeletonInspectorTool::connectSignals()
{
  connect(getViewState().selection().get(), SIGNAL(selectionChanged(SegmentationAdapterList)),
          this,                             SLOT(onSelectionChanged(SegmentationAdapterList)));

  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(onPressed(bool)));
}
