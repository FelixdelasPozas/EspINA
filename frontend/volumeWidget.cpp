#include "volumeWidget.h"

// Espina
#include "renderer.h"
#include "slicer.h"
#include "interfaces.h"

// ParaView
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

// LUT
#include "vtkSmartPointer.h"
#include "vtkLookupTable.h"
#include "vtkSMPVLookupTableProxy.h"

// GUI
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QAction>

// DEBUG
#include <QDebug>
#include <assert.h>

#define HINTWIDTH 40

//------------------------------------------------------------------------
VolumeWidget::VolumeWidget()
: m_init(false)
, m_showPlanes(false)
, m_showActors(false)
, m_renderer(NULL)
, m_view(NULL)
, m_viewWidget(NULL)

{
  for (SlicePlane plane = SLICE_PLANE_FIRST
    ; plane <= SLICE_PLANE_LAST
    ; plane=SlicePlane(plane+1)
  )
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
  //TODO: Review
  //Objects creted by pqObjectBuilder have to be destroyed by it
  if (m_view)	
    pqApplicationCore::instance()->getObjectBuilder()->destroy(m_view);
}

//-----------------------------------------------------------------------------
void VolumeWidget::setPlane(SliceBlender *slice, const SlicePlane plane)
{
  if (slice)
    m_planes[plane] = slice;
  //TODO: Manage previous plane if already existed?
}

//-----------------------------------------------------------------------------
void VolumeWidget::connectToServer()
{
  //TODO: Review
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
  // TODO: Review
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
void VolumeWidget::showPlanes(bool value)
{
  if (m_showPlanes == value)
    return;
  
  m_showPlanes = value;
  m_togglePlanes->setIcon(m_showPlanes?QIcon(":/espina/showPlanes"):QIcon(":/espina/hidePlanes"));
  
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  for (SlicePlane plane = SLICE_PLANE_FIRST
    ; plane <= SLICE_PLANE_LAST
    ; plane = SlicePlane(plane+1))
  {
    assert(m_planes); //DEBUG
    dp->setRepresentationVisibility(m_planes[plane]->getOutput(),m_view,m_showPlanes);
  }
  
  updateScene();
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
  
  updateScene();
}

void VolumeWidget::refresh(IRenderable* actor)
{
  m_actors.push_back(actor);
  updateScene();
}

//-----------------------------------------------------------------------------
void VolumeWidget::updateScene()
{
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  pqRepresentation *rep;
  foreach(rep,m_view->getRepresentations())
  {
    rep->setVisible(false);
  }
  
  //TODO: Center on selection bounding box or active stack if no selection
  if (m_showPlanes)
    dp->setRepresentationVisibility(m_planes[SLICE_AXIS_X]->getBgOutput(),m_view,true);
  
  // Render renderable products
  IRenderable *actor;
  foreach(actor,m_actors)
  {
    if (m_showActors)
      m_renderer->render(actor,m_view);
  }
    /* XXX: Is really necessary? They should be already hidden by 
     * initial foreach loop
    else
      m_renderer->hide(actor,m_view);
  }
    */
  
  // Render planes
  for (SlicePlane plane = SLICE_PLANE_FIRST
    ; plane <= SLICE_PLANE_LAST
    ; plane = SlicePlane(plane+1))
    dp->setRepresentationVisibility(m_planes[plane]->getOutput(),m_view,m_showPlanes);
  
  m_view->render();
}


void VolumeWidget::setMeshRenderer()
{
  m_renderer = MeshRenderer::renderer();
  // Which one is easier to read/understand? This one or the VolumeRenderer one?
  m_toggleActors->setIcon(
    m_showActors?
      QIcon(":/espina/showPlanes")
    :
      QIcon(":/espina/hidePlanes")
	);
  
  updateScene();
}

void VolumeWidget::setVolumeRenderer()
{
  m_renderer = VolumeRenderer::renderer();
  m_toggleActors->setIcon(m_showActors?QIcon(":/espina/show3D"):QIcon(":/espina/hide3D"));
  
  updateScene();
}

/*
void VolumeWidget::renderValidActors()
{
  Segmentation *seg;
  foreach(seg,*m_valid)
  {
    m_renderer->render(seg,m_view);
  }
}
*/