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

#include <QAction>
#include <QWidgetAction>
#include <QPushButton>
#include <QHBoxLayout>

using namespace ESPINA;

#include <QDebug>

//----------------------------------------------------------------------------
RepresentationsGroupTool::RepresentationsGroupTool(const QIcon &icon, QString description)
: m_globalSwitch{new QAction(icon, description, this)}
, m_content{new QWidgetAction(this)}
, m_contentWidget{new QWidget()}
, m_viewFlags{ViewType::VIEW_2D|ViewType::VIEW_3D}
, m_representationsVisibility{false}
{
  m_globalSwitch->setCheckable(true);

  connect(m_globalSwitch, SIGNAL(toggled(bool)),
          this,           SLOT(setActiveRepresentationsVisibility(bool)));

  m_contentWidget->setLayout(new QHBoxLayout());

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
  m_contentWidget->layout()->addWidget(representationSwitch->widget());

  changeSwitchStatus(representationSwitch, m_representationsVisibility);

  m_switches << representationSwitch;
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
      representationSwitch->showRepresentations();
    }
    else
    {
      representationSwitch->hideRepresentations();
    }
  }
}

//----------------------------------------------------------------------------
void RepresentationsGroupTool::setActiveRepresentationsVisibility(bool value)
{
  if (m_representationsVisibility != value)
  {
    for (auto repSwitch : m_switches)
    {
      changeSwitchStatus(repSwitch, value);
    }
    m_content->setVisible(value);

  }

  m_representationsVisibility = value;
}
