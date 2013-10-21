/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
 *
 *    This program is free software: you can redistribute it and/or modify
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
#include "AppositionSurfaceToolbar.h"

#include "Analysis/SynapticAppositionSurfaceAnalysis.h"
#include "GUI/Settings/AppositionSurfaceSettings.h"
#include "GUI/FilterInspector/AppositionSurfaceFilterInspector.h"
#include "GUI/AppositionSurfaceAction.h"
#include "Undo/AppositionSurfaceCommand.h"

// EspINA
#include <Core/Extensions/SegmentationExtension.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/EspinaModel.h>
#include <Core/EspinaSettings.h>
#include <Undo/TaxonomiesCommand.h>
#include <GUI/Representations/BasicGraphicalRepresentationFactory.h>

// Qt
#include <QColorDialog>
#include <QMessageBox>
#include <QSettings>
#include <QDebug>
#include <QString>
#include <QVariant>

using namespace EspINA;

//-----------------------------------------------------------------------------
AppositionSurfaceToolbar::AppositionSurfaceToolbar(ModelAdapter *model,
                                                   QUndoStack  *undoStack,
                                                   ViewManager *viewManager)
: m_model(model)
, m_undoStack(undoStack)
, m_viewManager(viewManager)
, m_action(new AppositionSurfaceAction(m_viewManager, m_undoStack, m_model, this))
, m_settings(ISettingsPanelPrototype(new AppositionSurfaceSettings()))
, m_extension(new AppositionSurfaceExtension())
{
  setObjectName("SinapticAppositionSurfaceToolbarPlugin");
  setWindowTitle(tr("Sinaptic Apposition Surface Tool Bar"));

  m_action->setToolTip("Create a synaptic apposition surface from selected segmentations.");
  addAction(m_action);
}

//-----------------------------------------------------------------------------
AppositionSurfaceToolbar::~AppositionSurfaceToolbar()
{
  delete m_action;
}

//-----------------------------------------------------------------------------
void AppositionSurfaceToolbar::selectionChanged(ViewManager::Selection selection, bool unused)
{
  QString toolTip("Create a synaptic apposition surface from selected segmentations.");
  bool enabled = false;

  foreach(PickableItemPtr item, selection)
  {
    if (item->type() == SEGMENTATION && isSynapse(segmentationPtr(item)))
    {
      enabled = true;
      break;
    }
  }

  if (!enabled)
    toolTip += QString("\n(Requires a selection of one or more segmentations from 'Synapse' taxonomy)");

  m_action->setToolTip(toolTip);
  m_action->setEnabled(enabled);
}

//-----------------------------------------------------------------------------
bool AppositionSurfaceToolbar::isSynapse(SegmentationPtr segmentation)
{
  return segmentation->taxonomy()->qualifiedName().contains(tr("Synapse"));
}