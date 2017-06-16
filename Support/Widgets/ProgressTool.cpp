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

#include "ProgressTool.h"
#include <Support/Context.h>
#include <GUI/Widgets/ProgressAction.h>
#include <GUI/Widgets/Styles.h>
#include <QPushButton>
#include <QAction>
#include <QHBoxLayout>
#include <QGraphicsWidget>

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::Support;
using namespace ESPINA::Support::Widgets;

const QString TOOL_ENABLED_KEY = "Checked";

//----------------------------------------------------------------------------
ProgressTool::NestedWidgets::NestedWidgets(QObject *parent)
: QWidgetAction(parent)
, m_layout(new QHBoxLayout())
{
  m_layout->setContentsMargins(4, 0, 4, 0);

  auto widget = new QWidget();
  widget->setLayout(m_layout);

  Styles::setNestedStyle(widget);

  setDefaultWidget(widget);
}

//----------------------------------------------------------------------------
ProgressTool::NestedWidgets::~NestedWidgets()
{
  for(int i = m_layout->count() - 1; i > 0; --i)
  {
    auto widget = m_layout->itemAt(i)->widget();
    m_layout->removeWidget(widget);
    delete widget;
  }
}

//----------------------------------------------------------------------------
void ProgressTool::NestedWidgets::addWidget(QWidget *widget)
{
  m_layout->addWidget(widget);
}

//----------------------------------------------------------------------------
bool ProgressTool::NestedWidgets::isEmpty() const
{
  return m_layout->isEmpty();
}

//----------------------------------------------------------------------------
ProgressTool::ProgressTool(const QString &id, const QString &icon, const QString &tooltip, Context &context)
: ProgressTool{id, QIcon(icon), tooltip, context}
{
}

//----------------------------------------------------------------------------
ProgressTool::ProgressTool(const QString &id, const QIcon &icon, const QString &tooltip, Context &context)
: WithContext(context)
, m_action    {new ProgressAction(icon, tooltip, this)}
, m_settings  {new ProgressTool::NestedWidgets(this)}
, m_isExlusive{false}
, m_groupName {""}
, m_id        {id}
{
  connect(m_action, SIGNAL(toggled(bool)),
          this,     SLOT(onActionToggled(bool)));

  connect(m_action, SIGNAL(triggered(bool)),
          this,     SIGNAL(triggered(bool)));

  connect(&m_taskProgress, SIGNAL(progress(int)),
          m_action,        SLOT(setProgress(int)));

  auto selection = getSelection().get();

  connect(selection, SIGNAL(selectionStateChanged()),
          this,      SLOT(updateStatus()));

  m_settings->setVisible(false);
}

//----------------------------------------------------------------------------
ProgressTool::~ProgressTool()
{
  m_action->disconnect();
  m_taskProgress.disconnect();

  abortOperation();

  delete m_action;
  delete m_settings;
}

//----------------------------------------------------------------------------
void ProgressTool::setEnabled(bool value)
{
  m_action->setActionEnabled(value);
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
bool ProgressTool::isChecked() const
{
  return m_action->isChecked();
}

//----------------------------------------------------------------------------
void ProgressTool::setChecked(bool value)
{
  if(isChecked() != value)
  {
    m_action->setActionChecked(value);
  }
}

//----------------------------------------------------------------------------
void ProgressTool::setExclusive(bool value)
{
  m_isExlusive = value;
}

//----------------------------------------------------------------------------
void ProgressTool::setOrder(const QString& name, const QString& group)
{
  m_positionName = name;
  m_groupName    = group;
}

//----------------------------------------------------------------------------
QString ProgressTool::groupWith() const
{
  return m_groupName;
}

//----------------------------------------------------------------------------
QString ProgressTool::positionName() const
{
  return m_positionName + "-" + id();
}

//----------------------------------------------------------------------------
void ProgressTool::setToolTip(const QString &tooltip)
{
  m_action->setActionToolTip(tooltip);
}

//----------------------------------------------------------------------------
QList<QAction*> ProgressTool::actions() const
{
  QList<QAction *> result;

  result << m_action;

  if(!m_settings->isEmpty())
  {
    result << m_settings;
  }

  return result;
}

//----------------------------------------------------------------------------
void ProgressTool::onExclusiveToolInUse(ProgressTool* tool)
{
  if (this != tool && m_isExlusive && isChecked())
  {
    setChecked(false);
  }
}

//----------------------------------------------------------------------------
void ProgressTool::trigger()
{
  if(m_action->isCheckable())
  {
    setChecked(!isChecked());
  }
  else
  {
    m_action->trigger();
  }
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
void ProgressTool::setEventHandler(EventHandlerSPtr handler)
{
  bool wasInUse = false;

  if (m_handler)
  {
    wasInUse = m_handler->isInUse();

    disconnect(m_handler.get(), SIGNAL(eventHandlerInUse(bool)),
               this,            SLOT(onEventHandlerInUse(bool)));

    if (wasInUse)
    {
      deactivateEventHandler();
    }
  }

  m_handler = handler;

  if (m_handler)
  {
    if (wasInUse)
    {
      activateEventHandler();
    }

    connect(m_handler.get(), SIGNAL(eventHandlerInUse(bool)),
            this,            SLOT(onEventHandlerInUse(bool)));
  }
}

//----------------------------------------------------------------------------
void ProgressTool::activateEventHandler()
{
  getViewState().setEventHandler(m_handler);
}

//----------------------------------------------------------------------------
void ProgressTool::deactivateEventHandler()
{
  getViewState().unsetEventHandler(m_handler);
}

//----------------------------------------------------------------------------
void ProgressTool::onActionToggled(bool value)
{
  m_settings->setVisible(value && !m_settings->isEmpty());

  if (m_handler)
  {
    if (value)
    {
      activateEventHandler();
    }
    else
    {
      deactivateEventHandler();
    }
  }

  if (value && m_isExlusive)
  {
    emit exclusiveToolInUse(this);
  }

  emit toggled(value);
}

//----------------------------------------------------------------------------
void ProgressTool::onEventHandlerInUse(bool isUsed)
{
  m_action->setActionChecked(isUsed);
}

//----------------------------------------------------------------------------
const QString &ProgressTool::id() const
{
  return m_id;
}

//----------------------------------------------------------------------------
void ProgressTool::setShortcut(QKeySequence keySequence)
{
  if(!m_shortcutSequences.contains(keySequence))
  {
    m_shortcutSequences << keySequence;
  }
}

//----------------------------------------------------------------------------
QList<QKeySequence> ProgressTool::shortcuts() const
{
  return m_shortcutSequences;
}

//----------------------------------------------------------------------------
void ProgressTool::saveCheckedState(std::shared_ptr<QSettings> settings)
{
  settings->setValue(TOOL_ENABLED_KEY, isChecked());
}

//----------------------------------------------------------------------------
void ProgressTool::restoreCheckedState(std::shared_ptr<QSettings> settings)
{
  auto checked = settings->value(TOOL_ENABLED_KEY, false).toBool();

  if(checked != isChecked())
  {
    setChecked(checked);
  }
}

//----------------------------------------------------------------------------
void ProgressTool::setIcon(const QIcon &icon)
{
  m_action->setActionIcon(icon);
}

//----------------------------------------------------------------------------
QIcon ProgressTool::icon() const
{
  return m_action->icon();
}
