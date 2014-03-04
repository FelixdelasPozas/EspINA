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

#include "GUI/Widgets/ActionSelector.h"
#include "GUI/Widgets/ActionSelectorWidget.h"

#include <QDebug>

//------------------------------------------------------------------------
ActionSelector::ActionSelector(QObject *parent)
: QWidgetAction(parent)
, m_enabled(true)
, m_checked(false)
{
  m_button = nullptr;
  m_defaultAction = -1;
}

//------------------------------------------------------------------------
QWidget* ActionSelector::createWidget(QWidget* parent)
{
  m_button = new ActionSelectorWidget(parent);
  m_button->setIconSize(QSize(22,22));
  m_button->setCheckable(true);
  m_button->setEnabled(m_enabled);
  m_button->setChecked(m_checked);

  connect(m_button, SIGNAL(destroyed(QObject*)),
          this,     SLOT(destroySignalEmmited()));

  for(auto action: m_actions)
    m_button->addAction(action);

  if (m_defaultAction != -1)
    m_button->setButtonAction(m_actions.at(m_defaultAction));

  connect(m_button, SIGNAL(actionTriggered(QAction*)),
          this,     SLOT(actionTriggered(QAction*)));

  connect(m_button, SIGNAL(actionCanceled()),
          this,     SLOT(onActionCanceled()));

  connect(this,     SIGNAL(cancelAction()),
          m_button, SLOT(cancelAction()));

  return m_button;
}

//------------------------------------------------------------------------
void ActionSelector::addAction(QAction* action)
{
  m_actions.append(action);

  if (nullptr != m_button)
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
  {
    if (m_button)
    {
      m_button->setButtonAction(action);
    }

    m_defaultAction = m_actions.indexOf(action);
  }
}

//------------------------------------------------------------------------
bool ActionSelector::isChecked()
{
  return m_checked;
}

//------------------------------------------------------------------------
void ActionSelector::setChecked(bool value)
{
  m_checked = value;

  if (m_button)
  {
    m_button->setChecked(value);
  }
}

//------------------------------------------------------------------------
QAction* ActionSelector::getCurrentAction()
{
  return m_button?m_button->getButtonAction():nullptr;
}

//------------------------------------------------------------------------
QString ActionSelector::getCurrentActionAsQString()
{
  return m_button?m_button->getButtonAction()->text():QString();
}

//------------------------------------------------------------------------
void ActionSelector::setIcon(const QIcon &icon)
{
  if (m_button)
  {
    m_button->setIcon(icon);
  }
}

//------------------------------------------------------------------------
void ActionSelector::setEnabled(bool value)
{
  m_enabled = value;

  if (m_button)
  {
    m_button->setEnabled(value);
  }
}

//------------------------------------------------------------------------
bool ActionSelector::isEnabled() const
{
  return m_enabled;
}

//------------------------------------------------------------------------
void ActionSelector::destroySignalEmmited()
{
  m_button = nullptr;
}
