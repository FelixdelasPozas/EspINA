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

#include <QDebug>

SliceBlender::SliceBlender(SlicePlane plane)
	: m_background(NULL)
	, m_bgSlicer(NULL)
	, m_plane(plane)
	, m_blending(false)
{
	m_inputs = new QList<Segmentation *>;
	m_slicers = new QList<pqPipelineSource *>;

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
	//Store real input
	m_background = stack;
	pqPipelineSource *input = m_background->data();
	
	//Create its slicer
	pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
	m_bgSlicer = ob->createFilter("filters","ImageSlicer",input,0);
	
	assert(m_bgSlicer);
	
	vtkSMIntVectorProperty *sliceMode = vtkSMIntVectorProperty::SafeDownCast(
	  m_bgSlicer->getProxy()->GetProperty("SliceMode"));
	
	if (sliceMode)
	  sliceMode->SetElements1(5+m_plane);

	vtkSMSourceProxy * reader = vtkSMSourceProxy::SafeDownCast(input->getProxy());
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
	//Store real input
	m_inputs->push_back(seg);
	
	//Create its slider
	pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
	pqPipelineSource *slicer = ob->createFilter("filters","ImageSlicer",seg->data(),0);
	
	vtkSMIntVectorProperty *sliceMode = 
		vtkSMIntVectorProperty::SafeDownCast(slicer->getProxy()->GetProperty("SliceMode"));
	sliceMode->SetElements1(5+m_plane);
	m_slicers->push_back(slicer);

// 	vtkSMSourceProxy * reader = vtkSMSourceProxy::SafeDownCast(source->getProxy());
// 	//source->getOutputPort(0)->getDataInformation()->PrintSelf(std::cout,vtkIndent(0));
// 	double *bounds = source->getOutputPort(0)->getDataInformation()->GetBounds();
// 	int *extent = source->getOutputPort(0)->getDataInformation()->GetExtent();
// 	memcpy(m_bounds,bounds,6*sizeof(double));
// 	memcpy(m_extent,extent,6*sizeof(int));
// 	//vtkSMDataSourceProxy *data = vtkSMDataSourceProxy::SafeDownCast(source->getOutputPort(0)->getOutputPortProxy());
// 	//if (data) qDebug() << "OK";
// 	//double *spacing;
// 	//reader->GetPixelSpacing();
// 	//qDebug() << spacing[0] << spacing[1] << spacing[2];
	
	emit outputChanged(this->getOutput());
}


pqOutputPort *SliceBlender::getOutput()
{
	if (m_blending)
	  return NULL;
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
