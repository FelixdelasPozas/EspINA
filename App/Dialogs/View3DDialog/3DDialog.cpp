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
#include <GUI/View/ViewState.h>
#include <Support/Settings/EspinaSettings.h>

// Qt
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::Support;
using namespace ESPINA::Support::Widgets;

const QString ENABLED               = "Tool enabled";
const QString GEOMETRY_SETTINGS_KEY = "View3D geometry";

//------------------------------------------------------------------------
Dialog3D::Dialog3D(Support::Context   &context)
: QDialog  {nullptr, Qt::WindowStaysOnTopHint}
, WithContext(context)
, m_view3D {context.viewState(), false}
{
  setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose, false);
  setWindowModality(Qt::WindowModality::NonModal);
  setWindowTitle("View 3D");

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
void Dialog3D::closeEvent(QCloseEvent *e)
{
  saveGeometryState();

  emit dialogVisible(false);

  QDialog::closeEvent(e);
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
  auto tool = std::make_shared<Dialog3DTool>(context(), this);

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
  if(!m_switches.contains(repSwitch))
  {
    m_switches << repSwitch;

    for(auto action: repSwitch->actions())
    {
      m_toolbar.addAction(action);
    }
  }
}

//------------------------------------------------------------------------
Dialog3DTool::Dialog3DTool(Support::Context &context, Dialog3D* dialog)
: ProgressTool("Dialog3DTool", ":espina/panel_3d.svg", tr("Display YZ View"), context)
, m_dialog    {dialog}
{
}

//------------------------------------------------------------------------
void Dialog3DTool::restoreSettings(std::shared_ptr<QSettings> settings)
{
  auto enabled = settings->value(ENABLED, false).toBool();

  for(auto tool: m_dialog->m_switches)
  {
    if(tool->id().isEmpty()) continue;

    settings->beginGroup(tool->id());
    tool->restoreSettings(settings);
    settings->endGroup();
  }

  if(isChecked() != enabled)
  {
    setChecked(enabled);
  }
}

//------------------------------------------------------------------------
void Dialog3DTool::saveSettings(std::shared_ptr<QSettings> settings)
{
  settings->setValue(ENABLED, isChecked());

  for(auto tool: m_dialog->m_switches)
  {
    if(tool->id().isEmpty() || !settings->childGroups().contains(tool->id())) continue;

    settings->beginGroup(tool->id());
    tool->saveSettings(settings);
    settings->endGroup();
  }
}
