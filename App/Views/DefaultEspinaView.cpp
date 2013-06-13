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

#include "DefaultEspinaView.h"
#include <Settings/SliceView/SliceViewSettingsPanel.h>
#include <Settings/VolumeView/VolumeViewSettingsPanel.h>
#include <Menus/SegmentationContextualMenu.h>

#include <QDebug>

// EspINA
#include <GUI/QtWidget/SliceView.h>
#include <GUI/QtWidget/VolumeView.h>
#include <Core/EspinaSettings.h>
#include <Core/Model/ModelItem.h>
#include <Core/Model/Channel.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Segmentation.h>

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

//----------------------------------------------------------------------------
DefaultEspinaView::DefaultEspinaView(EspinaModel *model,
                                     QUndoStack  *undoStack,
                                     ViewManager *viewManager,
                                     QMainWindow *parent)
: QAbstractItemView(parent)
, m_model(model)
, m_viewManager(viewManager)
, m_showProcessing(false)
, m_showSegmentations(true)
, m_contextMenu(new DefaultContextualMenu(SegmentationList(), model, undoStack, viewManager))
{
  double cyan[3] = { 0, 1, 1 };
  double blue[3] = { 0, 0, 1 };
  double magenta[3] = { 1, 0, 1 };

  setObjectName("xyView");



  //   qDebug() << "New Default EspinaView";
  xyView = new SliceView(m_model->factory(), viewManager, AXIAL);
  xzView = new SliceView(m_model->factory(), viewManager, CORONAL);
  yzView = new SliceView(m_model->factory(), viewManager, SAGITTAL);
  volView = new VolumeView(m_model->factory(), viewManager, false, this);
  volView->setViewType(VOLUME);

  xyView->setCrosshairColors(blue, magenta);
  initSliceView(xyView);
  this->setLayout(new QVBoxLayout());
  this->layout()->addWidget(xyView);
  this->layout()->setMargin(0);

  volDock = new QDockWidget(tr("3D"), parent);
  volDock->setObjectName("volDock");
  volDock->setWidget(volView);
  connect(volView, SIGNAL(centerChanged(Nm, Nm, Nm)),
          this, SLOT(setCrosshairPoint(Nm,Nm,Nm)));

  yzDock = new QDockWidget(tr("ZY"), parent);
  yzDock->setObjectName("yzDock");
  yzView->setCrosshairColors(blue, cyan);
  initSliceView(yzView);
  yzDock->setWidget(yzView);

  xzDock = new QDockWidget(tr("XZ"), parent);
  xzDock->setObjectName("xzDock");
  xzView->setCrosshairColors(cyan, magenta);
  initSliceView(xzView);
  xzDock->setWidget(xzView);

  m_settingsPanel = ISettingsPanelPrototype(
    new SettingsPanel(xyView->settings(),
                      yzView->settings(),
                      xzView->settings(),
                      volView->settings(),
                      m_model->factory())
  );

  parent->addDockWidget(Qt::RightDockWidgetArea, volDock);
  parent->addDockWidget(Qt::RightDockWidgetArea, yzDock);
  parent->addDockWidget(Qt::RightDockWidgetArea, xzDock);

  parent->setCentralWidget(this);

  setModel(m_model);
}

//-----------------------------------------------------------------------------
DefaultEspinaView::~DefaultEspinaView()
{
//   qDebug() << "Destroy Default EspINA View";
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::initSliceView(SliceView* view)
{
  connect(view, SIGNAL(centerChanged(Nm, Nm, Nm)),
          this, SLOT(setCrosshairPoint(Nm,Nm,Nm)));
//   connect(view, SIGNAL(focusChanged(const Nm[3])),
//           this, SLOT(setCameraFocus(const Nm[3])));
//   connect(view, SIGNAL(selectedFromSlice(double, PlaneType)),
//           this, SLOT(selectFromSlice(double, PlaneType)));
//   connect(view, SIGNAL(selectedToSlice(double, PlaneType)),
//           this, SLOT(selectToSlice(double, PlaneType)));
  connect(view, SIGNAL(sliceChanged(PlaneType, Nm)),
          this, SLOT(changePlanePosition(PlaneType, Nm)));
  view->setContextualMenu(m_contextMenu);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::createViewMenu(QMenu* menu)
{
  QMenu *renderMenu = new QMenu(tr("Views"), this);
  renderMenu->addAction(yzDock ->toggleViewAction());
  renderMenu->addAction(xzDock ->toggleViewAction());
  renderMenu->addAction(volDock->toggleViewAction());
  //renderMenu->addSeparator();
  menu->addMenu(renderMenu);

  QSettings settings(CESVIMA, ESPINA);

  bool sr = settings.value("ShowRuler", true).toBool();
  bool st = settings.value("ShowThumbnail", true).toBool();

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
void DefaultEspinaView::setModel(QAbstractItemModel* model)
{
  QAbstractItemView::setModel(model);
  connect(model, SIGNAL(modelAboutToBeReset()),
          this, SLOT(sourceModelReset()));
}

//----------------------------------------------------------------------------
ISettingsPanelPtr DefaultEspinaView::settingsPanel()
{
  return m_settingsPanel.get();
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::addChannel(ChannelPtr channel)
{
  xyView->addChannel(channel);
  yzView->addChannel(channel);
  xzView->addChannel(channel);
  volView->addChannel(channel);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::removeChannel(ChannelPtr channel)
{
  xyView->removeChannel(channel);
  yzView->removeChannel(channel);
  xzView->removeChannel(channel);
  volView->removeChannel(channel);
}

//-----------------------------------------------------------------------------
bool DefaultEspinaView::updateChannel(ChannelPtr channel)
{
  bool modified = false;
  modified |= xyView ->updateChannelRepresentation(channel);
  modified |= yzView ->updateChannelRepresentation(channel);
  modified |= xzView ->updateChannelRepresentation(channel);
  modified |= volView->updateChannelRepresentation(channel);
  return modified;
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::addSegmentation(SegmentationPtr seg)
{
  xyView->addSegmentation(seg);
  yzView->addSegmentation(seg);
  xzView->addSegmentation(seg);
  volView->addSegmentation(seg);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::removeSegmentation(SegmentationPtr seg)
{
  xyView->removeSegmentation(seg);
  yzView->removeSegmentation(seg);
  xzView->removeSegmentation(seg);
  volView->removeSegmentation(seg);

}

//-----------------------------------------------------------------------------
bool DefaultEspinaView::updateSegmentation(SegmentationPtr seg)
{
  bool modified = false;
  modified |= xyView->updateSegmentationRepresentation(seg);
  modified |= yzView->updateSegmentationRepresentation(seg);
  modified |= xzView->updateSegmentationRepresentation(seg);
  modified |= volView->updateSegmentationRepresentation(seg);
  return modified;
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::rowsInserted(const QModelIndex& parent, int start, int end)
{
  if (!parent.isValid())
    return;

  for (int child = start; child <= end; child++)
  {
    QModelIndex index = parent.child(child, 0);
    ModelItemPtr item = indexPtr(index);
    switch (item->type())
    {
      case EspINA::CHANNEL:
      {
        Q_ASSERT(start == end);
        // Only 1-row-at-a-time insertions are allowed
        ChannelPtr channel = channelPtr(item);
        //       item.dynamicCast<ChannelPtr>();
        //   qDebug() << "Add Channel:" << channel->data(Qt::DisplayRole).toString();

        addChannel(channel);
        break;
      }
      case EspINA::SEGMENTATION:
      {
        SegmentationPtr seg = segmentationPtr(item);
        //   qDebug() << "Add Segmentation:" << seg->data(Qt::DisplayRole).toString();
        addSegmentation(seg);
        break;
      }
      default:
        break;
    };
  }
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
  if (!parent.isValid())
    return;

  //   qDebug() << parent.data(Qt::DisplayRole).toString();
  for (int child = start; child <= end; child++)
  {
    QModelIndex index = parent.child(child, 0);
    ModelItemPtr item = indexPtr(index);
    switch (item->type())
    {
      case EspINA::CHANNEL:
      {
        ChannelPtr channel = channelPtr(item);
        //   qDebug() << "Remove Channel:" << channel->data(Qt::DisplayRole).toString();
        removeChannel(channel);
        break;
      }
      case EspINA::SEGMENTATION:
      {
        SegmentationPtr seg = segmentationPtr(item);
        //   qDebug() << "Remove Segmentation:" << seg->data(Qt::DisplayRole).toString();
        removeSegmentation(seg);
        break;
      }
      default:
        break;
    };
  }
}

//----------------------------------------------------------------------------
void DefaultEspinaView::sourceModelReset()
{
  xyView->reset();
  yzView->reset();
  xzView->reset();
  volView->reset();
}

//----------------------------------------------------------------------------
void DefaultEspinaView::showCrosshair(bool visible)
{
  xyView->showCrosshairs(visible);
  yzView->showCrosshairs(visible);
  xzView->showCrosshairs(visible);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::setRulerVisibility(bool visible)
{
  QSettings settings(CESVIMA, ESPINA);
  settings.setValue("ShowRuler", m_showRuler->isChecked() );
  xyView->setRulerVisibility(visible);
  yzView->setRulerVisibility(visible);
  xzView->setRulerVisibility(visible);
}

//----------------------------------------------------------------------------
void DefaultEspinaView::showSegmentations(bool visible)
{
  xyView->setSegmentationsVisibility(visible);
  yzView->setSegmentationsVisibility(visible);
  xzView->setSegmentationsVisibility(visible);

  m_showSegmentations = visible;
}
//-----------------------------------------------------------------------------
void DefaultEspinaView::showThumbnail(bool visible)
{
  QSettings settings(CESVIMA, ESPINA);
  settings.setValue("ShowThumbnail", m_showThumbnail->isChecked());
  xyView->setThumbnailVisibility(visible);
  yzView->setThumbnailVisibility(visible);
  xzView->setThumbnailVisibility(visible);
}

//----------------------------------------------------------------------------
void DefaultEspinaView::switchPreprocessing()
{
  //Current implementation changes channel visibility and then
  //notifies it's been updated to other views
  m_showProcessing = !m_showProcessing;
  xyView->setShowPreprocessing(m_showProcessing);
  yzView->setShowPreprocessing(m_showProcessing);
  xzView->setShowPreprocessing(m_showProcessing);
}

//----------------------------------------------------------------------------
void DefaultEspinaView::updateViews()
{
 xyView->updateView();
 yzView->updateView();
 xzView->updateView();
 volView->updateView();
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::setCrosshairPoint(Nm x, Nm y, Nm z, bool force)
{
  //qDebug() << "Espina View Updating centers";
  Nm point[3] = { x, y, z };
  xyView->centerViewOn(point, force);
  yzView->centerViewOn(point, force);
  xzView->centerViewOn(point, force);
  volView->centerViewOn(point, force);
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
void DefaultEspinaView::setFitToSlices(bool unused)
{
  const Nm *step = xyView->sceneResolution();

  xyView->setSlicingStep(step);
  yzView->setSlicingStep(step);
  xzView->setSlicingStep(step);
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

//-----------------------------------------------------------------------------
DefaultEspinaView::SettingsPanel::SettingsPanel(SliceView::SettingsSPtr xy,
                                                SliceView::SettingsSPtr yz,
                                                SliceView::SettingsSPtr xz,
                                                VolumeView::SettingsPtr vol,
                                                EspinaFactoryPtr factory)
: m_xy(xy)
, m_yz(yz)
, m_xz(xz)
, m_factory(factory)
, m_slicingStep(0)
, m_vol(vol)
{
  QVBoxLayout *layout = new QVBoxLayout();
  QGroupBox *group;
  QVBoxLayout *groupLayout;

  // Axial View
  m_xyPanel = new SliceViewSettingsPanel(xy);
  group = new QGroupBox(m_xyPanel->shortDescription());
  groupLayout = new QVBoxLayout();
  groupLayout->addWidget(m_xyPanel);
  group->setLayout(groupLayout);
  layout->addWidget(group);

  // Sagittal View
  m_yzPanel = new SliceViewSettingsPanel(yz);
  group = new QGroupBox(m_yzPanel->shortDescription());
  groupLayout = new QVBoxLayout();
  groupLayout->addWidget(m_yzPanel);
  group->setLayout(groupLayout);
  layout->addWidget(group);

  // Coronal View
  m_xzPanel = new SliceViewSettingsPanel(xz);
  group = new QGroupBox(m_xzPanel->shortDescription());
  groupLayout = new QVBoxLayout();
  groupLayout->addWidget(m_xzPanel);
  group->setLayout(groupLayout);
  layout->addWidget(group);

  // 3D View
  m_volPanel = new VolumeViewSettingsPanel(factory, vol);
  group = new QGroupBox(m_volPanel->shortDescription());
  groupLayout = new QVBoxLayout();
  groupLayout->addWidget(m_volPanel);
  group->setLayout(groupLayout);
  layout->addWidget(group);

  this->setLayout(layout);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::SettingsPanel::acceptChanges()
{
  m_xyPanel->acceptChanges();
  m_yzPanel->acceptChanges();
  m_xzPanel->acceptChanges();
  m_volPanel->acceptChanges();
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::SettingsPanel::rejectChanges()
{

}

//-----------------------------------------------------------------------------
bool DefaultEspinaView::SettingsPanel::modified() const
{
  return m_xyPanel->modified()
      || m_yzPanel->modified()
      || m_xzPanel->modified()
      || m_volPanel->modified();
}

//-----------------------------------------------------------------------------
ISettingsPanelPtr DefaultEspinaView::SettingsPanel::clone()
{
  return ISettingsPanelPtr(new SettingsPanel(m_xy, m_yz, m_xz, m_vol, m_factory));
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::changePlanePosition(PlaneType plane, Nm dist)
{
  switch(plane)
  {
    case AXIAL:
      this->yzView->updateCrosshairPoint(plane, dist);
      this->xzView->updateCrosshairPoint(plane, dist);
      break;
    case CORONAL:
      this->xyView->updateCrosshairPoint(plane, dist);
      this->yzView->updateCrosshairPoint(plane, dist);
      break;
    case SAGITTAL:
      this->xyView->updateCrosshairPoint(plane, dist);
      this->xzView->updateCrosshairPoint(plane, dist);
      break;
    default:
      Q_ASSERT(false);
      break;
  }
  volView->changePlanePosition(plane, dist);
}
