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
#include "AppositionSurfaceToolGroup.h"
#include "AppositionSurfacePlugin.h"
#include <Filter/AppositionSurfaceFilter.h>

// ESPINA
#include <GUI/Model/Utils/QueryAdapter.h>
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

//-----------------------------------------------------------------------------
AppositionSurfaceToolGroup::AppositionSurfaceToolGroup(ModelAdapterSPtr model,
                                                       QUndoStack      *undoStack,
                                                       ModelFactorySPtr factory,
                                                       ViewManagerSPtr viewManager,
                                                       AppositionSurfacePlugin *plugin)
: ToolGroup(viewManager, QIcon(":/AppSurface.svg"), tr("Apposition Surface Tools"), nullptr)
, m_model    {model}
, m_factory  {factory}
, m_undoStack{undoStack}
, m_tool     {SASToolSPtr{new AppositionSurfaceTool{QIcon(":/AppSurface.svg"), tr("Create a synaptic apposition surface from selected segmentations.")}}}
, m_enabled  {true}
, m_plugin   {plugin}
{
  m_tool->setToolTip("Create a synaptic apposition surface from selected segmentations.");
  connect(m_tool.get(), SIGNAL(triggered()), this, SLOT(createSAS()));

  connect(viewManager->selection().get(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
}

//-----------------------------------------------------------------------------
AppositionSurfaceToolGroup::~AppositionSurfaceToolGroup()
{
  disconnect(m_tool.get(), SIGNAL(triggered()), this, SLOT(createSAS()));
  disconnect(m_viewManager->selection().get(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
}

//-----------------------------------------------------------------------------
void AppositionSurfaceToolGroup::setEnabled(bool enable)
{
  m_enabled = enable;
  selectionChanged();
}

//-----------------------------------------------------------------------------
bool AppositionSurfaceToolGroup::enabled() const
{
  return m_enabled;
}

//-----------------------------------------------------------------------------
ToolSList AppositionSurfaceToolGroup::tools()
{
  ToolSList tools;

  tools << m_tool;

  return tools;
}

//-----------------------------------------------------------------------------
void AppositionSurfaceToolGroup::selectionChanged()
{
  QString toolTip("Create a synaptic apposition surface from selected segmentations.");
  bool enabled = false;

  for(auto segmentation: m_viewManager->selection()->segmentations())
  {
    if (m_plugin->isSynapse(segmentation))
    {
      enabled = true;
      break;
    }
  }

  if (!enabled)
    toolTip += QString("\n(Requires a selection of one or more segmentations from 'Synapse' taxonomy)");

  m_tool->setToolTip(toolTip);
  m_tool->setEnabled(enabled && m_enabled);
}

//-----------------------------------------------------------------------------
void AppositionSurfaceToolGroup::createSAS()
{
  auto segmentations = m_viewManager->selection()->segmentations();
  SegmentationAdapterList validSegmentations;
  for(auto seg: segmentations)
  {
    if (m_plugin->isSynapse(seg))
    {
      bool valid = true;
      for (auto item : m_model->relatedItems(seg, RELATION_OUT))
        if (item->type() == ItemAdapter::Type::SEGMENTATION)
        {
          SegmentationAdapterSPtr sasCandidate = std::dynamic_pointer_cast<SegmentationAdapter>(item);
          if (sasCandidate->category()->classificationName().startsWith("SAS/") ||
              sasCandidate->category()->classificationName().compare("SAS") == 0)
          {
            valid = false;
          }
        }

      if (valid)
        validSegmentations << seg;
    }
  }

  if(validSegmentations.empty())
  {
    QMessageBox::information(nullptr, tr("ESPINA"), tr("Selected Synapses already have an associated Apposittion Surface."));
    return;
  }

  for(auto seg: validSegmentations)
  {
    InputSList inputs;
    inputs << seg->asInput();

    auto adapter = m_factory->createFilter<AppositionSurfaceFilter>(inputs, AS_FILTER);

    struct AppositionSurfacePlugin::Data data(adapter, m_model->smartPointer(seg));
    m_plugin->m_executingTasks.insert(adapter.get(), data);

    connect(adapter.get(), SIGNAL(finished()), m_plugin, SLOT(finishedTask()));
    adapter->submit();
  }
}

//-----------------------------------------------------------------------------
AppositionSurfaceTool::AppositionSurfaceTool(const QIcon& icon, const QString& text)
: m_action {new QAction{icon, text, nullptr}}
{
  connect(m_action, SIGNAL(triggered()), this, SLOT(activated()));
}

//-----------------------------------------------------------------------------
AppositionSurfaceTool::~AppositionSurfaceTool()
{
  disconnect(m_action, SIGNAL(triggered()), this, SLOT(activated()));
}

//-----------------------------------------------------------------------------
QList<QAction *> AppositionSurfaceTool::actions() const
{
  QList<QAction *> actions;
  actions << m_action;

  return actions;
}

