/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include "ResetViewTool.h"
#include <Support/Context.h>

// Qt
#include <QAction>

using namespace ESPINA;

//----------------------------------------------------------------------------
ResetViewTool::ResetViewTool(Support::Context &context)
: ProgressTool{"ResetView", ":/espina/reset_view.svg", tr("Reset View"), context}
, m_viewState (context.viewState())
{
  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(resetViews()));
}

//----------------------------------------------------------------------------
ResetViewTool::~ResetViewTool()
{
  disconnect();
}

//----------------------------------------------------------------------------
void ResetViewTool::resetViews()
{
  m_viewState.resetCamera();
}
