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
#include <Support/Settings/EspinaSettings.h>
#include <GUI/Representations/Renderers/SliceRenderer.h>
#include <GUI/Representations/Renderers/ContourRenderer.h>

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
const QString RENDERERS     = "DefaultView::renderers";
const QString SETTINGS_FILE = "Extra/DefaultView.ini";

//----------------------------------------------------------------------------
DefaultView::DefaultView(ModelAdapterSPtr     model,
                         ViewManagerSPtr      viewManager,
                         QUndoStack          *undoStack,
                         QMainWindow         *parent)
: QAbstractItemView(parent)
, m_model(model)
, m_viewManager(viewManager)
, m_showProcessing(false)
, m_showSegmentations(true)
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
  initView2D(m_viewXY);
  setLayout(new QVBoxLayout());
  layout()->addWidget(m_viewXY);
  layout()->setMargin(0);

  dockYZ = new QDockWidget(tr("ZY"), parent);
  dockYZ->setObjectName("DockZY");
  m_viewYZ->setCrosshairColors(m_zLine, m_yLine);
  initView2D(m_viewYZ);
  dockYZ->setWidget(m_viewYZ);

  dockXZ = new QDockWidget(tr("XZ"), parent);
  dockXZ->setObjectName("xzDock");
  m_viewXZ->setCrosshairColors(m_xLine, m_zLine);
  initView2D(m_viewXZ);
  dockXZ->setWidget(m_viewXZ);

  dock3D = new QDockWidget(tr("3D"), parent);
  dock3D->setObjectName("Dock3D");
  dock3D->setWidget(m_view3D);

  QMap<QString, bool> viewSettings;
  QStringList storedRenderers;
  settings.beginGroup(DEFAULT_VIEW_SETTINGS);
  if(settings.contains(RENDERERS) && settings.value(RENDERERS).isValid())
  {
    storedRenderers = settings.value(RENDERERS).toStringList();
    for(auto name: storedRenderers)
      viewSettings[name] = settings.value(name).toBool();
  }
  else
  {
    // default init state: (can be overridden by seg file settings).
    // 2D -> cached slice renderer active, and the rest not included in XY view.
    // 3D -> all renderers included but initially inactive.
    storedRenderers << QString("Slice (Cached)");
    storedRenderers << QString("Contour");
    viewSettings[QString("Slice (Cached)")] = true;
    viewSettings[QString("Contour")] = false;
    storedRenderers << m_viewManager->renderers(ESPINA::RendererType::RENDERER_VIEW3D);

    settings.setValue(RENDERERS, storedRenderers);
  }
  settings.endGroup();

  QStringList available2DRenderers = m_viewManager->renderers(ESPINA::RendererType::RENDERER_VIEW2D);
  QStringList available3DRenderers = m_viewManager->renderers(ESPINA::RendererType::RENDERER_VIEW3D);
  RendererSList renderers2D, renderers3D;

  for(auto name : storedRenderers)
  {
    if(available2DRenderers.contains(name))
      renderers2D << m_viewManager->cloneRenderer(name);
    else
      if(available3DRenderers.contains(name))
        renderers3D << m_viewManager->cloneRenderer(name);
  }

  m_view3D->setRenderers(renderers3D);
  m_viewXY->setRenderers(renderers2D);
  m_viewXZ->setRenderers(renderers2D);
  m_viewYZ->setRenderers(renderers2D);

  m_view3D->setRenderersState(viewSettings);
  m_viewXY->setRenderersState(viewSettings);
  m_viewXZ->setRenderersState(viewSettings);
  m_viewYZ->setRenderersState(viewSettings);

  connect(m_view3D, SIGNAL(centerChanged(NmVector3)),
          this, SLOT(setCrosshairPoint(NmVector3)));

  m_viewManager->registerView(m_viewXY);
  m_viewManager->registerView(m_viewXZ);
  m_viewManager->registerView(m_viewYZ);
  m_viewManager->registerView(m_view3D);

  parent->addDockWidget(Qt::RightDockWidgetArea, dock3D);
  parent->addDockWidget(Qt::RightDockWidgetArea, dockYZ);
  parent->addDockWidget(Qt::RightDockWidgetArea, dockXZ);

  parent->setCentralWidget(this);

  setModel(m_model.get());
}

//-----------------------------------------------------------------------------
DefaultView::~DefaultView()
{
  ESPINA_SETTINGS(settings);
  settings.beginGroup(DEFAULT_VIEW_SETTINGS);
  QStringList activeRenderersNames;
  QMap<QString, bool> viewState;

  for(auto renderer : m_view3D->renderers())
  {
    activeRenderersNames << renderer->name();
    viewState[renderer->name()] = !renderer->isHidden();
  }

  for(auto renderer : m_viewXY->renderers())
  {
    activeRenderersNames << renderer->name();
    viewState[renderer->name()] = !renderer->isHidden();
  }

  settings.setValue(RENDERERS, activeRenderersNames);
  for(auto name: activeRenderersNames)
  {
    settings.setValue(name, viewState[name]);
  }

  settings.endGroup();
  settings.sync();

  m_viewManager->unregisterView(m_viewXY);
  m_viewManager->unregisterView(m_viewXZ);
  m_viewManager->unregisterView(m_viewYZ);
  m_viewManager->unregisterView(m_view3D);

  delete m_viewXY;
  delete m_viewXZ;
  delete m_viewYZ;
  delete m_view3D;

  delete m_renderersMenu;
  delete m_camerasMenu;
}

//-----------------------------------------------------------------------------
void DefaultView::initView2D(View2D *view)
{
  connect(view, SIGNAL(centerChanged(NmVector3)),
          this, SLOT(setCrosshairPoint(NmVector3)));
  connect(view, SIGNAL(sliceChanged(Plane, Nm)),
          this, SLOT(changePlanePosition(Plane, Nm)));
}

//-----------------------------------------------------------------------------
RendererSPtr DefaultView::renderer(const QString& name) const
{
  for(auto renderer : m_viewManager->renderers(RendererType::RENDERER_VIEW2D))
    if (renderer == name)
      return m_viewManager->cloneRenderer(name);

  for(auto renderer : m_viewManager->renderers(RendererType::RENDERER_VIEW3D))
    if (renderer == name)
      return m_viewManager->cloneRenderer(name);

  return nullptr;
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
  m_renderersMenu = new RenderersMenu(m_viewManager, this);
  for(auto renderer: m_view3D->renderers())
    m_renderersMenu->addRenderer(renderer);
  for(auto renderer: m_viewXY->renderers())
    m_renderersMenu->addRenderer(renderer);
  menu->addMenu(m_renderersMenu);

  m_camerasMenu = new CamerasMenu(m_viewManager, this);
  menu->addMenu(m_camerasMenu);

  QMenu *renderMenu = new QMenu(tr("Views"), this);
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

  QAction *togglePreprocessingVisibility = new QAction(tr("Switch Channel"), menu);
  togglePreprocessingVisibility->setShortcut(QString("Ctrl+Space"));
  connect(togglePreprocessingVisibility, SIGNAL(triggered(bool)), this, SLOT(switchPreprocessing()));
  menu->addAction(togglePreprocessingVisibility);

  QAction *fitToSlices = m_viewManager->fitToSlices();
  menu->addAction(fitToSlices);
  connect(fitToSlices, SIGNAL(toggled(bool)),
          this, SLOT(setFitToSlices(bool)));

  setRulerVisibility(sr);
  showThumbnail(st);
}

//-----------------------------------------------------------------------------
void DefaultView::setModel(QAbstractItemModel* model)
{
  QAbstractItemView::setModel(model);
  connect(model, SIGNAL(modelAboutToBeReset()),
          this, SLOT(sourceModelReset()));
}

//-----------------------------------------------------------------------------
SettingsPanelSPtr DefaultView::settingsPanel()
{
  RendererSList renderers;

  for(auto name : m_viewManager->renderers(RendererType::RENDERER_VIEW2D))
    renderers << m_viewManager->cloneRenderer(name);

  for(auto name : m_viewManager->renderers(RendererType::RENDERER_VIEW3D))
    renderers << m_viewManager->cloneRenderer(name);

  return SettingsPanelSPtr(new DefaultViewSettingsPanel(m_viewXY, m_viewXZ, m_viewYZ, m_view3D, renderers, m_renderersMenu));
}

//-----------------------------------------------------------------------------
void DefaultView::add(ChannelAdapterPtr channel)
{
  m_viewXY->add(channel);
  m_viewYZ->add(channel);
  m_viewXZ->add(channel);
  m_view3D->add(channel);
}

//-----------------------------------------------------------------------------
void DefaultView::remove(ChannelAdapterPtr channel)
{
  m_viewXY->remove(channel);
  m_viewYZ->remove(channel);
  m_viewXZ->remove(channel);
  m_view3D->remove(channel);
}

//-----------------------------------------------------------------------------
bool DefaultView::updateRepresentation(ChannelAdapterPtr channel)
{
  bool modified = false;
  modified |= m_viewXY->updateRepresentation(channel);
  modified |= m_viewYZ->updateRepresentation(channel);
  modified |= m_viewXZ->updateRepresentation(channel);
  modified |= m_view3D->updateRepresentation(channel);
  return modified;
}

//-----------------------------------------------------------------------------
void DefaultView::add(SegmentationAdapterPtr seg)
{
  m_viewXY->add(seg);
  m_viewYZ->add(seg);
  m_viewXZ->add(seg);
  m_view3D->add(seg);
}

//-----------------------------------------------------------------------------
void DefaultView::remove(SegmentationAdapterPtr seg)
{
  m_viewXY->remove(seg);
  m_viewYZ->remove(seg);
  m_viewXZ->remove(seg);
  m_view3D->remove(seg);
}

//-----------------------------------------------------------------------------
bool DefaultView::updateRepresentation(SegmentationAdapterPtr seg)
{
  bool modified = false;
  modified |= m_viewXY->updateRepresentation(seg);
  modified |= m_viewYZ->updateRepresentation(seg);
  modified |= m_viewXZ->updateRepresentation(seg);
  modified |= m_view3D->updateRepresentation(seg);
  return modified;
}

//-----------------------------------------------------------------------------
void DefaultView::rowsInserted(const QModelIndex& parent, int start, int end)
{
  if (!parent.isValid())
    return;

  for (int child = start; child <= end; child++)
  {
    QModelIndex index = parent.child(child, 0);
    ItemAdapterPtr item = itemAdapter(index);
    switch (item->type())
    {
      case ItemAdapter::Type::CHANNEL:
      {
        ChannelAdapterPtr channel = channelPtr(item);
        //   qDebug() << "Add Channel:" << channel->data(Qt::DisplayRole).toString();

        add(channel);
        break;
      }
      case ItemAdapter::Type::SEGMENTATION:
      {
        SegmentationAdapterPtr seg = segmentationPtr(item);
        //   qDebug() << "Add Segmentation:" << seg->data(Qt::DisplayRole).toString();
        add(seg);
        break;
      }
      default:
        break;
    };
  }
}

//-----------------------------------------------------------------------------
void DefaultView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
  if (!parent.isValid())
    return;

  for (int child = start; child <= end; child++)
  {
    QModelIndex index = parent.child(child, 0);
    ItemAdapterPtr item = itemAdapter(index);
    switch (item->type())
    {
      case ItemAdapter::Type::CHANNEL:
      {
        ChannelAdapterPtr channel = channelPtr(item);
        remove(channel);
        break;
      }
      case ItemAdapter::Type::SEGMENTATION:
      {
        SegmentationAdapterPtr seg = segmentationPtr(item);
        remove(seg);
        break;
      }
      default:
        break;
    };
  }
}

//----------------------------------------------------------------------------
void DefaultView::sourceModelReset()
{
  m_viewXY->reset();
  m_viewYZ->reset();
  m_viewXZ->reset();
  m_view3D->reset();
}

//----------------------------------------------------------------------------
void DefaultView::showCrosshair(bool visible)
{
  m_viewXY->setCrosshairVisibility(visible);
  m_viewYZ->setCrosshairVisibility(visible);
  m_viewXZ->setCrosshairVisibility(visible);
}

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

//----------------------------------------------------------------------------
void DefaultView::showSegmentations(bool visible)
{
  m_viewXY->setSegmentationsVisibility(visible);
  m_viewYZ->setSegmentationsVisibility(visible);
  m_viewXZ->setSegmentationsVisibility(visible);

  m_showSegmentations = visible;
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

//----------------------------------------------------------------------------
void DefaultView::switchPreprocessing()
{
  // Current implementation changes channel visibility and then
  // notifies it's been updated to other views
  m_showProcessing = !m_showProcessing;
  m_viewXY->setShowPreprocessing(m_showProcessing);
  m_viewYZ->setShowPreprocessing(m_showProcessing);
  m_viewXZ->setShowPreprocessing(m_showProcessing);
}

//----------------------------------------------------------------------------
void DefaultView::updateViews()
{
 m_viewXY->updateView();
 m_viewYZ->updateView();
 m_viewXZ->updateView();
 m_view3D->updateView();
}

//-----------------------------------------------------------------------------
void DefaultView::setCrosshairPoint(const NmVector3& point, bool force)
{
  m_viewXY->centerViewOn(point, force);
  m_viewYZ->centerViewOn(point, force);
  m_viewXZ->centerViewOn(point, force);
  m_view3D->centerViewOn(point, force);
}

//-----------------------------------------------------------------------------
void DefaultView::changePlanePosition(Plane plane, Nm dist)
{
  switch(plane)
  {
    case Plane::XY:
      this->m_viewYZ->updateCrosshairPoint(plane, dist);
      this->m_viewXZ->updateCrosshairPoint(plane, dist);
      break;
    case Plane::XZ:
      this->m_viewXY->updateCrosshairPoint(plane, dist);
      this->m_viewYZ->updateCrosshairPoint(plane, dist);
      break;
    case Plane::YZ:
      this->m_viewXY->updateCrosshairPoint(plane, dist);
      this->m_viewXZ->updateCrosshairPoint(plane, dist);
      break;
    default:
      return;
      break;
  }
  m_view3D->changePlanePosition(plane, dist);
}


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
  if(storage->exists(SETTINGS_FILE))
  {
    QSettings settings(storage->absoluteFilePath(SETTINGS_FILE), QSettings::IniFormat);
    QStringList available2DRenderers = m_viewManager->renderers(ESPINA::RendererType::RENDERER_VIEW2D);
    QStringList available3DRenderers = m_viewManager->renderers(ESPINA::RendererType::RENDERER_VIEW3D);
    QMap<QString, bool> viewState;
    RendererSList viewRenderers;

    settings.beginGroup("View2D");
    for(auto availableRenderer: available2DRenderers)
    {
      auto renderer = m_viewManager->cloneRenderer(availableRenderer);
      if(renderer == nullptr || !settings.contains(availableRenderer))
        continue;

      viewRenderers << renderer;
      auto enabled = settings.value(availableRenderer, false).toBool();
      viewState[availableRenderer] = enabled;
    }

    m_viewXY->setRenderers(viewRenderers);
    m_viewXY->setRenderersState(viewState);
    viewState.clear();
    viewRenderers.clear();
    settings.endGroup();

    settings.beginGroup("View3D");
    for(auto availableRenderer: available3DRenderers)
    {
      auto renderer = m_viewManager->cloneRenderer(availableRenderer);
      if(renderer == nullptr || !settings.contains(availableRenderer))
        continue;

      viewRenderers << renderer;
      auto enabled = settings.value(availableRenderer, false).toBool();
      viewState[availableRenderer] = enabled;
    }

    m_view3D->setRenderers(viewRenderers);
    m_view3D->setRenderersState(viewState);
    viewState.clear();
    viewRenderers.clear();
    settings.endGroup();
  }
}

//-----------------------------------------------------------------------------
void DefaultView::saveSessionSettings(TemporalStorageSPtr storage)
{
  QSettings settings(storage->absoluteFilePath(SETTINGS_FILE), QSettings::IniFormat);

  settings.beginGroup("View2D");
  for(auto renderer: m_viewXY->renderers())
    settings.setValue(renderer->name(), !renderer->isHidden());
  settings.endGroup();

  settings.beginGroup("View3D");
  for(auto renderer: m_view3D->renderers())
    settings.setValue(renderer->name(), !renderer->isHidden());
  settings.endGroup();

  settings.sync();
}

// //-----------------------------------------------------------------------------
// void DefaultEspinaView::setSliceSelectors(SliceView::SliceSelectors selectors)
// {
//   xyView->setSliceSelectors(selectors);
//   yzView->setSliceSelectors(selectors);
//   xzView->setSliceSelectors(selectors);
// }
// //-----------------------------------------------------------------------------
// void DefaultEspinaView::selectFromSlice(double slice, PlaneType plane)
// {
//   emit selectedFromSlice(slice, plane);
// }
//
// //-----------------------------------------------------------------------------
// void DefaultEspinaView::selectToSlice(double slice, PlaneType plane)
// {
//   emit selectedToSlice(slice, plane);
// }
