#include "slicer.h"

#include "stack.h"
#include "segmentation.h"

// Standard
#include <assert.h>

//Qt
#include <QList>

#include "proxies/vtkSMRGBALookupTableProxy.h"

//ParaQ
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMOutputPort.h"
#include "vtkSMDataSourceProxy.h"
#include "vtkPVDataInformation.h"
#include "vtkSMInputProperty.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMProxyProperty.h"
#include "vtkImageMapToColors.h"
#include "vtkProcessModule.h"
#include <pqPQLookupTableManager.h>
#include <pqScalarsToColors.h>

#include <QDebug>

SliceBlender::SliceBlender(SlicePlane plane)
: m_background(NULL)
, m_bgSlicer(NULL)
, m_bgMapper(NULL)
, m_plane(plane)
, m_blending(true)
{
  m_segmentations = new QList<IRenderable *>;
  m_slicers = new QList<pqPipelineSource *>;
  m_sliceMappers = new QList<pqPipelineSource *>;
  
  updateAxis();
}


void SliceBlender::setBackground ( IRenderable* background )
{
  vtkSMProperty* p;
  vtkSMIntVectorProperty* intVectProp;
  vtkSMDoubleVectorProperty* doubleVectProp;
  
  pqApplicationCore *core = pqApplicationCore::instance();
  pqServer *server =  core->getActiveServer();
  pqLookupTableManager *lutManager = core->getLookupTableManager();
  
  //Use a stack as background image
  m_background = background;
  
  //Slice the background image
  pqObjectBuilder *ob = core->getObjectBuilder();
  m_bgSlicer = ob->createFilter("filters","ImageSlicer",m_background->data(), m_background->portNumber());
  assert(m_bgSlicer);
  
  p = m_bgSlicer->getProxy()->GetProperty("SliceMode");
  vtkSMIntVectorProperty *sliceMode = vtkSMIntVectorProperty::SafeDownCast(p);
  
  if (sliceMode)
    sliceMode->SetElements1(5+m_plane);
  
  //Map the background values using a lut
  // This filter is the output of the sliceBlender class when blending is off
  m_bgMapper = ob->createFilter("filters","ImageMapToColors",m_bgSlicer);
  assert(m_bgMapper);
  
  // Get (or create if it doesn't exit) the lut for the background image
  pqScalarsToColors *greyLUT = lutManager->getLookupTable(server,QString("GreyLUT"),4,0);
  if (greyLUT)
  {
    p = greyLUT->getProxy()->GetProperty("RGBPoints");
    vtkSMDoubleVectorProperty *rgbs = vtkSMDoubleVectorProperty::SafeDownCast(p);
    if (rgbs)
    {
      // TODO: Use segmentation's information
      double colors[8] = {0,0,0,0,255,1,1,1};
      rgbs->SetElements(colors);
    }
    greyLUT->getProxy()->UpdateVTKObjects();
  }
  
  // Set the greyLUT for the slicemapper
  p = m_bgMapper->getProxy()->GetProperty("LookupTable");
  vtkSMProxyProperty *lut = vtkSMProxyProperty::SafeDownCast(p);
  if (lut)
  {
    lut->SetProxy(0,greyLUT->getProxy());
  }
  
  
  m_bgMapper->getProxy()->UpdateVTKObjects();
  //m_bgMapper->updatePipeline();
  
  // Set the colored slice as the first input of the blender algorithm
  // This filter is the output of the sliceBlender class when blending is on
  m_blender = ob->createFilter("filters","ImageBlend",m_bgMapper,0);
  assert(m_blender);
  
  vtkPVDataInformation *info = m_background->outputPort()->getDataInformation();
  double *bounds = info->GetBounds();
  int *extent = info->GetExtent();
  memcpy(m_bounds,bounds,6*sizeof(double));
  memcpy(m_extent,extent,6*sizeof(int));
  
  emit outputChanged(this->getOutput());
}

void SliceBlender::addSegmentation ( IRenderable* seg )
{
  vtkSMProperty* p;
  vtkSMIntVectorProperty* intVectProp;
  vtkSMDoubleVectorProperty* doubleVectProp;
  
  pqApplicationCore *core = pqApplicationCore::instance();
  pqServer *server =  core->getActiveServer();
  pqLookupTableManager *lutManager = core->getLookupTableManager();
  
  //Add seg to segmentation's blending list
  m_segmentations->push_back(seg);;
  
  //Slice the background image
  pqObjectBuilder *ob = core->getObjectBuilder();
  pqPipelineSource *slicer = ob->createFilter("filters","ImageSlicer",seg->data());
  assert(slicer);
  m_slicers->push_back(slicer);
  
  p = slicer->getProxy()->GetProperty("SliceMode");
  vtkSMIntVectorProperty *sliceMode = vtkSMIntVectorProperty::SafeDownCast(p);
  if (sliceMode)
  {
    sliceMode->SetElements1(5+m_plane);
  }
  
  //Map the background values using a lut
  pqPipelineSource *sliceMapper= ob->createFilter("filters","ImageMapToColors",slicer);
  assert(sliceMapper);
  m_sliceMappers->push_back(sliceMapper);
  
  /*
   *  // Get (or create if it doesn't exit) the lut for the segmentations' images
   *  pqServer *server =  core->getActiveServer();
   *  pqLookupTableManager *lutManager = core->getLookupTableManager();
   *  pqScalarsToColors *segLUT = lutManager->getLookupTable(server,"SegmentationsLUT",4,0);
   */
  
  vtkSMRGBALookupTableProxy *segLUT = vtkSMRGBALookupTableProxy::New();//= vtkSMRGBALookupTableProxy::SafeDownCast(ob->createProxy("lookup_tables","EspinaLookupTable",server,"LUTs"));//I'm not sure about second group name 
  segLUT->SetTableValue(0,0,0,0,0);
  segLUT->SetTableValue(255,0,0,1,1);
  segLUT->UpdateVTKObjects();
  
  // Set the greyLUT for the slicemapper
  p = sliceMapper->getProxy()->GetProperty("LookupTable");
  vtkSMProxyProperty *lut = vtkSMProxyProperty::SafeDownCast(p);
  if (lut)
  {
    lut->SetProxy(0,segLUT);
  }
  
  sliceMapper->getProxy()->UpdateVTKObjects();
  
  //Add the colored segmentation slice to the list of blending 
  //inputs of the blender algorithm
  assert(m_blender);
  p = m_blender->getProxy()->GetProperty("BlendInput");
  vtkSMInputProperty *input = vtkSMInputProperty::SafeDownCast(p);
  if (input) 
  {
    //input->SetMultipleInput(1);
    m_blender->getProxy()->UpdateVTKObjects();
    input->AddInputConnection(sliceMapper->getProxy(),0);
  }
  m_blender->getProxy()->UpdateVTKObjects();
  //mblender->updatePipeline();
  
  emit outputChanged(this->getOutput());
}


pqOutputPort *SliceBlender::getOutput()
{
  if (m_blending)
    return m_blender->getOutputPort(0);
  else
    return m_bgSlicer->getOutputPort(0);
}

pqOutputPort* SliceBlender::getBgOutput()
{
  return m_background->outputPort();
}


int SliceBlender::getNumSlices()
{
  return m_extent[2*m_zAxis+1];
}

void SliceBlender::setSlice(int slice)
{
  vtkSMIntVectorProperty *sliceIdx = vtkSMIntVectorProperty::SafeDownCast(
    m_bgSlicer->getProxy()->GetProperty("Slice"));
    if (sliceIdx)
      sliceIdx->SetElements1(slice);
    
    pqPipelineSource *slicer;
    foreach(slicer,*m_slicers)
    {
      sliceIdx = vtkSMIntVectorProperty::SafeDownCast(
	slicer->getProxy()->GetProperty("Slice"));
	if (sliceIdx)
	  sliceIdx->SetElements1(slice);
    }
    emit updated();
}

void SliceBlender::updateAxis()
{
  switch (m_plane)
  {
    case SLICE_PLANE_XY:
      m_xAxis = SLICE_AXIS_X;
      m_yAxis = SLICE_AXIS_Y;
      m_zAxis = SLICE_AXIS_Z;
      break;
    case SLICE_PLANE_YZ:
      m_xAxis = SLICE_AXIS_Y;
      m_yAxis = SLICE_AXIS_Z;
      m_zAxis = SLICE_AXIS_X;
      break;
    case SLICE_PLANE_XZ:
      m_xAxis = SLICE_AXIS_Z;
      m_yAxis = SLICE_AXIS_X;
      m_zAxis = SLICE_AXIS_Y;
      break;
    default:
      qDebug() << "Error";
  }
}
