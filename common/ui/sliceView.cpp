/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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
#include "sliceView.h"

#include "interfaces.h"
#include "filter.h"
// ParaQ includes
#include "pqRenderView.h"
#include "pqApplicationCore.h"
#include "pqActiveObjects.h"
#include "pqDisplayPolicy.h"
#include "pqObjectBuilder.h"
#include "pqPipelineRepresentation.h"
#include "vtkSMImageSliceRepresentationProxy.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMViewProxy.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkPVGenericRenderWindowInteractor.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkAbstractPicker.h"
#include "vtkPropCollection.h"
#include "vtkPVDataInformation.h"
#include "pqOutputPort.h"
#include "vtkSMOutputPort.h"
#include "selectionManager.h"
#include <vtkInteractorObserver.h>
#include <vtkInteractorStyleImage.h>
#include <vtkPVInteractorStyle.h>
// Qt includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QSpinBox>
#include <QMouseEvent>

// DEBUG
#include <QDebug>
#include <assert.h>

#include <pqPipelineSource.h>
#include <vtkSMInputProperty.h>
#include <vtkSMDoubleVectorProperty.h>
#include <proxies/vtkSMRGBALookupTableProxy.h>
#include <pqScalarsToColors.h>
#include <pqLookupTableManager.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkCamera.h>
#include <vtkSMPropertyHelper.h>
#include <pq3DWidget.h>
#include <vtkSMNewWidgetRepresentationProxy.h>
#include <vtkObjectFactory.h>

#include <pqPipelineFilter.h>
#include <QApplication>
#include <crosshairExtension.h>

#include <vtkPropPicker.h>

#define HINTWIDTH 40

/*
//-----------------------------------------------------------------------------
void Blender::setBackground(Sample* product)
{
  if (m_currentSample == product)
    return;
  
  m_currentSample = product;
  
  vtkSMProperty* p;
  
  pqApplicationCore *core = pqApplicationCore::instance();
  pqServer *server =  core->getActiveServer();
  pqObjectBuilder *ob = core->getObjectBuilder();
  pqLookupTableManager *lutManager = core->getLookupTableManager();
  
  //Map the background values using a lut
  // This filter is the output of the sliceBlender class when blending is off
  if (m_bgMapper)
  {
    assert(false);
  }
  m_bgMapper = ob->createFilter("filters", "ImageMapToColors", product->creator()->pipelineSource(),product->portNumber());
  assert(m_bgMapper);
  
  // Get (or create if it doesn't exit) the lut for the background image
  pqScalarsToColors *greyLUT = lutManager->getLookupTable(server, "Greyscale", 4, 0);
  if (greyLUT)
  {
    p = greyLUT->getProxy()->GetProperty("RGBPoints");
    vtkSMDoubleVectorProperty *rgbs = vtkSMDoubleVectorProperty::SafeDownCast(p);
    if (rgbs)
    {
      // TODO: Use segmentation's information
      double colors[8] = {0, 0, 0, 0, 255, 1, 1, 1};
      rgbs->SetElements(colors);
    }
    greyLUT->getProxy()->UpdateVTKObjects();
  }
  
  // Set the greyLUT for the mapper
  p = m_bgMapper->getProxy()->GetProperty("LookupTable");
  vtkSMProxyProperty *lut = vtkSMProxyProperty::SafeDownCast(p);
  if (lut)
  {
    lut->SetProxy(0, greyLUT->getProxy());
  }
  
  m_bgMapper->getProxy()->UpdateVTKObjects();
  
  m_imageBlender = ob->createFilter("filters", "ImageBlend", m_bgMapper);
  assert(m_imageBlender);
}


//-----------------------------------------------------------------------------
void Blender::blend(Segmentation* seg)
{
  if (m_blendingMappers.contains(seg))
    return;
  
  ISegmentationRepresentation *segMapper = seg->representation("Color");

  segMapper->pipelineSource()->getProxy()->UpdateVTKObjects();
  segMapper->pipelineSource()->updatePipeline();
  
  m_blendingMappers[seg] = segMapper;
  
  updateImageBlenderInput();
}

//-----------------------------------------------------------------------------
void Blender::unblend(Segmentation* seg)
{
  if (!m_blendingMappers.contains(seg))
    return;

  vtkSMProperty* p;
  vtkSMIntVectorProperty* intVectProp;
  vtkSMDoubleVectorProperty* doubleVectProp;
  ISegmentationRepresentation* rep = m_blendingMappers.take(seg);
  assert(rep);
  pqPipelineSource *mapper = rep->pipelineSource();
  assert(mapper);

  //std::cout << "N. Consumers of mapper before " << mapper->getNumberOfConsumers() << std::endl;
  //std::cout << "N. Producers of blender before " << m_imageBlender->getProxy()->GetNumberOfProducers() << std::endl;
  //p = m_imageBlender->getProxy()->GetProperty("Input");
  //vtkSMInputProperty *input = vtkSMInputProperty::SafeDownCast(p);
  //if (input)
  //{
  //input->RemoveProxy(mapper->getProxy());
  //}
  updateImageBlenderInput();
  //std::cout << "N. Consumers of mapper after update vtk " << mapper->getNumberOfConsumers() << std::endl;
  //std::cout << "N. Producers of blender after update vtk " << m_imageBlender->getProxy()->GetNumberOfProducers() << std::endl;
}

//-----------------------------------------------------------------------------
void Blender::clear()
{
  assert(m_blendingMappers.size() == 0);
  
  if (m_imageBlender->getNumberOfConsumers() > 0)
    return;
  
  qDebug() << "Blender: Destroying blender";
  pqApplicationCore *core = pqApplicationCore::instance();
  pqObjectBuilder *ob = core->getObjectBuilder();
  
  ob->destroy(m_imageBlender);
  ob->destroy(m_bgMapper);
  m_bgMapper = NULL;
  m_imageBlender = NULL;
  
  
}


//-----------------------------------------------------------------------------
void Blender::updateImageBlenderInput()
{
  m_mutex.lock();
  vtkSMProperty* p;

  vtkstd::vector<vtkSMProxy *> inputs;
  vtkstd::vector<unsigned int> ports;

  // Ensure sample's mapper is the first input
  inputs.push_back(m_bgMapper->getProxy());
  ports.push_back(0);

  foreach(ISegmentationRepresentation *rep, m_blendingMappers)
  {
    IModelItem *item = m_blendingMappers.key(rep);
    Segmentation *seg = dynamic_cast<Segmentation *>(item);
    if (seg->visible())
    {
      inputs.push_back(rep->pipelineSource()->getProxy());
      ports.push_back(0);
    }
  }
  
  p = m_imageBlender->getProxy()->GetProperty("Input");
  vtkSMInputProperty *input = vtkSMInputProperty::SafeDownCast(p);
  if (input)
  {
        //input->RemoveAllProxies();
    m_imageBlender->getProxy()->UpdateVTKObjects();
    input->SetProxies(static_cast<unsigned int>(inputs.size())
                      , &inputs[0]
                      , &ports[0]);
    m_imageBlender->getProxy()->UpdateVTKObjects();
  }
  m_mutex.unlock();
}
*/


class vtkInteractorStyleEspina : public vtkInteractorStyleImage
{
public:
  static vtkInteractorStyleEspina *New();
  vtkTypeMacro(vtkInteractorStyleEspina,vtkInteractorStyleImage);
  
  virtual void OnMouseWheelForward(){}
  virtual void OnMouseWheelBackward(){}
};

vtkStandardNewMacro(vtkInteractorStyleEspina);


#define LOWER(coord) (2*(coord))
#define UPPER(coord) (2*(coord) + 1)

//Blender *SliceView::s_blender = NULL;

//-----------------------------------------------------------------------------
SliceView::SliceView(QWidget* parent)
    : QAbstractItemView(parent)
    , m_showSegmentations(true)
    , m_plane(VIEW_PLANE_XY)
    , m_sampleRep(NULL)
    , m_focusedSample(NULL)
    , m_viewWidget(NULL)
    , m_view(NULL)
{
  m_controlLayout = new QHBoxLayout();
  m_scrollBar = new QScrollBar(Qt::Horizontal);
  m_scrollBar->setMaximum(0);
  m_scrollBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  m_spinBox = new QSpinBox();
  m_spinBox->setMaximum(0);
  m_spinBox->setMinimumWidth(HINTWIDTH);
  m_spinBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  QObject::connect(m_scrollBar, SIGNAL(valueChanged(int)), this, SLOT(setSlice(int)));
  QObject::connect(m_spinBox, SIGNAL(valueChanged(int)), this, SLOT(setSlice(int)));
  connect(SelectionManager::instance(),SIGNAL(VOIChanged(IVOI*)),this,SLOT(setVOI(IVOI*)));
  m_controlLayout->addWidget(m_scrollBar);
  m_controlLayout->addWidget(m_spinBox);

  m_mainLayout = new QVBoxLayout();
  m_mainLayout->addLayout(m_controlLayout);
  this->setAutoFillBackground(true);
  setLayout(m_mainLayout);

  // Color background
  QPalette pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Window));
  this->setPalette(pal);
  this->setStyleSheet("QSpinBox { background-color: white;}");
}

SliceView::~SliceView()
{
  disconnectFromServer();
}


//-----------------------------------------------------------------------------
QModelIndex SliceView::indexAt(const QPoint& point) const
{
  return QModelIndex();
}

//-----------------------------------------------------------------------------
void SliceView::scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint)
{
  IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
  Sample * sample = dynamic_cast<Sample *>(item);
  if (sample)
    qDebug() << "Scroll to Sample";
    //s_focusedSample = sample;
}

//-----------------------------------------------------------------------------
QRect SliceView::visualRect(const QModelIndex& index) const
{
  return QRect();
}

//! If several regions select the same object, consistently it will return
//! the same number of (regions,elements) as selected objects. Thus, the client
//! that decided such configuration has to resolve it
//-----------------------------------------------------------------------------
void SliceView::setSelection(SelectionFilters& filters, ViewRegions& regions)
{
  //TODO: Discuss if we apply VOI at application level or at plugin level
  // i.e. whether clicks out of VOI are discarted or not
  ISelectionHandler::Selection sel;
  
  //qDebug() << "EspINA::SliceView" << m_plane << ": Making selection";
  // Select all products that belongs to all the regions
  foreach(const QPolygonF &region, regions)
  {
    ISelectionHandler::VtkRegion vtkRegion;
    // Translate view pixels into Vtk pixels
    vtkRegion = display2vtk(region);
    
    if (SelectionManager::instance()->voi() && !SelectionManager::instance()->voi()->contains(vtkRegion))
      return;
    
    // Apply filtering criteria at given region
    foreach(QString filter, filters)
    {
      //! Special case, where sample is selected
      if (filter == "EspINA_Sample")
      {
	ISelectionHandler::SelElement selSample;
	selSample.first = vtkRegion;
	selSample.second = m_focusedSample;
	sel.append(selSample);
      }
      //! Select all segmented objects
      else 
      {
	// Find segmented objects inside regions
	// Discard by filter
	// Adjust spacing
	assert(false); //TODO: Taxonomy selection 
	//NOTE: shall other filtering criterias be implemented?? Size??
      }
    }
  }
  //TODO: Update Qt selection
  // Notify the manager about the new selection
  SelectionManager::instance()->setSelection(sel);
}


bool SliceView::eventFilter(QObject* obj, QEvent* event)
{
  if (event->type() == QEvent::Wheel)
  {
    QWheelEvent *we = static_cast<QWheelEvent *>(event);
    int numSteps = we->delta()/8/15;//Refer to QWheelEvent doc.
    m_spinBox->setValue(m_spinBox->value() + numSteps);
    event->ignore();
  }
  return QObject::eventFilter(obj, event);
}


//-----------------------------------------------------------------------------
void SliceView::connectToServer()
{
  //qDebug() << "Creating View";
  pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
  pqServer * server = pqActiveObjects::instance().activeServer();
  
  m_view = qobject_cast<pqRenderView*>(ob->createView(
             pqRenderView::renderViewType(), server));
  m_viewWidget = m_view->getWidget();
  m_viewWidget->installEventFilter(this);
  QObject::connect(m_viewWidget, SIGNAL(mouseEvent(QMouseEvent *)),
                   this, SLOT(vtkWidgetMouseEvent(QMouseEvent *)));
  
  m_mainLayout->insertWidget(0, m_viewWidget);//To preserve view order

  
  m_viewProxy = vtkSMRenderViewProxy::SafeDownCast(m_view->getProxy());
  assert(m_viewProxy);
  
  m_rwi = vtkRenderWindowInteractor::SafeDownCast(
    m_viewProxy->GetRenderWindow()->GetInteractor());
  assert(m_rwi);
  
  
  m_style = vtkInteractorStyleEspina::New();
  m_rwi->SetInteractorStyle(m_style);
  
  m_cam = m_viewProxy->GetActiveCamera();
  assert(m_cam);
  
  m_cam->ParallelProjectionOn();
  
  switch (m_plane)
  {
    case VIEW_PLANE_XY:
      m_cam->SetPosition(0, 0, -1);
      m_cam->SetFocalPoint(0, 0, 0);
      m_cam->SetRoll(180);
      break;
    case VIEW_PLANE_YZ:
      m_cam->SetPosition(1, 0, 0);
      m_cam->SetFocalPoint(0, 0, 0);
      break;
    case VIEW_PLANE_XZ:
      m_cam->SetPosition(0, 1, 0);
      m_cam->SetFocalPoint(0, 0, 0);
      break;
    default:
      assert(false);
  };
    
  double black[3] = {0, 0, 0};
  m_viewProxy->SetBackgroundColorCM(black);
  m_view->setCenterAxesVisibility(false);
  //m_view->resetCamera();
}

//-----------------------------------------------------------------------------
void SliceView::disconnectFromServer()
{
  qDebug() << "Disconnecting from the server";
  /*
  pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
  if (m_view)
  {
    m_mainLayout->removeWidget(m_viewWidget);
    ob->destroy(m_view);
    m_style->Delete();
    m_view = NULL;
  }
  */
}


//-----------------------------------------------------------------------------
void SliceView::showSegmentations(bool value)
{
  if (m_showSegmentations != value)
  {
    m_showSegmentations = value;
    updateScene();
  }
}

//-----------------------------------------------------------------------------
void SliceView::setPlane(ViewType plane)
{
  m_plane = plane;
}


//-----------------------------------------------------------------------------
QRegion SliceView::visualRegionForSelection(const QItemSelection& selection) const
{
  return QRect();
}

//-----------------------------------------------------------------------------
void SliceView::setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command)
{
  qDebug() << "Selection";
}

//-----------------------------------------------------------------------------
bool SliceView::isIndexHidden(const QModelIndex& index) const
{
  if (!index.isValid())
    return true;

  if (index.internalId() < 3)
    return true;

  IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
  EspinaProduct *actor = dynamic_cast<EspinaProduct *>(item);
  return !actor;
}

//-----------------------------------------------------------------------------
int SliceView::verticalOffset() const
{
  return 0;
}

//-----------------------------------------------------------------------------
int SliceView::horizontalOffset() const
{
  return 0;
}

//-----------------------------------------------------------------------------
QModelIndex SliceView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
  return QModelIndex();
}

//-----------------------------------------------------------------------------
void SliceView::rowsInserted(const QModelIndex& parent, int start, int end)
{
  //QAbstractItemView::rowsInserted(parent, start, end);
  
  //TODO: Multi samples
  
  assert(start == end);// Only 1-row-at-a-time inserts are allowed
  
  //QModelIndex index = parent.child(r,0);
  QModelIndex index  = model()->index(start, 0, parent);
  IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
  // Check for sample
  Sample *sample = dynamic_cast<Sample *>(item);
  if (sample)
  {
    //Use croshairs representation
    qDebug() << "Slice View"<< m_plane << ": Renders Sample";
    int mextent[6];
    sample->extent(mextent);
    int normalCoorToPlane = (m_plane + 2) % 3;
    int sliceOffset = m_plane==VIEW_PLANE_XY?1:0;
    int minSlices = mextent[2*normalCoorToPlane] + sliceOffset;
    int maxSlices = mextent[2*normalCoorToPlane+1] + sliceOffset;
    m_scrollBar->setMinimum(minSlices);
    m_spinBox->setMinimum(minSlices);
    m_scrollBar->setMaximum(maxSlices);
    m_spinBox->setMaximum(maxSlices);

    m_sampleRep = dynamic_cast<CrosshairRepresentation *>(sample->representation("03_Crosshair"));
    connect(m_sampleRep,SIGNAL(representationUpdated()),this,SLOT(updateScene()));
    m_sampleRep->render(m_view,m_plane);
    
    m_focusedSample = sample;
    m_view->resetCamera();
  }  
  updateScene();
}
/** DEPRECATED:
  IModelItem *newItem = static_cast<IModelItem *>(newIndex.internalPointer());
  Sample *newSample = dynamic_cast<Sample *>(newItem);
  if (newSample)
  {
    qDebug("SliceView: New sample Inserted");
    
    assert(!m_slicer);
    pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
    m_slicer = ob->createFilter("filters", "ImageSlicer", newSample->creator()->pipelineSource(), newSample->portNumber());
    setPlane(m_plane);
    newSample->creator()->pipelineSource()->updatePipeline();
  }
  else
  {
    Segmentation *newSeg = dynamic_cast<Segmentation *>(newItem);
    assert(newSeg); // If not sample, it has to be a segmentation
  }
  updateScene();
}
**/

//-----------------------------------------------------------------------------
void SliceView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
  //QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
  pqApplicationCore *core = pqApplicationCore::instance();
  pqObjectBuilder *ob = core->getObjectBuilder();
  assert(start == end);

  QModelIndex index = model()->index(start, 0, parent);
  IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
  // Check for sample
  Sample *sample = dynamic_cast<Sample *>(item);
  if (sample)
  {
    m_focusedSample = NULL;
    m_sampleRep = NULL;
    foreach(pqRepresentation *rep, m_view->getRepresentations())
    {
      rep->setVisible(false);
      ob->destroy(rep);
    }
    qDebug() << "Remaining representations" << m_view->getNumberOfRepresentations();
  }
  updateScene();
}

//-----------------------------------------------------------------------------
void SliceView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
  if (!topLeft.isValid() || !bottomRight.isValid())
    return;
  
  updateScene();
}


// //-----------------------------------------------------------------------------
// void SliceView::focusOnSample(Sample* sample)
// {
//   s_focusedSample = sample;
//   s_blender->focusOnSample(sample);
//   if (!m_slicer)
//   {
//     pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
//     m_slicer = ob->createFilter("filters", "ImageSlicer", sample->creator()->pipelineSource(), sample->portNumber());
//     setPlane(m_plane);
//     sample->creator()->pipelineSource()->updatePipeline();
//     /*
//     vtkPVDataInformation *info = sample->outputPort()->getDataInformation();
//     double *bounds = info->GetBounds();
//     int *extent = info->GetExtent();
//     */
//     int mextent[6];
//     sample->extent(mextent);
//     int normalCoorToPlane = (m_plane + 2) % 3;
//     int numSlices = mextent[2*normalCoorToPlane+1];
//     m_scrollBar->setMaximum(numSlices);
//     m_spinBox->setMaximum(numSlices);
// 
//     pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
//     pqDataRepresentation *rep = dp->setRepresentationVisibility(m_slicer->getOutputPort(0), m_view, true);
//     
//     m_view->resetCamera();
//   }
//   else
//   {
//     qDebug() << "Need to change slicer input";
//     //TODO: change slicer input
//     //assert(false);
//   }
// 
//   double camPoint[3], sampleBound[6];
//   sample->bounds(sampleBound);
// }


//-----------------------------------------------------------------------------
void SliceView::centerViewOn(int x, int y, int z)
{
  //qDebug() << "Center view on" << x << y << z;
  if (m_sampleRep)
    m_sampleRep->centerOn(x,y,z);
}

//-----------------------------------------------------------------------------
ISelectionHandler::VtkRegion SliceView::display2vtk(const QPolygonF &region)
{
  //Use Render Window Interactor's Picker to find the world coordinates
  //of the stack
  //vtkSMRenderViewProxy* renModule = view->GetRenderWindow()->GetInteractor()->GetRenderView();
  ISelectionHandler::VtkRegion vtkRegion;
  
  //! thus, use its spacing
  double spacing[3];//Image Spacing
  m_focusedSample->spacing(spacing);
  
  double pickPos[3];//World coordinates
  vtkPropPicker *wpicker = vtkPropPicker::New();
  foreach(QPointF point, region)
  {  
    wpicker->Pick(point.x(), point.y(), 0.0, m_viewProxy->GetRenderer());
    wpicker->GetPickPosition(pickPos);
    Point vtkPoint;
    for (int i=0; i<3; i++)
      vtkPoint[i] = round(pickPos[i] / spacing[i]);
    vtkRegion << vtkPoint;
  }
  return vtkRegion;

}



//-----------------------------------------------------------------------------
pqRenderView* SliceView::view()
{
  return m_view;
}


//-----------------------------------------------------------------------------
void SliceView::vtkWidgetMouseEvent(QMouseEvent* event)
{
  if (!m_sampleRep)
    return;
  
  //Use Render Window Interactor's to obtain event's position
  vtkSMRenderViewProxy* view = 
    vtkSMRenderViewProxy::SafeDownCast(m_view->getProxy());
  //vtkSMRenderViewProxy* renModule = view->GetRenderView();
  vtkRenderWindowInteractor *rwi =
    vtkRenderWindowInteractor::SafeDownCast(
      view->GetRenderWindow()->GetInteractor());
      //renModule->GetInteractor());
  assert(rwi);

  int xPos, yPos;
  rwi->GetEventPosition(xPos, yPos);
  //qDebug() << "EspINA::SliceView" << m_plane << ": Clicked Position" << xPos << " " << yPos;
  QPoint pos(xPos,yPos);
  
  if (event->type() == QMouseEvent::MouseButtonPress &&
      event->buttons() == Qt::LeftButton)
  {
    double spacing[3];//Image Spacing
    m_focusedSample->spacing(spacing);
  
    double pickPos[3];//World coordinates
    vtkPropPicker *wpicker = vtkPropPicker::New();
    wpicker->Pick(xPos, yPos, 0.1, m_viewProxy->GetRenderer());
    wpicker->GetPickPosition(pickPos);
    
    if (pickPos[0] == 0 && pickPos[1] == 0 && pickPos[2] == 0)
    {
      qDebug() << "Ignoring picking outside sample";
      return;
    }
   
    SelectionManager::instance()->onMouseDown(pos, this);
    //qDebug() << "Pick Position:" << pickPos[0] << pickPos[1] << pickPos[2];
    centerViewOn(round(pickPos[0] / spacing[0]),round(pickPos[1] / spacing[1]),round(pickPos[2] / spacing[2]));
  }
  //BUG: Only MouseButtonPress events are received
  if (event->type() == QMouseEvent::MouseMove &&
      event->buttons() == Qt::LeftButton)
  {
    SelectionManager::instance()->onMouseMove(pos, this);
  }
  if (event->type() == QMouseEvent::MouseButtonRelease &&
      event->buttons() == Qt::LeftButton)
  {
    SelectionManager::instance()->onMouseUp(pos, this);
  }
}

//-----------------------------------------------------------------------------
void SliceView::setSlice(int slice)
{
  if (m_spinBox->value() != slice)
    m_spinBox->setValue(slice);
  if (m_scrollBar->value() != slice)
    m_scrollBar->setValue(slice);
  int sliceOffset = m_plane==VIEW_PLANE_XY?1:0;
  if (m_sampleRep)
    m_sampleRep->setSlice(slice-sliceOffset,m_plane);
}

//-----------------------------------------------------------------------------
void SliceView::setVOI(IVOI* voi)
{
  if (m_VOIWidget)
  {
    m_VOIWidget->deselect();
    m_VOIWidget->setVisible(false);
    m_voi->deleteWidget(m_VOIWidget);
  }
  
  if(!m_focusedSample)
    return;
  
  m_voi = voi;
  
  if (!voi)
    return;
  
  m_VOIWidget = voi->newWidget();
  m_VOIWidget->setView(m_view);
  m_VOIWidget->setWidgetVisible(true);
  m_VOIWidget->select();
}

//-----------------------------------------------------------------------------
void SliceView::updateScene()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  if (m_sampleRep)
  {
    int sliceOffset = m_plane==VIEW_PLANE_XY?1:0;
    setSlice(m_sampleRep->slice(m_plane)+sliceOffset);
  }
  m_view->render();
  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
// void SliceView::slice(pqPipelineSource* source)
// {
//   vtkSMProperty* p;
//   vtkSMInputProperty *inputProp;
// 
//   p = m_slicer->getProxy()->GetProperty("Input");
//   inputProp = vtkSMInputProperty::SafeDownCast(p);
// 
//   inputProp->SetInputConnection(0, source->getProxy(), 0);
//   m_slicer->getProxy()->UpdateVTKObjects();
// }
