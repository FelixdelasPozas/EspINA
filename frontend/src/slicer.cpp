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
