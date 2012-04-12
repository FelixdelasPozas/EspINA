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
#include "SliceView.h"

// Debug
#include <QDebug>
// #include "espina_debug.h"
//
// // EspINA
#include "IPreferencePanel.h"
#include "common/model/Channel.h"
#include "common/model/Representation.h"
#include "common/model/Segmentation.h"
#include "common/processing/pqData.h"
#include "common/selection/SelectionManager.h"
#include "common/views/pqSliceView.h"
#include "common/views/vtkSMSliceViewProxy.h"

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
#include "ColorEngine.h"

//-----------------------------------------------------------------------------
SliceViewPreferencesPanel::SliceViewPreferencesPanel(SliceViewPreferences* preferences)
: QWidget(preferences)
, m_pref(preferences)
{
  QVBoxLayout *layout = new QVBoxLayout();
/*
  QCheckBox *invertWheel = new QCheckBox(tr("Invert Wheel"));
  invertWheel->setChecked(m_pref->invertWheel());
  connect(invertWheel,SIGNAL(toggled(bool)),
	  this, SLOT(setInvertWheel(bool)));

//   QCheckBox *invertNormal = new QCheckBox(tr("Invert Normal"));
//   invertNormal->setChecked(m_pref->invertNormal());
//   connect(invertNormal,SIGNAL(toggled(bool)),
// 	  this, SLOT(setInvertNormal(bool)));

  QCheckBox *showAxis = new QCheckBox(tr("Show Axis"));
  showAxis->setChecked(m_pref->showAxis());
  connect(showAxis,SIGNAL(toggled(bool)),
	  this, SLOT(setShowAxis(bool)));

  ltyyout->addWidget(invertWheel);
//   layout->addWidget(invertNormal);
  layout->addWidget(showAxis);
  QSpacerItem *spacer = new QSpacerItem(10,10,QSizePolicy::Expanding,QSizePolicy::Expanding);
  layout->addSpacerItem(spacer);*/
  setLayout(layout);
}

//-----------------------------------------------------------------------------
void SliceViewPreferencesPanel::setInvertWheel(bool value)
{
  m_pref->setInvertWheel(value);
}

//-----------------------------------------------------------------------------
void SliceViewPreferencesPanel::setInvertNormal(bool value)
{
  m_pref->setInvertSliceOrder(value);
}

//-----------------------------------------------------------------------------
void SliceViewPreferencesPanel::setShowAxis(bool value)
{
  m_pref->setShowAxis(value);
}



//-----------------------------------------------------------------------------
SliceViewPreferences::SliceViewPreferences(vtkPVSliceView::VIEW_PLANE plane)
: m_InvertWheel(false)
, m_InvertSliceOrder(false)
, m_ShowAxis(false)
, m_plane(plane)
{
  QSettings settings("CeSViMa", "EspINA");

  switch (plane)
  {
    case vtkPVSliceView::AXIAL:
      viewSettings = "AxialSliceView";
      break;
    case vtkPVSliceView::SAGITTAL:
      viewSettings = "SagittalSliceView";
      break;
    case vtkPVSliceView::CORONAL:
      viewSettings = "CoronalSliceView";
      break;
    default:
      Q_ASSERT(false);
  };

  const QString wheelSettings = QString(viewSettings+"::invertWheel");
  if (!settings.contains(wheelSettings))
    settings.setValue(wheelSettings,m_InvertWheel);
  m_InvertWheel = settings.value(wheelSettings).toBool();

  const QString sliceOrderSettings = QString(viewSettings+"::invertSliceOrder");
  if (!settings.contains(sliceOrderSettings))
    settings.setValue(sliceOrderSettings,m_InvertSliceOrder);
  m_InvertSliceOrder = settings.value(sliceOrderSettings).toBool();

  const QString axisSettings = QString(viewSettings+"::showAxis");
  if (!settings.contains(axisSettings))
    settings.setValue(axisSettings,m_ShowAxis);
  m_ShowAxis = settings.value(axisSettings).toBool();

}

//-----------------------------------------------------------------------------
const QString SliceViewPreferences::shortDescription()
{
  switch (m_plane)
  {
    case vtkPVSliceView::AXIAL:
      viewSettings = "Axial View";
      break;
    case vtkPVSliceView::SAGITTAL:
      viewSettings = "Sagittal View";
      break;
    case vtkPVSliceView::CORONAL:
      viewSettings = "Coronal View";
      break;
    default:
      Q_ASSERT(false);
      return "ERROR";
  };
  return viewSettings;
}

//-----------------------------------------------------------------------------
void SliceViewPreferences::setInvertWheel(bool value)
{
  QSettings settings("CeSViMa", "EspINA");

  const QString wheelSettings = QString(viewSettings+"::invertWheel");
  m_InvertWheel = value;
  settings.setValue(wheelSettings,value);
}

//-----------------------------------------------------------------------------
void SliceViewPreferences::setInvertSliceOrder(bool value)
{
  QSettings settings("CeSViMa", "EspINA");

  const QString sliceOrderSettings = QString(viewSettings+"::invertSliceOrder");
  m_InvertSliceOrder = value;
  settings.setValue(sliceOrderSettings,value);
}

//-----------------------------------------------------------------------------
void SliceViewPreferences::setShowAxis(bool value)
{
  QSettings settings("CeSViMa", "EspINA");

  const QString axisSettings = QString(viewSettings+"::showAxis");
  m_ShowAxis = value;
  settings.setValue(axisSettings,value);
}


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
    , m_fitToGrid       (true)
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

  m_preferences = new SliceViewPreferences(m_plane);

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

// //-----------------------------------------------------------------------------
// QModelIndex SliceView::indexAt(const QPoint& point) const
// {
// //   qDebug() << "Selected " << "FAKE" << " segmentation";
//   return QModelIndex();
// }
// 
// //-----------------------------------------------------------------------------
// void SliceView::scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint)
// {
//   qDebug() << "Scroll to Sample";
//   IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
//   Sample * sample = dynamic_cast<Sample *>(item);
//   if (sample)
//     qDebug() << "Scroll to Sample";
//     //s_focusedSample = sample;
// }
// 
// //-----------------------------------------------------------------------------
// QRect SliceView::visualRect(const QModelIndex& index) const
// {
//   return QRect();
// }
// 
// //-----------------------------------------------------------------------------
// QList<Segmentation* > SliceView::pickSegmentationsAt(int x, int y, int z)
// {
//   QList<Segmentation *> res;
//   
//   if (m_focusedSample)
//   {
//     for (int i=0; i < m_focusedSample->segmentations().size(); i++)
//     {
//       Segmentation *seg = m_focusedSample->segmentations()[i];
//       assert(seg);
//       
//       seg->creator()->pipelineSource()->updatePipeline();;
//       seg->creator()->pipelineSource()->getProxy()->UpdatePropertyInformation();
//       vtkPVDataInformation *info = seg->outputPort()->getDataInformation();
//       int extent[6];
//       info->GetExtent(extent);
//       if ((extent[0] <= x && x <= extent[1]) &&
// 	(extent[2] <= y && y <= extent[3]) &&
// 	(extent[4] <= z && z <= extent[5]))
//       {
// 	SegmentationSelectionExtension *selector = dynamic_cast<SegmentationSelectionExtension *>(
// 	  seg->extension(SegmentationSelectionExtension::ID));
// 	
// 	if (selector->isSegmentationPixel(x,y,z))
// 	  res.append(seg);
// 	/*
// 	double pixelValue[4]; //NOTE: hack to redefine vtkVectorMacro so Paraview can find it
// 	pixelValue[0] = x;
// 	pixelValue[1] = y;
// 	pixelValue[2] = z;
// 	pixelValue[3] = 4;
// 	vtkSMPropertyHelper(seg->creator()->pipelineSource()->getProxy(),"CheckPixel").Set(pixelValue,4);
// 	seg->creator()->pipelineSource()->getProxy()->UpdateVTKObjects();
// 	int value;
// 	seg->creator()->pipelineSource()->getProxy()->UpdatePropertyInformation();
// 	vtkSMPropertyHelper(seg->creator()->pipelineSource()->getProxy(),"PixelValue").Get(&value,1);
// // 	qDebug() << "Pixel Value" << value;
// 	if (value == 255)
// 	  res.append(seg);
// 	*/
//       }
//     }
//   }
//   return res;
// }
// 
// //-----------------------------------------------------------------------------
// QList< Segmentation* > SliceView::pickSegmentationsAt(ISelectionHandler::VtkRegion region)
// {
//   QList<Segmentation *> res;
//   foreach(Point p, region)
//   {
//     res.append(pickSegmentationsAt(p.x,p.y,p.z));
//   }
//   return res;
// }
// 
// 
// 
// //-----------------------------------------------------------------------------
// void SliceView::selectSegmentations(int x, int y, int z)
// {
//   QItemSelection selection;
//   if (m_focusedSample)
//   {
//     QModelIndex selIndex;
//     for (int i=0; i < m_focusedSample->segmentations().size(); i++)
//     {
//       QModelIndex segIndex = rootIndex().child(i,0);
//       IModelItem *segItem = static_cast<IModelItem *>(segIndex.internalPointer());
//       Segmentation *seg = dynamic_cast<Segmentation *>(segItem);
//       assert(seg);
//       
// 
//       seg->creator()->pipelineSource()->updatePipeline();;
//       seg->creator()->pipelineSource()->getProxy()->UpdatePropertyInformation();
//       vtkPVDataInformation *info = seg->outputPort()->getDataInformation();
//       int extent[6];
//       info->GetExtent(extent);
//       if ((extent[0] <= x && x <= extent[1]) &&
// 	(extent[2] <= y && y <= extent[3]) &&
// 	(extent[4] <= z && z <= extent[5]))
//       {
// 	SegmentationSelectionExtension *selector = dynamic_cast<SegmentationSelectionExtension *>(
// 	  seg->extension(SegmentationSelectionExtension::ID));
// 	
// 	if (selector->isSegmentationPixel(x,y,z))
// 	  selIndex = segIndex;
// // 	seg->outputPort()->getDataInformation()->GetPointDataInformation();
// 	//selection.indexes().append(segIndex);
// 	/*double pixelValue[4];
// 	pixelValue[0] = x;
// 	pixelValue[1] = y;
// 	pixelValue[2] = z;
// 	pixelValue[3] = 4;
// 	vtkSMPropertyHelper(seg->creator()->pipelineSource()->getProxy(),"CheckPixel").Set(pixelValue,4);
// 	seg->creator()->pipelineSource()->getProxy()->UpdateVTKObjects();
// 	int value;
// 	seg->creator()->pipelineSource()->getProxy()->UpdatePropertyInformation();
// 	vtkSMPropertyHelper(seg->creator()->pipelineSource()->getProxy(),"PixelValue").Get(&value,1);
// // 	qDebug() << "Pixel Value" << value;
// 	if (value == 255)
// 	  selIndex = segIndex;
// 	*/
//       }
//     }
//     if (selIndex.isValid())
//       selectionModel()->select(selIndex,QItemSelectionModel::ClearAndSelect);
//     else
//       selectionModel()->clearSelection();
//   }
// }
// 
// 
// 

//-----------------------------------------------------------------------------
void SliceView::setCrossHairColors(double hcolor[3], double vcolor[3])
{
  vtkSMPropertyHelper(m_view->getViewProxy(),"HCrossLineColor").Set(hcolor,3);
  vtkSMPropertyHelper(m_view->getViewProxy(),"VCrossLineColor").Set(vcolor,3);
  m_view->getProxy()->UpdateVTKObjects();
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
  SelectionHandler::MultiSelection msel;
  //TODO: Discuss if we apply VOI at application level or at plugin level
  // i.e. whether clicks out of VOI are discarted or not

  //qDebug() << "EspINA::SliceView" << m_plane << ": Making selection";
  // Select all products that belongs to all the regions
  foreach(const QPolygonF &region, regions)
  {
    SelectionHandler::VtkRegion vtkRegion;
    // Translate view pixels into Vtk pixels
    vtkRegion = display2vtk(region);

    if (vtkRegion.isEmpty())
      return msel;

//     if (SelectionManager::instance()->voi() && !SelectionManager::instance()->voi()->contains(vtkRegion))
//     {
//       return;
//     }

    // Apply filtering criteria at given region
    foreach(QString filter, filters)
    {
      //! Special case, where sample is selected
      if (filter == SelectionHandler::EspINA_Channel)
      {
	Channel *channel = m_channels.keys().first();

	SelectionHandler::Selelection selChannel(vtkRegion,channel);
	msel.append(selChannel);
      } //! Select all segmented objects
//       else if (filter == "EspINA_Segmentation")
//       {
// 	foreach(Segmentation *seg, pickSegmentationsAt(vtkRegion))
// 	{
// 	  ISelectionHandler::Selelection selSegmentaion;
// 	  selSegmentaion.first = vtkRegion;
// 	  selSegmentaion.second = seg;
// 	  msel.append(selSegmentaion);
// 	}
//       }
      else
      {
	// Find segmented objects inside regions
	// Discard by filter
	// Adjust spacing
	Q_ASSERT(false); //TODO: Taxonomy selection
	//NOTE: should other filtering criterias be implemented?? Size??
      }
    }
  }
  //TODO: Update Qt selection

  return msel;
}


//-----------------------------------------------------------------------------
pqRenderViewBase* SliceView::view()
{
  return m_view;
}


//-----------------------------------------------------------------------------
void SliceView::onConnect()
{
  qDebug() << this << ": Connecting to a new server";

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
//   QObject::connect(m_viewWidget, SIGNAL(mouseEvent(QMouseEvent *)),
//                    this, SLOT(vtkWidgetMouseEvent(QMouseEvent *)));
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
  qDebug() << this << ": Disconnecting from server";
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
void SliceView::scrollValueChanged(int pos)
{
  if (m_fitToGrid)
    m_view->setSlice(m_gridSize[m_plane]*pos);
  else
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
    int numSteps = we->delta()/8/15*(m_preferences->invertWheel()?-1:1);//Refer to QWheelEvent doc.
    m_spinBox->setValue(m_spinBox->value() - numSteps);
    e->ignore();
  }else if (e->type() == QEvent::Enter)
  {
    QWidget::enterEvent(e);
    QApplication::setOverrideCursor(SelectionManager::instance()->cursor());
    e->accept();
  }else if (e->type() == QEvent::Leave)
  {
    QWidget::leaveEvent(e);
    QApplication::restoreOverrideCursor();
    e->accept();
  }else if (e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    if (me->button() == Qt::LeftButton && me->modifiers() == Qt::CTRL)
    {
      centerViewOnMousePosition();
    }
  }

  return QWidget::eventFilter(caller, e);
}

//-----------------------------------------------------------------------------
void SliceView::centerViewOnMousePosition()
{
  int xPos, yPos;
  eventPosition(xPos, yPos);

  double center[3];//World coordinates
  pickChannel(xPos, yPos, center);

  centerViewOn(center);
}

//-----------------------------------------------------------------------------
void SliceView::updateChannelOpacity()
{
  double numVisibleRep = 0;

  foreach(Channel *channel, m_channels.keys())
    if (channel->isVisible())
      numVisibleRep++;

  if (numVisibleRep == 0)
    return;

  double opacity = 1.0 /  numVisibleRep;

  foreach(Channel *channel, m_channels.keys())
  {
    vtkSMRepresentationProxy *rep = m_channels[channel];
    vtkSMPropertyHelper(rep, "Opacity").Set(&opacity,1);
    rep->UpdateVTKObjects();
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
  vtkRenderer * renderer = view->GetRenderer();
  Q_ASSERT(renderer);

  vtkPropPicker *propPicker = vtkPropPicker::New();
//   vtkPropCollection *col = vtkPropCollection::New();
//   qDebug() << propPicker->PickProp(x, y, renderer, col);
  if (!propPicker->Pick(x, y, 0.1, renderer))
    return false;

  propPicker->GetPickPosition(pickPos);

  pickPos[m_plane] = m_fitToGrid?m_scrollBar->value()*m_gridSize[m_plane]:m_scrollBar->value();
  qDebug() << "Pick Position" << pickPos[0] << pickPos[1] << pickPos[2];
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
  m_channels[channel] = reprProxy;

    // Set the reprProxy's input.
  pqSMAdaptor::setInputProperty(reprProxy->GetProperty("Input"),
				source->getProxy(), oport->getPortNumber());
  int pos[3];
  channel->position(pos);
  vtkSMPropertyHelper(reprProxy, "Position").Set(pos,3);
  double color = channel->color();
  vtkSMPropertyHelper(reprProxy, "Color").Set(&color,1);
  updateChannelOpacity();

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
  vtkSMRepresentationProxy *repProxy = m_channels[channel];
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
  m_segmentations[seg].selected = !seg->selected();
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
  SegRep rep = m_segmentations[seg];
  // Remove the reprProxy to render module.
  pqSMAdaptor::removeProxyProperty(
    viewModuleProxy->GetProperty("Representations"), rep.proxy);
  viewModuleProxy->UpdateVTKObjects();
  m_view->getProxy()->UpdateVTKObjects();

  rep.proxy->Delete();
  m_segmentations.remove(seg);
}

//-----------------------------------------------------------------------------
void SliceView::addRepresentation(pqOutputPort* oport)
{
  pqPipelineSource *source = oport->getSource();
  vtkSMProxyManager   *pxm = vtkSMProxyManager::GetProxyManager();

  vtkSMRepresentationProxy* reprProxy = vtkSMRepresentationProxy::SafeDownCast(
    pxm->NewProxy("representations", "SegmentationRepresentation"));
  Q_ASSERT(reprProxy);
  prevRep = reprProxy;

    // Set the reprProxy's input.
  pqSMAdaptor::setInputProperty(reprProxy->GetProperty("Input"),
				source->getProxy(), oport->getPortNumber());
// //   vtkSMPropertyHelper(reprProxy,"Type").Set(1);
  reprProxy->UpdateVTKObjects();

  vtkSMProxy* viewModuleProxy = m_view->getProxy();
  // Add the reprProxy to render module.
  pqSMAdaptor::addProxyProperty(
    viewModuleProxy->GetProperty("Representations"), reprProxy);
  viewModuleProxy->UpdateVTKObjects();

  // Following code could be ignored
//   pqApplicationCore* core= pqApplicationCore::instance();
//   pqDataRepresentation* repr = core->getServerManagerModel()->
//   findItem<pqDataRepresentation*>(reprProxy);
//   if (repr )
//   {
//     repr->setDefaultPropertyValues();
//   }
}

//-----------------------------------------------------------------------------
void SliceView::removeRepresentation(pqOutputPort* oport)
{
  vtkSMProxy* viewModuleProxy = m_view->getProxy();
  // Add the reprProxy to render module.
  pqSMAdaptor::removeProxyProperty(
    viewModuleProxy->GetProperty("Representations"), prevRep);
  viewModuleProxy->UpdateVTKObjects();
//   m_view->getRenderViewProxy()->GetRenderer()->RemoveAllViewProps();
//   m_view->getRenderViewProxy()->GetOverviewRenderer()->RemoveAllViewProps();
  m_view->getProxy()->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
bool SliceView::updateSegmentationRepresentation(Segmentation* seg)
{
  Q_ASSERT(m_segmentations.contains(seg));
  SegRep &rep = m_segmentations[seg];
  if (seg->outputPort() != rep.outport)
  {
    //remove representation using previous proxy
    removeSegmentationRepresentation(seg);
    //add representation using new proxy
    addSegmentationRepresentation(seg);
    return true;
  }
  if (seg->selected() != rep.selected
    || seg->visible() != rep.visible
    || m_colorEngine->color(seg) != rep.color)
  {
    rep.selected = seg->selected();
    rep.visible  = seg->visible();
    rep.color = m_colorEngine->color(seg);
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
  addRepresentation(preview);
}

void SliceView::removePreview(pqOutputPort* preview)
{
  removeRepresentation(preview);
}

//-----------------------------------------------------------------------------
void SliceView::addWidget(pq3DWidget* widget)
{
  widget->setView(m_view);
  widget->setWidgetVisible(true);
  widget->select();
  m_widgets << widget;
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
//     qDebug() << "Rendering View";
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
void SliceView::centerViewOn(double center[3])
{
  if (m_center[0] == center[0] &&
      m_center[1] == center[1] &&
      m_center[2] == center[2])
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

  m_view->centerViewOn(m_center[0], m_center[1], m_center[2]);
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