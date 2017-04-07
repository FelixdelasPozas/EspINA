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
#include "DeleteROITool.h"

#include "RestrictToolGroup.h"
#include <Undo/ROIUndoCommand.h>

// Qt
#include <QAction>

using namespace ESPINA;

//-----------------------------------------------------------------------------
DeleteROITool::DeleteROITool(Support::Context &context,
                           RestrictToolGroup *toolGroup)
: ProgressTool{"DeleteROI", ":/espina/roi_delete_roi.svg", tr("Delete Current ROI"), context}
, m_context   (context)
, m_toolGroup {toolGroup}
{
  connect(m_toolGroup, SIGNAL(ROIChanged(ROISPtr)),
          this,        SLOT(onROIChanged()));

  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(cancelROI()));
}

//-----------------------------------------------------------------------------
DeleteROITool::~DeleteROITool()
{
  disconnect();
}

//-----------------------------------------------------------------------------
void DeleteROITool::cancelROI()
{
  auto undoStack = m_context.undoStack();

  undoStack->beginMacro("Delete ROI");
  undoStack->push(new ClearROIUndoCommand{m_toolGroup});
  undoStack->endMacro();
}

//-----------------------------------------------------------------------------
void DeleteROITool::onROIChanged()
{
  setEnabled(m_toolGroup->hasValidROI());
}
