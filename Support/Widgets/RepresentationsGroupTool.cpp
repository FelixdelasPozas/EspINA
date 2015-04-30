/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#include "RepresentationsGroupTool.h"
#include "Styles.h"

#include <QAction>
#include <QWidgetAction>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>

#include <QDebug>
#include <QEvent>

using namespace ESPINA;
using namespace ESPINA::Support::Widgets;

//----------------------------------------------------------------------------
RepresentationsGroupTool::RepresentationsGroupTool(const QString &icon, const QString &description, Timer &timer)
: RepresentationsGroupTool{QIcon(icon), description, timer}
{

}
//----------------------------------------------------------------------------
RepresentationsGroupTool::RepresentationsGroupTool(const QIcon &icon, const QString &description, Timer &timer)
: m_timer(timer)
, m_globalSwitch{new QAction(icon, description, this)}
, m_content{new QWidgetAction(this)}
, m_contentWidget{new QWidget()}
, m_layout2D{new QHBoxLayout()}
, m_layout3D{new QHBoxLayout()}
, m_representationsVisibility{false}
{
  m_globalSwitch->setCheckable(true);

  connect(m_globalSwitch, SIGNAL(toggled(bool)),
          this,           SLOT(setActiveRepresentationsVisibility(bool)));

  m_separator2D = createSeparator(":/espina/representation_group_2D_separator.svg");
  m_separator3D = createSeparator(":/espina/representation_group_3D_separator.svg");

  m_layout2D->addWidget(m_separator2D);
  m_layout3D->addWidget(m_separator3D);

  auto layout = new QHBoxLayout();
  layout->addLayout(m_layout2D);
  layout->addLayout(m_layout3D);
  layout->addWidget(createSeparator(""));

  m_contentWidget->setLayout(layout);
  Styles::setNestedStyle(m_contentWidget);

  m_content->setDefaultWidget(m_contentWidget);
  m_content->setVisible(m_globalSwitch->isChecked());
}

//----------------------------------------------------------------------------
QList<QAction *> RepresentationsGroupTool::actions() const
{
  QList<QAction *> renderGroupActions;

  renderGroupActions << m_globalSwitch;
  renderGroupActions << m_content;

  return renderGroupActions;
}

//----------------------------------------------------------------------------
void RepresentationsGroupTool::abortOperation()
{
}

//----------------------------------------------------------------------------
void RepresentationsGroupTool::showActiveRepresentations()
{
  m_globalSwitch->setChecked(true);
}

//----------------------------------------------------------------------------
void RepresentationsGroupTool::hideActiveRepresentations()
{
  m_globalSwitch->setChecked(false);
}

//----------------------------------------------------------------------------
void RepresentationsGroupTool::addRepresentationSwitch(RepresentationSwitchSPtr representationSwitch)
{
  auto widget = representationSwitch->widget();

  if (representationSwitch->supportedViews().testFlag(ESPINA::VIEW_2D))
  {
    m_layout2D->addWidget(widget);
    m_viewFlags |= ViewType::VIEW_2D;
  }
  else if (representationSwitch->supportedViews().testFlag(ESPINA::VIEW_3D))
  {
    m_layout3D->addWidget(widget);
    m_viewFlags |= ViewType::VIEW_3D;
  }

  changeSwitchStatus(representationSwitch, m_representationsVisibility);

  m_switches << representationSwitch;

  bool showSeparators = m_viewFlags.testFlag(ESPINA::VIEW_2D) && m_viewFlags.testFlag(ESPINA::VIEW_3D);

  m_separator2D->setVisible(showSeparators);
  m_separator3D->setVisible(showSeparators);
}

//----------------------------------------------------------------------------
void RepresentationsGroupTool::toggleRepresentationsVisibility()
{
  m_globalSwitch->setChecked(!m_representationsVisibility);
}

//----------------------------------------------------------------------------
void RepresentationsGroupTool::onToolEnabled(bool enabled)
{
  m_content->setEnabled(enabled);
}

//----------------------------------------------------------------------------
void RepresentationsGroupTool::changeSwitchStatus(RepresentationSwitchSPtr representationSwitch, bool showRepresentations)
{
  if (representationSwitch->isActive())
  {
    if (showRepresentations)
    {
      representationSwitch->showRepresentations(m_timer.timeStamp());
    }
    else
    {
      representationSwitch->hideRepresentations(m_timer.timeStamp());
    }
  }
}

//----------------------------------------------------------------------------
QWidget *RepresentationsGroupTool::createSeparator(const QString &icon) const
{
  auto separator = new QLabel();

  separator->setPixmap(QPixmap(icon));
  separator->setScaledContents(true);
  separator->setMaximumSize(8, 30);
  separator->setVisible(false);

  return separator;
}

//----------------------------------------------------------------------------
void RepresentationsGroupTool::setActiveRepresentationsVisibility(bool value)
{
  if (m_representationsVisibility != value)
  {
    m_timer.increment();
    for (auto repSwitch : m_switches)
    {
      changeSwitchStatus(repSwitch, value);
    }
    m_content->setVisible(value);

  }

  m_representationsVisibility = value;
}
