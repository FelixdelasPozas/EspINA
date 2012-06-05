/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "ActionSelectorWidget.h"

#include <QMenu>

#include <QDebug>

ActionSelectorWidget::ActionSelectorWidget(QWidget* parent)
: QToolButton(parent)
, m_actions(new QMenu())
{
  setCheckable(true);
  setAutoRaise(true);
  setMenu(m_actions);
  connect(this, SIGNAL(triggered(QAction*)),
	  this, SLOT(changeAction(QAction*)));
  connect(this, SIGNAL(toggled(bool)),
	  this, SLOT(triggerAction(bool)));
}

void ActionSelectorWidget::addAction(QAction* action)
{
  m_actions->addAction(action);
  changeAction(action);
}

void ActionSelectorWidget::cancelAction()
{
  setChecked(false);
}

void ActionSelectorWidget::triggerAction(bool trigger)
{
  if (trigger)
    emit actionTriggered(m_selectedAction);
  else
    emit actionCanceled();
}


void ActionSelectorWidget::changeAction(QAction* action)
{
  setIcon(action->icon());
  m_selectedAction = action;
  if (isChecked())
    emit actionTriggered(action);
}
