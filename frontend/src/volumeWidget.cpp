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
#include <QToolButton>

// LUT
#include "vtkSmartPointer.h"
#include "vtkLookupTable.h"
#include "vtkSMPVLookupTableProxy.h"

#include <QMenu>
#include <QAction>

#include <QDebug>
#include <assert.h>

#define HINTWIDTH 40


//-----------------------------------------------------------------------------
VolumeWidget::VolumeWidget()
	: m_view(NULL)
	, m_viewWidget(NULL)
	, m_init(false)
	, m_showPlanes(false)
{
	for (SlicePlane plane = SLICE_PLANE_FIRST; 
			plane <= SLICE_PLANE_LAST; 
			plane=SlicePlane(plane+1))
		m_planes[plane] = NULL;

	// Create Layout and Widgets
	m_controlLayout = new QHBoxLayout();
	m_toggle3D = new QToolButton(this);
	m_toggle3D->setIcon(QIcon(":/espina/hide_3D.svg"));
	m_toggle3D->setCheckable(true);
	m_togglePlanes = new QToolButton(this);
	m_togglePlanes->setIcon(QIcon(":/espina/hide_planes.svg"));
	m_togglePlanes->setCheckable(true);
	connect(m_togglePlanes,SIGNAL(toggled(bool)),this,SLOT(showPlanes(bool)));
	m_controlLayout->addStretch();
	m_controlLayout->addWidget(m_toggle3D);
	QMenu *renders = new QMenu();
	QAction *meshRenderer = new QAction(QIcon(":/espina/hide_3D.svg"),tr("Mesh"),renders);
	QAction *volumeRenderer = new QAction(QIcon(":/espina/hide_planes.svg"),tr("Volume"),renders);
	renders->addAction(meshRenderer);
	renders->addAction(volumeRenderer);
	m_toggle3D->setMenu(renders);
	m_controlLayout->addWidget(m_togglePlanes);

	m_mainLayout = new QVBoxLayout();
	m_mainLayout->addLayout(m_controlLayout);
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
void VolumeWidget::setPlane(pqOutputPort *opPort, const SlicePlane plane)
{
	if (opPort)
		m_planes[plane] = opPort;
}

//-----------------------------------------------------------------------------
void VolumeWidget::showPlanes(bool value)
{
	if (m_showPlanes == value)
		return;

	m_togglePlanes->setIcon(m_showPlanes?QIcon(":/espina/hide_planes.svg"):QIcon(":/espina/show_planes.svg"));
	
	m_showPlanes = value;
	pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
	for (SlicePlane plane = SLICE_PLANE_FIRST; 
			plane <= SLICE_PLANE_LAST; 
			plane=SlicePlane(plane+1))
		dp->setRepresentationVisibility(m_planes[plane],m_view,m_showPlanes);
	updateRepresentation();
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

	vtkSMProxyProperty *cat;
	//TODO: Representation specific code must be addded
	switch (rep)
	{
		case POINTS:
		case OUTLINE:
		case SURFACE:
			{
				pipelineRep->setRepresentation(rep);
				 // Ambient Color
				vtkSMDoubleVectorProperty *ambient = 
					vtkSMDoubleVectorProperty::SafeDownCast(pipelineRep->getProxy()->GetProperty("DiffuseColor"));
				if (ambient)
				{
					ambient->SetElements3(0,1,0); 
					pipelineRep->getProxy()->UpdateVTKObjects();
				}
				//// Opacity
				//vtkSMDoubleVectorProperty *opacity = 
				//	vtkSMDoubleVectorProperty::SafeDownCast(pipelineRep->getProxy()->GetProperty("Opacity"));
				//if (opacity)
				//{
				//	opacity->SetElements1(0.5); 
				//	pipelineRep->getProxy()->UpdateVTKObjects();
				//}
				////pipelineRep->getProxy()->PrintSelf(std::cout,vtkIndent(2));
			}
			break;
		case VOLUME:
			{
				pipelineRep->setRepresentation(rep);
				// Change LUT colors to gray scale
				vtkSMPVLookupTableProxy *lut = 
					vtkSMPVLookupTableProxy::SafeDownCast(pipelineRep->getLookupTableProxy());
				if (lut)
				{
					lut->UpdatePropertyInformation();
					//lut->PrintSelf(std::cout,vtkIndent(2));
					vtkSMDoubleVectorProperty *rgbs = 
						vtkSMDoubleVectorProperty::SafeDownCast(lut->GetProperty("RGBPoints"));
					if (rgbs)
					{
						double colors[8] = {0,0,0,0,1,1,1,1};
						rgbs->SetElements(colors);
					}
				}
			}
			break;
		case SLICE:
			break;
		case HIDEN:
			break;
		default:
			assert(false);
	}

	//// Create LUT
	//vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
	//lut->SetRange(0, 256); // image intensity range
	//lut->SetValueRange(0.0, 1.0); // from black to white
	//lut->SetSaturationRange(0.0, 0.0); // no color saturation
	//lut->SetRampToLinear();
	//lut->Build();

	// Create Server LUT
	//pqObjectBuilder *builder = pqApplicationCore::instance()->getObjectBuilder();
	//pqServer * server= pqActiveObjects::instance().activeServer();
	//vtkSMProxy *pLUT = builder->createSource("sources","vtkLookupTable",server);
	
	
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


