/*

    Copyright (C) 2015  Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Dialogs/View3DDialog/3DDialog.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/View/ViewState.h>
#include <GUI/Widgets/Styles.h>
#include <Support/Settings/EspinaSettings.h>

// Qt
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets::Styles;
using namespace ESPINA::Support;
using namespace ESPINA::Support::Widgets;

const QString GEOMETRY_SETTINGS_KEY = "View3D geometry";

//------------------------------------------------------------------------
Dialog3D::Dialog3D(Support::Context   &context)
: QDialog          {DefaultDialogs::defaultParentWidget(), Qt::WindowStaysOnTopHint}
, WithContext      (context)
, m_view3D         {context.viewState(), false}
, m_representations("", "")
{
  setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose, false);
  setWindowModality(Qt::WindowModality::NonModal);
  setWindowTitle("View 3D");

  m_toolbar.setMinimumHeight(CONTEXTUAL_BAR_HEIGHT);
  m_toolbar.setMaximumHeight(CONTEXTUAL_BAR_HEIGHT);
  layout()->setMenuBar(&m_toolbar);

  initView3D();

  restoreGeometryState();
}

//------------------------------------------------------------------------
Dialog3D::~Dialog3D()
{
}

//------------------------------------------------------------------------
RenderView *Dialog3D::renderView()
{
  return &m_view3D;
}

//------------------------------------------------------------------------
QAction *Dialog3D::toggleViewAction()
{
  auto action = new QAction(tr("3D"), this);

  action->setCheckable(true);

  connect(action, SIGNAL(toggled(bool)),
          this,   SLOT(onToggled(bool)));

  connect(this,   SIGNAL(dialogVisible(bool)),
          action, SLOT(setChecked(bool)));

  return action;
}

//------------------------------------------------------------------------
void Dialog3D::showEvent(QShowEvent* event)
{
  for(auto group : m_representations.groupedTools())
  {
    for (auto tool : group)
    {
      for (auto action : tool->actions())
      {
        m_toolbar.addAction(action);
      }
    }
  }

  QDialog::showEvent(event);
}

//------------------------------------------------------------------------
void Dialog3D::closeEvent(QCloseEvent *event)
{
  saveGeometryState();

  emit dialogVisible(false);

  QDialog::closeEvent(event);
}

//------------------------------------------------------------------------
void Dialog3D::initView3D()
{
  auto layout = new QVBoxLayout();
  layout->addWidget(&m_view3D);
  m_view->setLayout(layout);
}

//------------------------------------------------------------------------
void Dialog3D::restoreGeometryState()
{
  ESPINA_SETTINGS(settings);

  auto geometry = settings.value(GEOMETRY_SETTINGS_KEY, QByteArray()).toByteArray();
  if (!geometry.isEmpty())
  {
    restoreGeometry(geometry);
  }
}

//------------------------------------------------------------------------
void Dialog3D::saveGeometryState()
{
  ESPINA_SETTINGS(settings);

  settings.setValue(GEOMETRY_SETTINGS_KEY, this->saveGeometry());
  settings.sync();
}

//------------------------------------------------------------------------
std::shared_ptr<ProgressTool> Dialog3D::tool()
{
  auto tool = std::make_shared<Dialog3DTool>(getContext(), this);

  tool->setCheckable(true);
  tool->setChecked(this->isVisible());

  connect(tool.get(), SIGNAL(toggled(bool)),
          this,       SLOT(onToggled(bool)));

  connect(this,       SIGNAL(dialogVisible(bool)),
          tool.get(), SLOT(setChecked(bool)));

  return tool;
}

//------------------------------------------------------------------------
void Dialog3D::onToggled(bool checked)
{
  if(checked)
  {
    this->show();
  }
  else
  {
    this->hide();
  }

  emit dialogVisible(checked);
}

//------------------------------------------------------------------------
void Dialog3D::addRepresentationSwitch(RepresentationSwitchSPtr repSwitch)
{
  m_representations.addTool(repSwitch);
}

//------------------------------------------------------------------------
Dialog3DTool::Dialog3DTool(Support::Context &context, Dialog3D* dialog)
: ProgressTool("Dialog3DTool", ":espina/panel_3d.svg", tr("Display 3D View"), context)
, m_dialog    {dialog}
{
}

//------------------------------------------------------------------------
void Dialog3DTool::restoreSettings(std::shared_ptr<QSettings> settings)
{
  restoreCheckedState(settings);

  for(auto tool: tools())
  {
    if (!tool->id().isEmpty()
      && settings->childGroups().contains(tool->id()))
    {
      settings->beginGroup(tool->id());
      tool->restoreSettings(settings);
      settings->endGroup();
    }
  }
}

//------------------------------------------------------------------------
void Dialog3DTool::saveSettings(std::shared_ptr<QSettings> settings)
{
  saveCheckedState(settings);

  for (auto tool : tools())
  {
    if(!tool->id().isEmpty())
    {
      settings->beginGroup(tool->id());
      tool->saveSettings(settings);
      settings->endGroup();
    }
  }
}

//------------------------------------------------------------------------
ToolSList Dialog3DTool::tools() const
{
  ToolSList tools;

  for(auto group : m_dialog->m_representations.groupedTools())
  {
    for (auto tool : group)
    {
      tools << tool;
    }
  }

  return tools;
}
