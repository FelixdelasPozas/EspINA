#include "slicer.h"

#include "stack.h"
#include "segmentation.h"

// Standard
#include <assert.h>

//Qt
#include <QList>

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
	m_segmentations = new QList<Segmentation *>;
	m_slicers = new QList<pqPipelineSource *>;
	m_sliceMappers = new QList<pqPipelineSource *>;

	//pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
	//m_blender = ob->createFilter("filters","ImageSlicer",source,0);
	
	//assert(m_blender);
	
	//vtkSMIntVectorProperty *sliceMode = vtkSMIntVectorProperty::SafeDownCast(
	//  m_bgSlicer->getProxy()->GetProperty("SliceMode"));
	//sliceMode->SetElements1(5+m_plane);

	updateAxis();
}


void SliceBlender::setBackground ( Stack* stack )
{
	//Use a stack as background image
	m_background = stack;
	pqPipelineSource *input = m_background->data();
	
	//Slice the background image
	pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
	m_bgSlicer = ob->createFilter("filters","ImageSlicer",input);
	assert(m_bgSlicer);
	
	vtkSMIntVectorProperty *sliceMode = vtkSMIntVectorProperty::SafeDownCast(
	  m_bgSlicer->getProxy()->GetProperty("SliceMode"));
	if (sliceMode)
	  sliceMode->SetElements1(5+m_plane);
	
	//Map the background values using a lut
	// This filter is the output of the sliceBlender class when blending is off
	m_bgMapper = ob->createFilter("filters","ImageMapToColors",m_bgSlicer);
	assert(m_bgMapper);
	
	// Get (or create if it doesn't exit) the lut for the background image
	pqServer *server =  pqApplicationCore::instance()->getActiveServer();
	pqScalarsToColors *greyLUT = pqApplicationCore::instance()->getLookupTableManager()
	  ->getLookupTable(server,QString("GreyLUT"),4,0);
	if (greyLUT)
	{
	  //std::cout << "ScalarToColors\n";
	  vtkSMDoubleVectorProperty *rgbs = vtkSMDoubleVectorProperty::SafeDownCast(
	    greyLUT->getProxy()->GetProperty("RGBPoints"));
	    if (rgbs)
	    {
	      // TODO: Use segmentation's information
	      double colors[8] = {0,0,0,0,255,1,1,1};
	      rgbs->SetElements(colors);
	    }
	  //lut->getProxy()->InvokeCommand("UpdateLookupTableScalarRange");
	  greyLUT->getProxy()->UpdateVTKObjects();
	  //lut->getProxy()->PrintSelf(std::cout,vtkIndent(0));
	}
	
	// Set the greyLUT for the slicemapper
	vtkSMProxyProperty *lut = vtkSMProxyProperty::SafeDownCast(
	  m_bgMapper->getProxy()->GetProperty("LookupTable"));
	if (lut)
	{
	  lut->SetProxy(0,greyLUT->getProxy());
	}
	
// 	vtkSMInputProperty *input_prop = vtkSMInputProperty::SafeDownCast(
// 	  m_slicerMapper->getProxy()->GetProperty("Input"));
// 	if (input_prop)
// 	{
// 	  input_prop->AddInputConnection(m_bgSlicer->getProxy(),0);
// 	}
	
	m_bgMapper->getProxy()->UpdateVTKObjects();
	//std::cout << "ImageMapToColors\n";
	//m_sliceMapper->getProxy()->PrintSelf(std::cout,vtkIndent(2));
	
	// Set the colored slice as the first input of the blender algorithm
	// This filter is the output of the sliceBlender class when blending is on
	m_blender = ob->createFilter("filters","ImageBlend",m_bgMapper,0);
	assert(m_blender);
	

	//vtkSMSourceProxy * reader = vtkSMSourceProxy::SafeDownCast(input->getProxy());
	//source->getOutputPort(0)->getDataInformation()->PrintSelf(std::cout,vtkIndent(0));
	double *bounds = input->getOutputPort(0)->getDataInformation()->GetBounds();
	int *extent = input->getOutputPort(0)->getDataInformation()->GetExtent();
	memcpy(m_bounds,bounds,6*sizeof(double));
	memcpy(m_extent,extent,6*sizeof(int));
	//vtkSMDataSourceProxy *data = vtkSMDataSourceProxy::SafeDownCast(source->getOutputPort(0)->getOutputPortProxy());
	//if (data) qDebug() << "OK";
	//double *spacing;
	//reader->GetPixelSpacing();
	//qDebug() << spacing[0] << spacing[1] << spacing[2];
	
	emit outputChanged(this->getOutput());
}

void SliceBlender::addSegmentation ( Segmentation* seg )
{
	//Add seg to segmentation's blending list
	m_segmentations->push_back(seg);;
	
	//Slice the background image
	pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
	pqPipelineSource *slicer = ob->createFilter("filters","ImageSlicer",seg->data());
	assert(slicer);
	m_slicers->push_back(slicer);
	
	vtkSMIntVectorProperty *sliceMode = vtkSMIntVectorProperty::SafeDownCast(
	  slicer->getProxy()->GetProperty("SliceMode"));
	if (sliceMode)
	  sliceMode->SetElements1(5+m_plane);
	
	//Map the background values using a lut
	pqPipelineSource *sliceMapper= ob->createFilter("filters","ImageMapToColors",slicer);
	assert(sliceMapper);
	m_sliceMappers->push_back(sliceMapper);
	
	// Get (or create if it doesn't exit) the lut for the segmentations' images
	pqServer *server =  pqApplicationCore::instance()->getActiveServer();
	pqScalarsToColors *segLUT = pqApplicationCore::instance()->getLookupTableManager()
	  ->getLookupTable(server,QString("SegmentationsLUT"),4,0);
	if (segLUT)
	{
	  vtkSMIntVectorProperty *colorSpace = vtkSMIntVectorProperty::SafeDownCast(
	    segLUT->getProxy()->GetProperty("ColorSpace"));
	    if (colorSpace)
	      colorSpace->SetElements1(0);
	  segLUT->getProxy()->UpdateVTKObjects();
	  
	  //std::cout << "Seg LUT\n";
	  //segLUT->getProxy()->PrintSelf(std::cout,vtkIndent(5));
	  vtkSMDoubleVectorProperty *rgbs = vtkSMDoubleVectorProperty::SafeDownCast(
	    segLUT->getProxy()->GetProperty("RGBPoints"));
	    if (rgbs)
	    {
	      // TODO: Use segmentation's information
	      double colors[8] = {0,0,0,0,1,0,0,1};
	      rgbs->SetElements(colors);
	    }
	  //lut->getProxy()->InvokeCommand("UpdateLookupTableScalarRange");
	  segLUT->getProxy()->UpdateVTKObjects();
	  //segLUT->getProxy()->PrintSelf(std::cout,vtkIndent(0));
	}
	
	// Set the greyLUT for the slicemapper
	vtkSMProxyProperty *lut = vtkSMProxyProperty::SafeDownCast(
	  sliceMapper->getProxy()->GetProperty("LookupTable"));
	if (lut)
	{
	  lut->SetProxy(0,segLUT->getProxy());
	}
	
// 	vtkSMInputProperty *input_prop = vtkSMInputProperty::SafeDownCast(
// 	  m_slicerMapper->getProxy()->GetProperty("Input"));
// 	if (input_prop)
// 	{
// 	  input_prop->AddInputConnection(m_bgSlicer->getProxy(),0);
// 	}
	
	sliceMapper->getProxy()->UpdateVTKObjects();
	//std::cout << "ImageMapToColors\n";
	//m_sliceMapper->getProxy()->PrintSelf(std::cout,vtkIndent(2));
	
	//Add the colored segmentation slice to the list of blending 
	//inputs of the blender algorithm
	assert(m_blender);
	vtkSMInputProperty *input = vtkSMInputProperty::SafeDownCast(
	  m_blender->getProxy()->GetProperty("BlendInput"));
	if (input) 
	{
	  input->SetMultipleInput(1);
	  input->SetInputConnection(0,m_bgMapper->getProxy(),0);
	  input->SetInputConnection(1,sliceMapper->getProxy(),0);
	  //m_blender->updatePipeline();
	  //m_blender->getProxy()->UpdateVTKObjects();
	  //std::cout << "Input Property:\n";
	  //input->PrintSelf(std::cout,vtkIndent(0));
	  //qDebug() << "Multiple Input" << input->GetMultipleInput();
	}
	//m_blender->getProxy()->PrintSelf(std::cout,vtkIndent(6));
	m_blender->getProxy()->UpdateVTKObjects();
	m_blender->updatePipeline();
	
	
	//std::cout << "BLENDER INPUT INFO:\n";
	//std::cout << "BG INFO:\n";
	//m_bgSlicer->getOutputPort(0)->getDataInformation()->PrintSelf(std::cout,vtkIndent(0));
	////qDebug() << m_bgSlicer->getOutputPort(0)->getDataInformation()->GetPrettyDataTypeString();
	//std::cout << "SEG INFO:\n";
	//slicer->updatePipeline();
	//slicer->getOutputPort(0)->getDataInformation()->PrintSelf(std::cout,vtkIndent(2));
	////qDebug() << slicer->getOutputPort(0)->getDataInformation();
	
	emit outputChanged(this->getOutput());
}


pqOutputPort *SliceBlender::getOutput()
{
	if (m_blending)
	  return m_blender->getOutputPort(0);
	else
	  return m_bgSlicer->getOutputPort(0);
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
