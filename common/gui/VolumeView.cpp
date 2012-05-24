/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#include "VolumeView.h"

// Debug
#include <QDebug>

// EspINA
#include <common/model/Segmentation.h>
#include <common/views/pqVolumeView.h>

// GUI
#include <QEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

// ParaView
#include <pqActiveObjects.h>
#include <pqApplicationCore.h>
#include <pqObjectBuilder.h>
#include <pqServer.h>
#include <pqServerManagerObserver.h>
#include <pqViewExporterManager.h>

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSMRenderViewProxy.h>
#include <pqDisplayPolicy.h>
#include <pqDataRepresentation.h>
#include <pqPipelineRepresentation.h>
#include <pqScalarsToColors.h>
#include <pq3DWidget.h>
#include <pqOutputPort.h>
#include "ColorEngine.h"
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxyProperty.h>
#include <pqRenderView.h>
#include <pqPipelineSource.h>
#include <vtkSMProxyManager.h>
#include <vtkSMRepresentationProxy.h>
#include <pqSMAdaptor.h>
#include <model/Channel.h>
#include <model/Representation.h>
#include <QMouseEvent>
#include <vtkPropPicker.h>
#include <vtkVolumetricRepresentation.h>
#include <vtkCrosshairRepresentation.h>

//-----------------------------------------------------------------------------
VolumeView::VolumeView(QWidget* parent)
: QWidget(parent)
, m_mainLayout      (new QVBoxLayout())
, m_controlLayout   (new QHBoxLayout())
// , m_VOIWidget(NULL)
// , m_lastSample(NULL)
{
  buildControls();

  setLayout(m_mainLayout);
  //   connect(SelectionManager::instance(),SIGNAL(VOIChanged(IVOI*)),this,SLOT(setVOI(IVOI*)));

  // Color background
  QPalette pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Window));
  this->setPalette(pal);
  //   this->setStyleSheet("background-color: grey;");

  pqServerManagerObserver *SMObserver = pqApplicationCore::instance()->getServerManagerObserver();
  connect(SMObserver, SIGNAL(connectionCreated(vtkIdType)),
	  this, SLOT(onConnect()));
  connect(SMObserver, SIGNAL(connectionClosed(vtkIdType)),
	  this, SLOT(onDisconnect()));

  if (pqApplicationCore::instance()->getActiveServer())
    onConnect();
}

//-----------------------------------------------------------------------------
void VolumeView::buildControls()
{
  m_controlLayout = new QHBoxLayout();
  m_controlLayout->addStretch();

  m_snapshot.setIcon(QIcon(":/espina/snapshot_scene.svg"));
  m_snapshot.setToolTip(tr("Save Scene as Image"));
  m_snapshot.setFlat(true);
  m_snapshot.setIconSize(QSize(22,22));
  m_snapshot.setMaximumSize(QSize(32,32));
  connect(&m_snapshot,SIGNAL(clicked(bool)),this,SLOT(takeSnapshot()));

  m_export.setIcon(QIcon(":/espina/export_scene.svg"));
  m_export.setToolTip(tr("Export 3D Scene"));
  m_export.setFlat(true);
  m_export.setIconSize(QSize(22,22));
  m_export.setMaximumSize(QSize(32,32));
  connect(&m_export,SIGNAL(clicked(bool)),this,SLOT(exportScene()));
  
  QSpacerItem * horizontalSpacer = new QSpacerItem(4000, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

  m_controlLayout->addWidget(&m_snapshot);
  m_controlLayout->addWidget(&m_export);
  m_controlLayout->addItem(horizontalSpacer);

  m_mainLayout->addLayout(m_controlLayout);
}


//-----------------------------------------------------------------------------
void VolumeView::centerViewOn(double center[3])
{
  if (m_center[0] == center[0] &&
      m_center[1] == center[1] &&
      m_center[2] == center[2])
    return;

  memcpy(m_center, center, 3*sizeof(double));

  m_view->setCrosshairCenter(m_center[0], m_center[1], m_center[2]);
}

//-----------------------------------------------------------------------------
void VolumeView::setCameraFocus(double center[3])
{
  m_view->setCameraFocus(m_center[0], m_center[1], m_center[2]);
}

//-----------------------------------------------------------------------------
void VolumeView::resetCamera()
{
  m_view->resetCamera();
}

//-----------------------------------------------------------------------------
void VolumeView::addChannelRepresentation(Channel* channel)
{
  pqData volume = channel->representation("Volumetric")->output();
  pqOutputPort      *oport = volume.outputPort();
  pqPipelineSource *source = oport->getSource();
  vtkSMProxyManager   *pxm = vtkSMProxyManager::GetProxyManager();

  vtkSMRepresentationProxy* repProxy = vtkSMRepresentationProxy::SafeDownCast(
    pxm->NewProxy("representations", "CrosshairRepresentation"));
  Q_ASSERT(repProxy);
  m_channels[channel].proxy = repProxy;

  // Set repProxy's input.
  pqSMAdaptor::setInputProperty(repProxy->GetProperty("Input"),
				source->getProxy(), oport->getPortNumber());
  updateChannelRepresentation(channel);
//   int pos[3];
//   channel->position(pos);
//   vtkSMPropertyHelper(repProxy, "Position").Set(pos,3);
//   double color = channel->color();
//   vtkSMPropertyHelper(repProxy, "Color").Set(&color,1);
//   repProxy->UpdateVTKObjects();

  vtkSMProxy* viewModuleProxy = m_view->getProxy();
  // Add the reprProxy to render module.
  pqSMAdaptor::addProxyProperty(
    viewModuleProxy->GetProperty("Representations"), repProxy);
  viewModuleProxy->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void VolumeView::removeChannelRepresentation(Channel* channel)
{
  vtkSMProxy* viewModuleProxy = m_view->getProxy();
  Q_ASSERT(m_channels.contains(channel));
  vtkSMRepresentationProxy *repProxy = m_channels[channel].proxy;
  // Remove the reprProxy to render module.
  pqSMAdaptor::removeProxyProperty(
    viewModuleProxy->GetProperty("Representations"), repProxy);
  viewModuleProxy->UpdateVTKObjects();
  m_view->getProxy()->UpdateVTKObjects();
  repProxy->Delete();
  m_channels.remove(channel);

  m_view->resetCamera();
}

//-----------------------------------------------------------------------------
bool VolumeView::updateChannelRepresentation(Channel* channel)
{
  Q_ASSERT(m_channels.contains(channel));
  Representation &rep = m_channels[channel];

  double pos[3];
  channel->position(pos);
  //TODO: update if position changes
  if (channel->isVisible() != rep.visible)
  {
    rep.visible  = channel->isVisible();

    vtkSMPropertyHelper(rep.proxy, "Position").Set(pos,3);
    double color = channel->color();
    vtkSMPropertyHelper(rep.proxy, "Color").Set(&color,1);
    vtkSMPropertyHelper(rep.proxy, "Visibility").Set(rep.visible);
    double opacity = suggestedChannelOpacity();
    vtkSMPropertyHelper(rep.proxy, "Opacity").Set(&opacity,1);
    rep.proxy->UpdateVTKObjects();
    return true;
  }
  return false;
}


//-----------------------------------------------------------------------------
void VolumeView::addSegmentationRepresentation(Segmentation *seg)
{
  pqOutputPort      *oport = seg->outputPort();
  pqPipelineSource *source = oport->getSource();
  vtkSMProxyManager   *pxm = vtkSMProxyManager::GetProxyManager();

  vtkSMRepresentationProxy* repProxy = vtkSMRepresentationProxy::SafeDownCast(
    pxm->NewProxy("representations", "VolumetricRepresentation"));
  Q_ASSERT(repProxy);
  m_segmentations[seg].outport = oport;
  m_segmentations[seg].proxy = repProxy;
  m_segmentations[seg].selected = !seg->selected();
  m_segmentations[seg].visible  = seg->visible();
  m_segmentations[seg].color  = m_colorEngine->color(seg);

  // Set the reprProxy's input.
  pqSMAdaptor::setInputProperty(repProxy->GetProperty("Input"),
				source->getProxy(), oport->getPortNumber());

  updateSegmentationRepresentation(seg);

  vtkSMProxy* viewModuleProxy = m_view->getProxy();
  // Add the reprProxy to render module.
  pqSMAdaptor::addProxyProperty(
    viewModuleProxy->GetProperty("Representations"), repProxy);
  viewModuleProxy->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void VolumeView::removeSegmentationRepresentation(Segmentation* seg)
{
  vtkSMProxy* viewModuleProxy = m_view->getProxy();
  Q_ASSERT(m_segmentations.contains(seg));
  Representation rep = m_segmentations[seg];
  // Remove the reprProxy to render module.
  pqSMAdaptor::removeProxyProperty(
    viewModuleProxy->GetProperty("Representations"), rep.proxy);
  viewModuleProxy->UpdateVTKObjects();
  m_view->getProxy()->UpdateVTKObjects();

  rep.proxy->Delete();
  m_segmentations.remove(seg);
}

//-----------------------------------------------------------------------------
bool VolumeView::updateSegmentationRepresentation(Segmentation* seg)
{
  Q_ASSERT(m_segmentations.contains(seg));
  Representation &rep = m_segmentations[seg];
  if (seg->outputPort() != rep.outport)
  {
    removeSegmentationRepresentation(seg);
    addSegmentationRepresentation(seg);
    return true;
  }
  if (seg->selected() != rep.selected
    || seg->visible() != rep.visible
    || seg->data(Qt::DecorationRole).value<QColor>() != rep.color)
  {
    rep.selected = seg->selected();
    rep.visible  = seg->visible();
    rep.color = seg->data(Qt::DecorationRole).value<QColor>();
    //   repProxy->PrintSelf(std::cout,vtkIndent(0));
    double rgb[3] = {rep.color.redF(), rep.color.greenF(), rep.color.blueF()};
    vtkSMPropertyHelper(rep.proxy, "Color").Set(rgb, 3);
    vtkSMPropertyHelper(rep.proxy, "Opacity").Set(rep.selected?1.0:0.7);
    vtkSMPropertyHelper(rep.proxy, "Visibility").Set(rep.visible);
    rep.proxy->UpdateVTKObjects();
    return true;
  }
  return false;
}


//-----------------------------------------------------------------------------
void VolumeView::addRepresentation(pqOutputPort* oport)
{
//   pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
//   pqDataRepresentation *dr = dp->setRepresentationVisibility(oport, m_view, true);
//   if (!dr)
//     return;
//   pqPipelineRepresentation *rep = qobject_cast<pqPipelineRepresentation *>(dr);
//   Q_ASSERT(rep);
//   rep->setRepresentation("Volume");
}

//-----------------------------------------------------------------------------
void VolumeView::addWidget(pq3DWidget* widget)
{
  widget->setView(m_view);
  widget->setWidgetVisible(true);
  widget->select();
  widget->setEnabled(false);
}

//-----------------------------------------------------------------------------
void VolumeView::removeWidget(pq3DWidget* widget)
{
  widget->setWidgetVisible(false);
  widget->deselect();
}


//-----------------------------------------------------------------------------
void VolumeView::onConnect()
{
//   qDebug() << this << ": Connecting to a new server";
  pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
  pqServer    *server = pqActiveObjects::instance().activeServer();

  m_view = qobject_cast<pqVolumeView *>(ob->createView(
             pqVolumeView::espinaRenderViewType(), server));

  m_viewWidget = m_view->getWidget();
  // We want to manage events on the view
  m_viewWidget->installEventFilter(this);
  m_mainLayout->insertWidget(0,m_viewWidget);//To preserver view order

  vtkSMRenderViewProxy *viewProxy = vtkSMRenderViewProxy::SafeDownCast(m_view->getProxy());

  // Change Render Window Interactor
  vtkRenderWindowInteractor *rwi = vtkRenderWindowInteractor::SafeDownCast(
    viewProxy->GetRenderWindow()->GetInteractor());
  Q_ASSERT(rwi);

  vtkInteractorStyleTrackballCamera *style = vtkInteractorStyleTrackballCamera::New();
  rwi->SetInteractorStyle(style);
  style->Delete();

  double black[3] = {0,0,0};
  viewProxy->GetRenderer()->SetBackground(black);
}

//-----------------------------------------------------------------------------
void VolumeView::onDisconnect()
{
//   pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
//    qDebug() << this << ": Disconnecting from server";
//   if (m_view)
//   {
//     m_mainLayout->removeWidget(m_viewWidget);
//     ob->destroy(m_view);
//     m_view = NULL;
//   }
}

//-----------------------------------------------------------------------------
void VolumeView::forceRender()
{
  if(isVisible())
  {
//     qDebug() << "Render 3D View";
    m_view->forceRender();
  }
}

//-----------------------------------------------------------------------------
double VolumeView::suggestedChannelOpacity()
{
  double numVisibleRep = 0;

  foreach(Channel *channel, m_channels.keys())
    if (channel->isVisible())
      numVisibleRep++;

  if (numVisibleRep == 0)
    return 0.0;

  return 1.0 /  numVisibleRep;
}

//-----------------------------------------------------------------------------
void VolumeView::selectPickedItems(bool append)
{
  vtkSMRenderViewProxy *view =
    vtkSMRenderViewProxy::SafeDownCast(m_view->getProxy());
  Q_ASSERT(view);
  vtkRenderer *renderer = view->GetRenderer();
  Q_ASSERT(renderer);
  vtkRenderWindowInteractor *rwi =
    vtkRenderWindowInteractor::SafeDownCast(
      view->GetRenderWindow()->GetInteractor());
  Q_ASSERT(rwi);

  int x, y;
  rwi->GetEventPosition(x, y);

  vtkPropPicker *propPicker = vtkPropPicker::New();
  if (propPicker->Pick(x, y, 0.1, renderer))
  {
    vtkProp3D *pickedProp = propPicker->GetProp3D();
    vtkObjectBase *object;

    foreach(Channel *channel, m_channels.keys())
    {
      vtkCrosshairRepresentation *rep;
      object = m_channels[channel].proxy->GetClientSideObject();
      rep = dynamic_cast<vtkCrosshairRepresentation *>(object);
      Q_ASSERT(rep);
      if (rep->GetCrosshairProp() == pickedProp)
      {
// 	qDebug() << "Channel" << channel->data(Qt::DisplayRole).toString() << "Selected";
	emit channelSelected(channel);
	return;
      }
    }

    foreach(Segmentation *seg, m_segmentations.keys())
    {
      vtkVolumetricRepresentation *rep;
      object = m_segmentations[seg].proxy->GetClientSideObject();
      rep = dynamic_cast<vtkVolumetricRepresentation *>(object);
      Q_ASSERT(rep);
      if (rep->GetVolumetricProp() == pickedProp)
      {
// 	qDebug() << "Segmentation" << seg->data(Qt::DisplayRole).toString() << "Selected";
	emit segmentationSelected(seg, append);
	return;
      }
    }
  }
}
//-----------------------------------------------------------------------------
bool VolumeView::eventFilter(QObject* caller, QEvent* e)
{
  return QObject::eventFilter(caller, e);

  if (e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent *me = static_cast<QMouseEvent*>(e);
    if (me->button() == Qt::LeftButton)
    {
      selectPickedItems(me->modifiers() == Qt::SHIFT);
    }
  }

  return QObject::eventFilter(caller, e);
}


//-----------------------------------------------------------------------------
void VolumeView::exportScene()
{
  pqViewExporterManager *exporter = new pqViewExporterManager();
  exporter->setView(m_view);
  QString fileName = QFileDialog::getSaveFileName(this,
     tr("Save Scene"), "", tr("3D Scene (*.x3d *.pov *.vrml)"));
  exporter->write(fileName);
  delete exporter;
}

void VolumeView::takeSnapshot()
{
  QString fileName = QFileDialog::getSaveFileName(this,
     tr("Save Scene"), "", tr("Image Files (*.jpg *.png)"));
  m_view->saveImage(1024,768,fileName);
}