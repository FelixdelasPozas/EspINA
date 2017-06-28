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
#include <Dialogs/ConnectionCount/ConnectionCountDialog.h>
#include <ToolGroups/Analyze/ConnectionCount/ConnectionCountTool.h>

using namespace ESPINA;

//--------------------------------------------------------------------
ConnectionCountTool::ConnectionCountTool(Support::Context& context)
: ProgressTool{"ConnectionCount", ":/espina/connectionTool.svg", tr("Counts and classifies synaptic connections on the session."), context}
, m_dialog{nullptr}
{
  connect(this, SIGNAL(triggered(bool)), this, SLOT(onPressed(bool)));
}

//--------------------------------------------------------------------
void ESPINA::ConnectionCountTool::onPressed(bool value)
{
  if(!m_dialog)
  {
    m_dialog = new ConnectionCountDialog(getContext());
    m_dialog->setAttribute(Qt::WA_DeleteOnClose, true);

    connect(m_dialog, SIGNAL(destroyed(QObject *)), this, SLOT(onDialogClosed(QObject *)));

    m_dialog->show();
  }
}
