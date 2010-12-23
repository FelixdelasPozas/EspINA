#include "volumeWidget.h"

#include "pqRenderView.h"
#include "pqApplicationCore.h"
#include "pqActiveObjects.h"
#include "pqDisplayPolicy.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqPipelineRepresentation.h"
#include "vtkSMUniformGridVolumeRepresentationProxy.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMStringVectorProperty.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QSpinBox>

// LUT
#include "vtkSmartPointer.h"
#include "vtkLookupTable.h"

#include <QDebug>
#include <assert.h>

#define HINTWIDTH 40

//-----------------------------------------------------------------------------
VolumeWidget::VolumeWidget()
	: m_view(NULL)
	, m_viewWidget(NULL)
	, m_init(false)
{
	m_controlLayout = new QHBoxLayout();
	m_scroll = new QScrollBar(Qt::Horizontal);
	m_scroll->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	m_slice = new QSpinBox();
	m_slice->setMinimumWidth(HINTWIDTH);
	m_slice->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
	m_controlLayout->addWidget(m_scroll);
	m_controlLayout->addWidget(m_slice);

	m_mainLayout = new QVBoxLayout();
	//m_mainLayout->addLayout(m_controlLayout);
	setLayout(m_mainLayout);
}

//-----------------------------------------------------------------------------
VolumeWidget::~VolumeWidget()
{
	//Objects creted by pqObjectBuilder have to be destroyed by it
	if (m_view)	
		pqApplicationCore::instance()->getObjectBuilder()->destroy(m_view);
}

//-----------------------------------------------------------------------------
void VolumeWidget::showSource(pqOutputPort *opPort, Rep3D rep)
{
	pqDisplayPolicy *displayManager = pqApplicationCore::instance()->getDisplayPolicy();
	// Creates the representation if it doesn't exit
	bool visible = rep != HIDEN;
	displayManager->setRepresentationVisibility(opPort,m_view,visible);

	// Configures the specified representation
	pqPipelineRepresentation* pipelineRep = 
		qobject_cast<pqPipelineRepresentation*>(opPort->getRepresentation(m_view));
	assert(!visible ||  pipelineRep);

	//TODO: Representation specific code must be addded
	switch (rep)
	{
		case POINTS:
		case OUTLINE:
		case SURFACE:
		case VOLUME:
			pipelineRep->setRepresentation(rep);
			break;
		case SLICE:
			break;
		case HIDEN:
			break;
		default:
			assert(false);
	}

	//vtkSMIntVectorProperty *rt = 
	//	vtkSMIntVectorProperty::SafeDownCast(orepproxy->GetProperty("Representation"));
	//if (rt)
	//{
	//	std::cout << "Representation\n";
	//	rt->PrintSelf(std::cout,vtkIndent(2));
	//	rt->SetElements1(2);
	//	rep->UpdateVTKObjects();
	//}

	//// Create LUT
	//vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
	//lut->SetRange(0, 256); // image intensity range
	//lut->SetValueRange(0.0, 1.0); // from black to white
	//lut->SetSaturationRange(0.0, 0.0); // no color saturation
	//lut->SetRampToLinear();

	// Create Server LUT
	//pqObjectBuilder *builder = pqApplicationCore::instance()->getObjectBuilder();
	//pqServer * server= pqActiveObjects::instance().activeServer();
	//vtkSMProxy *pLUT = builder->createSource("sources","vtkLookupTable",server);
	
	
	//vtkSMProxyProperty *cat = 
	//	vtkSMProxyProperty::SafeDownCast(pipelineRep->getProxy()->GetProperty("LookupTable"));
	//if (cat)
	//{
	//	std::cout << "LUT\n";
	//	//cat->SetElements3(0,1,0);
	//	cat->GetProxy(0)->PrintSelf(std::cout,vtkIndent(2));
	//	//rep->UpdateVTKObjects();
	//}
}


//-----------------------------------------------------------------------------
void VolumeWidget::connectToServer()
{
	//qDebug() << "Creating View";
	pqObjectBuilder *builder = pqApplicationCore::instance()->getObjectBuilder();
	pqServer * server= pqActiveObjects::instance().activeServer();
	m_view = qobject_cast<pqRenderView*>(builder->createView(
			  pqRenderView::renderViewType(), server));
	m_viewWidget = m_view->getWidget();
	m_mainLayout->insertWidget(0,m_viewWidget);//To preserver view order
}


//-----------------------------------------------------------------------------
void VolumeWidget::disconnectFromServer()
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

void VolumeWidget::updateRepresentation()
{
	m_view->render();
}


