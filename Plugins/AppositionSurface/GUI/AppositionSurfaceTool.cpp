/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// plugin
#include "AppositionSurfaceTool.h"

#include "AppositionSurfacePlugin.h"
#include <Filter/AppositionSurfaceFilter.h>

// ESPINA
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <Undo/AddCategoryCommand.h>
#include <Undo/AddRelationCommand.h>
#include <Undo/AddSegmentations.h>

// Qt
#include <QApplication>
#include <QUndoStack>
#include <QIcon>
#include <QString>
#include <QVariant>
#include <QDebug>
#include <QMessageBox>

using namespace ESPINA;
using namespace ESPINA::GUI;

//-----------------------------------------------------------------------------
AppositionSurfaceTool::AppositionSurfaceTool(AppositionSurfacePlugin *plugin,
                                             Support::Context        &context)
: ProgressTool("SynapticAppositionSurface", ":/AppSurface.svg", defaultTooltip(), context)
, m_plugin (plugin)
{
  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(createSAS()));

  connect(getSelection().get(), SIGNAL(selectionChanged()),
          this,                 SLOT(selectionChanged()));

  selectionChanged();
}

//-----------------------------------------------------------------------------
AppositionSurfaceTool::~AppositionSurfaceTool()
{
}

//-----------------------------------------------------------------------------
void AppositionSurfaceTool::selectionChanged()
{
  auto toolTip = defaultTooltip();

  bool enabled = false;

  for(auto segmentation: getSelectedSegmentations())
  {
    if (AppositionSurfacePlugin::isValidSynapse(segmentation))
    {
      enabled = true;
      break;
    }
  }

  if (!enabled)
  {
    toolTip += tr("\n(Requires at least one 'Synapse' selected)");
  }

  setToolTip(toolTip);
  setEnabled(enabled);
}

//-----------------------------------------------------------------------------
void AppositionSurfaceTool::createSAS()
{
  auto model         = getModel();
  auto segmentations = getSelectedSegmentations();

  SegmentationAdapterList validSegmentations;
  for(auto seg: segmentations)
  {
    if (AppositionSurfacePlugin::isValidSynapse(seg))
    {
      // Only create SAS for segmentations which don't already have a SAS
      bool valid = true;
      for (auto item : model->relatedItems(seg, RELATION_OUT))
      {
        if (AppositionSurfacePlugin::isSAS(item))
        {
          valid = false;
        }
      }

      if (valid)
      {
        validSegmentations << seg;
      }
    }
  }

  if(validSegmentations.empty())
  {
    DefaultDialogs::InformationMessage(tr("Selected Synapses already have an associated Apposition Surface."));
  }
  else
  {
    for(auto seg : validSegmentations)
    {
      InputSList inputs;
      inputs << seg->asInput();

      auto filter = getFactory()->createFilter<AppositionSurfaceFilter>(inputs, AS_FILTER);

      AppositionSurfacePlugin::Data data(filter, model->smartPointer(seg));
      m_plugin->m_executingTasks.insert(filter.get(), data);

      showTaskProgress(filter);

      connect(filter.get(), SIGNAL(finished()),
              m_plugin,     SLOT(finishedTask()));

      Task::submit(filter);
    }
  }
}

//-----------------------------------------------------------------------------
QString AppositionSurfaceTool::defaultTooltip() const
{
  return tr("Synaptic Apposition Surface");
}
