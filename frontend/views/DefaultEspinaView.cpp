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

#include "common/gui/SliceView.h"
#include "common/gui/TaxonomyColorEngine.h"
#include "common/gui/VolumeView.h"
#include "common/model/ModelItem.h"
#include "common/model/Channel.h"
#include "common/model/Segmentation.h"
#include "common/widgets/RectangularSelection.h"

#include <QDockWidget>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QSettings>

#include <QDir>
#include <QMenu>
#include <QApplication>
#include <QGroupBox>
#include <EspinaCore.h>

//----------------------------------------------------------------------------
DefaultEspinaView::DefaultEspinaView(QMainWindow* parent, const QString activity)
: EspinaView(parent, activity)
, m_colorEngine(NULL)
, m_showProcessing(false)
, m_showSegmentations(true)
{
  double cyan[3] = { 0, 1, 1 };
  double blue[3] = { 0, 0, 1 };
  double magenta[3] = { 1, 0, 1 };

  setObjectName("xyView");

  //   qDebug() << "New Default EspinaView";
  xyView = new SliceView(AXIAL);
  xzView = new SliceView(CORONAL);
  yzView = new SliceView(SAGITTAL);
  volView = new VolumeView(this);

  xyView->setCrosshairColors(blue, magenta);
  initSliceView(xyView);
  this->setLayout(new QVBoxLayout());
  this->layout()->addWidget(xyView);
  this->layout()->setMargin(0);

  volDock = new QDockWidget(tr("3D"), parent);
  volDock->setObjectName("volDock");
  connect(volView, SIGNAL(channelSelected(Channel*)),
          this, SLOT(channelSelected(Channel*)));
  connect(volView, SIGNAL(segmentationSelected(Segmentation*, bool)),
          this, SLOT(segmentationSelected(Segmentation*, bool)));
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
  connect(view, SIGNAL(focusChanged(const Nm[3])),
          this, SLOT(setCameraFocus(const Nm[3])));
  connect(view, SIGNAL(selectedFromSlice(double, PlaneType)),
          this, SLOT(selectFromSlice(double, PlaneType)));
  connect(view, SIGNAL(selectedToSlice(double, PlaneType)),
          this, SLOT(selectToSlice(double, PlaneType)));
  connect(view, SIGNAL(channelSelected(Channel*)),
          this, SLOT(channelSelected(Channel*)));
  connect(view, SIGNAL(segmentationSelected(Segmentation*, bool)),
          this, SLOT(segmentationSelected(Segmentation*, bool)));
  connect(view, SIGNAL(showCrosshairs(bool)),
          this, SLOT(showCrosshair(bool)));
  connect(view, SIGNAL(sliceChanged(PlaneType, Nm)), this, SLOT(changePlanePosition(PlaneType, Nm)));
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

  //TODO: Synchronize with maintoolbar action
  //QAction *toggleSegmentationsVisibility = new QAction(tr("Show Segmentations"),menu);
  //toggleSegmentationsVisibility->setCheckable(true);
  //toggleSegmentationsVisibility->setShortcut(QString("Space"));
  //connect(toggleSegmentationsVisibility, SIGNAL(triggered(bool)),
  //this, SLOT(showSegmentations(bool)));
  //menu->addAction(toggleSegmentationsVisibility);
  QSettings settings("CeSViMa", "EspINA");

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

//----------------------------------------------------------------------------
void DefaultEspinaView::restoreLayout()
{
  //   qDebug() << "Restore " << m_activity << volDock->objectName();
  QSettings settings("CeSViMa", "EspINA");

  m_window->restoreState(settings.value(m_activity + "/state").toByteArray());
  m_window->restoreGeometry(settings.value(m_activity + "/geometry").toByteArray());
}

//----------------------------------------------------------------------------
QSize DefaultEspinaView::sizeHint() const
{
  return QSize(500, 500);
}

//----------------------------------------------------------------------------
void DefaultEspinaView::saveLayout()
{
  //   qDebug() << "Save " << m_activity << volDock->objectName();
  QSettings settings("CeSViMa", "EspINA");

  settings.setValue(m_activity + "/state", m_window->saveState());
  settings.setValue(m_activity + "/geometry", m_window->saveGeometry());
}

//----------------------------------------------------------------------------
void DefaultEspinaView::forceRender()
{
  //QApplication::setOverrideCursor(Qt::WaitCursor);
  xyView->forceRender();
  yzView->forceRender();
  xzView->forceRender();
  volView->forceRender();
  //QApplication::restoreOverrideCursor();
}

//----------------------------------------------------------------------------
void DefaultEspinaView::resetCamera()
{
  xyView->resetCamera();
  yzView->resetCamera();
  xzView->resetCamera();
  volView->resetCamera();
}

//----------------------------------------------------------------------------
void DefaultEspinaView::slicingStep(Nm steps[3])
{
  memcpy(steps, m_slicingStep, 3 * sizeof(Nm));
}

//----------------------------------------------------------------------------
void DefaultEspinaView::setSlicingStep(Nm steps[3])
{
  xyView->setSlicingStep(steps);
  yzView->setSlicingStep(steps);
  xzView->setSlicingStep(steps);
  memcpy(m_slicingStep, steps, 3 * sizeof(Nm));
}

//----------------------------------------------------------------------------
void DefaultEspinaView::addWidget(EspinaWidget* widget)
{
  Widgtes widgets;
  widgets.xy  = widget->createSliceWidget(AXIAL);
  widgets.yz  = widget->createSliceWidget(SAGITTAL);
  widgets.xz  = widget->createSliceWidget(CORONAL);
  //widgets.vol = widget->createWidget();

  xyView->addWidget (widgets.xy);
  yzView->addWidget (widgets.yz);
  xzView->addWidget (widgets.xz);
  //volView->addWidget(widgets.vol);

  m_widgets[widget] = widgets;

  forceRender();
}

//----------------------------------------------------------------------------
void DefaultEspinaView::removeWidget(EspinaWidget* widget)
{
  Widgtes widgets = m_widgets[widget];

  xyView->removeWidget (widgets.xy);
  yzView->removeWidget (widgets.yz);
  xzView->removeWidget (widgets.xz);
  //volView->removeWidget(widgets.vol);

  m_widgets.remove(widget);
}

//----------------------------------------------------------------------------
void DefaultEspinaView::setColorEngine(ColorEngine* engine)
{
  m_colorEngine = engine;
  xyView->setColorEngine(m_colorEngine);
  yzView->setColorEngine(m_colorEngine);
  xzView->setColorEngine(m_colorEngine);
  volView->setColorEngine(m_colorEngine);
}

//----------------------------------------------------------------------------
ISettingsPanel* DefaultEspinaView::settingsPanel()
{
  return new SettingsPanel(xyView->settings(), yzView->settings(), xzView->settings(), volView->settings());
}

//----------------------------------------------------------------------------
void DefaultEspinaView::showCrosshair(bool visible)
{
  xyView->setCrosshairVisibility(visible);
  yzView->setCrosshairVisibility(visible);
  xzView->setCrosshairVisibility(visible);
}

//----------------------------------------------------------------------------
void DefaultEspinaView::switchPreprocessing()
{
  //Current implementation changes channel visibility and then
  //notifies it's been updated to other views
  m_showProcessing = !m_showProcessing;
  xyView->setShowPreprocessing(m_showProcessing);
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
  QSettings settings("CeSViMa", "EspINA");
  settings.setValue("ShowThumbnail", m_showThumbnail->isChecked());
  xyView->setThumbnailVisibility(visible);
  yzView->setThumbnailVisibility(visible);
  xzView->setThumbnailVisibility(visible);
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
  memcpy(m_crosshairPoint, point, 3*sizeof(Nm));
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::setCameraFocus(const Nm focus[3])
{
  volView->setCameraFocus(focus);
  volView->forceRender();
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::setSliceSelectors(SliceView::SliceSelectors selectors)
{
  xyView->setSliceSelectors(selectors);
  yzView->setSliceSelectors(selectors);
  xzView->setSliceSelectors(selectors);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::addChannelRepresentation(Channel* channel)
{
  xyView->addChannelRepresentation(channel);
  yzView->addChannelRepresentation(channel);
  xzView->addChannelRepresentation(channel);
  volView->addChannelRepresentation(channel);
  connect(channel, SIGNAL(modified(ModelItem*)),
	  this, SLOT(updateSceneRanges()));
  m_channels << channel;
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::removeChannelRepresentation(Channel* channel)
{
  xyView->removeChannelRepresentation(channel);
  yzView->removeChannelRepresentation(channel);
  xzView->removeChannelRepresentation(channel);
  volView->removeChannelRepresentation(channel);
  disconnect(channel, SIGNAL(modified(ModelItem*)),
	  this, SLOT(updateSceneRanges()));
  m_channels.removeAll(channel);
}

//-----------------------------------------------------------------------------
bool DefaultEspinaView::updateChannel(Channel* channel)
{
  bool modified = false;
  modified = xyView->updateChannelRepresentation(channel) || modified;
  modified = yzView->updateChannelRepresentation(channel) || modified;
  modified = xzView->updateChannelRepresentation(channel) || modified;
  modified = volView->updateChannelRepresentation(channel) || modified;

  return modified;
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::addSegmentation(Segmentation* seg)
{
  xyView->addSegmentationRepresentation(seg);
  yzView->addSegmentationRepresentation(seg);
  xzView->addSegmentationRepresentation(seg);
  volView->addSegmentationRepresentation(seg);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::removeSegmentation(Segmentation* seg)
{
  xyView->removeSegmentationRepresentation(seg);
  yzView->removeSegmentationRepresentation(seg);
  xzView->removeSegmentationRepresentation(seg);
  volView->removeSegmentationRepresentation(seg);
}

//-----------------------------------------------------------------------------
bool DefaultEspinaView::updateSegmentation(Segmentation* seg)
{
  bool modified = false;
  modified = xyView->updateSegmentationRepresentation(seg) || modified;
  modified = yzView->updateSegmentationRepresentation(seg) || modified;
  modified = xzView->updateSegmentationRepresentation(seg) || modified;
  modified = volView->updateSegmentationRepresentation(seg) || modified;

  return modified;
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::rowsInserted(const QModelIndex& parent, int start, int end)
{
  if (!parent.isValid())
    return;

  bool render = false;
  for (int child = start; child <= end; child++)
  {
    QModelIndex index = parent.child(child, 0);
    ModelItem *item = indexPtr(index);
    switch (item->type())
    {
      case ModelItem::CHANNEL:
      {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        Q_ASSERT(start == end);
        // Only 1-row-at-a-time insertions are allowed
        Channel *channel = dynamic_cast<Channel *>(item);
        //       item.dynamicCast<ChannelPtr>();
        // 	qDebug() << "Add Channel:" << channel->data(Qt::DisplayRole).toString();

        addChannelRepresentation(channel);
	updateSceneRanges();
        QApplication::restoreOverrideCursor();
        resetCamera();
        render = true;
        break;
      }
      case ModelItem::SEGMENTATION:
      {
        Segmentation *seg = dynamic_cast<Segmentation *>(item);
        // 	qDebug() << "Add Segmentation:" << seg->data(Qt::DisplayRole).toString();
        addSegmentation(seg);
        render = m_showSegmentations;
        break;
      }
      default:
        break;
    };
  }
  if (render)
    forceRender();
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
  if (!parent.isValid())
    return;
  QSharedPointer<EspinaModel> model = EspinaCore::instance()->model();

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
        // 	qDebug() << "Remove Channel:" << channel->data(Qt::DisplayRole).toString();
        removeChannelRepresentation(channel);
	updateSceneRanges();

        break;
      }
      case ModelItem::SEGMENTATION:
      {
        Segmentation *seg = dynamic_cast<Segmentation *>(item);
        // 	qDebug() << "Remove Segmentation:" << seg->data(Qt::DisplayRole).toString();
        removeSegmentation(seg);
        break;
      }
      default:
        break;
    };
  }
  forceRender();
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
      forceRender();
  }
  else if (ModelItem::SEGMENTATION == item->type())
  {
    Segmentation *seg = dynamic_cast<Segmentation *>(item);
    if (updateSegmentation(seg))
      forceRender();
  }
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::setFitToSlices(bool fit)
{
  Nm step[3] = {1.0, 1.0, 1.0};
  if (fit)
    memcpy(step, m_slicingStep, 3*sizeof(Nm));

  xyView->setSlicingStep(step);
  yzView->setSlicingStep(step);
  xzView->setSlicingStep(step);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::setRulerVisibility(bool visible)
{
  QSettings settings("CeSViMa", "EspINA");
  settings.setValue("ShowRuler", m_showRuler->isChecked() );
  xyView->setRulerVisibility(visible);
  yzView->setRulerVisibility(visible);
  xzView->setRulerVisibility(visible);
  forceRender();
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::selectFromSlice(double slice, PlaneType plane)
{
  emit selectedFromSlice(slice, plane);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::selectToSlice(double slice, PlaneType plane)
{
  emit selectedToSlice(slice, plane);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::channelSelected(Channel* channel)
{
  SelectionManager::Selection selection;

  selection << channel;

  SelectionManager::instance()->setSelection(selection);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::segmentationSelected(Segmentation* seg, bool append)
{
  SelectionManager::Selection selection;

  if (append)
    selection = SelectionManager::instance()->selection();

  if (!selection.contains(seg))
    selection << seg;

  SelectionManager::instance()->setSelection(selection);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::updateSceneRanges()
{
  double spacing[3];
  double minSpacing[3] = {1, 1, 1};
  double bounds[6];
  double ranges[6] = { 0, 1, 0, 1, 0, 1};

  Channel *channel;
  for(int c = 0; c < m_channels.size(); c++)
  {
    channel = m_channels[c];
    if (0 == c)
    {
      channel->spacing(minSpacing);
      channel->bounds(ranges);
    }else
    {
      channel->spacing(spacing);
      channel->bounds(bounds);
      for (int i = 0; i < 3; i++)
      {
	minSpacing[i] = std::min(minSpacing[i], spacing[i]);
	ranges[i] = std::min(ranges[i], bounds[i]);
	ranges[2*i+1] = std::max(ranges[2*i+1], bounds[2*i+1]);
      }
    }
  }

  setSlicingStep(minSpacing);
  xyView->setSlicingRanges(ranges);
  yzView->setSlicingRanges(ranges);
  xzView->setSlicingRanges(ranges);
}

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
  return m_xyPanel->modified() || m_yzPanel->modified() || m_xzPanel->modified() || m_volPanel->modified();
}

//-----------------------------------------------------------------------------
ISettingsPanel* DefaultEspinaView::SettingsPanel::clone()
{
  return new SettingsPanel(m_xy, m_yz, m_xz, m_vol);
}

//-----------------------------------------------------------------------------
void DefaultEspinaView::changePlanePosition(PlaneType plane, Nm dist)
{
  volView->changePlanePosition(plane, dist);
  volView->forceRender();
}
