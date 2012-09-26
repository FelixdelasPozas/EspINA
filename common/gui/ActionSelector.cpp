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


#include "common/gui/ActionSelector.h"
#include "common/gui/ActionSelectorWidget.h"

#include <QDebug>

//------------------------------------------------------------------------
ActionSelector::ActionSelector(QObject *parent)
: QWidgetAction(parent)
{
  m_button = NULL;
}

//------------------------------------------------------------------------
QWidget* ActionSelector::createWidget(QWidget* parent)
{
  m_button = new ActionSelectorWidget(parent);
  m_button->setIconSize(QSize(22,22));

  foreach(QAction *action, m_actions)
    m_button->addAction(action);

  connect(m_button, SIGNAL(actionTriggered(QAction*)), this, SLOT(actionTriggered(QAction*)));
  connect(m_button, SIGNAL(actionCanceled()), this, SLOT(onActionCanceled()));
  connect(this, SIGNAL(cancelAction()), m_button, SLOT(cancelAction()));

  return m_button;
}

//------------------------------------------------------------------------
void ActionSelector::addAction(QAction* action)
{
  m_actions.append(action);

  if (NULL != m_button)
    m_button->addAction(action);
}

//------------------------------------------------------------------------
void ActionSelector::actionTriggered(QAction* action)
{
  emit triggered(action);
}

//------------------------------------------------------------------------
void ActionSelector::onActionCanceled()
{
  emit actionCanceled();
}

//------------------------------------------------------------------------
void ActionSelector::setDefaultAction(QAction *action)
{
  if (m_actions.contains(action))
    m_button->setButtonAction(action);
}

//------------------------------------------------------------------------
bool ActionSelector::isChecked()
{
  return m_button->isChecked();
}

//------------------------------------------------------------------------
QAction* ActionSelector::getCurrentAction()
{
  return this->m_button->getButtonAction();
}

//------------------------------------------------------------------------
QString ActionSelector::getCurrentActionAsQString()
{
  return this->m_button->getButtonAction()->text();
}

void ActionSelector::setIcon(const QIcon &icon)
{
  this->m_button->setIcon(icon);
}
