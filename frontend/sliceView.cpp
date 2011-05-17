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

#include <pqPipelineFilter.h>

#define HINTWIDTH 40

//-----------------------------------------------------------------------------
Blender *Blender::m_blender = NULL;

//-----------------------------------------------------------------------------
Blender* Blender::instance()
{
  if (!m_blender)
    m_blender = new Blender();

  return m_blender;
}


void Blender::focusOnSample(Sample* sample)
{
  if (m_blendingMappers.contains(sample))
    return;
  ///TODO: Hay que ver bien como se gestionan los recursos para evitar memory leaks
  if (!m_imageBlender)
  {
    vtkSMProperty* p;

    pqApplicationCore *core = pqApplicationCore::instance();
    pqServer *server =  core->getActiveServer();
    pqObjectBuilder *ob = core->getObjectBuilder();
    pqLookupTableManager *lutManager = core->getLookupTableManager();

    //Map the background values using a lut
    // This filter is the output of the sliceBlender class when blending is off
    if (m_sampleMapper)
    {
      qDebug() << "Free previous state";
      assert(false);
    }
    m_sampleMapper = ob->createFilter("filters", "ImageMapToColors", sample->creator()->pipelineSource(),sample->portNumber());
    assert(m_sampleMapper);

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
    p = m_sampleMapper->getProxy()->GetProperty("LookupTable");
    vtkSMProxyProperty *lut = vtkSMProxyProperty::SafeDownCast(p);
    if (lut)
    {
      lut->SetProxy(0, greyLUT->getProxy());
    }

    m_sampleMapper->getProxy()->UpdateVTKObjects();

    m_imageBlender = ob->createFilter("filters", "ImageBlend", m_sampleMapper);
    assert(m_imageBlender);
  }
}


void Blender::blendSegmentation(Segmentation* seg)
{
  if (m_blendingMappers.contains(seg))
    return;

  vtkSMProperty* p;
  vtkSMIntVectorProperty* intVectProp;
  vtkSMDoubleVectorProperty* doubleVectProp;

  pqApplicationCore *core = pqApplicationCore::instance();
  pqObjectBuilder *ob = core->getObjectBuilder();

  //Map segmentation values using a lut
  pqPipelineSource *segMapper = ob->createFilter("filters", "ImageMapToColors", seg->creator()->pipelineSource(),seg->portNumber());
  assert(segMapper);
  m_blendingMappers[seg] = segMapper;

  //TODO: Use smart pointers
  vtkSMRGBALookupTableProxy *segLUT = vtkSMRGBALookupTableProxy::New();
  segLUT->SetTableValue(0, 0, 0, 0, 0);
  double rgba[4];
  seg->color(rgba);
  //TODO: change to binary segmentation images
  segLUT->SetTableValue(255, rgba[0], rgba[1], rgba[2], 0.6);
  segLUT->UpdateVTKObjects();

  // Set the greyLUT for the slicemapper
  p = segMapper->getProxy()->GetProperty("LookupTable");
  vtkSMProxyProperty *lut = vtkSMProxyProperty::SafeDownCast(p);
  if (lut)
  {
    lut->SetProxy(0, segLUT);
  }

  segMapper->getProxy()->UpdateVTKObjects();
  segMapper->updatePipeline();
  //p = m_imageBlender->getProxy()->GetProperty("Input");
  //vtkSMInputProperty *input = vtkSMInputProperty::SafeDownCast(p);
  //if (input)
  //{
  //input->AddProxy(segMapper->getProxy());
  //}
  updateImageBlenderInput();
}

void Blender::unblendSegmentation(Segmentation* seg)
{
  if (!m_blendingMappers.contains(seg))
    return;

  vtkSMProperty* p;
  vtkSMIntVectorProperty* intVectProp;
  vtkSMDoubleVectorProperty* doubleVectProp;

  pqPipelineSource *mapper = m_blendingMappers.take(seg);

  std::cout << "N. Consumers of mapper before " << mapper->getNumberOfConsumers() << std::endl;
  std::cout << "N. Producers of blender before " << m_imageBlender->getProxy()->GetNumberOfProducers() << std::endl;
  //p = m_imageBlender->getProxy()->GetProperty("Input");
  //vtkSMInputProperty *input = vtkSMInputProperty::SafeDownCast(p);
  //if (input)
  //{
  //input->RemoveProxy(mapper->getProxy());
  //}
  updateImageBlenderInput();
  std::cout << "N. Consumers of mapper after update vtk " << mapper->getNumberOfConsumers() << std::endl;
  std::cout << "N. Producers of blender after update vtk " << m_imageBlender->getProxy()->GetNumberOfProducers() << std::endl;

  pqApplicationCore *core = pqApplicationCore::instance();
  pqObjectBuilder *ob = core->getObjectBuilder();
  ob->destroy(mapper);
}

void Blender::updateImageBlenderInput()
{
  m_mutex.lock();
  vtkSMProperty* p;

  vtkstd::vector<vtkSMProxy *> inputs;
  vtkstd::vector<unsigned int> ports;

  // Ensure sample's mapper is the first input
  inputs.push_back(m_sampleMapper->getProxy());
  ports.push_back(0);

  foreach(pqPipelineSource *source, m_blendingMappers)
  {
    IModelItem *item = m_blendingMappers.key(source);
    Segmentation *seg = dynamic_cast<Segmentation *>(item);
    if (seg->visible())
    {
      inputs.push_back(source->getProxy());
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


















#define LOWER(coord) (2*(coord))
#define UPPER(coord) (2*(coord) + 1)



//Sample *SliceView::s_focusedSample = NULL;
Blender *SliceView::s_blender = NULL;

//-----------------------------------------------------------------------------
SliceView::SliceView(QWidget* parent)
    : QAbstractItemView(parent)
    , m_init(false)
    , m_showSegmentations(true)
    , m_rep(NULL)
    , m_plane(SLICE_PLANE_XY)
    , m_slice(NULL)
    , m_slicer(NULL)
    , s_focusedSample(NULL)
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
  QObject::connect(m_scrollBar, SIGNAL(valueChanged(int)), m_spinBox, SLOT(setValue(int)));
  QObject::connect(m_spinBox, SIGNAL(valueChanged(int)), m_scrollBar, SLOT(setValue(int)));
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

  s_blender = Blender::instance();
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
  QObject::connect(m_viewWidget, SIGNAL(mouseEvent(QMouseEvent *)),
                   this, SLOT(vtkWidgetMouseEvent(QMouseEvent *)));
  
  m_mainLayout->insertWidget(0, m_viewWidget);//To preserve view order

  
  m_viewProxy = vtkSMRenderViewProxy::SafeDownCast(m_view->getProxy());
  assert(m_viewProxy);
  
  m_rwi = vtkRenderWindowInteractor::SafeDownCast(
    m_viewProxy->GetRenderWindow()->GetInteractor());
  assert(m_rwi);
  
  m_cam = m_viewProxy->GetActiveCamera();
  assert(m_cam);
  
  m_cam->ParallelProjectionOn();
  
  if (m_plane == SLICE_PLANE_XY)
  {
    //m_cam->SetPosition(0, 0, -1);
    //m_cam->SetFocalPoint(0, 0, 0);
    //m_cam->SetRoll(180);
  }
  else
    if (m_plane == SLICE_PLANE_YZ)
    {
      m_cam->SetPosition(1, 0, 0);
      m_cam->SetFocalPoint(0, 0, 0);
      m_cam->SetRoll(-90);
    }
    else
    {
      m_cam->SetPosition(0, 1, 0);
      m_cam->SetFocalPoint(0, 0, 0);
      m_cam->SetRoll(-90);
    }
    
  double black[3] = {0, 0, 0};
  m_viewProxy->SetBackgroundColorCM(black);
  
  //TODO: Change style
  m_view->setCenterAxesVisibility(false);
  m_view->resetCamera();
}

//-----------------------------------------------------------------------------
void SliceView::disconnectFromServer()
{
  if (m_view)
  {
    //qDebug() << "Deleting Widget";
    m_mainLayout->removeWidget(m_viewWidget);
    //qDebug() << "Deleting View";
    //TODO: BugFix -> destroy previous instance of m_view
    //pqApplicationCore::instance()->getObjectBuilder()->destroy(m_view);
  }
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
void SliceView::setVOI(IVOI* voi)
{
  if (m_VOIWidget)
  {
    //TODO: Destroy previous declaration
    assert(false);
  }
  
  if (!voi)
    return;
  
  m_VOIWidget = voi->widget(m_plane);
  m_VOIWidget->setView(m_view);
  m_VOIWidget->setWidgetVisible(true);
  m_VOIWidget->select();
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
  QAbstractItemView::rowsInserted(parent, start, end);
  assert(start == end);// Only 1-row-at-a-time inserts are allowed
  QModelIndex newIndex  = parent.child(start, 0);
  IModelItem *newItem = static_cast<IModelItem *>(newIndex.internalPointer());
  Sample *newSample = dynamic_cast<Sample *>(newItem);
  if (newSample)
    focusOnSample(newSample);
  else
  {
    Segmentation *seg = dynamic_cast<Segmentation *>(newItem);
    assert(seg); // If not sample, it has to be a segmentation
    s_blender->blendSegmentation(seg);
  }
  updateScene();
}

//-----------------------------------------------------------------------------
void SliceView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
  for (int r = start; r <= end; r++)
  {
    QModelIndex index = parent.child(r, 0);
    IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
    // Check for sample
    Sample *sample = dynamic_cast<Sample *>(item);
    if (sample)
    {
      //TODO: Remove sample
      qDebug() << "Render planes";
    }
    else
      if (!sample)
      {
        Segmentation *seg = dynamic_cast<Segmentation *>(item);
        assert(seg); // If not sample, it has to be a segmentation
        std::cout << seg->label().toStdString() << " about to be destroyed\n";
        s_blender->unblendSegmentation(seg);
      }
  }
  updateScene();
}

//-----------------------------------------------------------------------------
void SliceView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
  //if (!topLeft.isValid() || !bottomRight.isValid())
    //return;
  
  if (!s_focusedSample)
    return;
  
  s_blender->updateImageBlenderInput();
  updateScene();
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
    s_focusedSample = sample;
}

//-----------------------------------------------------------------------------
QRect SliceView::visualRect(const QModelIndex& index) const
{
  return QRect();
}

//-----------------------------------------------------------------------------
void SliceView::focusOnSample(Sample* sample)
{
  s_focusedSample = sample;
  s_blender->focusOnSample(sample);
  if (!m_slicer)
  {
    pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
    m_slicer = ob->createFilter("filters", "ImageSlicer", sample->creator()->pipelineSource(), sample->portNumber());
    setPlane(m_plane);
    sample->creator()->pipelineSource()->updatePipeline();
    /*
    vtkPVDataInformation *info = sample->outputPort()->getDataInformation();
    double *bounds = info->GetBounds();
    int *extent = info->GetExtent();
    */
    int mextent[6];
    sample->extent(mextent);
    int normalCoorToPlane = (m_plane + 2) % 3;
    int numSlices = mextent[2*normalCoorToPlane+1];
    m_scrollBar->setMaximum(numSlices);
    m_spinBox->setMaximum(numSlices);

    pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
    pqDataRepresentation *rep = dp->setRepresentationVisibility(m_slicer->getOutputPort(0), m_view, true);
    
    m_view->resetCamera();
  }
  else
  {
    qDebug() << "Need to change slicer input";
    //TODO: change slicer input
    //assert(false);
  }

  double camPoint[3], sampleBound[6];
  sample->bounds(sampleBound);
}


//-----------------------------------------------------------------------------
ISelectionHandler::VtkRegion SliceView::display2vtk(const QPolygonF &region)
{
  //Use Render Window Interactor's Picker to find the world coordinates
  //of the stack
  //vtkSMRenderViewProxy* renModule = view->GetRenderWindow()->GetInteractor()->GetRenderView();
  vtkAbstractPicker *picker = m_rwi->GetPicker();
  assert(picker);
  
  ISelectionHandler::VtkRegion vtkRegion;
  
  //! thus, use its spacing
  double pos[3];//World coordinates
  double spacing[3];//Image Spacing
  s_focusedSample->spacing(spacing);
  
  foreach(QPointF point, region)
  {  
    picker->Pick(point.x(), point.y(), 0.0, m_viewProxy->GetRenderer());
    picker->GetPickPosition(pos);
    Point vtkPoint;
    for (int i=0; i<3; i++)
      vtkPoint[i] = pos[i] / spacing[i];
    vtkRegion << vtkPoint;
  }
  return vtkRegion;

}


/** DEPRECATED
//-----------------------------------------------------------------------------
Point SliceView::convert(const QPointF& point)
{
  
  //Use Render Window Interactor's Picker to find the world coordinates
  //of the stack
  //vtkSMRenderViewProxy* renModule = view->GetRenderWindow()->GetInteractor()->GetRenderView();
  vtkAbstractPicker *picker = m_rwi->GetPicker();
  assert(picker);

  picker->Pick(point.x(), point.y(), 0.0, m_viewProxy->GetRenderer());
  
  double pos[3];//World coordinates
  picker->GetPickPosition(pos);
  qDebug() << "EspINA::SliceView" << m_plane << ": Pick Position" << pos[0] << " " << pos[1] << " " << pos[2];
  
  Point result;
  result[0] = pos[0];
  result[1] = pos[1];
  result[2] = pos[2];
  
  qDebug() << "EspINA::SliceView" << m_plane << ": World Position" << result.x << " " << result.y << " " << result.z;
  
  return result;
}

//-----------------------------------------------------------------------------
ISelectionHandler::VtkRegion SliceView::correctSpacing(ISelectionHandler::VtkRegion& region)
{
  ISelectionHandler::VtkRegion correctedRegion;
  
  ///! All segmented objects shown in this view must belong to the focused sample
  double spacing[3];//Image Spacing
  s_focusedSample->spacing(spacing);
  
  foreach(Point point, region)
  {
    Point correctedPoint;
    for (int i=0; i<3; i++)
      correctedPoint[i] = point[i] / spacing[i];
    correctedRegion << correctedPoint;
  }
  return correctedRegion;
}
**/

//! If several regions select the same object, consistently it will return
//! the same number of (regions,elements) as selected objects. Thus, the client
//! that decided such configuration has to resolve it
//-----------------------------------------------------------------------------
void SliceView::setSelection(SelectionFilters& filters, ViewRegions& regions)
{
  //TODO: Discuss if we apply VOI at application level or at plugin level
  // i.e. whether clicks out of VOI are discarted or not
  ISelectionHandler::Selection sel;
  
  qDebug() << "EspINA::SliceView" << m_plane << ": Making selection";
  // Select all products that belongs to all the regions
  foreach(const QPolygonF &region, regions)
  {
    ISelectionHandler::VtkRegion vtkRegion;
    // Translate view pixels into Vtk pixels
    vtkRegion = display2vtk(region);
    
    // Apply filtering criteria at given region
    foreach(QString filter, filters)
    {
      //! Special case, where sample is selected
      if (filter == "EspINA_Sample")
      {
	ISelectionHandler::SelElement selSample;
	selSample.first = vtkRegion;
	selSample.second = s_focusedSample;
	sel.append(selSample);
      }
      //! Select all segmented objects
      else 
      {
	// Find segmented objects inside regions
	// Discard by filter
	// Adjust spacing
	assert(false); //TODO: Taxonomy selection NOTE: shall other filtering criterias be implemented?? Size??
      }
    }
  }
  
  //TODO: Update Qt selection
  // Notify the manager about the new selection
  SelectionManager::instance()->setSelection(sel);
}

//-----------------------------------------------------------------------------
pqRenderView* SliceView::view()
{
  return m_view;
}


//-----------------------------------------------------------------------------
void SliceView::setPlane(SlicePlane plane)
{
  if (m_slicer)
  {
    vtkSMProperty* p;
    vtkSMIntVectorProperty* sliceMode;
    p = m_slicer->getProxy()->GetProperty("SliceMode");
    sliceMode = vtkSMIntVectorProperty::SafeDownCast(p);
    if (sliceMode)
      sliceMode->SetElements1(5 + plane);
  }

  m_plane = plane;
  

  const int X=0, Y=1, Z=2;
  
  switch (plane)
  {
    case SLICE_PLANE_XY:
      m_xAxisDisp = X;
      m_yAxisDisp = Y;
      m_zAxisDisp = Z;
      break;
    case SLICE_PLANE_YZ:
      m_xAxisDisp = Z;
      m_yAxisDisp = Y;
      m_zAxisDisp = X;
      break;
    case SLICE_PLANE_XZ:
      m_xAxisDisp = X;
      m_yAxisDisp = Z;
      m_zAxisDisp = Y;
      break;
    default:
      assert(false);
  };
}

//-----------------------------------------------------------------------------
void SliceView::setSlice(int value)
{
  vtkSMProperty *p;
  vtkSMIntVectorProperty *slice;

  //WARNING: Use vtkSMPropertyHelper instead of basic properties
  vtkSMPropertyHelper(m_slicer->getProxy(),"Slice").Set(value);
  /*
  p = m_slicer->getProxy()->GetProperty("Slice");
  slice = vtkSMIntVectorProperty::SafeDownCast(p);
  if (slice)
    slice->SetElements1(value);
  */
  emit sliceChanged();
  updateScene();
}

//-----------------------------------------------------------------------------
void SliceView::vtkWidgetMouseEvent(QMouseEvent* event)
{
  
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
  qDebug() << "EspINA::SliceView" << m_plane << ": Clicked Position" << xPos << " " << yPos;
  QPoint pos(xPos,yPos);
  
  if (event->type() == QMouseEvent::MouseButtonPress &&
      event->buttons() == Qt::LeftButton)
  {
    SelectionManager::instance()->onMouseDown(pos, this);
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
void SliceView::updateScene()
{
  if (!s_focusedSample)
    return;
  if (m_showSegmentations)
    slice(s_blender->source());
  else
    slice(s_focusedSample->creator()->pipelineSource());

  m_view->render();
}

//-----------------------------------------------------------------------------
void SliceView::slice(pqPipelineSource* source)
{

  vtkSMProperty* p;
  vtkSMInputProperty *inputProp;

  p = m_slicer->getProxy()->GetProperty("Input");
  inputProp = vtkSMInputProperty::SafeDownCast(p);

  inputProp->SetInputConnection(0, source->getProxy(), 0);
  m_slicer->getProxy()->UpdateVTKObjects();
}
