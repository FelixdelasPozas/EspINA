#include "slicer.h"

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
	: m_plane(plane)
{
	m_inputs = new QList<pqPipelineSource *>;
	m_slicers = new QList<pqPipelineSource *>;

	updateAxis();
}


void SliceBlender::addInput(pqPipelineSource *source)
{
	//Store real input
	m_inputs->push_back(source);
	//Create its slider
	pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
	pqPipelineSource *slicer = ob->createFilter("filters","ImageSlicer",source,0);
	vtkSMIntVectorProperty *sliceMode = 
		vtkSMIntVectorProperty::SafeDownCast(slicer->getProxy()->GetProperty("SliceMode"));
	sliceMode->SetElements1(5+m_plane);
	m_slicers->push_back(slicer);

	vtkSMSourceProxy * reader = vtkSMSourceProxy::SafeDownCast(source->getProxy());
	//source->getOutputPort(0)->getDataInformation()->PrintSelf(std::cout,vtkIndent(0));
	double *bounds = source->getOutputPort(0)->getDataInformation()->GetBounds();
	int *extent = source->getOutputPort(0)->getDataInformation()->GetExtent();
	memcpy(m_bounds,bounds,6*sizeof(double));
	memcpy(m_extent,extent,6*sizeof(int));
	//vtkSMDataSourceProxy *data = vtkSMDataSourceProxy::SafeDownCast(source->getOutputPort(0)->getOutputPortProxy());
	//if (data) qDebug() << "OK";
	//double *spacing;
	//reader->GetPixelSpacing();
	//qDebug() << spacing[0] << spacing[1] << spacing[2];
	
	emit outputChanged(slicer->getOutputPort(0));
}

pqOutputPort *SliceBlender::getOutput()
{
	if (m_slicers->size() > 0)
		return (*m_slicers)[0]->getOutputPort(0);
	else
		return NULL;
}

int SliceBlender::getNumSlices()
{
	return m_extent[2*m_zAxis+1];
}

void SliceBlender::setSlice(int slice)
{
	pqPipelineSource *slicer;
	foreach(slicer,*m_slicers)
	{
		vtkSMIntVectorProperty *sliceIdx = 
			vtkSMIntVectorProperty::SafeDownCast(slicer->getProxy()->GetProperty("Slice"));
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
