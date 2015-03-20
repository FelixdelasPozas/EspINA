/*

    Copyright (C) 2014 Felix de las Pozas Alvarez <@>

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
#include "VisualizeToolGroup.h"

#include <GUI/Utils/DefaultIcons.h>
#include <Support/Representations/RepresentationUtils.h>

// Qt
#include <QIcon>
#include <QAction>
#include <QObject>
#include <QEvent>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::Support::Representations::Utils;

class VisualizeToolGroup::SettingsTool
: public Tool
{
public:
  SettingsTool()
  : m_showSettings{new QAction(DefaultIcons::Settings(), tr("Show Representation Options"), this)}
  {
  }

  virtual QList<QAction *> actions() const
  {
    QList<QAction *> settingsActions;

    settingsActions << m_showSettings;

    return settingsActions;
  }

private:
  virtual void onToolEnabled(bool enabled) override
  {
  }

private:
  QAction *m_showSettings;
};

//----------------------------------------------------------------------------
VisualizeToolGroup::VisualizeToolGroup(QWidget *parent)
: ToolGroup              {QIcon(":/espina/toolgroup_visualize.svg"), tr("Visualize"), parent}
, m_channelsRepGroup     {new RepresentationsGroupTool(QIcon(":/espina/channels_switch.png"), tr("Show Channels"))}
, m_segmentationsRepGroup{new RepresentationsGroupTool(QIcon(":/espina/segmentations_switch.svg"), tr("Show Segmentations"))}
, m_segmentationsShortcut{new QShortcut(parent)}
, m_crosshairShortcut    {new QShortcut(parent)}
{
  addTool(m_channelsRepGroup);
  addTool(m_segmentationsRepGroup);

  m_segmentationsShortcut->setKey(Qt::Key_Space);
  m_segmentationsShortcut->setContext(Qt::ApplicationShortcut);
  connect(m_segmentationsShortcut,       SIGNAL(activated()),
          m_segmentationsRepGroup.get(), SLOT(toggleRepresentationsVisibility()));

//   m_crosshairShortcut->setKey(Qt::Key_C);
//   m_crosshairShortcut->setContext(Qt::ApplicationShortcut);
//   connect(m_crosshairShortcut,     SIGNAL(activated()),
//           m_toggleCrosshair.get(), SLOT(shortcut()));

  m_channelsRepGroup->showActiveRepresentations();
  m_segmentationsRepGroup->showActiveRepresentations();
}

//----------------------------------------------------------------------------
VisualizeToolGroup::~VisualizeToolGroup()
{
  delete m_segmentationsShortcut;
  delete m_crosshairShortcut;
}

//----------------------------------------------------------------------------
void VisualizeToolGroup::addRepresentationSwitch(RepresentationGroup      group,
                                               RepresentationSwitchSPtr repSwitch,
                                               const QIcon             &groupIcon,
                                               const QString           &groupDescription)
{
  if (CHANNELS_GROUP == group)
  {
    m_channelsRepGroup->addRepresentationSwitch(repSwitch);
  }
  else if (SEGMENTATIONS_GROUP == group)
  {
    m_segmentationsRepGroup->addRepresentationSwitch(repSwitch);
  }
  else
  {
    auto addRepresentationGroup = !m_dynamicRepresentationGroups.contains(group);

    auto representationGroup    = m_dynamicRepresentationGroups.value(group, std::make_shared<RepresentationsGroupTool>(groupIcon, groupDescription));

    representationGroup->addRepresentationSwitch(repSwitch);

    m_dynamicRepresentationGroups[group] = representationGroup;

    if (addRepresentationGroup)
    {
      addTool(representationGroup);
    }
  }
}