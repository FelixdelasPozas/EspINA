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
DefaultView::DefaultView(ModelAdapterSPtr model,
                         ViewManagerSPtr  viewManager,
                         QUndoStack      *undoStack,
                         QMainWindow     *parent)
: QAbstractItemView(parent)
, m_model(model)
, m_viewManager(viewManager)
, m_showProcessing(false)
, m_showSegmentations(true)
//, m_contextMenu(new DefaultContextualMenu(SegmentationList(), model, undoStack, viewManager))
{
  QSettings settins(CESVIMA, ESPINA);
  settins.beginGroup(DEFAULT_VIEW_SETTINGS);

  m_xLine = settins.value(X_LINE_COLOR, QColor(Qt::blue))   .value<QColor>();
  m_yLine = settins.value(Y_LINE_COLOR, QColor(Qt::magenta)).value<QColor>();
  m_zLine = settins.value(Z_LINE_COLOR, QColor(Qt::cyan))   .value<QColor>();

  settins.endGroup();

  //   qDebug() << "New Default EspinaView";
  viewXY = new View2D(Plane::XY);
  viewXZ = new View2D(Plane::XZ);
  viewYZ = new View2D(Plane::YZ);
  view3D = new View3D(false);


  setObjectName("viewXY");
  viewXY->setCrosshairColors(m_xLine, m_yLine);
  initView2D(viewXY);
  setLayout(new QVBoxLayout());
  layout()->addWidget(viewXY);
  layout()->setMargin(0);

  dockYZ = new QDockWidget(tr("ZY"), parent);
  dockYZ->setObjectName("DockZY");
  viewYZ->setCrosshairColors(m_yLine, m_zLine);
  initView2D(viewYZ);
  dockYZ->setWidget(viewYZ);

  dockXZ = new QDockWidget(tr("XZ"), parent);
  dockXZ->setObjectName("xzDock");
  viewXZ->setCrosshairColors(m_xLine, m_zLine);
  initView2D(viewXZ);
  dockXZ->setWidget(viewXZ);

  dock3D = new QDockWidget(tr("3D"), parent);
  dock3D->setObjectName("Dock3D");
  dock3D->setWidget(view3D);
  connect(view3D, SIGNAL(centerChanged(NmVector3)),
          this, SLOT(setCrosshairPoint(NmVector3)));

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

  viewXY->setCrosshairColors(m_xLine, m_yLine);
  viewXZ->setCrosshairColors(m_xLine, m_zLine);
  viewYZ->setCrosshairColors(m_yLine, m_zLine);
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
void DefaultView::add(ChannelAdapterPtr channel)
{
  viewXY->add(channel);
  viewYZ->add(channel);
  viewXZ->add(channel);
  view3D->add(channel);
}

//-----------------------------------------------------------------------------
void DefaultView::remove(ChannelAdapterPtr channel)
{
  viewXY->remove(channel);
  viewYZ->remove(channel);
  viewXZ->remove(channel);
  view3D->remove(channel);
}

//-----------------------------------------------------------------------------
bool DefaultView::updateRepresentation(ChannelAdapterPtr channel)
{
  bool modified = false;
  modified |= viewXY->updateRepresentation(channel);
  modified |= viewYZ->updateRepresentation(channel);
  modified |= viewXZ->updateRepresentation(channel);
  modified |= view3D->updateRepresentation(channel);
  return modified;
}

//-----------------------------------------------------------------------------
void DefaultView::add(SegmentationAdapterPtr seg)
{
  viewXY->add(seg);
  viewYZ->add(seg);
  viewXZ->add(seg);
  view3D->add(seg);
}

//-----------------------------------------------------------------------------
void DefaultView::remove(SegmentationAdapterPtr seg)
{
  viewXY->remove(seg);
  viewYZ->remove(seg);
  viewXZ->remove(seg);
  view3D->remove(seg);

}

//-----------------------------------------------------------------------------
bool DefaultView::updateRepresentation(SegmentationAdapterPtr seg)
{
  bool modified = false;
  modified |= viewXY->updateRepresentation(seg);
  modified |= viewYZ->updateRepresentation(seg);
  modified |= viewXZ->updateRepresentation(seg);
  modified |= view3D->updateRepresentation(seg);
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
        Q_ASSERT(start == end);
        // Only 1-row-at-a-time insertions are allowed
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
  viewXY->reset();
  viewYZ->reset();
  viewXZ->reset();
  view3D->reset();
}

//----------------------------------------------------------------------------
void DefaultView::showCrosshair(bool visible)
{
  viewXY->setCrosshairVisibility(visible);
  viewYZ->setCrosshairVisibility(visible);
  viewXZ->setCrosshairVisibility(visible);
}

//-----------------------------------------------------------------------------
void DefaultView::setRulerVisibility(bool visible)
{
  QSettings settings(CESVIMA, ESPINA);
  settings.beginGroup(DEFAULT_VIEW_SETTINGS);
  settings.setValue("ShowRuler", visible);
  settings.endGroup();

  viewXY->setRulerVisibility(visible);
  viewYZ->setRulerVisibility(visible);
  viewXZ->setRulerVisibility(visible);
}

//----------------------------------------------------------------------------
void DefaultView::showSegmentations(bool visible)
{
  viewXY->setSegmentationsVisibility(visible);
  viewYZ->setSegmentationsVisibility(visible);
  viewXZ->setSegmentationsVisibility(visible);

  m_showSegmentations = visible;
}
//-----------------------------------------------------------------------------
void DefaultView::showThumbnail(bool visible)
{
  QSettings settings(CESVIMA, ESPINA);
  settings.beginGroup(DEFAULT_VIEW_SETTINGS);
  settings.setValue("ShowThumbnail", visible);
  settings.endGroup();
  viewXY->setThumbnailVisibility(visible);
  viewYZ->setThumbnailVisibility(visible);
  viewXZ->setThumbnailVisibility(visible);
}

//----------------------------------------------------------------------------
void DefaultView::switchPreprocessing()
{
  //Current implementation changes channel visibility and then
  //notifies it's been updated to other views
  m_showProcessing = !m_showProcessing;
  viewXY->setShowPreprocessing(m_showProcessing);
  viewYZ->setShowPreprocessing(m_showProcessing);
  viewXZ->setShowPreprocessing(m_showProcessing);
}

//----------------------------------------------------------------------------
void DefaultView::updateViews()
{
 viewXY->updateView();
 viewYZ->updateView();
 viewXZ->updateView();
 view3D->updateView();
}

//-----------------------------------------------------------------------------
void DefaultView::setCrosshairPoint(const NmVector3& point, bool force)
{
  //qDebug() << "Espina View Updating centers";
  viewXY->centerViewOn(point, force);
  viewYZ->centerViewOn(point, force);
  viewXZ->centerViewOn(point, force);
  view3D->centerViewOn(point, force);
}

//-----------------------------------------------------------------------------
void DefaultView::changePlanePosition(Plane plane, Nm dist)
{
  switch(plane)
  {
    case Plane::XY:
      this->viewYZ->updateCrosshairPoint(plane, dist);
      this->viewXZ->updateCrosshairPoint(plane, dist);
      break;
    case Plane::XZ:
      this->viewXY->updateCrosshairPoint(plane, dist);
      this->viewYZ->updateCrosshairPoint(plane, dist);
      break;
    case Plane::YZ:
      this->viewXY->updateCrosshairPoint(plane, dist);
      this->viewXZ->updateCrosshairPoint(plane, dist);
      break;
  }
  view3D->changePlanePosition(plane, dist);
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
  NmVector3 step = viewXY->sceneResolution();

  viewXY->setSlicingStep(step);
  viewYZ->setSlicingStep(step);
  viewXZ->setSlicingStep(step);
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

// //-----------------------------------------------------------------------------
// DefaultView::SettingsPanel::SettingsPanel(SliceView::SettingsSPtr xy,
//                                                 SliceView::SettingsSPtr yz,
//                                                 SliceView::SettingsSPtr xz,
//                                                 VolumeView::SettingsPtr vol,
//                                                 EspinaFactoryPtr factory)
// : m_xy(xy)
// , m_yz(yz)
// , m_xz(xz)
// , m_factory(factory)
// , m_slicingStep(0)
// , m_vol(vol)
// {
//   QVBoxLayout *layout = new QVBoxLayout();
//   QGroupBox *group;
//   QVBoxLayout *groupLayout;
// 
//   // Axial View
//   m_xyPanel = new SliceViewSettingsPanel(xy);
//   group = new QGroupBox(m_xyPanel->shortDescription());
//   groupLayout = new QVBoxLayout();
//   groupLayout->addWidget(m_xyPanel);
//   group->setLayout(groupLayout);
//   layout->addWidget(group);
// 
//   // Sagittal View
//   m_yzPanel = new SliceViewSettingsPanel(yz);
//   group = new QGroupBox(m_yzPanel->shortDescription());
//   groupLayout = new QVBoxLayout();
//   groupLayout->addWidget(m_yzPanel);
//   group->setLayout(groupLayout);
//   layout->addWidget(group);
// 
//   // Coronal View
//   m_xzPanel = new SliceViewSettingsPanel(xz);
//   group = new QGroupBox(m_xzPanel->shortDescription());
//   groupLayout = new QVBoxLayout();
//   groupLayout->addWidget(m_xzPanel);
//   group->setLayout(groupLayout);
//   layout->addWidget(group);
// 
//   // 3D View
//   m_volPanel = new VolumeViewSettingsPanel(factory, vol);
//   group = new QGroupBox(m_volPanel->shortDescription());
//   groupLayout = new QVBoxLayout();
//   groupLayout->addWidget(m_volPanel);
//   group->setLayout(groupLayout);
//   layout->addWidget(group);
// 
//   this->setLayout(layout);
// }
// 
// //-----------------------------------------------------------------------------
// void DefaultView::SettingsPanel::acceptChanges()
// {
//   m_xyPanel->acceptChanges();
//   m_yzPanel->acceptChanges();
//   m_xzPanel->acceptChanges();
//   m_volPanel->acceptChanges();
// }
// 
// //-----------------------------------------------------------------------------
// void DefaultView::SettingsPanel::rejectChanges()
// {
// 
// }
// 
// //-----------------------------------------------------------------------------
// bool DefaultView::SettingsPanel::modified() const
// {
//   return m_xyPanel->modified()
//       || m_yzPanel->modified()
//       || m_xzPanel->modified()
//       || m_volPanel->modified();
// }
// 
// //-----------------------------------------------------------------------------
// ISettingsPanelPtr DefaultView::SettingsPanel::clone()
// {
//   return ISettingsPanelPtr(new SettingsPanel(m_xy, m_yz, m_xz, m_vol, m_factory));
// }
