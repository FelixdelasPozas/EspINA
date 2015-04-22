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

// ESPINA
#include "DefaultView.h"

#include <Support/Settings/EspinaSettings.h>
#include <Support/Representations/RepresentationUtils.h>
#include <Support/Context.h>

#include <Menus/CamerasMenu.h>
#include <ToolGroups/Visualize/VisualizeToolGroup.h>

// Qt
#include <QApplication>
#include <QDir>
#include <QDockWidget>
#include <QGroupBox>
#include <QMainWindow>
#include <QSettings>
#include <QVBoxLayout>
#include <QMenu>

using namespace ESPINA;
using namespace ESPINA::Support::Representations::Utils;

const QString DEFAULT_VIEW_SETTINGS = "DefaultView";

const QString X_LINE_COLOR  = "CrosshairXLineColor";
const QString Y_LINE_COLOR  = "CrosshairYLineColor";
const QString Z_LINE_COLOR  = "CrosshairZLineColor";
const QString SETTINGS_FILE = "Extra/DefaultView.ini";

//----------------------------------------------------------------------------
DefaultView::DefaultView(Support::Context &context,
                         QMainWindow      *parent)
: m_model(context.model())
, m_viewState(context.viewState())
, m_channelSources(m_model,  ItemAdapter::Type::CHANNEL, context.representationInvalidator())
, m_segmentationSources(m_model, ItemAdapter::Type::SEGMENTATION, context.representationInvalidator())
, m_viewXY{new View2D(context.viewState(), context.selection(), Plane::XY)}
, m_viewYZ{new View2D(context.viewState(), context.selection(), Plane::YZ)}
, m_viewXZ{new View2D(context.viewState(), context.selection(), Plane::XZ)}
, m_view3D{new View3D(context.viewState(), context.selection(), false)}
{

  setObjectName("viewXY");
  setLayout(new QVBoxLayout());
  layout()->addWidget(m_viewXY);
  layout()->setMargin(0);

  dockYZ = new QDockWidget(tr("ZY"), parent);
  dockYZ->setObjectName("DockZY");
  dockYZ->setWidget(m_viewYZ);

  dockXZ = new QDockWidget(tr("XZ"), parent);
  dockXZ->setObjectName("xzDock");
  dockXZ->setWidget(m_viewXZ);

  dock3D = new QDockWidget(tr("3D"), parent);
  dock3D->setObjectName("Dock3D");
  dock3D->setWidget(m_view3D);

  parent->addDockWidget(Qt::RightDockWidgetArea, dock3D);
  parent->addDockWidget(Qt::RightDockWidgetArea, dockYZ);
  parent->addDockWidget(Qt::RightDockWidgetArea, dockXZ);

  initView(m_viewXY);
  initView(m_viewXZ);
  initView(m_viewYZ);
  initView(m_view3D);

  parent->setCentralWidget(this);
}

//-----------------------------------------------------------------------------
DefaultView::~DefaultView()
{
}

//-----------------------------------------------------------------------------
void DefaultView::addRepresentation(const Representation& representation)
{
  for (auto manager : representation.Managers)
  {
    addRepresentationManager(manager);
  }

  for (auto pool : representation.Pools)
  {
    if (CHANNELS_GROUP == representation.Group)
    {
      pool->setPipelineSources(&m_channelSources);
      m_channelPools << pool;
    }
    else if (SEGMENTATIONS_GROUP == representation.Group)
    {
      pool->setPipelineSources(&m_segmentationSources);
      m_segmentationPools << pool;
    }
  }
}

//-----------------------------------------------------------------------------
void DefaultView::createViewMenu(QMenu* menu)
{
  //menu->addMenu(new CamerasMenu(m_viewManager, this));

  auto renderMenu = new QMenu(tr("Views"), this);
  renderMenu->addAction(dockYZ->toggleViewAction());
  renderMenu->addAction(dockXZ->toggleViewAction());
  renderMenu->addAction(dock3D->toggleViewAction());
  menu->addMenu(renderMenu);

  ESPINA_SETTINGS(settings);
  settings.beginGroup(DEFAULT_VIEW_SETTINGS);

  bool sr = settings.value("ShowRuler",     true).toBool();
  bool st = settings.value("ShowThumbnail", true).toBool();

  settings.endGroup();

  m_showRuler = new QAction(tr("Show Scale Bar"),menu);
  m_showRuler->setCheckable(true);
  m_showRuler->setChecked(sr);
  menu->addAction(m_showRuler);
  connect(m_showRuler, SIGNAL(toggled(bool)),
          this, SLOT(setRulerVisibility(bool)));

  m_showThumbnail = new QAction(tr("Show Thumbnail"),menu);
  m_showThumbnail->setCheckable(true);
  m_showThumbnail->setChecked(st);
  menu->addAction(m_showThumbnail);
  connect(m_showThumbnail, SIGNAL(toggled(bool)),
          this, SLOT(showThumbnail(bool)));

  // TODO 2015-04-20 Recuperar fit to slices
//   menu->addAction(fitToSlices);
//   connect(fitToSlices, SIGNAL(toggled(bool)),
//           this, SLOT(setFitToSlices(bool)));

  setRulerVisibility(sr);
  showThumbnail(st);
}

//-----------------------------------------------------------------------------
SettingsPanelSPtr DefaultView::settingsPanel()
{
  //return std::make_shared<DefaultViewSettingsPanel>(m_viewXY, m_viewXZ, m_viewYZ, m_view3D, renderers, m_renderersMenu);
  return SettingsPanelSPtr();
}

//-----------------------------------------------------------------------------
void DefaultView::setRulerVisibility(bool visible)
{
  ESPINA_SETTINGS(settings);
  settings.beginGroup(DEFAULT_VIEW_SETTINGS);
  settings.setValue("ShowRuler", visible);
  settings.endGroup();

  m_viewXY->setScaleVisibility(visible);
  m_viewYZ->setScaleVisibility(visible);
  m_viewXZ->setScaleVisibility(visible);
}

//-----------------------------------------------------------------------------
void DefaultView::showThumbnail(bool visible)
{
  ESPINA_SETTINGS(settings);
  settings.beginGroup(DEFAULT_VIEW_SETTINGS);
  settings.setValue("ShowThumbnail", visible);
  settings.endGroup();
  m_viewXY->setThumbnailVisibility(visible);
  m_viewYZ->setThumbnailVisibility(visible);
  m_viewXZ->setThumbnailVisibility(visible);
}

//-----------------------------------------------------------------------------
void DefaultView::setFitToSlices(bool value)
{
  m_viewState.setFitToSlices(value);
}

//-----------------------------------------------------------------------------
void DefaultView::loadSessionSettings(TemporalStorageSPtr storage)
{
}

//-----------------------------------------------------------------------------
void DefaultView::saveSessionSettings(TemporalStorageSPtr storage)
{
}

//-----------------------------------------------------------------------------
void DefaultView::initView(RenderView* view)
{
  m_views << view;
}

//-----------------------------------------------------------------------------
void DefaultView::addRepresentationManager(RepresentationManagerSPtr manager)
{
  for (auto view : m_views)
  {
    if (manager->supportedViews().testFlag(view->type()))
    {
      view->addRepresentationManager(manager->clone());
    }
  }
}
