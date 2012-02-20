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


#include "ActionSelector.h"

#include "ActionSelectorWidget.h"

#include <QDebug>

//------------------------------------------------------------------------
ActionSelector::ActionSelector(QObject *parent)
: QWidgetAction(parent)
{
}

//------------------------------------------------------------------------
QWidget* ActionSelector::createWidget(QWidget* parent)
{
//   qDebug() << "Create Segment Widget";
  // Segmentation Button
  ActionSelectorWidget *button = new ActionSelectorWidget(parent);
  button->setIconSize(QSize(22,22));

  foreach(QAction *action, m_actions)
    button->addAction(action);

  connect(button, SIGNAL(actionTriggered(QAction*)),
	  this, SLOT(actionTriggered(QAction*)));
  connect(this, SIGNAL(cancelAction()),
	  button, SLOT(cancelAction()));
  connect(button, SIGNAL(actionCanceled()),
	  this, SLOT(onActionCanceled()));

  return button;
}

//------------------------------------------------------------------------
void ActionSelector::addAction(QAction* action)
{
  m_actions.append(action);
}


//------------------------------------------------------------------------
void ActionSelector::actionTriggered(QAction* action)
{
//   qDebug() << action->text() << "has been triggered";
  emit triggered(action);
}

//------------------------------------------------------------------------
void ActionSelector::onActionCanceled()
{
  emit actionCanceled();
}
