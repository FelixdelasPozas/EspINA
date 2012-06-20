/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.es>

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
// NOTE: vtkRenderer::RemoveAllViewProps()  maybe free the memory of the representations...
#include "common/gui/SliceView.h"

// #include "espina_debug.h"
//
// // EspINA
#include "common/settings/ISettingsPanel.h"
#include "common/model/Channel.h"
#include "common/model/Representation.h"
#include "common/model/Segmentation.h"
#include "common/processing/pqData.h"
#include "common/selection/SelectionManager.h"
#include "common/views/pqSliceView.h"
#include "common/views/vtkSMSliceViewProxy.h"
#include "ColorEngine.h"

// Debug
#include <QDebug>
// Qt includes
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QVector3D>
#include <QWheelEvent>

// ParaQ includes
#include <pq3DWidget.h>
#include <pqActiveObjects.h>
#include <pqApplicationCore.h>
#include <pqDataRepresentation.h>
#include <pqDisplayPolicy.h>
#include <pqObjectBuilder.h>
#include <pqOutputPort.h>
#include <pqPipelineRepresentation.h>
#include <pqPipelineSource.h>
#include <pqRenderView.h>
#include <pqSMAdaptor.h>
#include <pqServer.h>
#include <pqServerManagerModel.h>
#include <pqServerManagerObserver.h>
#include <pqTwoDRenderView.h>
#include <vtkInteractorStyleImage.h>
#include <vtkObjectFactory.h>
#include <vtkPropCollection.h>
#include <vtkPropCollection.h>
#include <vtkCommand.h>
#include <vtkPropPicker.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxyManager.h>
#include <vtkSMProxyManager.h>
#include <vtkSMRepresentationProxy.h>
#include <vtkSMTwoDRenderViewProxy.h>
#include <vtkWorldPointPicker.h>
#include <vtkCamera.h>
#include <vtkAbstractWidget.h>
#include <vtkSMNewWidgetRepresentationProxy.h>
#include <vtkWidgetRepresentation.h>
#include <vtkSMProperty.h>
#include <vtkPVGenericRenderWindowInteractor.h>
#include <vtkChannelRepresentation.h>


// class MouseMoveCallback : public vtkCommand
// {
//   static MouseMoveCallback *New()
//   {
//     return new MouseMoveCallback();
//   }
// 
// virtual void execute(vtkobject* caller, long unsigned int eventid, void* calldata)
// {
//   //vtkinteractorstyle *style = static_cast<vtkinteractorstyle *>(caller);
// 
//   switch(eventid)
//   {
//     case vtkcommand::mousemoveevent:
//       qdebug() << "moviendose";
//   }
// }
// };
//-----------------------------------------------------------------------------
// SLICE VIEW
//-----------------------------------------------------------------------------
SliceView::SliceView(vtkPVSliceView::VIEW_PLANE plane, QWidget* parent)
: QWidget           (parent)
, m_plane           (plane)
, m_titleLayout     (new QHBoxLayout())
, m_title           (new QLabel("Sagital"))
, m_mainLayout      (new QVBoxLayout())
, m_controlLayout   (new QHBoxLayout())
, m_viewWidget      (NULL)
, m_scrollBar       (new QScrollBar(Qt::Horizontal))
, m_fromSlice       (new QPushButton("From"))
, m_spinBox         (new QSpinBox())
, m_toSlice         (new QPushButton("To"))
, m_settings        (new Settings(m_plane))
, m_fitToGrid       (true)
, m_inThumbnail     (false)
{
  memset(m_gridSize, 1, 3*sizeof(double));
  memset(m_range, 0, 6*sizeof(double));

//   buildTitle(); 
//   m_viewWidget->setSizePolicy(
//        QSizePolicy::Expanding,
//        QSizePolicy::Expanding);
//   m_viewWidget->setStyleSheet("background-color: black;");
//   m_mainLayout->addWidget(m_viewWidget);

  buildControls();

  this->setAutoFillBackground(true);
  setLayout(m_mainLayout);

  // Color background
  QPalette pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Window));
  this->setPalette(pal);
  this->setStyleSheet("QSpinBox { background-color: white;}");
  
  pqServerManagerObserver *SMObserver = pqApplicationCore::instance()->getServerManagerObserver();
  connect(SMObserver, SIGNAL(connectionCreated(vtkIdType)),
	  this, SLOT(onConnect()));
  connect(SMObserver, SIGNAL(connectionClosed(vtkIdType)),
	  this, SLOT(onDisconnect()));

  if (pqApplicationCore::instance()->getActiveServer())
    onConnect();


//   qDebug() << this << ": Created";
}

//-----------------------------------------------------------------------------
void SliceView::buildTitle()
{
  QPushButton *close = new QPushButton("x");
  close->setMaximumHeight(20);
  close->setMaximumWidth (20);

  QPushButton *max = new QPushButton("-");
  max->setMaximumHeight(20);
  max->setMaximumWidth (20);
  
  QPushButton *undock = new QPushButton("^");
  undock->setMaximumHeight(20);
  undock->setMaximumWidth (20);
  
  connect(close, SIGNAL(clicked(bool)),
	  this, SLOT(close()));
  connect(max, SIGNAL(clicked(bool)),
	  this, SLOT(maximize()));
  connect(undock, SIGNAL(clicked(bool)),
	  this, SLOT(undock()));
  
  m_titleLayout->addWidget(m_title);
  m_titleLayout->addWidget(max);
  m_titleLayout->addWidget(undock);
  m_titleLayout->addWidget(close);

  m_mainLayout->addLayout(m_titleLayout);
}

//-----------------------------------------------------------------------------
void SliceView::buildControls()
{
  m_scrollBar->setMaximum(0);
  m_scrollBar->setSizePolicy(
      QSizePolicy::Expanding,
      QSizePolicy::Preferred);

  m_fromSlice->setFlat(true);
  m_fromSlice->setVisible(false);
  m_fromSlice->setEnabled(false);
  m_fromSlice->setMaximumHeight(20);
  m_fromSlice->setSizePolicy(
      QSizePolicy::Minimum,
      QSizePolicy::Minimum);
  connect(m_fromSlice, SIGNAL(clicked(bool)),
	  this,SLOT(selectFromSlice()));

  m_spinBox->setMaximum(0);
  m_spinBox->setMinimumWidth(40);
  m_spinBox->setMaximumHeight(20);
  m_spinBox->setSizePolicy(
      QSizePolicy::Minimum,
      QSizePolicy::Preferred);
  m_spinBox->setAlignment(Qt::AlignRight);

  m_toSlice->setFlat(true);
  m_toSlice->setVisible(false);
  m_toSlice->setEnabled(false);
  m_toSlice->setMaximumHeight(20);
  m_toSlice->setSizePolicy(
      QSizePolicy::Minimum,
      QSizePolicy::Minimum);
  connect(m_toSlice, SIGNAL(clicked(bool)),
	  this,SLOT(selectToSlice()));

  connect(m_scrollBar, SIGNAL(valueChanged(int)),
	  m_spinBox, SLOT(setValue(int)));
  connect(m_scrollBar, SIGNAL(valueChanged(int)),
	  this, SLOT(scrollValueChanged(int)));
  connect(m_spinBox, SIGNAL(valueChanged(int)),
	  m_scrollBar, SLOT(setValue(int)));
  
//   connect(SelectionManager::instance(),SIGNAL(VOIChanged(IVOI*)),this,SLOT(setVOI(IVOI*)));
  m_controlLayout->addWidget(m_scrollBar);
  m_controlLayout->addWidget(m_fromSlice);
  m_controlLayout->addWidget(m_spinBox);
  m_controlLayout->addWidget(m_toSlice);

  m_mainLayout->addLayout(m_controlLayout);
}


//-----------------------------------------------------------------------------
SliceView::~SliceView()
{
  disconnect();
//   qDebug() << this << ": Destroyed";
}

//-----------------------------------------------------------------------------
QString SliceView::title() const
{
  return m_title->text();
}

//-----------------------------------------------------------------------------
void SliceView::setTitle(const QString& title)
{
  m_title->setText(title);
}

//-----------------------------------------------------------------------------
void SliceView::setCrosshairColors(double hcolor[3], double vcolor[3])
{
  vtkSMPropertyHelper(m_view->getViewProxy(),"HCrossLineColor").Set(hcolor,3);
  vtkSMPropertyHelper(m_view->getViewProxy(),"VCrossLineColor").Set(vcolor,3);
  m_view->getProxy()->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void SliceView::setCrosshairVisibility(bool visible)
{
  vtkSMPropertyHelper(m_view->getViewProxy(),"ShowCrosshair").Set(visible);
  m_view->getProxy()->UpdateVTKObjects();
  forceRender();
}

//-----------------------------------------------------------------------------
void SliceView::setThumbnailVisibility(bool visible)
{
  vtkSMPropertyHelper(m_view->getViewProxy(),"ShowThumbnail").Set(visible);
  m_view->getProxy()->UpdateVTKObjects();
  forceRender();
}

//-----------------------------------------------------------------------------
void SliceView::resetCamera()
{
  m_view->resetCamera();
}


//-----------------------------------------------------------------------------
void SliceView::eventPosition(int& x, int& y)
{
  //Use Render Window Interactor's to obtain event's position
  vtkSMSliceViewProxy* view =
    vtkSMSliceViewProxy::SafeDownCast(m_view->getProxy());
  vtkRenderWindowInteractor *rwi =
    vtkRenderWindowInteractor::SafeDownCast(
      view->GetRenderWindow()->GetInteractor());
  Q_ASSERT(rwi);
  rwi->GetEventPosition(x, y);
}


//-----------------------------------------------------------------------------
SelectionHandler::MultiSelection SliceView::select(
  SelectionHandler::SelectionFilters filters,
  SelectionHandler::ViewRegions regions)
{
  bool multiSelection = false;
  SelectionHandler::MultiSelection msel;

  if (m_inThumbnail)
    return msel;

  vtkSMSliceViewProxy* view =
    vtkSMSliceViewProxy::SafeDownCast(m_view->getProxy());
  Q_ASSERT(view);
  vtkRenderer *renderer = view->GetRenderer();
  Q_ASSERT(renderer);

//   qDebug() << "EspINA::SliceView" << m_plane << ": Making selection";
  // Select all products that belongs to all the regions
  foreach(const QPolygonF &region, regions)
  {
    SelectionHandler::VtkRegion vtkRegion;
    // Translate view pixels into Vtk pixels
    vtkRegion = display2vtk(region);

    if (vtkRegion.isEmpty())
      return msel;

//     qDebug() << "Analyze Region:";
    foreach(QPointF p, region)
    {
      foreach(QString filter, filters)
      {
	// 	  qDebug() << "\t\tLooking for" << filter;
	if (filter == SelectionHandler::EspINA_Channel)
	{
	  foreach(Channel *channel, pickChannels(p.x(), p.y(), renderer, multiSelection))
	  {
	    SelectionHandler::Selelection selection(vtkRegion,channel);
	    msel.append(selection);
	  }
	}
	else if (filter == SelectionHandler::EspINA_Segmentation)
	{
	  foreach(Segmentation *seg, pickSegmentations(p.x(), p.y(), renderer, multiSelection))
	  {
	    SelectionHandler::Selelection selection(vtkRegion, seg);
	    msel.append(selection);
	  }
// 	} else if (filter == SelectionHandler::EspINA_Representation)
// 	{
// 	  foreach(Representation *rep, pickRepresentation(propPicker, multiSelection))
// 	  {
// 	    SelectionHandler::Selelection selection(vtkRegion, rep);
// 	    msel.append(selection);
// 	  }
	}else
	{
	  Q_ASSERT(false);
	}
      }
    }
  }

  return msel;
}


//-----------------------------------------------------------------------------
pqRenderViewBase* SliceView::view()
{
  return m_view;
}


//-----------------------------------------------------------------------------
vtkRenderWindow* SliceView::renderWindow()
{
  vtkSMSliceViewProxy* view =
    vtkSMSliceViewProxy::SafeDownCast(m_view->getProxy());
  return view->GetRenderWindow();
}

//-----------------------------------------------------------------------------
void SliceView::onConnect()
{
//   qDebug() << this << ": Connecting to a new server";

  pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
  pqServer    *server = pqActiveObjects::instance().activeServer();

  m_view = qobject_cast<pqSliceView*>(ob->createView(
             pqSliceView::espinaRenderViewType(), server));

  m_view->setSlicingPlane(m_plane);
  connect(m_view, SIGNAL(centerChanged(double,double,double)),
	  this, SLOT(sliceViewCenterChanged(double,double,double)));

  m_viewWidget = m_view->getWidget();
  // We want to manage events on the view
  m_viewWidget->installEventFilter(this);
  m_mainLayout->insertWidget(0, m_viewWidget);//To preserve view order

  double black[] = {0,0,0};
  vtkSMPropertyHelper(m_view->getViewProxy(),"Background").Set(black,3);
  vtkSMPropertyHelper(m_view->getViewProxy(),"CenterAxesVisibility").Set(false);
  m_view->getViewProxy()->UpdateVTKObjects();

//   // Disable menu
//   // TODO: OLDVERSION m_view->getWidget()->removeAction(m_view->getWidget()->actions().first());

}

//-----------------------------------------------------------------------------
void SliceView::onDisconnect()
{
//   qDebug() << this << ": Disconnecting from server";
//   pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
  m_viewWidget = NULL;
//   if (m_view)
//   {
//     m_mainLayout->removeWidget(m_viewWidget);
//     m_style->Deete();
//     m_view = NULL;
//     m_viewWdget = NULL;
//   }
}

//-----------------------------------------------------------------------------
void SliceView::sliceViewCenterChanged(double x, double y, double z)
{
//   qDebug() << "Slice View: " << m_plane << " has new center";
  emit centerChanged(x,y,z);
}

//-----------------------------------------------------------------------------
void SliceView::scrollValueChanged(int value)
{
  double pos = value;//nm

  if (m_fitToGrid)
    pos *= m_gridSize[m_plane];

  m_view->setSlice(pos);
}

//-----------------------------------------------------------------------------
void SliceView::selectFromSlice()
{
  emit selectedFromSlice(sliceValue(), m_plane);
}

//-----------------------------------------------------------------------------
void SliceView::selectToSlice()
{
  emit selectedToSlice(sliceValue(), m_plane);
}

//-----------------------------------------------------------------------------
void SliceView::close()
{
  emit closeRequest();
}

//-----------------------------------------------------------------------------
void SliceView::maximize()
{
  emit maximizeRequest();
}

//-----------------------------------------------------------------------------
void SliceView::minimize()
{
  emit minimizeRequest();
}

//-----------------------------------------------------------------------------
void SliceView::undock()
{
  emit undockRequest();
}

//-----------------------------------------------------------------------------
bool SliceView::eventFilter(QObject* caller, QEvent* e)
{
  if (SelectionManager::instance()->filterEvent(e, this))
    return true;

  if (e->type() == QEvent::Wheel)
  {
    QWheelEvent *we = static_cast<QWheelEvent *>(e);
    int numSteps = we->delta()/8/15*(m_settings->invertWheel()?-1:1);//Refer to QWheelEvent doc.
    m_spinBox->setValue(m_spinBox->value() - numSteps);
    e->ignore();
  }else if (e->type() == QEvent::Enter)
  {
    QWidget::enterEvent(e);
    m_viewWidget->setCursor(SelectionManager::instance()->cursor());
    e->accept();
  }else if (e->type() == QEvent::MouseMove)
  {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    vtkSMSliceViewProxy* view =
    vtkSMSliceViewProxy::SafeDownCast(m_view->getProxy());
    Q_ASSERT(view);
    vtkRenderer * thumbnail = view->GetOverviewRenderer();
    Q_ASSERT(thumbnail);
    vtkPropPicker *propPicker = vtkPropPicker::New();
    int x, y;
    eventPosition(x, y);
    if (thumbnail->GetDraw() && propPicker->Pick(x, y, 0.1, thumbnail))
    {
      if (!m_inThumbnail)
	QApplication::setOverrideCursor(Qt::ArrowCursor);
      m_inThumbnail = true;
    }
    else
    {
      if (m_inThumbnail)
	QApplication::restoreOverrideCursor();
      m_inThumbnail = false;
    }

    if (m_inThumbnail && me->buttons() == Qt::LeftButton)
    {
      centerViewOnMousePosition();
      if (me->modifiers() == Qt::CTRL)
      {
	centerCrosshairOnMousePosition();
      }
    }
  }else if (e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    if (me->button() == Qt::LeftButton)
    {
      if (me->modifiers() == Qt::CTRL)
      {
	centerCrosshairOnMousePosition();
	emit showCrosshairs(true);
      }
      else if (m_inThumbnail)
      {
	centerViewOnMousePosition();
      } else
      {
	selectPickedItems(me->modifiers() == Qt::SHIFT);
      }
    }
  } else if (e->type() == QEvent::KeyRelease)
  {
    QKeyEvent *ke = static_cast<QKeyEvent *>(e);
    if (ke->key() == Qt::Key_Control && ke->count() == 1)
      emit showCrosshairs(false);
  }

  return QWidget::eventFilter(caller, e);
}

//-----------------------------------------------------------------------------
void SliceView::centerCrosshairOnMousePosition()
{
  int xPos, yPos;
  eventPosition(xPos, yPos);

  double center[3];//World coordinates
  pickChannel(xPos, yPos, center);

  centerViewOn(center);
  emit focusChanged(center);
}

//-----------------------------------------------------------------------------
void SliceView::centerViewOnMousePosition()
{
  int xPos, yPos;
  eventPosition(xPos, yPos);

  double center[3];//World coordinates
  pickChannel(xPos, yPos, center);

  vtkCamera * camera = m_view->getRenderViewProxy()->GetRenderer()->GetActiveCamera();
  camera->SetFocalPoint(center);
  m_view->render();
}

//-----------------------------------------------------------------------------
QList<Channel *> SliceView::pickChannels(double vx, double vy,
					 vtkRenderer* renderer,
					 bool repeatable)
{
  QList<Channel *> channels;

  vtkPropPicker *picker = m_view->channelPicker();
  if (picker->Pick(vx, vy, 0.1, renderer))
  {
    vtkProp3D *pickedProp = picker->GetProp3D();
    vtkObjectBase *object;
    vtkSliceRepresentation *rep;

    foreach(Channel *channel, m_channels.keys())
    {
      object = m_channels[channel].proxy->GetClientSideObject();
      rep = dynamic_cast<vtkSliceRepresentation *>(object);
      Q_ASSERT(rep);
      if (rep->GetSliceProp() == pickedProp)
      {
// 	qDebug() << "Channel" << channel->data(Qt::DisplayRole).toString() << "Selected";
	channels << channel;
	if (!repeatable)
	  return channels;
      }
    }
  }

  return channels;
}


//-----------------------------------------------------------------------------
QList<Segmentation *> SliceView::pickSegmentations(double vx, double vy,
						    vtkRenderer* renderer,
						    bool repeatable)
{
  QList<Segmentation *> segmentations;

  vtkPropPicker *picker = m_view->segmentationPicker();
  if (picker->Pick(vx, vy, 0.1, renderer))
  {
    vtkProp3D *pickedProp = picker->GetProp3D();
    vtkObjectBase *object;
    vtkSliceRepresentation *rep;

    foreach(Segmentation *seg, m_segmentations.keys())
    {
      object = m_segmentations[seg].proxy->GetClientSideObject();
      rep = dynamic_cast<vtkSliceRepresentation *>(object);
      Q_ASSERT(rep);
      if (rep->GetSliceProp() == pickedProp)
      {
// 	qDebug() << "Segmentation" << seg->data(Qt::DisplayRole).toString() << "Selected";
	segmentations << seg;
	if (!repeatable)
	  return segmentations;
      }
    }
  }

  return segmentations;
}

//-----------------------------------------------------------------------------
void SliceView::selectPickedItems(bool append)
{
//   qDebug() << "SliceView::SelectPickedItems";
  vtkSMSliceViewProxy* view =
    vtkSMSliceViewProxy::SafeDownCast(m_view->getProxy());
  Q_ASSERT(view);
  vtkRenderer *renderer = view->GetRenderer();
  Q_ASSERT(renderer);

  int vx, vy;
  eventPosition(vx, vy);

  bool segPicked = false;
  // If no append, segmentations have priority over channels
  foreach(Segmentation *seg, pickSegmentations(vx, vy, renderer, append))
  {
    segPicked = true;
    emit segmentationSelected(seg, append);
    if (!append)
      return;
  }

  // Heterogeneus picking is not supported
  if (segPicked)
    return;

  foreach(Channel *channel, pickChannels(vx, vy, renderer, append))
  {
    emit channelSelected(channel);
    if (!append)
      return;
  }

}

//-----------------------------------------------------------------------------
double SliceView::suggestedChannelOpacity()
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
void SliceView::updateWidgetVisibility()
{
  foreach(SliceWidget *widget, m_widgets)
  {
    widget->setSlice(m_center[m_plane], m_plane);
  }
}


//-----------------------------------------------------------------------------
double SliceView::sliceValue() const
{
  if (m_fitToGrid)
    return m_gridSize[m_plane]*m_spinBox->value();
  else
    return m_spinBox->value();
}


//-----------------------------------------------------------------------------
bool SliceView::pickChannel(int x, int y, double pickPos[3])
{
  vtkSMSliceViewProxy* view =
    vtkSMSliceViewProxy::SafeDownCast(m_view->getProxy());
  Q_ASSERT(view);
  vtkRenderer * thumbnail = view->GetOverviewRenderer();
  Q_ASSERT(thumbnail);

  vtkPropPicker *propPicker = vtkPropPicker::New();
  if (!thumbnail->GetDraw() || !propPicker->Pick(x, y, 0.1, thumbnail))
  {
    vtkRenderer *renderer = view->GetRenderer();
    Q_ASSERT(renderer);
    if (!propPicker->Pick(x, y, 0.1, renderer))
    {
//       qDebug() << "ePick Fail!";
      return false;
    }
  }

  propPicker->GetPickPosition(pickPos);

  pickPos[m_plane] = m_fitToGrid?m_scrollBar->value()*m_gridSize[m_plane]:m_scrollBar->value();
//   qDebug() << "Pick Position" << pickPos[0] << pickPos[1] << pickPos[2];
  propPicker->Delete();

  return true;
}


//-----------------------------------------------------------------------------
void SliceView::addChannelRepresentation(Channel* channel)
{
  pqData volumetric = channel->representation("Volumetric")->output();
  pqOutputPort      *oport = volumetric.outputPort();
  pqPipelineSource *source = oport->getSource();
  vtkSMProxyManager   *pxm = vtkSMProxyManager::GetProxyManager();

  vtkSMRepresentationProxy* reprProxy = vtkSMRepresentationProxy::SafeDownCast(
    pxm->NewProxy("representations", "ChannelRepresentation"));
  Q_ASSERT(reprProxy);
  m_channels[channel].outport  = oport;
  m_channels[channel].proxy    = reprProxy;
  m_channels[channel].selected = false;
  m_channels[channel].visible  = !channel->isVisible();// Force initialization
  m_channels[channel].color    = QColor("red");

    // Set the reprProxy's input.
  pqSMAdaptor::setInputProperty(reprProxy->GetProperty("Input"),
				source->getProxy(), oport->getPortNumber());
  updateChannelRepresentation(channel);

  vtkSMProxy* viewModuleProxy = m_view->getProxy();
  // Add the reprProxy to render module.
  pqSMAdaptor::addProxyProperty(
    viewModuleProxy->GetProperty("Representations"), reprProxy);
  viewModuleProxy->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void SliceView::removeChannelRepresentation(Channel* channel)
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
bool SliceView::updateChannelRepresentation(Channel* channel)
{
  Q_ASSERT(m_channels.contains(channel));
  RepInfo &rep = m_channels[channel];

  double pos[3];
  channel->position(pos);

  if (channel->isVisible() != rep.visible
    || channel->color() != rep.color.hueF()
    || memcmp(pos, rep.pos, 3*sizeof(double)))
  {
    rep.visible  = channel->isVisible();
    rep.color.setHsvF(channel->color(),1.0,1.0);
    memcpy(rep.pos, pos, 3*sizeof(double));

    vtkSMPropertyHelper(rep.proxy, "Position").Set(rep.pos,3);
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
void SliceView::addSegmentationRepresentation(Segmentation* seg)
{
  pqOutputPort      *oport = seg->outputPort();
  pqPipelineSource *source = oport->getSource();
  vtkSMProxyManager   *pxm = vtkSMProxyManager::GetProxyManager();

  vtkSMRepresentationProxy* reprProxy = vtkSMRepresentationProxy::SafeDownCast(
    pxm->NewProxy("representations", "SegmentationRepresentation"));
  Q_ASSERT(reprProxy);
  m_segmentations[seg].outport  = oport;
  m_segmentations[seg].proxy    = reprProxy;
  m_segmentations[seg].selected = !seg->isSelected();
  m_segmentations[seg].visible  = seg->visible();
  m_segmentations[seg].color  = m_colorEngine->color(seg);

  // Set the reprProxy's input.
  pqSMAdaptor::setInputProperty(reprProxy->GetProperty("Input"),
				source->getProxy(), oport->getPortNumber());
  updateSegmentationRepresentation(seg);
//   reprProxy->UpdateVTKObjects();

  vtkSMProxy* viewModuleProxy = m_view->getProxy();
  // Add the reprProxy to render module.
  pqSMAdaptor::addProxyProperty(
    viewModuleProxy->GetProperty("Representations"), reprProxy);
  viewModuleProxy->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void SliceView::removeSegmentationRepresentation(Segmentation* seg)
{
  vtkSMProxy* viewModuleProxy = m_view->getProxy();
  Q_ASSERT(m_segmentations.contains(seg));
  RepInfo rep = m_segmentations[seg];
  // Remove the reprProxy to render module.
  pqSMAdaptor::removeProxyProperty(
    viewModuleProxy->GetProperty("Representations"), rep.proxy);
  viewModuleProxy->UpdateVTKObjects();
  m_view->getProxy()->UpdateVTKObjects();

  rep.proxy->Delete();
  m_segmentations.remove(seg);
}

//-----------------------------------------------------------------------------
void SliceView::addRepresentation(pqOutputPort* oport, QColor color)
{
  pqPipelineSource *source = oport->getSource();
  vtkSMProxyManager   *pxm = vtkSMProxyManager::GetProxyManager();

  vtkSMRepresentationProxy* reprProxy = vtkSMRepresentationProxy::SafeDownCast(
    pxm->NewProxy("representations", "SegmentationRepresentation"));
  Q_ASSERT(reprProxy);
  m_representations[oport].proxy  = reprProxy;
  m_representations[oport].color  = color;

  // Set the reprProxy's input.
  pqSMAdaptor::setInputProperty(reprProxy->GetProperty("Input"),
                                source->getProxy(), oport->getPortNumber());

  double colorD[3] = {color.redF(), color.greenF(), color.blueF()};
  vtkSMPropertyHelper(reprProxy, "RGBColor").Set(colorD,3);

  reprProxy->UpdateVTKObjects();

  vtkSMProxy* viewModuleProxy = m_view->getProxy();
  // Add the reprProxy to render module.
  pqSMAdaptor::addProxyProperty(
    viewModuleProxy->GetProperty("Representations"), reprProxy);
  viewModuleProxy->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void SliceView::removeRepresentation(pqOutputPort* oport)
{
  vtkSMProxy* viewModuleProxy = m_view->getProxy();

  if (!m_representations.contains(oport))
    return;
  //Q_ASSERT(m_representations.contains(rep));

  RepInfo sliceRep = m_representations[oport];
  // Remove the reprProxy to render module.
  pqSMAdaptor::removeProxyProperty(
    viewModuleProxy->GetProperty("Representations"), sliceRep.proxy);
  viewModuleProxy->UpdateVTKObjects();
  m_view->getProxy()->UpdateVTKObjects();

  sliceRep.proxy->Delete();
  m_representations.remove(oport);
}

//-----------------------------------------------------------------------------
bool SliceView::updateSegmentationRepresentation(Segmentation* seg)
{
  Q_ASSERT(m_segmentations.contains(seg));
  RepInfo &rep = m_segmentations[seg];
  if (seg->outputPort() != rep.outport)
  {
    //remove representation using previous proxy
    removeSegmentationRepresentation(seg);
    //add representation using new proxy
    addSegmentationRepresentation(seg);
    return true;
  }
  if (seg->isSelected() != rep.selected
    || seg->visible() != rep.visible
    || seg->data(Qt::DecorationRole).value<QColor>() != rep.color
    || seg->updateForced()
  )
  {
    rep.selected = seg->isSelected();
    rep.visible  = seg->visible();
    rep.color = seg->data(Qt::DecorationRole).value<QColor>();
    //   repProxy->PrintSelf(std::cout,vtkIndent(0));
    double color[3] = {rep.color.redF(), rep.color.greenF(), rep.color.blueF()};
    vtkSMPropertyHelper(rep.proxy, "RGBColor").Set(color,3);
    vtkSMPropertyHelper(rep.proxy, "Visibility").Set(rep.visible);
    double opacity = rep.selected?1.0:0.7;
    vtkSMPropertyHelper(rep.proxy, "Opacity").Set(&opacity, 1);
    rep.proxy->UpdateVTKObjects();
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
void SliceView::addPreview(Filter* filter)
{
  addPreview(filter->preview().outputPort());
  m_preview = filter;
}

void SliceView::removePreview(Filter* filter)
{
  removePreview(filter->preview().outputPort());
}


//-----------------------------------------------------------------------------
void SliceView::addPreview(pqOutputPort* preview)
{
//   addRepresentation(preview, QColor(255,0,0));
}

void SliceView::removePreview(pqOutputPort* preview)
{
//   removeRepresentation(preview);
}

//-----------------------------------------------------------------------------
void SliceView::addWidget(SliceWidget *sWidget)
{
  pq3DWidget *widget = *sWidget;
  widget->setView(m_view);
  widget->setWidgetVisible(true);
  widget->select();
  connect(widget, SIGNAL(modified()),
	  this, SLOT(updateWidgetVisibility()));
  m_widgets << sWidget;
}

//-----------------------------------------------------------------------------
void SliceView::removeWidget(SliceWidget* sWidget)
{
  Q_ASSERT(m_widgets.contains(sWidget));
  m_widgets.removeOne(sWidget);
}


//-----------------------------------------------------------------------------
void SliceView::previewExtent(int VOI[6])
{
  memcpy(VOI, m_gridSize,6*sizeof(int));
  VOI[2*m_plane] = m_scrollBar->value();
  VOI[2*m_plane+1] = m_scrollBar->value();
}

//-----------------------------------------------------------------------------
void SliceView::setSegmentationVisibility(bool visible)
{
  m_view->setShowSegmentations(visible);
}

//-----------------------------------------------------------------------------
void SliceView::setShowPreprocessing(bool visible)
{
  if (m_channels.size() < 2)
    return;

  Channel *hiddenChannel = m_channels.keys()[visible];
  Channel *visibleChannel = m_channels.keys()[1-visible];
  hiddenChannel->setData(false, Qt::CheckStateRole);
  hiddenChannel->notifyModification();
  visibleChannel->setData(true, Qt::CheckStateRole);
  visibleChannel->notifyModification();
  for(int i=2; i < m_channels.keys().size(); i++)
  {
    Channel *otherChannel = m_channels.keys()[i];
    otherChannel->setData(false, Qt::CheckStateRole);
    otherChannel->notifyModification();
  }
}


//-----------------------------------------------------------------------------
void SliceView::setRulerVisibility(bool visible)
{
  m_view->setRulerVisibility(visible);
}

//-----------------------------------------------------------------------------
void SliceView::setSliceSelectors(SliceView::SliceSelectors selectors)
{
  m_fromSlice->setVisible(selectors.testFlag(From));
  m_toSlice->setVisible(selectors.testFlag(To));
}

//-----------------------------------------------------------------------------
void SliceView::forceRender()
{
  if (isVisible())
  {
//     qDebug() << "Rendering View" << m_plane;
    updateWidgetVisibility();
    m_view->forceRender();
  }
}

//-----------------------------------------------------------------------------
void SliceView::setGridSize(double size[3])
{
  if (size[0] <= 0 || size[1] <= 0 || size[2] <= 0)
  {
    qFatal("SliceView: Invalid Grid Size. Grid Size not changed");
    return;
  }

  memcpy(m_gridSize, size, 3*sizeof(double));
  setRanges(m_range);
}

//-----------------------------------------------------------------------------
void SliceView::setRanges(double ranges[6])
{
  if (ranges[1] < ranges[0] || ranges[3] < ranges[2] || ranges[5] < ranges[4])
  {
    qFatal("SliceView: Invalid Ranges. Ranges not changed");
    return;
  }

  double min = ranges[2*m_plane];
  double max = ranges[2*m_plane+1];
  if (m_fitToGrid)
  {
    min = min / m_gridSize[m_plane];
    max = max / m_gridSize[m_plane];
  }
  m_scrollBar->setMinimum(static_cast<int>(min));
  m_scrollBar->setMaximum(static_cast<int>(max));
  m_spinBox->setMinimum(static_cast<int>(min));
  m_spinBox->setMaximum(static_cast<int>(max));
  memcpy(m_range, ranges, 6*sizeof(double));

  bool enabled = m_spinBox->minimum() < m_spinBox->maximum();
  m_fromSlice->setEnabled(enabled);
  m_toSlice->setEnabled(enabled);
}
//-----------------------------------------------------------------------------
void SliceView::setFitToGrid(bool value)
{
  if (value == m_fitToGrid)
    return;

  int currentScrollValue = m_scrollBar->value();
  m_fitToGrid = value;
  m_scrollBar->blockSignals(true);
  m_spinBox->blockSignals(true);
  setRanges(m_range);
  if (m_fitToGrid)
  {
    m_scrollBar->setValue(currentScrollValue/m_gridSize[m_plane]);
    m_spinBox->setValue(currentScrollValue/m_gridSize[m_plane]);
    m_spinBox->setSuffix("");
  }
  else
  {
    m_scrollBar->setValue(currentScrollValue*m_gridSize[m_plane]);
    m_spinBox->setValue(currentScrollValue*m_gridSize[m_plane]);
    m_spinBox->setSuffix("nm");
  }
  m_spinBox->blockSignals(false);
  m_scrollBar->blockSignals(false);
}

//-----------------------------------------------------------------------------
void SliceView::centerViewOn(double center[3], bool force)
{
  if (!isVisible() ||
      (m_center[0] == center[0] &&
       m_center[1] == center[1] &&
       m_center[2] == center[2] &&
       !force))
    return;

  memcpy(m_center, center, 3*sizeof(double));

  // Round the value to the nearest unit (nm or grid)
  for (int i = 0; i < 3; i++)
  {
    if (m_fitToGrid)
      m_center[i] = m_center[i]/m_gridSize[i];
    m_center[i] = floor(m_center[i]+0.5);
  }

  // Disable scrollbox signals to avoid calling seting slice
  m_scrollBar->blockSignals(true);
  m_spinBox->setValue(m_center[m_plane]);
  m_scrollBar->setValue(m_center[m_plane]);
  m_scrollBar->blockSignals(false);

  // If fitToGrid is enabled, we must center the view on the
  // corresponding grid's position
  if (m_fitToGrid)
    for (int i = 0; i < 3; i++)
      m_center[i] = floor((m_center[i]*m_gridSize[i])+0.5);

  m_view->centerViewOn(m_center[0], m_center[1], m_center[2], force);
  updateWidgetVisibility();
}

//-----------------------------------------------------------------------------
SelectionHandler::VtkRegion SliceView::display2vtk(const QPolygonF &region)
{
  //Use Render Window Interactor's Picker to find the world coordinates
  //of the stack
  //vtkSMRenderViewProxy* renModule = view->GetRenderWindow()->GetInteractor()->GetRenderView();
  SelectionHandler::VtkRegion vtkRegion;

//   vtkWorldPointPicker *wpicker = vtkWorldPointPicker::New();
  double pickPos[3];//World coordinates
  foreach(QPointF point, region)
  {
    if (!pickChannel(point.x(), point.y(), pickPos))
      continue;

    QVector3D vtkPoint;
    vtkPoint.setX(floor((pickPos[0] / m_gridSize[0])+0.5));
    vtkPoint.setY(floor((pickPos[1] / m_gridSize[1])+0.5));
    vtkPoint.setZ(floor((pickPos[2] / m_gridSize[2])+0.5));
    vtkRegion << vtkPoint;
  }
  return vtkRegion;

}


//-----------------------------------------------------------------------------
SliceView::Settings::Settings(vtkPVSliceView::VIEW_PLANE plane, const QString prefix)
: INVERT_SLICE_ORDER (prefix + view(plane) + "::invertSliceOrder")
, INVERT_WHEEL       (prefix + view(plane) + "::invertWheel")
, SHOW_AXIS          (prefix + view(plane) + "::showAxis")
, m_InvertWheel(false)
, m_InvertSliceOrder(false)
, m_ShowAxis(false)
, m_plane(plane)
{
  QSettings settings("CeSViMa", "EspINA");

  if (!settings.contains(INVERT_SLICE_ORDER))
    settings.setValue(INVERT_SLICE_ORDER, m_InvertSliceOrder);
  if (!settings.contains(INVERT_WHEEL))
    settings.setValue(INVERT_WHEEL, m_InvertWheel);
  if (!settings.contains(SHOW_AXIS))
    settings.setValue(SHOW_AXIS, m_ShowAxis);

  m_InvertSliceOrder = settings.value(INVERT_SLICE_ORDER).toBool();
  m_InvertWheel      = settings.value(INVERT_WHEEL      ).toBool();
  m_ShowAxis         = settings.value(SHOW_AXIS         ).toBool();
}

//-----------------------------------------------------------------------------
const QString SliceView::Settings::view(vtkPVSliceView::VIEW_PLANE plane)
{
  switch (plane)
  {
    case vtkPVSliceView::AXIAL:
      return QString("AxialSliceView");
    case vtkPVSliceView::SAGITTAL:
      return QString("SagittalSliceView");
    case vtkPVSliceView::CORONAL:
      return QString("CoronalSliceView");
    default:
      Q_ASSERT(false);
  };
  return QString("Unknown");
}

//-----------------------------------------------------------------------------
void SliceView::Settings::setInvertSliceOrder(bool value)
{
  QSettings settings("CeSViMa", "EspINA");

  settings.setValue(INVERT_SLICE_ORDER, value);
  m_InvertSliceOrder = value;
}

//-----------------------------------------------------------------------------
void SliceView::Settings::setInvertWheel(bool value)
{
  QSettings settings("CeSViMa", "EspINA");

  settings.setValue(INVERT_WHEEL, value);
  m_InvertWheel = value;
}

//-----------------------------------------------------------------------------
void SliceView::Settings::setShowAxis(bool value)
{
  QSettings settings("CeSViMa", "EspINA");

  settings.setValue(SHOW_AXIS, value);
  m_ShowAxis = value;
}