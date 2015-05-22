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

#include "Tool.h"
#include "Styles.h"
#include <GUI/Widgets/ProgressAction.h>
#include <QPushButton>
#include <QAction>
#include <QHBoxLayout>

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::Support::Widgets;

//----------------------------------------------------------------------------
Tool::NestedWidgets::NestedWidgets(QObject *parent)
: QWidgetAction(parent)
, m_layout(new QHBoxLayout())
{
  m_layout->setMargin(0);

  auto widget = new QWidget();
  widget->setLayout(m_layout);

  Styles::setNestedStyle(widget);

  setDefaultWidget(widget);
}

//----------------------------------------------------------------------------
void Tool::NestedWidgets::addWidget(QWidget *widget)
{
  m_layout->addWidget(widget);
}

//----------------------------------------------------------------------------
bool Tool::NestedWidgets::isEmpty() const
{
  return m_layout->isEmpty();
}

//----------------------------------------------------------------------------
Tool::Tool()
: m_enabled{true}
{

}

//----------------------------------------------------------------------------
QAction *Tool::createAction(const QString &icon, const QString &tooltip, QObject *parent)
{
  return createAction(QIcon(icon), tooltip, parent);
}

//----------------------------------------------------------------------------
QAction *Tool::createAction(const QIcon &icon, const QString &tooltip, QObject *parent)
{
  auto action = new QAction(parent);

  action->setIcon(icon);
  action->setToolTip(tooltip);

  return action;
}

//----------------------------------------------------------------------------
QPushButton *Tool::createButton(const QString &icon, const QString &tooltip)
{
  return createButton(QIcon(icon), tooltip);
}

//----------------------------------------------------------------------------
QPushButton *Tool::createButton(const QIcon &icon, const QString &tooltip)
{
  auto button = new QPushButton();

  button->setIcon(icon);
  button->setIconSize(QSize(22, 22));
  button->setMaximumSize(30, 30);
  button->setFlat(true);
  button->setToolTip(tooltip);

  return button;
}

//----------------------------------------------------------------------------
void Tool::setEnabled(bool value)
{
  if (m_enabled != value)
  {
    onToolEnabled(value);
  }

  m_enabled = value;
}

//----------------------------------------------------------------------------
bool Tool::isEnabled() const
{
  return m_enabled;
}

//----------------------------------------------------------------------------
ProgressTool::ProgressTool(const QString &icon, const QString &tooltip)
: Tool()
, m_action  {new ProgressAction(icon, tooltip, this)}
, m_settings{new Tool::NestedWidgets(this)}
{
  connect(m_action, SIGNAL(toggled(bool)),
          this,     SLOT(onActionToggled(bool)));

  connect(m_action, SIGNAL(triggered(bool)),
          this,     SIGNAL(triggered(bool)));

  connect(&m_taskProgress, SIGNAL(progress(int)),
          m_action,        SLOT(setProgress(int)));

  m_settings->setVisible(false);
}

//----------------------------------------------------------------------------
ProgressTool::~ProgressTool()
{
  delete m_action;
  delete m_settings;
}

//----------------------------------------------------------------------------
void ProgressTool::setEnabled(bool value)
{
  m_action->setEnabled(value);
  m_settings->setEnabled(value);
}

//----------------------------------------------------------------------------
bool ProgressTool::isEnabled() const
{
  return m_action->isEnabled();
}

//----------------------------------------------------------------------------
void ProgressTool::setCheckable(bool value)
{
  m_action->setCheckable(value);
}

//----------------------------------------------------------------------------
QList<QAction*> ProgressTool::actions() const
{
  QList<QAction *> result;

  result << m_action << m_settings;

  return result;
}

//----------------------------------------------------------------------------
void ProgressTool::setProgress(int value)
{
  m_action->setProgress(value);
}

//----------------------------------------------------------------------------
void ProgressTool::addSettingsWidget(QWidget* widget)
{
  m_settings->addWidget(widget);
}

//----------------------------------------------------------------------------
void ProgressTool::showTaskProgress(TaskSPtr task)
{
  m_taskProgress.showTaskProgress(task);
}

//----------------------------------------------------------------------------
void ProgressTool::onActionToggled(bool value)
{
  m_settings->setVisible(value && !m_settings->isEmpty());

  emit toggled(value);
}
