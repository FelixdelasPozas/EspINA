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

// class VisualizeToolGroup::SettingsTool
// : public Tool
// {
// public:
//   SettingsTool()
//   : m_showSettings{new QAction(DefaultIcons::Settings(), tr("Show Representation Options"), this)}
//   {
//   }
//
//   virtual QList<QAction *> actions() const
//   {
//     QList<QAction *> settingsActions;
//
//     settingsActions << m_showSettings;
//
//     return settingsActions;
//   }
//
// private:
//   virtual void onToolEnabled(bool enabled) override
//   {
//   }
//
// private:
//   QAction *m_showSettings;
// };

//----------------------------------------------------------------------------
VisualizeToolGroup::VisualizeToolGroup(Support::Context &context, QWidget *parent)
: ToolGroup              {":/espina/toolgroup_visualize.svg", tr("Visualize"), parent}
, m_context(context)
, m_representationTools{context.timer()}
, m_segmentationsShortcut{new QShortcut(parent)}
{
  for (auto tool : m_representationTools.representationTools())
  {
    addTool(tool);
  }

  m_segmentationsShortcut->setKey(Qt::Key_Space);
  m_segmentationsShortcut->setContext(Qt::ApplicationShortcut);

  connect(&m_representationTools, SIGNAL(representationToolAdded(ToolSPtr)),
          this,                   SLOT(onRepresentationToolAdded(ToolSPtr)));

  auto segmentationTool = m_representationTools.segmentationRepresentations();

  connect(m_segmentationsShortcut, SIGNAL(activated()),
          segmentationTool.get(),  SLOT(toggleRepresentationsVisibility()));
}

//----------------------------------------------------------------------------
VisualizeToolGroup::~VisualizeToolGroup()
{
  delete m_segmentationsShortcut;
}

//----------------------------------------------------------------------------
void VisualizeToolGroup::addRepresentationSwitch(RepresentationGroup      group,
                                               RepresentationSwitchSPtr   repSwitch,
                                               const QIcon               &groupIcon,
                                               const QString             &groupDescription)
{
  m_representationTools.addRepresentationSwitch(group, repSwitch, groupIcon, groupDescription);
}

//----------------------------------------------------------------------------
void VisualizeToolGroup::onRepresentationToolAdded(ToolSPtr tool)
{
  addTool(tool);
}
