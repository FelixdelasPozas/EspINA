/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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


#include "ViewManager.h"

#include <QDebug>

#include "gui/ViewFrame.h"

#include <pqApplicationCore.h>
#include <pqServerManagerObserver.h>

QSharedPointer<ViewManager> ViewManager::m_singleton;

//----------------------------------------------------------------------------
ViewManager::ViewManager()
{
//     m_SMObserver = pqApplicationCore::instance()->getServerManagerObserver();
//     connect(m_SMObserver, SIGNAL(connectionCreated(vtkIdType)),
// 	    this, SLOT(onConnect()));
//     connect(m_SMObserver, SIGNAL(connectionClosed(vtkIdType)),
// 	    this, SLOT(onDisconnect()));
}

//----------------------------------------------------------------------------
ViewManager::~ViewManager()
{
}

//----------------------------------------------------------------------------
QWidget* ViewManager::createLayout(const QString& layout)
{
  ViewFrame *frame = new ViewFrame();
  // Returned frame will belong to another QObject, what happens when its
  // parent is destroyed? does m_frames's point become a dangling pointer?
  m_frames.append(frame);
  return frame;
}

//----------------------------------------------------------------------------
void ViewManager::onConnect()
{
  qDebug() << this << ": Connecting to a new server";
  foreach (ViewFrame *frame, m_frames)
    frame->onConnect();
}

//----------------------------------------------------------------------------
void ViewManager::onDisconnect()
{
  qDebug() << this << ": Disconnecting from current server";
  foreach (ViewFrame *frame, m_frames)
    frame->onDisconnect();
}


