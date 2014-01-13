/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

 This program is free software: you can redistribute it and/or modify
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

#include "DefaultView.h"
#include <Settings/DefaultView/DefaultViewSettingsPanel.h>

// EspINA
//#include <Menus/SegmentationContextualMenu.h>
#include <Support/Settings/EspinaSettings.h>
#include <GUI/Representations/Renderers/SliceRenderer.h>

#include <QDebug>

// Qt
#include <QApplication>
#include <QDir>
#include <QDockWidget>
#include <QGroupBox>
#include <QMainWindow>
#include <QSettings>
#include <QVBoxLayout>
#include <QMenu>

using namespace EspINA;

const QString DEFAULT_VIEW_SETTINGS = "DefaultView";

const QString X_LINE_COLOR = "CrosshairXLineColor";
const QString Y_LINE_COLOR = "CrosshairYLineColor";
const QString Z_LINE_COLOR = "CrosshairZLineColor";

//----------------------------------------------------------------------------
DefaultView::DefaultView(ModelAdapterSPtr     model,
                         ViewManagerSPtr      viewManager,
                         QUndoStack          *undoStack,
                         const RendererSList &renderers,
                         QMainWindow         *parent)
: QAbstractItemView(parent)
, m_model(model)
, m_viewManager(viewManager)
, m_showProcessing(false)
, m_showSegmentations(true)
, m_renderers(renderers)
//, m_contextMenu(new DefaultContextualMenu(SegmentationList(), model, undoStack, viewManager))
{
  QSettings settins(CESVIMA, ESPINA);
  settins.beginGroup(DEFAULT_VIEW_SETTINGS);

  m_xLine = settins.value(X_LINE_COLOR, QColor(Qt::blue))   .value<QColor>();
  m_yLine = settins.value(Y_LINE_COLOR, QColor(Qt::magenta)).value<QColor>();
  m_zLine = settins.value(Z_LINE_COLOR, QColor(Qt::cyan))   .value<QColor>();

  settins.endGroup();

  //   qDebug() << "New Default EspinaView";
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
//   qDebug() << "Destroy Default EspINA View";
}

//-----------------------------------------------------------------------------
void DefaultView::initView2D(View2D *view)
{
  connect(view, SIGNAL(centerChanged(NmVector3)),
          this, SLOT(setCrosshairPoint(NmVector3)));
//   connect(view, SIGNAL(focusChanged(const Nm[3])),
//           this, SLOT(setCameraFocus(const Nm[3])));
//   connect(view, SIGNAL(selectedFromSlice(double, PlaneType)),
//           this, SLOT(selectFromSlice(double, PlaneType)));
//   connect(view, SIGNAL(selectedToSlice(double, PlaneType)),
//           this, SLOT(selectToSlice(double, PlaneType)));
  connect(view, SIGNAL(sliceChanged(Plane, Nm)),
          this, SLOT(changePlanePosition(Plane, Nm)));
  view->setContextualMenu(m_contextMenu);
  RendererSList renderers;
  renderers << RendererSPtr(new SliceRenderer());
  view->setRenderers(renderers);
//   view->setColorEngine(m_viewManager->colorEngine());
//   view->setSelector(m_viewManager->selector());
}

//-----------------------------------------------------------------------------
void DefaultView::setCrosshairColor(const Plane plane, const QColor& color)
{
  QSettings settings(CESVIMA, ESPINA);
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
  QMenu *renderMenu = new QMenu(tr("Views"), this);
  renderMenu->addAction(dockYZ->toggleViewAction());
  renderMenu->addAction(dockXZ->toggleViewAction());
  renderMenu->addAction(dock3D->toggleViewAction());
  //renderMenu->addSeparator();
  menu->addMenu(renderMenu);

  QSettings settings(CESVIMA, ESPINA);
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
  return SettingsPanelSPtr(new DefaultViewSettingsPanel(m_viewXY, m_viewXZ, m_viewYZ, m_view3D, m_renderers));
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

  //   qDebug() << parent.data(Qt::DisplayRole).toString();
  for (int child = start; child <= end; child++)
  {
    QModelIndex index = parent.child(child, 0);
    ItemAdapterPtr item = itemAdapter(index);
    switch (item->type())
    {
      case ItemAdapter::Type::CHANNEL:
      {
        ChannelAdapterPtr channel = channelPtr(item);
        //   qDebug() << "Remove Channel:" << channel->data(Qt::DisplayRole).toString();
        remove(channel);
        break;
      }
      case ItemAdapter::Type::SEGMENTATION:
      {
        SegmentationAdapterPtr seg = segmentationPtr(item);
        //   qDebug() << "Remove Segmentation:" << seg->data(Qt::DisplayRole).toString();
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
  QSettings settings(CESVIMA, ESPINA);
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
  QSettings settings(CESVIMA, ESPINA);
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
  //Current implementation changes channel visibility and then
  //notifies it's been updated to other views
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
  //qDebug() << "Espina View Updating centers";
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
  }
  m_view3D->changePlanePosition(plane, dist);
}


// //-----------------------------------------------------------------------------
// void DefaultEspinaView::setCameraFocus(const Nm focus[3])
// {
//   volView->setCameraFocus(focus);
//   volView->forceRender();
// }
// 
// //-----------------------------------------------------------------------------
// void DefaultEspinaView::setSliceSelectors(SliceView::SliceSelectors selectors)
// {
//   xyView->setSliceSelectors(selectors);
//   yzView->setSliceSelectors(selectors);
//   xzView->setSliceSelectors(selectors);
// }

//-----------------------------------------------------------------------------
void DefaultView::setFitToSlices(bool unused)
{
  NmVector3 step = m_viewXY->sceneResolution();

  m_viewXY->setSlicingStep(step);
  m_viewYZ->setSlicingStep(step);
  m_viewXZ->setSlicingStep(step);
}

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
