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


#include "ViewFrame.h"

#include <QDebug>

#include "SliceView.h"

#include <pqApplicationCore.h>
#include <pqServerManagerObserver.h>

ViewFrame::ViewFrame()
{
  QSharedPointer<SliceView> defaultView =
    QSharedPointer<SliceView>(new SliceView());
  addWidget(defaultView.data());
  pqServerManagerObserver *SMObserver = pqApplicationCore::instance()->getServerManagerObserver();
  connect(SMObserver, SIGNAL(connectionCreated(vtkIdType)),
	  defaultView.data(), SLOT(onConnect()));
  connect(SMObserver, SIGNAL(connectionClosed(vtkIdType)),
	  defaultView.data(), SLOT(onDisconnect()));

  m_views.append(defaultView);
}

//----------------------------------------------------------------------------
ViewFrame::~ViewFrame()
{
  qDebug() << this << ": Frame Destroyed";
}

//----------------------------------------------------------------------------
void ViewFrame::split()
{
//   addWidget(new QPushButton("A"));
//   addWidget(new QPushButton("B"));
}

//----------------------------------------------------------------------------
void ViewFrame::onConnect()
{
  foreach(QSharedPointer<SliceView> view, m_views)
    view->onConnect();
}

//----------------------------------------------------------------------------
void ViewFrame::onDisconnect()
{
  foreach(QSharedPointer<SliceView> view, m_views)
    view->onDisconnect();
}
