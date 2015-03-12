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
#include <Settings/DefaultView/DefaultViewSettingsPanel.h>
#include <Menus/CamerasMenu.h>
#include <Menus/RenderersMenu.h>
#include <ToolGroups/View/ViewToolGroup.h>
#include <Support/Settings/EspinaSettings.h>

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

const QString DEFAULT_VIEW_SETTINGS = "DefaultView";

const QString X_LINE_COLOR  = "CrosshairXLineColor";
const QString Y_LINE_COLOR  = "CrosshairYLineColor";
const QString Z_LINE_COLOR  = "CrosshairZLineColor";
const QString SETTINGS_FILE = "Extra/DefaultView.ini";

//----------------------------------------------------------------------------
DefaultView::DefaultView(ModelAdapterSPtr     model,
                         ViewManagerSPtr      viewManager,
                         QUndoStack          *undoStack,
                         QMainWindow         *parent)
: m_model(model)
, m_viewManager(viewManager)
, m_viewState{new ViewState{m_model->timer()}}
, m_channelSources(m_model, ItemAdapter::Type::CHANNEL)
, m_segmentationSources(m_model, ItemAdapter::Type::SEGMENTATION)
, m_showProcessing(false)
, m_renderersMenu(nullptr)
, m_camerasMenu(nullptr)
{
  ESPINA_SETTINGS(settings);
  settings.beginGroup(DEFAULT_VIEW_SETTINGS);

  m_xLine = settings.value(X_LINE_COLOR, QColor(Qt::blue))   .value<QColor>();
  m_yLine = settings.value(Y_LINE_COLOR, QColor(Qt::magenta)).value<QColor>();
  m_zLine = settings.value(Z_LINE_COLOR, QColor(Qt::cyan))   .value<QColor>();

  settings.endGroup();

  m_viewXY = new View2D(Plane::XY);
  m_viewXZ = new View2D(Plane::XZ);
  m_viewYZ = new View2D(Plane::YZ);
  m_view3D = new View3D(false);

  setObjectName("viewXY");
  m_viewXY->setCrosshairColors(m_xLine, m_yLine);
  setLayout(new QVBoxLayout());
  layout()->addWidget(m_viewXY);
  layout()->setMargin(0);

  dockYZ = new QDockWidget(tr("ZY"), parent);
  dockYZ->setObjectName("DockZY");
  m_viewYZ->setCrosshairColors(m_zLine, m_yLine);
  dockYZ->setWidget(m_viewYZ);

  dockXZ = new QDockWidget(tr("XZ"), parent);
  dockXZ->setObjectName("xzDock");
  m_viewXZ->setCrosshairColors(m_xLine, m_zLine);
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
  deleteView(m_viewXY);
  deleteView(m_viewXZ);
  deleteView(m_viewYZ);
  deleteView(m_view3D);

  if (m_renderersMenu)
  {
    delete m_renderersMenu;
  }

  if (m_camerasMenu)
  {
    delete m_camerasMenu;
  }
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
    if (ViewToolGroup::CHANNELS_GROUP == representation.Group)
    {
      pool->setPipelineSources(&m_channelSources);
      m_channelPools << pool;
    }
    else if (ViewToolGroup::SEGMENTATIONS_GROUP == representation.Group)
    {
      pool->setPipelineSources(&m_segmentationSources);
      m_segmentationPools << pool;
    }
  }
}

//-----------------------------------------------------------------------------
void DefaultView::setCrosshairColor(const Plane plane, const QColor& color)
{
  ESPINA_SETTINGS(settings);
  settings.beginGroup(DEFAULT_VIEW_SETTINGS);
  switch (plane)
  {
    case Plane::XY:
      m_zLine = color;
      settings.setValue(Z_LINE_COLOR, color);
      break;
    case Plane::XZ:
      m_yLine = color;
      settings.setValue(Y_LINE_COLOR, color);
      break;
    case Plane::YZ:
      m_xLine = color;
      settings.setValue(X_LINE_COLOR, color);
      break;
    default:
      Q_ASSERT(false);
      break;
  };
  settings.endGroup();
  settings.sync();

  m_viewXY->setCrosshairColors(m_xLine, m_yLine);
  m_viewXZ->setCrosshairColors(m_xLine, m_zLine);
  m_viewYZ->setCrosshairColors(m_zLine, m_yLine);
}


//-----------------------------------------------------------------------------
void DefaultView::createViewMenu(QMenu* menu)
{
  m_camerasMenu = new CamerasMenu(m_viewManager, this);
  menu->addMenu(m_camerasMenu);

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

  auto fitToSlices = m_viewManager->fitToSlices();
  menu->addAction(fitToSlices);
  connect(fitToSlices, SIGNAL(toggled(bool)),
          this, SLOT(setFitToSlices(bool)));

  setRulerVisibility(sr);
  showThumbnail(st);
}

//-----------------------------------------------------------------------------
SettingsPanelSPtr DefaultView::settingsPanel()
{
  RendererSList renderers;

  return std::make_shared<DefaultViewSettingsPanel>(m_viewXY, m_viewXZ, m_viewYZ, m_view3D, renderers, m_renderersMenu);
}

// //----------------------------------------------------------------------------
// void DefaultView::sourceModelReset()
// {
//   m_viewManager->removeAllViewItems();
//   m_viewXY->reset();
//   m_viewYZ->reset();
//   m_viewXZ->reset();
//   m_view3D->reset();
// }
//
// //----------------------------------------------------------------------------
// void DefaultView::showCrosshair(bool visible)
// {
//   m_viewXY->setCrosshairVisibility(visible);
//   m_viewYZ->setCrosshairVisibility(visible);
//   m_viewXZ->setCrosshairVisibility(visible);
// }

//-----------------------------------------------------------------------------
void DefaultView::setRulerVisibility(bool visible)
{
  ESPINA_SETTINGS(settings);
  settings.beginGroup(DEFAULT_VIEW_SETTINGS);
  settings.setValue("ShowRuler", visible);
  settings.endGroup();

  m_viewXY->setRulerVisibility(visible);
  m_viewYZ->setRulerVisibility(visible);
  m_viewXZ->setRulerVisibility(visible);
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

// //----------------------------------------------------------------------------
// void DefaultView::switchPreprocessing()
// {
//   // Current implementation changes channel visibility and then
//   // notifies it's been updated to other views
//   m_showProcessing = !m_showProcessing;
//   m_viewXY->setShowPreprocessing(m_showProcessing);
//   m_viewYZ->setShowPreprocessing(m_showProcessing);
//   m_viewXZ->setShowPreprocessing(m_showProcessing);
// }

// //----------------------------------------------------------------------------
// void DefaultView::updateViews()
// {
//  m_viewXY->updateView();
//  m_viewXZ->updateView();
//  m_viewYZ->updateView();
//  m_view3D->updateView();
// }

//-----------------------------------------------------------------------------
void DefaultView::setFitToSlices(bool unused)
{
  NmVector3 step = m_viewXY->sceneResolution();

  m_viewXY->setSlicingStep(step);
  m_viewYZ->setSlicingStep(step);
  m_viewXZ->setSlicingStep(step);
}

//-----------------------------------------------------------------------------
void DefaultView::loadSessionSettings(TemporalStorageSPtr storage)
{
  // TODO
//   if(storage->exists(SETTINGS_FILE))
//   {
//     QSettings settings(storage->absoluteFilePath(SETTINGS_FILE), QSettings::IniFormat);
//     QStringList available2DRenderers = m_viewManager->renderers(ESPINA::RendererType::RENDERER_VIEW2D);
//     QStringList available3DRenderers = m_viewManager->renderers(ESPINA::RendererType::RENDERER_VIEW3D);
//     QMap<QString, bool> viewState;
//     RendererSList viewRenderers;
//
//     settings.beginGroup("View2D");
//     for(auto availableRenderer: available2DRenderers)
//     {
//       auto renderer = m_viewManager->cloneRenderer(availableRenderer);
//       if(renderer == nullptr || !settings.contains(availableRenderer))
//         continue;
//
//       viewRenderers << renderer;
//       auto enabled = settings.value(availableRenderer, false).toBool();
//       viewState[availableRenderer] = enabled;
//     }
//
//     m_viewXY->setRenderers(viewRenderers);
//     m_viewXY->setRenderersState(viewState);
//     viewState.clear();
//     viewRenderers.clear();
//     settings.endGroup();
//
//     settings.beginGroup("View3D");
//     for(auto availableRenderer: available3DRenderers)
//     {
//       auto renderer = m_viewManager->cloneRenderer(availableRenderer);
//       if(renderer == nullptr || !settings.contains(availableRenderer))
//         continue;
//
//       viewRenderers << renderer;
//       auto enabled = settings.value(availableRenderer, false).toBool();
//       viewState[availableRenderer] = enabled;
//     }
//
//     m_view3D->setRenderers(viewRenderers);
//     m_view3D->setRenderersState(viewState);
//     viewState.clear();
//     viewRenderers.clear();
//     settings.endGroup();
//   }
}

//-----------------------------------------------------------------------------
void DefaultView::saveSessionSettings(TemporalStorageSPtr storage)
{
//   QSettings settings(storage->absoluteFilePath(SETTINGS_FILE), QSettings::IniFormat);
//
//   settings.sync();
}

//-----------------------------------------------------------------------------
void DefaultView::initView(RenderView* view)
{
  view->setState(m_viewState);
  view->setChannelSources(&m_channelSources);

  m_viewManager->registerView(view);

  m_views << view;
}

//-----------------------------------------------------------------------------
void DefaultView::deleteView(RenderView* view)
{
  m_viewManager->unregisterView(view);

  delete view;
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
