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
#include <GUI/Utils/DefaultIcons.h>
#include "ViewToolGroup.h"

// Qt
#include <QIcon>
#include <QAction>
#include <QObject>
#include <QEvent>

using namespace ESPINA;
using namespace ESPINA::GUI;

class ViewToolGroup::SettingsTool
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

ViewToolGroup::RenderGroup ViewToolGroup::CHANNELS_GROUP      = "ChannelsRenderGroup";
ViewToolGroup::RenderGroup ViewToolGroup::SEGMENTATIONS_GROUP = "SegmentationsRenderGroup";

//----------------------------------------------------------------------------
ViewToolGroup::ViewToolGroup(ViewManagerSPtr viewManager, QWidget* parent)
: ToolGroup                 {viewManager, QIcon(":/espina/show_all.svg"), tr("View Tools"), parent}
, m_toggleCrosshair         {new ToggleCrosshairVisibility(viewManager)}
, m_resetZoom               {new ResetZoom(viewManager)}
, m_zoomArea                {new ZoomAreaTool(viewManager)}
, m_renderSettings          {new SettingsTool()}
, m_channelsRenderGroup     {new RepresentationsGroupTool(QIcon(":/espina/channels_switch.png"), tr("Show Channels"))}
, m_segmentationsRenderGroup{new RepresentationsGroupTool(QIcon(":/espina/segmentations_switch.svg"), tr("Show Segmentations"))}
, m_segmentationsShortcut   {new QShortcut(parent)}
, m_crosshairShortcut       {new QShortcut(parent)}
{
  m_segmentationsShortcut->setKey(Qt::Key_Space);
  m_segmentationsShortcut->setContext(Qt::ApplicationShortcut);
  connect(m_segmentationsShortcut,          SIGNAL(activated()),
          m_segmentationsRenderGroup.get(), SLOT(toggleRepresentationsVisibility()));

  m_crosshairShortcut->setKey(Qt::Key_C);
  m_crosshairShortcut->setContext(Qt::ApplicationShortcut);

  connect(m_crosshairShortcut,     SIGNAL(activated()),
          m_toggleCrosshair.get(), SLOT(shortcut()));

  connect(parent, SIGNAL(abortOperation()),
          this,   SLOT(abortOperation()), Qt::QueuedConnection);
  connect(parent, SIGNAL(analysisClosed()),
          this,   SLOT(abortOperation()), Qt::QueuedConnection);

  m_channelsRenderGroup->showActiveRepresentations();
  m_segmentationsRenderGroup->showActiveRepresentations();
}

//----------------------------------------------------------------------------
ViewToolGroup::~ViewToolGroup()
{
  delete m_segmentationsShortcut;
  delete m_crosshairShortcut;
}

//----------------------------------------------------------------------------
void ViewToolGroup::setEnabled(bool value)
{
  m_enabled = value;

  m_toggleCrosshair->setEnabled(value);
  m_zoomArea->setEnabled(value);
  m_resetZoom->setEnabled(value);

  m_segmentationsShortcut->setEnabled(value);
  m_crosshairShortcut->setEnabled(value);
}

//----------------------------------------------------------------------------
bool ViewToolGroup::enabled() const
{
  return m_enabled;
}

//----------------------------------------------------------------------------
ToolSList ViewToolGroup::tools()
{
  ToolSList viewTools;

  viewTools << m_zoomArea;
  viewTools << m_resetZoom;
  viewTools << m_renderSettings;
  viewTools << m_channelsRenderGroup;
  viewTools << m_segmentationsRenderGroup;

  for (auto renderGroup : m_dynamicRenderGroups)
  {
    viewTools << renderGroup;
  }

  return viewTools;
}

//----------------------------------------------------------------------------
void ViewToolGroup::addRepresentationSwitch(RenderGroup              group,
                                            RepresentationSwitchSPtr repSwitch,
                                            const QIcon             &groupIcon,
                                            const QString           &groupDescription)
{
  if (CHANNELS_GROUP == group)
  {
    m_channelsRenderGroup->addRepresentationSwitch(repSwitch);
  }
  else if (SEGMENTATIONS_GROUP == group)
  {
    m_segmentationsRenderGroup->addRepresentationSwitch(repSwitch);
  }
  else
  {
    auto renderGroup = m_dynamicRenderGroups.value(group, std::make_shared<RepresentationsGroupTool>(groupIcon, groupDescription));

    renderGroup->addRepresentationSwitch(repSwitch);

    m_dynamicRenderGroups[group] = renderGroup;
  }
}

//----------------------------------------------------------------------------
void ViewToolGroup::abortOperation()
{
  m_zoomArea->abortOperation();
}

