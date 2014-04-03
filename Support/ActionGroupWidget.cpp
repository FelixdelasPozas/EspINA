/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
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

#include "ActionGroupWidget.h"
#include <QHBoxLayout>
#include <QPushButton>

using namespace EspINA;

//------------------------------------------------------------------------
ActionGroupWidget::ActionGroupWidget(QObject* parent)
: QWidgetAction(parent)
, m_widget(nullptr)
{

}


//------------------------------------------------------------------------
QWidget* ActionGroupWidget::createWidget(QWidget* parent)
{
  m_widget = new QWidget(parent);
  m_widget->setLayout(new QHBoxLayout());

  for(auto action : m_actions)
  {
    addActionButton(action);
  }

  return m_widget;
}

//------------------------------------------------------------------------
void ActionGroupWidget::add(QAction* action)
{
  m_actions << action;

  addActionButton(action);
}

//------------------------------------------------------------------------
void ActionGroupWidget::remove(QAction* action)
{
  m_actions.removeOne(action);
}

//------------------------------------------------------------------------
void ActionGroupWidget::addActionButton(QAction* action)
{
  if (m_widget)
  {
    auto button = new QPushButton(m_widget);
    button->setIcon(action->icon());
    button->setMaximumSize(32, 32);
    button->setFlat(true);
    connect(button, SIGNAL(clicked(bool)),
            action, SLOT(trigger()));
    m_widget->layout()->addWidget(button);
  }
}
