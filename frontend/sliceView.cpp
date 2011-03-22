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
#include "traceNodes.h"

// ParaQ includes
#include "pqTwoDRenderView.h"
#include "pqApplicationCore.h"
#include "pqActiveObjects.h"
#include "pqDisplayPolicy.h"
#include "pqObjectBuilder.h"
#include "pqPipelineRepresentation.h"
#include "vtkSMImageSliceRepresentationProxy.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMViewProxy.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMTwoDRenderViewProxy.h"
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
    m_sampleMapper = ob->createFilter("filters", "ImageMapToColors", sample->sourceData(), 0);
    assert(m_sampleMapper);
    
    // Get (or create if it doesn't exit) the lut for the background image
    pqScalarsToColors *greyLUT = lutManager->getLookupTable(server, QString("GreyLUT"), 4, 0);
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
  pqPipelineSource *segMapper = ob->createFilter("filters", "ImageMapToColors", seg->sourceData());
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
    input->SetProxies( static_cast<unsigned int>(inputs.size())
    , &inputs[0]
    , &ports[0]);
    m_imageBlender->getProxy()->UpdateVTKObjects();
  }
  m_mutex.unlock();
}
























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
  m_view = qobject_cast<pqTwoDRenderView*>(ob->createView(
             pqTwoDRenderView::twoDRenderViewType(), server));
  m_viewWidget = m_view->getWidget();
  QObject::connect(m_viewWidget, SIGNAL(mouseEvent(QMouseEvent *)),
                   this, SLOT(vtkWidgetMouseEvent(QMouseEvent *)));
  m_mainLayout->insertWidget(0, m_viewWidget);//To preserve view order
  
  vtkSMTwoDRenderViewProxy* view = vtkSMTwoDRenderViewProxy::SafeDownCast(
    m_view->getProxy());
  
    
  vtkCamera * cam = view->GetRenderView()->GetActiveCamera();
  if (m_plane == SLICE_PLANE_XY)
  {
    cam->SetPosition(0,0,-50);
    cam->SetFocalPoint(0,0,0);
    cam->SetRoll(180);
  }else if (m_plane == SLICE_PLANE_YZ)
  {
    cam->SetRoll(270);
  } else
  {
    cam->SetRoll(-90);
  }
    
  m_view->resetCamera();
  
  double black[3] = {0,0,0};
  view->GetRenderView()->SetBackgroundColorCM(black);
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
  Product *actor = dynamic_cast<Product *>(item);
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
    QModelIndex index = parent.child(r,0);
    IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
    // Check for sample
    Sample *sample = dynamic_cast<Sample *>(item);
    if (sample)
    {
      //TODO: Remove sample
      qDebug() << "Render planes";
    } 
    else if (!sample)
    {
      Segmentation *seg = dynamic_cast<Segmentation *>(item);
      assert(seg); // If not sample, it has to be a segmentation
      std::cout << seg->name.toStdString() << " about to be destroyed\n";
      s_blender->unblendSegmentation(seg);
    }
  }
  updateScene();
}

//-----------------------------------------------------------------------------
void SliceView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
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
    m_slicer = ob->createFilter("filters", "ImageSlicer", sample->sourceData(), 0);
    setPlane(m_plane);
    sample->sourceData()->updatePipeline();
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
    //memcpy(m_bounds,bounds,6*sizeof(double));
    //memcpy(m_extent,extent,6*sizeof(int));

    pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
    pqDataRepresentation *rep = dp->setRepresentationVisibility(m_slicer->getOutputPort(0), m_view, true);
    
  }
  else
  {
    qDebug() << "Need to change slicer input";
    //TODO: change slicer input
    assert(false);
  }
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
}

//-----------------------------------------------------------------------------
void SliceView::setSlice(int value)
{
  vtkSMProperty *p;
  vtkSMIntVectorProperty *slice;

  p = m_slicer->getProxy()->GetProperty("Slice");
  slice = vtkSMIntVectorProperty::SafeDownCast(p);
  if (slice)
    slice->SetElements1(value);
  emit sliceChanged();
  updateScene();
}

//-----------------------------------------------------------------------------
void SliceView::vtkWidgetMouseEvent(QMouseEvent* event)
{
  if (event->type() == QMouseEvent::MouseButtonPress &&
      event->buttons() == Qt::LeftButton)
  {
    //Use Render Window Interactor's Picker to find the world coordinates
    //of the stack
    vtkSMTwoDRenderViewProxy* view = vtkSMTwoDRenderViewProxy::SafeDownCast(
                                       m_view->getProxy());
    vtkSMRenderViewProxy* renModule = view->GetRenderView();
    vtkRenderWindowInteractor *rwi = vtkRenderWindowInteractor::SafeDownCast(
                                       renModule->GetInteractor());
    if (!rwi)
      return;

    // Because we display all slice planes in the same display coordinates
    // it is necesary to translate the axis correspondence between the
    // display coordinates and the plane coordinates
    int selection[3] = {0.0, 0.0, 0.0}; //Selection in plane coordinates
    rwi->GetEventPosition(selection[0], selection[1]);
    //rwi->GetEventPosition(selection[m_input->getAxisX()],selection[m_input->getAxisY()]);
    //selection[m_input->getAxisZ()] = m_scroll->value();
    vtkAbstractPicker *picker = rwi->GetPicker();
    if (!picker)
      return;

    //Change coordinates acording the plane
    picker->Pick(selection[0], selection[1], selection[2], renModule->GetRenderer());
    //picker->PrintSelf(std::cout, vtkIndent(0));
    double pos[3];//World coordinates
    picker->GetPickPosition(pos);
    //m_input->getOutput()->getDataInformation()->PrintSelf(std::cout,vtkIndent(0));
    //Get Spacing
    s_focusedSample->sourceData()->getOutputPort(0)->getDataInformation()->PrintSelf(std::cout,vtkIndent(0));
    double spacing[3];//Image Spacing
    s_focusedSample->spacing(spacing);
    Point coord;
    coord.x = pos[0] / spacing[0];
    coord.y = pos[1] / spacing[1];
    coord.z = m_spinBox->value();
    qDebug() << "Event Position" << selection[0] << " " << selection[1] << " " << selection[2];
    qDebug() << "Pick Position" << pos[0] << " " << pos[1] << " " << m_spinBox->value();
    qDebug() << "Real Position" << coord.x << " " << coord.y << " " << m_spinBox->value();
    
    emit pointSelected(coord);
  }
}


//-----------------------------------------------------------------------------
void SliceView::updateScene()
{
  if (m_showSegmentations)
    slice(s_blender->source());
  else
    slice(s_focusedSample->sourceData());
  
    
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
