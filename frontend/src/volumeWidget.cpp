#include "volumeWidget.h"
#include "renderer.h"
#include "slicer.h"

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

//------------------------------------------------------------------------
VolumeWidget::VolumeWidget()
	: m_view(NULL)
	, m_viewWidget(NULL)
	, m_init(false)
	, m_showPlanes(false)
	, m_showActors(false)
	, m_renderer(NULL)
	, m_valid(NULL)
	, m_rejected(NULL)
	, m_userSelection(NULL)

{
	for (SlicePlane plane = SLICE_PLANE_FIRST; 
			plane <= SLICE_PLANE_LAST; 
			plane=SlicePlane(plane+1))
		m_planes[plane] = NULL;

	// Create Layout and Widgets
	m_controlLayout = new QHBoxLayout();
	
	m_toggleActors = new QToolButton(this);
	m_toggleActors->setIcon(QIcon(":/espina/hide3D"));
	m_toggleActors->setCheckable(true);
	
	m_togglePlanes = new QToolButton(this);
	m_togglePlanes->setIcon(QIcon(":/espina/hidePlanes"));
	m_togglePlanes->setCheckable(true);
	connect(m_togglePlanes,SIGNAL(toggled(bool)),this,SLOT(showPlanes(bool)));
	
	m_controlLayout->addStretch();
	m_controlLayout->addWidget(m_toggleActors);
	
	QMenu *renders = new QMenu();
	QAction *volumeRenderer = new QAction(QIcon(":/espina/hide3D"),tr("Volume"),renders);
	QAction *meshRenderer = new QAction(QIcon(":/espina/hidePlanes"),tr("Mesh"),renders);
	renders->addAction(volumeRenderer);
	renders->addAction(meshRenderer);
	m_toggleActors->setMenu(renders);
	connect(m_toggleActors,SIGNAL(toggled(bool)),this,SLOT(showActors(bool)));
	connect(volumeRenderer,SIGNAL(triggered()),this,SLOT(setVolumeRenderer()));
	connect(meshRenderer,SIGNAL(triggered()),this,SLOT(setMeshRenderer()));
	m_controlLayout->addWidget(m_togglePlanes);

	m_mainLayout = new QVBoxLayout();
	m_mainLayout->addLayout(m_controlLayout);
	setLayout(m_mainLayout);
	
	m_renderer = VolumeRenderer::renderer();
}

//-----------------------------------------------------------------------------
VolumeWidget::~VolumeWidget()
{
	//Objects creted by pqObjectBuilder have to be destroyed by it
	if (m_view)	
		pqApplicationCore::instance()->getObjectBuilder()->destroy(m_view);
}

//-----------------------------------------------------------------------------
void VolumeWidget::setPlane(SliceBlender *slice, const SlicePlane plane)
{
	if (slice)
		m_planes[plane] = slice;
	//TODO: Manage previous plane if existen?
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
    pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
    pqRepresentation *rep;
    foreach(rep,m_view->getRepresentations())
    {
      rep->setVisible(false);
    }
    for (SlicePlane plane = SLICE_PLANE_FIRST; 
	 plane <= SLICE_PLANE_LAST; 
         plane=SlicePlane(plane+1))
	 dp->setRepresentationVisibility(m_planes[plane]->getOutput(),m_view,m_showPlanes);
	  
    // If there are segmentations
	if (m_valid)  
	{
	  Segmentation *seg;
	  foreach(seg,*m_valid)
	  {
		if (m_showActors)
		  m_renderer->render(seg,m_view);
		else
		  m_renderer->hide(seg,m_view);
	  }
	}
	m_view->render();
}


//-----------------------------------------------------------------------------
void VolumeWidget::showPlanes(bool value)
{
	if (m_showPlanes == value)
		return;

	m_showPlanes = value;
	m_togglePlanes->setIcon(m_showPlanes?QIcon(":/espina/showPlanes"):QIcon(":/espina/hidePlanes"));

	pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
	for (SlicePlane plane = SLICE_PLANE_FIRST; 
			plane <= SLICE_PLANE_LAST; 
			plane=SlicePlane(plane+1))
		dp->setRepresentationVisibility(m_planes[plane]->getOutput(),m_view,m_showPlanes);
	updateRepresentation();
}


//-----------------------------------------------------------------------------
void VolumeWidget::showActors(bool value)
{
	if (m_showActors == value)
		return;

	m_showActors = value;
	switch (m_renderer->type())
	{
		case MESH_RENDERER:
			m_toggleActors->setIcon(m_showActors?QIcon(":/espina/showPlanes"):QIcon(":/espina/hidePlanes"));
			break;
		case VOLUME_RENDERER:
			m_toggleActors->setIcon(m_showActors?QIcon(":/espina/show3D"):QIcon(":/espina/hide3D"));
			break;
		default:
			assert(false);
	}

	updateRepresentation();
}

void VolumeWidget::setMeshRenderer()
{
	m_renderer = MeshRenderer::renderer();
	m_toggleActors->setIcon(m_showActors?QIcon(":/espina/showPlanes"):QIcon(":/espina/hidePlanes"));

	updateRepresentation();
}

void VolumeWidget::setVolumeRenderer()
{
	m_renderer = VolumeRenderer::renderer();
	m_toggleActors->setIcon(m_showActors?QIcon(":/espina/show3D"):QIcon(":/espina/hide3D"));
	
	updateRepresentation();
}

void VolumeWidget::renderValidActors()
{
	Segmentation *seg;
	foreach(seg,*m_valid)
	{
	  m_renderer->render(seg,m_view);
	}
}
