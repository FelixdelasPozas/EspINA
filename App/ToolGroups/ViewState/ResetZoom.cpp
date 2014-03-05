/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

// EspINA
#include "ResetZoom.h"

// Qt
#include <QAction>

namespace EspINA
{
  //----------------------------------------------------------------------------
  ResetZoom::ResetZoom(ViewManagerSPtr vm)
  : m_viewManager{vm}
  , m_action{new QAction(QIcon(":/espina/zoom_reset.png"),tr("Reset Zoom"),this)}
  , m_enabled{true}
  {
    connect(m_action, SIGNAL(triggered(bool)), this, SLOT(resetViews()), Qt::QueuedConnection);
  }

  //----------------------------------------------------------------------------
  ResetZoom::~ResetZoom()
  {
    if (m_action)
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
  bool ResetZoom::enabled() const
  {
    return m_enabled;
  }

  //----------------------------------------------------------------------------
  void ResetZoom::setEnabled(bool value)
  {
    if (m_enabled == value)
      return;

    m_enabled = value;
    m_action->setEnabled(m_enabled);

    if (!m_enabled)
      disconnect(m_action, SIGNAL(triggered(bool)), this, SLOT(resetViews()));
    else
      connect(m_action, SIGNAL(triggered(bool)), this, SLOT(resetViews()), Qt::QueuedConnection);

  }

  //----------------------------------------------------------------------------
  void ResetZoom::resetViews()
  {
    m_viewManager->resetViewCameras();
    m_viewManager->updateViews();
  }
}

