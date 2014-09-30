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
#include "ActionSelectorWidget.h"

// Qt
#include <QMenu>

//------------------------------------------------------------------------
ActionSelectorWidget::ActionSelectorWidget(QWidget* parent)
: QToolButton{parent}
, m_actions  {new QMenu()}
{
  m_selectedAction = nullptr;
  setCheckable(true);
  setAutoRaise(true);
  connect(this, SIGNAL(triggered(QAction*)), this, SLOT(changeAction(QAction*)));
  connect(this, SIGNAL(toggled(bool)), this, SLOT(triggerAction(bool)));
  connect(this, SIGNAL(clicked(bool)), this, SLOT(triggerAction(bool)));
}

//------------------------------------------------------------------------
void ActionSelectorWidget::addAction(QAction* action)
{
  if (!m_actions->actions().contains(action))
  {
    m_actions->addAction(action);
    changeAction(action);
  }

  if (m_actions->actions().size() != 1)
  {
    setMenu(m_actions);
    this->setToolTip(QString());
  }
  else
    this->setToolTip(action->text());
}

//------------------------------------------------------------------------
void ActionSelectorWidget::cancelAction()
{
  setChecked(false);
}

//------------------------------------------------------------------------
void ActionSelectorWidget::triggerAction(bool trigger)
{
  if (isCheckable())
  {
    if (trigger)
      emit actionTriggered(m_selectedAction);
    else
      emit actionCanceled();
  }
  else
  {
    m_selectedAction->trigger();
  }
}

//------------------------------------------------------------------------
void ActionSelectorWidget::changeAction(QAction* action)
{
  if (isChecked())
  {
    setChecked(false);          // cancels previous action
    setIcon(action->icon());
    m_selectedAction = action;
    setChecked(true);           // starts the new action
  }
  else
  {
    setIcon(action->icon());
    m_selectedAction = action;
  }
}

//------------------------------------------------------------------------
void ActionSelectorWidget::setButtonAction(QAction *action)
{
  if (m_actions->actions().contains(action))
  {
    setIcon(action->icon());
    m_selectedAction = action;
  }
}

//------------------------------------------------------------------------
QAction* ActionSelectorWidget::getButtonAction()
{
  return this->m_selectedAction;
}
