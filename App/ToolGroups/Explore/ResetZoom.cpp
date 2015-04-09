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
#include "ResetZoom.h"

// Qt
#include <QAction>

using namespace ESPINA;

//----------------------------------------------------------------------------
ResetZoom::ResetZoom(GUI::View::ViewState &state)
: m_viewState{state}
, m_action   {new QAction(QIcon(":/espina/zoom_reset.png"),tr("Reset Zoom"),this)}
{
  connect(m_action, SIGNAL(triggered(bool)),
          this,     SLOT(resetViews()));
}

//----------------------------------------------------------------------------
ResetZoom::~ResetZoom()
{
  delete m_action;
}

//----------------------------------------------------------------------------
QList<QAction *> ResetZoom::actions() const
{
  QList<QAction *> actions;

  actions << m_action;

  return actions;
}

//----------------------------------------------------------------------------
void ResetZoom::abortOperation()
{
}

//----------------------------------------------------------------------------
void ResetZoom::resetViews()
{
  m_viewState.resetCamera();
  m_viewState.refresh();
}

//----------------------------------------------------------------------------
void ResetZoom::onToolEnabled(bool enabled)
{
  m_action->setEnabled(enabled);

  if (enabled)
  {
    connect(m_action, SIGNAL(triggered(bool)),
            this,     SLOT(resetViews()));
  }
  else
  {
    disconnect(m_action, SIGNAL(triggered(bool)),
               this,     SLOT(resetViews()));
  }
}