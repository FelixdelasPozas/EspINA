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
                                             ModelAdapterSPtr         model,
                                             ModelFactorySPtr         factory,
                                             ViewManagerSPtr          viewManager)
: m_plugin{plugin}
, m_model{model}
, m_factory{factory}
, m_viewManager{viewManager}
, m_action{new QAction(QIcon(":/AppSurface.svg"), tr("Apposition Surface Tools"), this)}
{
  setToolTip("Create a synaptic apposition surface from selected segmentations.");

  connect(m_action, SIGNAL(triggered()),
          this,     SLOT(createSAS()));

  connect(viewManager->selection().get(), SIGNAL(selectionChanged()),
          this,                           SLOT(selectionChanged()));

  selectionChanged();
}

//-----------------------------------------------------------------------------
AppositionSurfaceTool::~AppositionSurfaceTool()
{
}

//-----------------------------------------------------------------------------
QList<QAction *> AppositionSurfaceTool::actions() const
{
  QList<QAction *> actions;

  actions << m_action;

  return actions;
}

//-----------------------------------------------------------------------------
void AppositionSurfaceTool::abortOperation()
{

}

//-----------------------------------------------------------------------------
void AppositionSurfaceTool::selectionChanged()
{
  QString toolTip("Create a synaptic apposition surface from selected segmentations.");

  bool enabled = false;

  for(auto segmentation: m_viewManager->selection()->segmentations())
  {
    if (AppositionSurfacePlugin::isValidSynapse(segmentation))
    {
      enabled = true;
      break;
    }
  }

  if (!enabled)
  {
    toolTip += QString("\n(Requires a selection of one or more segmentations from 'Synapse' taxonomy)");
  }

  setToolTip(toolTip);
  setEnabled(enabled && isEnabled());
}

//-----------------------------------------------------------------------------
void AppositionSurfaceTool::createSAS()
{
  auto segmentations = m_viewManager->selection()->segmentations();

  SegmentationAdapterList validSegmentations;
  for(auto seg: segmentations)
  {
    if (AppositionSurfacePlugin::isValidSynapse(seg))
    {
      // Only create SAS for segmentations which don't already have a SAS
      bool valid = true;
      for (auto item : m_model->relatedItems(seg, RELATION_OUT))
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
    DefaultDialogs::InformationMessage(tr("ESPINA"), tr("Selected Synapses already have an associated Apposittion Surface."));
  }
  else
  {
    for(auto seg : validSegmentations)
    {
      InputSList inputs;
      inputs << seg->asInput();

      auto filter = m_factory->createFilter<AppositionSurfaceFilter>(inputs, AS_FILTER);

      AppositionSurfacePlugin::Data data(filter, m_model->smartPointer(seg));
      m_plugin->m_executingTasks.insert(filter.get(), data);

      connect(filter.get(), SIGNAL(finished()),
              m_plugin,     SLOT(finishedTask()));

      Task::submit(filter);
    }
  }

}


//-----------------------------------------------------------------------------
void AppositionSurfaceTool::onToolEnabled(bool enabled)
{
  m_action->setEnabled(enabled);
}
