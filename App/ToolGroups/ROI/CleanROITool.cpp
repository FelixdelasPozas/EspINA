/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

// EspINA
#include "CleanROITool.h"
#include "ROITools.h"
#include <Undo/ROIUndoCommand.h>

// Qt
#include <QDebug>
#include <QAction>

using namespace EspINA;

//-----------------------------------------------------------------------------
CleanROITool::CleanROITool(ModelAdapterSPtr  model,
                           ViewManagerSPtr   viewManager,
                           QUndoStack       *undoStack,
                           ROIToolsGroup    *toolGroup)
: m_model      {model}
, m_viewManager{viewManager}
, m_undoStack  {undoStack}
, m_toolGroup  {toolGroup}
, m_cleanROI   {new QAction(QIcon(":/espina/voi_clean.svg"), tr("Clean Volume Of Interest"), this)}
, m_enabled    {true}
{
  connect(m_viewManager.get(), SIGNAL(ROIChanged()),
          this,                SLOT(ROIChanged()));

  connect(m_cleanROI, SIGNAL(triggered(bool)),
          this,       SLOT(cancelROI()));

  ROIChanged();
}

//-----------------------------------------------------------------------------
CleanROITool::~CleanROITool()
{
  disconnect(m_viewManager.get(), SIGNAL(ROIChanged()),
             this,                SLOT(ROIChanged()));

  disconnect(m_cleanROI, SIGNAL(triggered(bool)),
             this,       SLOT(cancelROI()));

  delete m_cleanROI;
}

//-----------------------------------------------------------------------------
void CleanROITool::setEnabled(bool value)
{
  if (m_enabled == value)
    return;

  m_enabled = value;
  ROIChanged();
}

//-----------------------------------------------------------------------------
bool CleanROITool::enabled() const
{
  return m_enabled;
}

//-----------------------------------------------------------------------------
QList<QAction *> CleanROITool::actions() const
{
  QList<QAction *> actions;

  actions << m_cleanROI;

  return actions;
}

//-----------------------------------------------------------------------------
void CleanROITool::cancelROI()
{
  m_undoStack->beginMacro("Clear Region Of Interest");
  m_undoStack->push(new ClearROIUndoCommand{m_toolGroup});
  m_undoStack->endMacro();
}

//-----------------------------------------------------------------------------
void CleanROITool::ROIChanged()
{
  m_cleanROI->setEnabled(m_enabled && (m_viewManager->currentROI() != nullptr));
}
