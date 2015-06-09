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
#include <App/EspinaMainWindow.h>
#include <Support/Settings/EspinaSettings.h>
#include <Support/Representations/RepresentationUtils.h>
#include <Support/Context.h>
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
using namespace ESPINA::GUI::Representations;
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
, m_viewXY{new View2D(context.viewState(), Plane::XY)}
, m_viewYZ{new View2D(context.viewState(), Plane::YZ)}
, m_viewXZ{new View2D(context.viewState(), Plane::XZ)}
, m_view3D{new View3D(context.viewState(), false)}
{

  setObjectName("viewXY");
  setLayout(new QVBoxLayout());
  layout()->addWidget(m_viewXY);
  layout()->setMargin(0);

  m_panelYZ = new DockWidget(tr("ZY"), parent);
  m_panelYZ->setObjectName("DockZY");
  m_panelYZ->setWidget(m_viewYZ);

  m_panelXZ = new DockWidget(tr("XZ"), parent);
  m_panelXZ->setObjectName("xzDock");
  m_panelXZ->setWidget(m_viewXZ);

  m_panel3D = new DockWidget(tr("3D"), parent);
  m_panel3D->setObjectName("Dock3D");
  m_panel3D->setWidget(m_view3D);

  parent->addDockWidget(Qt::RightDockWidgetArea, m_panel3D);
  parent->addDockWidget(Qt::RightDockWidgetArea, m_panelYZ);
  parent->addDockWidget(Qt::RightDockWidgetArea, m_panelXZ);

  initView(m_viewXY, parent);
  initView(m_viewXZ, parent);
  initView(m_viewYZ, parent);
  initView(m_view3D, parent);

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
  renderMenu->addAction(m_panelYZ->toggleViewAction());
  renderMenu->addAction(m_panelXZ->toggleViewAction());
  renderMenu->addAction(m_panel3D->toggleViewAction());
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
DockWidget* DefaultView::panelXZ()
{
  return m_panelXZ;
}

//-----------------------------------------------------------------------------
DockWidget* DefaultView::panelYZ()
{
  return m_panelYZ;
}

//-----------------------------------------------------------------------------
DockWidget *DefaultView::panel3D()
{
  return m_panel3D;
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
void DefaultView::initView(RenderView* view, QMainWindow *parent)
{
  m_views << view;

  connect(parent, SIGNAL(analysisClosed()),
          view,   SLOT(reset()));
}

//-----------------------------------------------------------------------------
QList<RenderView*> ESPINA::DefaultView::renderviews() const
{
  QList<RenderView *> result;
  result << m_viewXY << m_viewXZ << m_viewYZ << m_view3D;

  return result;
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
