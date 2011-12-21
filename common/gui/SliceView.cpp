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

    You shoulh this program.  If not, see <http://www.gnu.org/licenses/>.
*/
// NOTE: vtkRenderer::RemoveAllViewProps()  maybe free the memory of the representations...
#include "SliceView.h"

// Debug
#include <QDebug>
#include <QPushButton>
// #include "espina_debug.h"
// 
// // EspINA
// #include "interfaces.h"
// #include "filter.h"
// #include "sample.h"
// #include "segmentation.h"
#include "../../Views/EspinaView.h"
#include "../Views/vtkSMEspinaViewProxy.h"

// Qt includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QSpinBox>
// #include <QMouseEvent>

// ParaQ includes
#include <pqActiveObjects.h>
#include <pqApplicationCore.h>
#include <pqDisplayPolicy.h>
#include <pqObjectBuilder.h>
#include <pqPipelineSource.h>
#include <pqRenderView.h>
#include <vtkSMRepresentationProxy.h>
#include <pqSMAdaptor.h>

// #include "pqDisplayPolicy.h"
// #include "pqPipelineRepresentation.h"
// //TODO: OLDVERSION #include "vtkSMImageSliceRepresentationProxy.h"
// #include "vtkSMIntVectorProperty.h"
// #include "vtkSMViewProxy.h"
// #include "vtkSMRenderViewProxy.h"
// #include "vtkSMRenderViewProxy.h"
// #include "vtkPVGenericRenderWindowInteractor.h"
// #include "vtkRenderWindowInteractor.h"
// #include "vtkAbstractPicker.h"
// #include "vtkPropCollection.h"
// #include "vtkPVDataInformation.h"
// #include "pqOutputPort.h"
// #include "vtkSMOutputPort.h"
// #include "selectionManager.h"
// #include <vtkInteractorObserver.h>
// #include <vtkInteractorStyleImage.h>
// #include <vtkPVInteractorStyle.h>
//
//
// #include <vtkSMInputProperty.h>
// #include <vtkSMDoubleVectorProperty.h>
// #include <proxies/vtkSMRGBALookupTableProxy.h>
// #include <pqScalarsToColors.h>
// #include <pqLookupTableManager.h>
// #include <vtkRenderer.h>
// #include <vtkRenderWindow.h>
// #include <vtkCamera.h>
// #include <vtkSMPropertyHelper.h>
// #include <pq3DWidget.h>
// #include <vtkSMNewWidgetRepresentationProxy.h>
// #include <vtkSMBoxRepresentationProxy.h>
// #include <vtkBoxWidget2.h>
// #include <vtkObjectFactory.h>
// #include <vtkBoxRepresentation.h>
// #include <vtkWidgetEventTranslator.h>
// 
// #include <pqPipelineFilter.h>
// #include <QApplication>
// #include <crosshairExtension.h>
// 
// #include <vtkPropPicker.h>
// #include <vtkProperty.h>
// #include <QSettings>
// #include <QCheckBox>
// #include <SegmentationSelectionExtension.h>
// 
// #include <vtkCubeSource.h>
// #include <vtkPolyDataMapper.h>
// 
// class vtkInteractorStyleEspina : public vtkInteractorStyleImage
// {
// public:
//   static vtkInteractorStyleEspina *New();
//   vtkTypeMacro(vtkInteractorStyleEspina,vtkInteractorStyleImage);
//   
//   virtual void OnMouseWheelForward(){}
//   virtual void OnMouseWheelBackward(){}
//   virtual void OnMouseMove();
// };
// 
// vtkStandardNewMacro(vtkInteractorStyleEspina);
// 
// //-----------------------------------------------------------------------------
// void vtkInteractorStyleEspina::OnMouseMove()
// {
//   if (Interactor->GetControlKey())
//     return;
//   
//   vtkInteractorStyleImage::OnMouseMove();
// }
// 
// 
// //-----------------------------------------------------------------------------
// SliceViewPreferencesPanel::SliceViewPreferencesPanel(SliceViewPreferences* preferences)
// : QWidget(preferences)
// , m_pref(preferences)
// {
//   QVBoxLayout *layout = new QVBoxLayout();
// 
//   QCheckBox *invertWheel = new QCheckBox(tr("Invert Wheel"));
//   invertWheel->setChecked(m_pref->invertWheel());
//   connect(invertWheel,SIGNAL(toggled(bool)),
// 	  this, SLOT(setInvertWheel(bool)));
//   
// //   QCheckBox *invertNormal = new QCheckBox(tr("Invert Normal"));
// //   invertNormal->setChecked(m_pref->invertNormal());
// //   connect(invertNormal,SIGNAL(toggled(bool)),
// // 	  this, SLOT(setInvertNormal(bool)));
// 
//   QCheckBox *showAxis = new QCheckBox(tr("Show Axis"));
//   showAxis->setChecked(m_pref->showAxis());
//   connect(showAxis,SIGNAL(toggled(bool)),
// 	  this, SLOT(setShowAxis(bool)));
//   
//   layout->addWidget(invertWheel);
// //   layout->addWidget(invertNormal);
//   layout->addWidget(showAxis);
//   QSpacerItem *spacer = new QSpacerItem(10,10,QSizePolicy::Expanding,QSizePolicy::Expanding);
//   layout->addSpacerItem(spacer);
//   setLayout(layout);
// }
// 
// //-----------------------------------------------------------------------------
// void SliceViewPreferencesPanel::setInvertWheel(bool value)
// {
//   m_pref->setInvertWheel(value);
// }
// 
// //-----------------------------------------------------------------------------
// void SliceViewPreferencesPanel::setInvertNormal(bool value)
// {
//   m_pref->setInvertNormal(value);
// }
// 
// //-----------------------------------------------------------------------------
// void SliceViewPreferencesPanel::setShowAxis(bool value)
// {
//   m_pref->setShowAxis(value);
// }
// 
// 
// //-----------------------------------------------------------------------------
// SliceViewPreferences::SliceViewPreferences(ViewType plane)
// : m_plane(plane)
// {
//   QSettings settings;
//   
//   switch (plane)
//   {
//     case VIEW_PLANE_XY:
//       viewSettings = "XYSliceView";    
//       break;
//     case VIEW_PLANE_YZ:
//       viewSettings = "YZSliceView";    
//       break;
//     case VIEW_PLANE_XZ:
//       viewSettings = "XZSliceView";    
//       break;
//     default:
//       assert(false);
//   };
//   
//   const QString wheelSettings = QString(viewSettings+"::invertWheel");
//   if (!settings.contains(wheelSettings))
//     settings.setValue(wheelSettings,false);
//   m_InvertWheel = settings.value(wheelSettings).toBool();
// 
//   const QString normalSettings = QString(viewSettings+"::invertNormal");
//   if (!settings.contains(normalSettings))
//     settings.setValue(normalSettings,false);
//   m_InvertNormal = settings.value(normalSettings).toBool();
//   
//   const QString axisSettings = QString(viewSettings+"::showAxis");
//   if (!settings.contains(axisSettings))
//     settings.setValue(axisSettings,false);
//   m_ShowAxis = settings.value(axisSettings).toBool();
// }
// 
// //-----------------------------------------------------------------------------
// const QString SliceViewPreferences::shortDescription()
// {
//   switch (m_plane)
//   {
//     case VIEW_PLANE_XY:
//       return "XY View";
//     case VIEW_PLANE_YZ:
//       return "YZ View";
//     case VIEW_PLANE_XZ:
//       return "XZ View";
//     default:
//       return "";
//   };
// }
// 
// //-----------------------------------------------------------------------------
// void SliceViewPreferences::setInvertWheel(bool value)
// {
//   QSettings settings;
//   
//   const QString wheelSettings = QString(viewSettings+"::invertWheel");
//   m_InvertWheel = value;
//   settings.setValue(wheelSettings,value);
// }
// 
// //-----------------------------------------------------------------------------
// void SliceViewPreferences::setInvertNormal(bool value)
// {
//   QSettings settings;
//   
//   const QString normalSettings = QString(viewSettings+"::invertNormal");
//   m_InvertNormal = value;
//   settings.setValue(normalSettings,value);
// }
// //-----------------------------------------------------------------------------
// void SliceViewPreferences::setShowAxis(bool value)
// {
//   QSettings settings;
//   
//   const QString axisSettings = QString(viewSettings+"::showAxis");
//   m_ShowAxis = value;
//   settings.setValue(axisSettings,value);
// }
// 
// 
// #define LOWER(coord) (2*(coord))
// #define UPPER(coord) (2*(coord) + 1)

//-----------------------------------------------------------------------------
SliceView::SliceView(QWidget* parent)
    : QAbstractItemView(parent)
//     , m_showSegmentations(true)
//     , m_plane(VIEW_PLANE_XY)
//     , m_sampleRep(NULL)
//     , m_focusedSample(NULL)
    , m_mainLayout   (new QVBoxLayout())
    , m_controlLayout(new QHBoxLayout())
    , m_viewWidget   (NULL)
    , m_scrollBar    (new QScrollBar(Qt::Horizontal))
    , m_spinBox      (new QSpinBox())
//     , m_view(NULL)
//     , m_regionCut(NULL)
{
  m_scrollBar->setMaximum(0);
  m_scrollBar->setSizePolicy(
      QSizePolicy::Expanding,
      QSizePolicy::Preferred);

  m_spinBox->setMaximum(0);
  m_spinBox->setMinimumWidth(40);
  m_spinBox->setSizePolicy(
      QSizePolicy::Minimum,
      QSizePolicy::Preferred);

  QPushButton *fakeLoad = new QPushButton(tr("Load Test Stack"));
  connect(fakeLoad,SIGNAL(clicked(bool)),
          this, SLOT(loadTestImage()));
  m_controlLayout->addWidget(fakeLoad);
  
//   QObject::connect(m_scrollBar, SIGNAL(valueChanged(int)), this, SLOT(setSlice(int)));
//   QObject::connect(m_spinBox, SIGNAL(valueChanged(int)), this, SLOT(setSlice(int)));
//   connect(SelectionManager::instance(),SIGNAL(VOIChanged(IVOI*)),this,SLOT(setVOI(IVOI*)));
  m_controlLayout->addWidget(m_scrollBar);
  m_controlLayout->addWidget(m_spinBox);

//   m_viewWidget->setSizePolicy(
//        QSizePolicy::Expanding,
//        QSizePolicy::Expanding);
//   m_viewWidget->setStyleSheet("background-color: black;");

//   m_mainLayout->addWidget(m_viewWidget);
  m_mainLayout->addLayout(m_controlLayout);
//   this->setAutoFillBackground(true);
  setLayout(m_mainLayout);

  // Color background
  QPalette pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Window));
  this->setPalette(pal);
  this->setStyleSheet("QSpinBox { background-color: white;}");
  
//   m_preferences = new SliceViewPreferences(m_plane);
  qDebug() << this << ": Created";
}

//-----------------------------------------------------------------------------
SliceView::~SliceView()
{
  qDebug() << this << ": Destroyed";
  disconnect();
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
// //! If several regions select the same object, consistently it will return
// //! the same number of (regions,elements) as selected objects. Thus, the client
// //! that decided such configuration has to resolve it
// //-----------------------------------------------------------------------------
// void SliceView::setSelection(SelectionFilters& filters, ViewRegions& regions)
// {
//   //TODO: Discuss if we apply VOI at application level or at plugin level
//   // i.e. whether clicks out of VOI are discarted or not
//   ISelectionHandler::MultiSelection msel;
//   
//   //qDebug() << "EspINA::SliceView" << m_plane << ": Making selection";
//   // Select all products that belongs to all the regions
//   foreach(const QPolygonF &region, regions)
//   {
//     ISelectionHandler::VtkRegion vtkRegion;
//     // Translate view pixels into Vtk pixels
//     vtkRegion = display2vtk(region);
//     
//     if (vtkRegion.isEmpty())
//       return;
//     
//     if (SelectionManager::instance()->voi() && !SelectionManager::instance()->voi()->contains(vtkRegion))
//     {
//       return;
//     }
//     
//     // Apply filtering criteria at given region
//     foreach(QString filter, filters)
//     {
//       //! Special case, where sample is selected
//       if (filter == "EspINA_Sample")
//       {
// 	ISelectionHandler::Selelection selSample;
// 	selSample.first = vtkRegion;
// 	selSample.second = m_focusedSample;
// 	msel.append(selSample);
//       } //! Select all segmented objects
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
//       else 
//       {
// 	// Find segmented objects inside regions
// 	// Discard by filter
// 	// Adjust spacing
// 	assert(false); //TODO: Taxonomy selection 
// 	//NOTE: shall other filtering criterias be implemented?? Size??
//       }
//     }
//   }
//   //TODO: Update Qt selection
//   // Notify the manager about the new selection
//   SelectionManager::instance()->setSelection( msel);
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
// //-----------------------------------------------------------------------------
// bool SliceView::eventFilter(QObject* obj, QEvent* event)
// {
//   if (event->type() == QEvent::Wheel)
//   {
//     QWheelEvent *we = static_cast<QWheelEvent *>(event);
//     int numSteps = we->delta()/8/15*(m_preferences->invertWheel()?-1:1);//Refer to QWheelEvent doc.
//     m_spinBox->setValue(m_spinBox->value() - numSteps);
//     event->ignore();
//   }else if (event->type() == QEvent::Enter)
//   {
//     QWidget::enterEvent(event);
//     QApplication::setOverrideCursor(SelectionManager::instance()->cursor());
//     event->accept();
//   }else if (event->type() == QEvent::Leave)
//   {
//     QWidget::leaveEvent(event);
//     QApplication::restoreOverrideCursor();
//     event->accept();
//   }
//   return QObject::eventFilter(obj, event);
// }
// 

//-----------------------------------------------------------------------------
void SliceView::onConnect()
{
  qDebug() << this << ": Connecting to a new server";

  pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
  pqServer    *server = pqActiveObjects::instance().activeServer();

  m_view = qobject_cast<EspinaView*>(ob->createView(
             EspinaView::espinaRenderViewType(), server));
//   connect(m_view,SIGNAL(beginRender()),this,SLOT(beginRender()));
//   connect(m_view,SIGNAL(endRender()),this,SLOT(endRender()));

  m_viewWidget = m_view->getWidget();
//   m_viewWidget->installEventFilter(this);
//   QObject::connect(m_viewWidget, SIGNAL(mouseEvent(QMouseEvent *)),
//                    this, SLOT(vtkWidgetMouseEvent(QMouseEvent *)));
  m_mainLayout->insertWidget(0, m_viewWidget);//To preserve view order

//   m_viewProxy = vtkSMRenderViewProxy::SafeDownCast(m_view->getProxy());
//   assert(m_viewProxy);
//   
//   m_rwi = vtkRenderWindowInteractor::SafeDownCast(
//     m_viewProxy->GetRenderWindow()->GetInteractor());
//   assert(m_rwi);
//   
//   
//   m_style = vtkInteractorStyleEspina::New();
//   m_rwi->SetInteractorStyle(m_style);
//   
// //   vtkRenderWindow *clientRW = m_view->getRenderViewProxy()->GetRenderWindow();
// //   vtkSmartPointer<vtkRenderer> clientRenderer = vtkSmartPointer<vtkRenderer>::New();
// //   
// //   vtkSmartPointer<vtkCubeSource> cube = vtkSmartPointer<vtkCubeSource>::New();
// //   vtkPolyDataMapper *cubeMapper = vtkPolyDataMapper::New();
// //   cubeMapper->SetInput(cube->GetOutput());
// //   vtkActor * cubeActor = vtkActor::New();
// //   cubeActor->SetMapper(cubeMapper);
// //   clientRenderer->AddActor(cubeActor);
// //   clientRenderer->SetViewport(0.5,0,1,0.5);
// //   
// //   clientRW->AddRenderer(clientRenderer);
// //   clientRW->AlphaBitPlanesOn();
// //   clientRW->SetDoubleBuffer(true);
// //   clientRW->SetNumberOfLayers(2);
// //   clientRenderer->SetLayer(1);
//   
//   m_cam = m_viewProxy->GetActiveCamera();
//   assert(m_cam);
//   
//   m_cam->ParallelProjectionOn();
//   
//   switch (m_plane)
//   {
//     case VIEW_PLANE_XY:
//       m_cam->SetPosition(0, 0, -1);
//       m_cam->SetFocalPoint(0, 0, 0);
//       m_cam->SetRoll(180);
//       break;
//     case VIEW_PLANE_YZ:
//       m_cam->SetPosition(1, 0, 0);
//       m_cam->SetFocalPoint(0, 0, 0);
//       m_cam->SetRoll(180);
//       break;
//     case VIEW_PLANE_XZ:
// //       m_cam->SetPosition(0, 1, 0);
// //       m_cam->SetFocalPoint(0, 0, 0);
//       m_cam->Roll(90);
//       m_cam->Azimuth(90);
//       m_cam->Roll(90);
//       m_cam->Elevation(180);
//       break;
//     default:
//       assert(false);
//   };
//     
//   double black[3] = {0, 0, 0};
//   m_view->getRenderViewProxy()->GetRenderer()->SetBackground(black);
//   m_view->setCenterAxesVisibility(false);
//   m_view->setOrientationAxesVisibility(m_preferences->showAxis());
//   //m_view->resetCamera();
// 
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
//     m_style->Delete();
//     m_view = NULL;
//     m_viewWidget = NULL;
//   }
}

//-----------------------------------------------------------------------------
void SliceView::loadTestImage()
{
  qDebug() << this << ": Loading Test Image";
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  pqPipelineSource *img = builder->createReader("sources","PVDReader",
						QStringList("/home/jpena/Stacks/paraPeque.pvd"),
						pqActiveObjects::instance().activeServer());
  vtkSMEspinaViewProxy *ep =  vtkSMEspinaViewProxy::SafeDownCast(m_view->getViewProxy());
  Q_ASSERT(ep);
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  dp->createPreferredRepresentation(img->getOutputPort(0),m_view,true);
  m_scrollBar->setMaximum(114);
  m_spinBox->setMaximum(114);
}

// //-----------------------------------------------------------------------------
// void SliceView::showSegmentations(bool value)
// {
//   if (m_showSegmentations != value)
//   {
//     m_showSegmentations = value;
//     updateScene();
//   }
// }
// 
// //-----------------------------------------------------------------------------
// void SliceView::setPlane(ViewType plane)
// {
//   m_plane = plane;
//   delete m_preferences;
//   m_preferences = new SliceViewPreferences(m_plane);
// }
// 
// 
// //-----------------------------------------------------------------------------
// QRegion SliceView::visualRegionForSelection(const QItemSelection& selection) const
// {
// //   qDebug() << "Visual region required";
//   return QRect();
// }
// 
// //-----------------------------------------------------------------------------
// void SliceView::setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command)
// {
//   qDebug() << "Selection in sliceview";
// }
// 
// //-----------------------------------------------------------------------------
// bool SliceView::isIndexHidden(const QModelIndex& index) const
// {
//   if (!index.isValid())
//     return true;
// 
//   if (index.internalId() < 3)
//     return true;
// 
//   IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
//   EspinaProduct *actor = dynamic_cast<EspinaProduct *>(item);
//   return !actor;
// }
// 
// //-----------------------------------------------------------------------------
// int SliceView::verticalOffset() const
// {
//   return 0;
// }
// 
// //-----------------------------------------------------------------------------
// int SliceView::horizontalOffset() const
// {
//   return 0;
// }
// 
// //-----------------------------------------------------------------------------
// QModelIndex SliceView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
// {
//   return QModelIndex();
// }
// 
// //-----------------------------------------------------------------------------
// void SliceView::rowsInserted(const QModelIndex& parent, int start, int end)
// {
//   //QAbstractItemView::rowsInserted(parent, start, end);
//   
//   //TODO: Multi samples
//   assert(start == end);// Only 1-row-at-a-time inserts are allowed
//   
//   //QModelIndex index = parent.child(r,0);
//   QModelIndex index  = model()->index(start, 0, parent);
//   IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
//   // Check for sample
//   Sample *sample = dynamic_cast<Sample *>(item);
//   if (sample)
//   {
//     //Use croshairs representation
//     int mextent[6];
//     sample->extent(mextent);
//     int normalCoorToPlane = (m_plane + 2) % 3;
//     int sliceOffset = m_plane==VIEW_PLANE_XY?1:0;
//     int minSlices = mextent[2*normalCoorToPlane] + sliceOffset;
//     int maxSlices = mextent[2*normalCoorToPlane+1] + sliceOffset;
//     m_scrollBar->setMinimum(minSlices);
//     m_spinBox->setMinimum(minSlices);
//     m_scrollBar->setMaximum(maxSlices);
//     m_spinBox->setMaximum(maxSlices);
// 
//     m_sampleRep = dynamic_cast<CrosshairExtension::SampleRepresentation *>(sample->representation("Crosshairs"));
//     connect(m_sampleRep,SIGNAL(representationUpdated()),this,SLOT(updateScene()));
//     connect(sample,SIGNAL(updated(Sample*)),this,SLOT(updateScene()));
//     m_sampleRep->render(m_view,m_plane);
//     
//     m_focusedSample = sample;
//     m_view->resetCamera();
//   }  
//   updateScene();
// }
// 
// //-----------------------------------------------------------------------------
// void SliceView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
// {
//   //QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
//   pqApplicationCore *core = pqApplicationCore::instance();
//   pqObjectBuilder *ob = core->getObjectBuilder();
//   assert(start == end);
// 
//   QModelIndex index = model()->index(start, 0, parent);
//   IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
//   // Check for sample
//   Sample *sample = dynamic_cast<Sample *>(item);
//   if (sample)
//   {
//     m_focusedSample = NULL;
//     m_sampleRep = NULL;
//     foreach(pqRepresentation *rep, m_view->getRepresentations())
//     {
//       rep->setVisible(false);
//       ob->destroy(rep);
//     }
//     m_view->getRenderViewProxy()->GetRenderer()->RemoveAllViewProps();
//   }
//   updateScene();
// }
// 
// //-----------------------------------------------------------------------------
// void SliceView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
// {
//   if (!topLeft.isValid() || !bottomRight.isValid())
//     return;
//   
//   updateScene();
// }
// 
// 
// // //-----------------------------------------------------------------------------
// // void SliceView::focusOnSample(Sample* sample)
// // {
// //   s_focusedSample = sample;
// //   s_blender->focusOnSample(sample);
// //   if (!m_slicer)
// //   {
// //     pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
// //     m_slicer = ob->createFilter("filters", "ImageSlicer", sample->creator()->pipelineSource(), sample->portNumber());
// //     setPlane(m_plane);
// //     sample->creator()->pipelineSource()->updatePipeline();
// //     /*
// //     vtkPVDataInformation *info = sample->outputPort()->getDataInformation();
// //     double *bounds = info->GetBounds();
// //     int *extent = info->GetExtent();
// //     */
// //     int mextent[6];
// //     sample->extent(mextent);
// //     int normalCoorToPlane = (m_plane + 2) % 3;
// //     int numSlices = mextent[2*normalCoorToPlane+1];
// //     m_scrollBar->setMaximum(numSlices);
// //     m_spinBox->setMaximum(numSlices);
// // 
// //     pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
// //     pqDataRepresentation *rep = dp->setRepresentationVisibility(m_slicer->getOutputPort(0), m_view, true);
// //     
// //     m_view->resetCamera();
// //   }
// //   else
// //   {
// //     qDebug() << "Need to change slicer input";
// //     //TODO: change slicer input
// //     //assert(false);
// //   }
// // 
// //   double camPoint[3], sampleBound[6];
// //   sample->bounds(sampleBound);
// // }
// 
// 
// //-----------------------------------------------------------------------------
// void SliceView::centerViewOn(int x, int y, int z)
// {
//   //qDebug() << "Center view on" << x << y << z;
//   if (m_sampleRep)
//     m_sampleRep->centerOn(x,y,z);
// }
// 
// //-----------------------------------------------------------------------------
// ISelectionHandler::VtkRegion SliceView::display2vtk(const QPolygonF &region)
// {
//   //Use Render Window Interactor's Picker to find the world coordinates
//   //of the stack
//   //vtkSMRenderViewProxy* renModule = view->GetRenderWindow()->GetInteractor()->GetRenderView();
//   ISelectionHandler::VtkRegion vtkRegion;
//   
//   //! thus, use its spacing
//   double spacing[3];//Image Spacing
//   m_focusedSample->spacing(spacing);
//   
//   double pickPos[3];//World coordinates
//   vtkPropPicker *wpicker = vtkPropPicker::New();
//   foreach(QPointF point, region)
//   {  
//     wpicker->Pick(point.x(), point.y(), 0.1, m_viewProxy->GetRenderer());
//     wpicker->GetPickPosition(pickPos);
//     qDebug() << "Second Picked pixel" << pickPos[0] << pickPos[1] << pickPos[2];
//     if (round(pickPos[2]) < 0)
//       return vtkRegion;
//     
//     Point vtkPoint;
//     for (int i=0; i<3; i++)
//       vtkPoint[i] = round(pickPos[i] / spacing[i]);
//     vtkRegion << vtkPoint;
//   }
//   return vtkRegion;
// 
// }
// 
// 
// 
// //-----------------------------------------------------------------------------
// pqRenderView* SliceView::view()
// {
//   return m_view;
// }
// 
// 
// //-----------------------------------------------------------------------------
// void SliceView::vtkWidgetMouseEvent(QMouseEvent* event)
// {
//   if (!m_sampleRep)
//     return;
//   
//   //Use Render Window Interactor's to obtain event's position
//   vtkSMRenderViewProxy* view = 
//     vtkSMRenderViewProxy::SafeDownCast(m_view->getProxy());
//   //vtkSMRenderViewProxy* renModule = view->GetRenderView();
//   vtkRenderWindowInteractor *rwi =
//     vtkRenderWindowInteractor::SafeDownCast(
//       view->GetRenderWindow()->GetInteractor());
//       //renModule->GetInteractor());
//   assert(rwi);
// 
//   int xPos, yPos;
//   rwi->GetEventPosition(xPos, yPos);
//   //qDebug() << "EspINA::SliceView" << m_plane << ": Clicked Position" << xPos << " " << yPos;
//   QPoint pos(xPos,yPos);
//   
//   if (event->type() == QMouseEvent::MouseButtonPress &&
//       event->buttons() == Qt::LeftButton)
//   {
//     double spacing[3];//Image Spacing
//     m_focusedSample->spacing(spacing);
//   
//     double pickPos[3];//World coordinates
//     vtkPropPicker *wpicker = vtkPropPicker::New();
//     //TODO: Check this--> wpicker->AddPickList();
//     wpicker->Pick(xPos, yPos, 0.1, m_viewProxy->GetRenderer());
//     wpicker->GetPickPosition(pickPos);
//     
//     if (pickPos[0] == 0 && pickPos[1] == 0 && pickPos[2] == 0)
//     {
//       qDebug() << "Ignoring picking outside sample";
//       return;
//     }
//    
// //    qDebug() << "Picked pixel" << pickPos[0] << pickPos[1] << pickPos[2];
//     SelectionManager::instance()->onMouseDown(pos, this);
//     //qDebug() << "Pick Position:" << pickPos[0] << pickPos[1] << pickPos[2];
//     int selectedPixel[3];
//     for(int dim = 0; dim < 3; dim++)
//       selectedPixel[dim] = round(pickPos[dim]/spacing[dim]);
//     if (rwi->GetControlKey())
//       centerViewOn(selectedPixel[0], selectedPixel[1], selectedPixel[2]);
//     
//     selectSegmentations(selectedPixel[0], selectedPixel[1], selectedPixel[2]);
//     
//   }
//   //BUG: Only MouseButtonPress events are received
//   if (event->type() == QMouseEvent::MouseMove &&
//       event->buttons() == Qt::LeftButton)
//   {
//     SelectionManager::instance()->onMouseMove(pos, this);
//   }
//   if (event->type() == QMouseEvent::MouseButtonRelease &&
//       event->buttons() == Qt::LeftButton)
//   {
//     SelectionManager::instance()->onMouseUp(pos, this);
//   }
// }
// 
// //-----------------------------------------------------------------------------
// void SliceView::setSlice(int slice)
// {
//   
//   if (m_spinBox->value() != slice)
//   {
//     m_spinBox->setValue(slice);
//     updateVOIVisibility();
//   }
//   if (m_scrollBar->value() != slice)
//   {
//     m_scrollBar->setValue(slice);
//     updateVOIVisibility();
//   }
// 
//   if (m_sampleRep)
//   {
//     int sliceOffset = m_plane==VIEW_PLANE_XY?1:0;
//     m_sampleRep->setSlice(slice-sliceOffset,m_plane);
//   }
//   
// }
// 
// //-----------------------------------------------------------------------------
// void SliceView::setVOI(IVOI* voi)
// {
//   if (m_VOIWidget)
//   {
//     m_VOIWidget->deselect();
//     m_VOIWidget->setVisible(false);
//     m_voi->deleteWidget(m_VOIWidget);
//   }
//   
//   if(!m_focusedSample)
//     return;
//   
//   m_voi = voi;
//   
//   if (!voi)
//     return;
//  
//   
//   m_VOIWidget = voi->newWidget(m_plane);
//   m_VOIWidget->setView(m_view);
//   m_VOIWidget->setWidgetVisible(true);
//   m_VOIWidget->select();
// //   m_VOIWidget->accept();
//   m_voi->resizeToDefaultSize();
// //   m_VOIWidget->accept(); //Required to initialize internal proxy properties
//   
//   connect(m_voi,SIGNAL(voiModified()),this,SLOT(updateVOIVisibility()));
//   
//   updateVOIVisibility();
// }
// 
// //-----------------------------------------------------------------------------
// void SliceView::updateVOIVisibility()
// {
// //   std::cout << "updating voi in plane: " << m_plane << std::endl;
//   if (!m_VOIWidget)
//     return;
// 
//   int sliceOffset = m_plane==VIEW_PLANE_XY?1:0;
//   if (m_voi->intersectPlane(m_plane,m_spinBox->value()-sliceOffset))
//     m_VOIWidget->setWidgetVisible(true);
//   else
//     m_VOIWidget->setWidgetVisible(false);
// }
// 
// 
// //-----------------------------------------------------------------------------
// void SliceView::updateScene()
// {
//   if (m_sampleRep)
//   {
//     int sliceOffset = m_plane==VIEW_PLANE_XY?1:0;
//     int newSlice = m_sampleRep->slice(m_plane)+sliceOffset;
//     if (newSlice != m_spinBox->value())
//       setSlice(m_sampleRep->slice(m_plane)+sliceOffset);
// //   std::cout << "Render in SliceView" << std::endl;
//       ISampleExtension *ext = m_focusedSample->extension("CountingRegionExtension");
//       if (ext)
//       {
// 	foreach(QString rep, ext->availableRepresentations())
// 	  m_focusedSample->representation(rep)->render(m_view,m_plane);
//       }
//   }
//   m_view->render();
// //   m_view->forceRender();
// }
// 
// void SliceView::beginRender()
// {
//   QApplication::setOverrideCursor(Qt::WaitCursor);
// }
// 
// 
// void SliceView::endRender()
// {
//   QApplication::restoreOverrideCursor();
// }