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
#include "SliceViewSettingsPanel.h"
#include "VolumeViewSettingsPanel.h"

#include <QDebug>

// EspINA
#include "common/gui/SliceView.h"
#include "common/gui/VolumeView.h"
#include "common/model/ModelItem.h"
#include "common/model/Channel.h"
#include "common/model/EspinaModel.h"
#include "common/model/Segmentation.h"
#include "common/settings/EspinaSettings.h"
#include "common/widgets/RectangularSelection.h"

// Qt
#include <QApplication>
#include <QDir>
#include <QDockWidget>
#include <QGroupBox>
#include <QMainWindow>
#include <QSettings>
#include <QVBoxLayout>
#include <QMenu>

//----------------------------------------------------------------------------
DefaultEspinaView::DefaultEspinaView(EspinaModel* model, ViewManager* vm, QMainWindow *parent)
: QAbstractItemView(parent)
, m_showProcessing(false)
, m_showSegmentations(true)
{
  double cyan[3] = { 0, 1, 1 };
  double blue[3] = { 0, 0, 1 };
  double magenta[3] = { 1, 0, 1 };

  setObjectName("xyView");

  //   qDebug() << "New Default EspinaView";
  xyView = new SliceView(vm, AXIAL);
  xzView = new SliceView(vm, CORONAL);
  yzView = new SliceView(vm, SAGITTAL);
  volView = new VolumeView(vm, this);

  xyView->setCrosshairColors(blue, magenta);
  initSliceView(xyView);
  this->setLayout(new QVBoxLayout());
  this->layout()->addWidget(xyView);
  this->layout()->setMargin(0);

  volDock = new QDockWidget(tr("3D"), parent);
  volDock->setObjectName("volDock");

  //volDock->setModel(model);
  volDock->setWidget(volView);

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

  //   setColorEngine(new TaxonomyColorEngine());

  parent->addDockWidget(Qt::RightDockWidgetArea, volDock);
  parent->addDockWidget(Qt::RightDockWidgetArea, yzDock);
  parent->addDockWidget(Qt::RightDockWidgetArea, xzDock);

  parent->setCentralWidget(this);

  setModel(model);
}

//-----------------------------------------------------------------------------
DefaultEspinaView::~DefaultEspinaView()
{
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
  connect(view, SIGNAL(showCrosshairs(bool)),
          this, SLOT(showCrosshair(bool)));
  connect(view, SIGNAL(sliceChanged(PlaneType, Nm)),
          this, SLOT(changePlanePosition(PlaneType, Nm)));
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::createViewMenu(QMenu* menu)
{
  QMenu *renderMenu = new QMenu(tr("Views"), this);
  renderMenu->addAction(yzDock->toggleViewAction());
  renderMenu->addAction(xzDock->toggleViewAction());
  renderMenu->addAction(volDock->toggleViewAction());
  //renderMenu->addSeparator();
  menu->addMenu(renderMenu);

//   //TODO: Synchronize with maintoolbar action
//   QAction *toggleSegmentationsVisibility = new QAction(tr("Show Segmentations"),menu);
//   toggleSegmentationsVisibility->setCheckable(true);
//   toggleSegmentationsVisibility->setShortcut(QString("Space"));
//   connect(toggleSegmentationsVisibility, SIGNAL(triggered(bool)),
//   this, SLOT(showSegmentations(bool)));
//   menu->addAction(toggleSegmentationsVisibility);

  QSettings settings(CESVIMA, ESPINA);

  bool sr = settings.value("ShowRuler", true).toBool();
  bool st = settings.value("ShowThumbnail", true).toBool();

  m_showRuler = new QAction(tr("Show Ruler"),menu);
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

  QAction *fitToSlices = new QAction(tr("Fit To Slices"), menu);
  fitToSlices->setCheckable(true);
  fitToSlices->setChecked(true);
  menu->addAction(fitToSlices);
  connect(fitToSlices, SIGNAL(toggled(bool)),
          this, SLOT(setFitToSlices(bool)));

  setRulerVisibility(sr);
  showThumbnail(st);
}

////----------------------------------------------------------------------------
//void DefaultEspinaView::resetCamera()
//{
//  xyView->resetCamera();
//  yzView->resetCamera();
//  xzView->resetCamera();
//  volView->resetCamera();
//}

// //----------------------------------------------------------------------------
// void DefaultEspinaView::addWidget(EspinaWidget* widget)
// {
//   Widgtes widgets;
//   widgets.xy  = widget->createSliceWidget(AXIAL);
//   widgets.yz  = widget->createSliceWidget(SAGITTAL);
//   widgets.xz  = widget->createSliceWidget(CORONAL);
//   //widgets.vol = widget->createWidget();
// 
//   xyView->addWidget (widgets.xy);
//   yzView->addWidget (widgets.yz);
//   xzView->addWidget (widgets.xz);
//   //volView->addWidget(widgets.vol);
// 
//   m_widgets[widget] = widgets;
// 
//   forceRender();
// }

// //----------------------------------------------------------------------------
// void DefaultEspinaView::removeWidget(EspinaWidget* widget)
// {
//   Widgtes widgets = m_widgets[widget];
// 
//   xyView->removeWidget (widgets.xy);
//   yzView->removeWidget (widgets.yz);
//   xzView->removeWidget (widgets.xz);
//   //volView->removeWidget(widgets.vol);
// 
//   m_widgets.remove(widget);
// }
/*
//----------------------------------------------------------------------------
void DefaultEspinaView::setColorEngine(ColorEngine* engine)
{
  m_colorEngine = engine;
  xyView->setColorEngine(m_colorEngine);
  yzView->setColorEngine(m_colorEngine);
  xzView->setColorEngine(m_colorEngine);
  volView->setColorEngine(m_colorEngine);
  forceRender();
}*/

//----------------------------------------------------------------------------
ISettingsPanel* DefaultEspinaView::settingsPanel()
{
  return new SettingsPanel(xyView->settings(), yzView->settings(), xzView->settings(), volView->settings());
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::addChannel(Channel* channel)
{
  xyView->addChannel(channel);
  yzView->addChannel(channel);
  xzView->addChannel(channel);
  volView->addChannel(channel);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::removeChannel(Channel* channel)
{
  xyView->removeChannel(channel);
  yzView->removeChannel(channel);
  xzView->removeChannel(channel);
  volView->removeChannel(channel);
}

//-----------------------------------------------------------------------------
bool DefaultEspinaView::updateChannel(Channel* channel)
{
  bool modified = false;
  modified |= xyView->updateChannel(channel);
  modified |= yzView->updateChannel(channel);
  modified |= xzView->updateChannel(channel);
  modified |= volView->updateChannel(channel);
  return modified;
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::addSegmentation(Segmentation* seg)
{
  xyView->addSegmentation(seg);
  yzView->addSegmentation(seg);
  xzView->addSegmentation(seg);
  volView->addSegmentation(seg);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::removeSegmentation(Segmentation* seg)
{
  xyView->removeSegmentation(seg);
  yzView->removeSegmentation(seg);
  xzView->removeSegmentation(seg);
  volView->removeSegmentation(seg);

}

//-----------------------------------------------------------------------------
bool DefaultEspinaView::updateSegmentation(Segmentation* seg)
{
  bool modified = false;
  modified |= xyView->updateSegmentation(seg);
  modified |= yzView->updateSegmentation(seg);
  modified |= xzView->updateSegmentation(seg);
  modified |= volView->updateSegmentation(seg);
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
    ModelItem *item = indexPtr(index);
    switch (item->type())
    {
      case ModelItem::CHANNEL:
      {
        Q_ASSERT(start == end);
        // Only 1-row-at-a-time insertions are allowed
        Channel *channel = dynamic_cast<Channel *>(item);
        //       item.dynamicCast<ChannelPtr>();
        //   qDebug() << "Add Channel:" << channel->data(Qt::DisplayRole).toString();

        addChannel(channel);
        break;
      }
      case ModelItem::SEGMENTATION:
      {
        Segmentation *seg = dynamic_cast<Segmentation *>(item);
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
    ModelItem *item = indexPtr(index);
    switch (item->type())
    {
      case ModelItem::CHANNEL:
      {
        Channel *channel = dynamic_cast<Channel *>(item);
        //   qDebug() << "Remove Channel:" << channel->data(Qt::DisplayRole).toString();
        removeChannel(channel);
        break;
      }
      case ModelItem::SEGMENTATION:
      {
        Segmentation *seg = dynamic_cast<Segmentation *>(item);
        //   qDebug() << "Remove Segmentation:" << seg->data(Qt::DisplayRole).toString();
        removeSegmentation(seg);
        break;
      }
      default:
        break;
    };
  }
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
  if (!topLeft.isValid() || !topLeft.parent().isValid())
    return;

  ModelItem *item = indexPtr(topLeft);
  if (ModelItem::CHANNEL == item->type())
  {
    Channel *channel = dynamic_cast<Channel *>(item);
    if (updateChannel(channel))
      updateViews();
  }
  else if (ModelItem::SEGMENTATION == item->type())
  {
    Segmentation *seg = dynamic_cast<Segmentation *>(item);
    if (updateSegmentation(seg))
      updateViews();
  }
}

//----------------------------------------------------------------------------
void DefaultEspinaView::showCrosshair(bool visible)
{
  xyView->setCrosshairVisibility(visible);
  yzView->setCrosshairVisibility(visible);
  xzView->setCrosshairVisibility(visible);
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
  xyView->setSegmentationVisibility(visible);
  yzView->setSegmentationVisibility(visible);
  xzView->setSegmentationVisibility(visible);
  //   EspinaCore::instance()->model()->serializeRelations(std::cout, RelationshipGraph::GRAPHVIZ);
  m_showSegmentations = visible;
}
//-----------------------------------------------------------------------------
void DefaultEspinaView::showThumbnail(bool visible)
{
  QSettings settings("CeSViMa", ESPINA);
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
  volView->centerViewOn(point);
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
void DefaultEspinaView::setFitToSlices(bool fit)
{
  Nm step[3] = {1.0, 1.0, 1.0};
  if (fit)
    memcpy(step, xyView->sceneResolution(), 3*sizeof(Nm));

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
DefaultEspinaView::SettingsPanel::SettingsPanel(SliceView::SettingsPtr xy, SliceView::SettingsPtr yz, SliceView::SettingsPtr xz, VolumeView::SettingsPtr vol) :
m_xy(xy), m_yz(yz), m_xz(xz), m_vol(vol)
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
  m_volPanel = new VolumeViewSettingsPanel(vol);
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
bool DefaultEspinaView::SettingsPanel::modified() const
{
  return m_xyPanel->modified()
      || m_yzPanel->modified()
      || m_xzPanel->modified()
      || m_volPanel->modified();
}

//-----------------------------------------------------------------------------
ISettingsPanel* DefaultEspinaView::SettingsPanel::clone()
{
  return new SettingsPanel(m_xy, m_yz, m_xz, m_vol);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::changePlanePosition(PlaneType plane, Nm dist)
{
  switch(plane)
  {
    case AXIAL:
      this->yzView->UpdateCrosshairPoint(plane, dist);
      this->xzView->UpdateCrosshairPoint(plane, dist);
      break;
    case CORONAL:
      this->xyView->UpdateCrosshairPoint(plane, dist);
      this->yzView->UpdateCrosshairPoint(plane, dist);
      break;
    case SAGITTAL:
      this->xyView->UpdateCrosshairPoint(plane, dist);
      this->xzView->UpdateCrosshairPoint(plane, dist);
      break;
    default:
      Q_ASSERT(false);
      break;
  }
  volView->changePlanePosition(plane, dist);
  volView->updateView();
}
