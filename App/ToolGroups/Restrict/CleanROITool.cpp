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

// ESPINA
#include "CleanROITool.h"

#include "RestrictToolGroup.h"
#include <Undo/ROIUndoCommand.h>

// Qt
#include <QDebug>
#include <QAction>

using namespace ESPINA;

//-----------------------------------------------------------------------------
CleanROITool::CleanROITool(Support::Context &context,
                           RestrictToolGroup *toolGroup)
: m_context  {context}
, m_toolGroup{toolGroup}
, m_cleanROI {new QAction(QIcon(":/espina/voi_clean.svg"), tr("Clean Volume Of Interest"), this)}
{
  m_cleanROI->setEnabled(false);

  connect(m_toolGroup, SIGNAL(roiChanged(ROISPtr)),
          this,        SLOT(onROIChanged()));

  connect(m_cleanROI, SIGNAL(triggered(bool)),
          this,       SLOT(cancelROI()));
}

//-----------------------------------------------------------------------------
CleanROITool::~CleanROITool()
{
  disconnect(m_toolGroup, SIGNAL(roiChanged(ROISPtr)),
             this,        SLOT(onROIChanged()));

  disconnect(m_cleanROI, SIGNAL(triggered(bool)),
             this,       SLOT(cancelROI()));

  delete m_cleanROI;
}

//-----------------------------------------------------------------------------
QList<QAction *> CleanROITool::actions() const
{
  QList<QAction *> actions;

  actions << m_cleanROI;

  return actions;
}

//-----------------------------------------------------------------------------
void CleanROITool::abortOperation()
{
}

//-----------------------------------------------------------------------------
void CleanROITool::cancelROI()
{
  auto undoStack = m_context.undoStack();

  undoStack->beginMacro("Clear Region Of Interest");
  undoStack->push(new ClearROIUndoCommand{m_toolGroup});
  undoStack->endMacro();
}

//-----------------------------------------------------------------------------
void CleanROITool::onROIChanged()
{
  m_cleanROI->setEnabled(m_toolGroup->hasValidROI());
}

//-----------------------------------------------------------------------------
void CleanROITool::onToolEnabled(bool enabled)
{
  onROIChanged();
}
