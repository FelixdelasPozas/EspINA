/*=========================================================================

   Program: Espina
   Module:    EspinaMainWindow.cpp

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
// ESPINA includes
#include "espinaMainWindow.h"
#include "ui_espinaMainWindow.h"
#include "objectManager.h"
#include "segmentation.h"
#include "sliceWidget.h"
#include "slicer.h"
#include "volumeWidget.h"
#include "volumeView.h"
#include "stack.h"
#include "distance.h"
#include "unitExplorer.h"
#include "selectionManager.h"
#include "traceNodes.h"
#include "cache/cache.h"
#include "segmentationModel.h"
#include "data/taxonomy.h"

//ParaQ includes
#include "pqHelpReaction.h"
#include "pqObjectInspectorWidget.h"
#include "pqParaViewBehaviors.h"
#include "pqParaViewMenuBuilders.h"
#include "pqLoadDataReaction.h"
#include "pqPipelineSource.h"
#include "vtkPVPlugin.h"
#include "pqOutputPort.h"
#include "pqServerManagerObserver.h"
#include "vtkSMOutputPort.h"
#include "vtkSMProperty.h"

#include "pqRenderView.h"
#include "pqTwoDRenderView.h"
#include "pqRepresentation.h"
#include "pqApplicationCore.h"
#include "pqActiveObjects.h"
#include "pqObjectBuilder.h"
#include "pqObjectInspectorWidget.h"
#include "pqDisplayPolicy.h"

//VTK Includes
#include "vtkStructuredData.h" 
#include "vtkImageData.h" 

//New
#include "vtkPVImageSlicer.h"
#include "vtkSMIntVectorProperty.h"

//Debug includes
#include <QDebug>
#include <iostream>
#include <QPushButton>

class EspinaMainWindow::pqInternals : public Ui::pqClientMainWindow
{
};

//-----------------------------------------------------------------------------
EspinaMainWindow::EspinaMainWindow()
	: m_xy(NULL)
	, m_yz(NULL)
	, m_xz(NULL)
	, m_3d(NULL)
	, m_unit(NM)
	, m_selectionManager(NULL)
{
  this->Internals = new pqInternals();
  this->Internals->setupUi(this);

  // Setup default GUI layout.
  connect(this->Internals->toggleVisibility,SIGNAL(toggled(bool)),this,SLOT(toggleVisibility(bool)));

  // Set up the dock window corners to give the vertical docks more room.
  this->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
  this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

  //Create File Menu
  buildFileMenu(*this->Internals->menu_File);
  
  buildTaxonomy();
  m_productManager = ObjectManager::instance();
  
  m_segModel = new SegmentationModel;
  m_segModel->setTaxonomy(m_taxonomies);
  m_segModel->setObjectManager(m_productManager);
  this->Internals->objectTreeView->setModel(m_segModel);;
  
  //// Populate application menus with actions.
  pqParaViewMenuBuilders::buildFileMenu(*this->Internals->menu_File);

  //// Populate filters menu.
  pqParaViewMenuBuilders::buildFiltersMenu(*this->Internals->menuTools, this);

  //// Populate Tools menu.
  pqParaViewMenuBuilders::buildToolsMenu(*this->Internals->menuTools);

  //// setup the context menu for the pipeline browser.
  //pqParaViewMenuBuilders::buildPipelineBrowserContextMenu(
  //  *this->Internals->pipelineBrowser);

  //// Setup the View menu. This must be setup after all toolbars and dockwidgets
  //// have been created.
  m_unitExplorer = new UnitExplorer();
  connect(this->Internals->actionUnits,SIGNAL(triggered()),m_unitExplorer,SLOT(show()));
  pqParaViewMenuBuilders::buildViewMenu(*this->Internals->menu_View, *this);

  //// Setup the help menu.
  pqParaViewMenuBuilders::buildHelpMenu(*this->Internals->menu_Help);

  // ParaView Server
  pqServerManagerObserver *server = pqApplicationCore::instance()->getServerManagerObserver();

  //Create ESPINA
  for (SlicePlane plane = SLICE_PLANE_FIRST; plane <= SLICE_PLANE_LAST; plane=SlicePlane(plane+1))
	  m_planes[plane] = new SliceBlender(plane);
  m_selectionManager = SelectionManager::singleton();

  //Create ESPINA views
  m_xy = new SliceWidget(m_planes[SLICE_PLANE_XY]);
  this->setCentralWidget(m_xy);
  connect(server,SIGNAL(connectionCreated(vtkIdType)),m_xy,SLOT(connectToServer()));
  connect(server,SIGNAL(connectionClosed(vtkIdType)),m_xy,SLOT(disconnectFromServer()));
  connect(m_xy,SIGNAL(pointSelected(const Point)),m_selectionManager,SLOT(pointSelected(const Point)));
  
  m_yz = new SliceWidget(m_planes[SLICE_PLANE_YZ]);
  this->Internals->yzSliceDock->setWidget(m_yz);
  connect(server,SIGNAL(connectionCreated(vtkIdType)),m_yz,SLOT(connectToServer()));
  connect(server,SIGNAL(connectionClosed(vtkIdType)),m_yz,SLOT(disconnectFromServer()));
  
  m_xz = new SliceWidget(m_planes[SLICE_PLANE_XZ]);
  this->Internals->xzSliceDock->setWidget(m_xz);
  connect(server,SIGNAL(connectionCreated(vtkIdType)),m_xz,SLOT(connectToServer()));
  connect(server,SIGNAL(connectionClosed(vtkIdType)),m_xz,SLOT(disconnectFromServer()));
  
  m_3d = new VolumeView();
  m_3d->setModel(m_segModel);
  m_3d->setSelectionModel(this->Internals->objectTreeView->selectionModel());
  this->Internals->volumeDock->setWidget(m_3d);
  connect(server,SIGNAL(connectionCreated(vtkIdType)),m_3d,SLOT(connectToServer()));
  connect(server,SIGNAL(connectionClosed(vtkIdType)),m_3d,SLOT(disconnectFromServer()));
  
  connect(m_productManager,SIGNAL(render(IRenderable*)),
	  m_3d,SLOT(refresh(IRenderable*)));
  // Final step, define application behaviors. Since we want all ParaView
  // behaviors, we use this convenience method.
  new pqParaViewBehaviors(this, this);

}

//-----------------------------------------------------------------------------
EspinaMainWindow::~EspinaMainWindow()
{
  //delete this->Internals;
  //delete m_xy;
  //delete m_yz;
  //delete m_xz;
 // delete m_3d;
}


//-----------------------------------------------------------------------------
void EspinaMainWindow::loadData(pqPipelineSource *source)
{ 
  Product *stack = new Product(source,0);
  stack->name = "/home/jorge/Stacks/peque.mha";
  stack->setVisible(false);
  
  Cache *cache = Cache::instance();
  cache->insert(stack->id(),source);
  
  /*
  // Create a fake segmentation to make the tests
  pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
  pqServer * server= pqActiveObjects::instance().activeServer();
  QStringList file;
  //file << "/home/jfernandez/workspace/bbp_workflow/data_experiments/Espina_files/segmentita.mha";
  file << "/home/jorge/Stacks/segmentita.mha";
  pqPipelineSource *fakeSource = ob->createReader("sources","MetaImageReader",file,server);
  fakeSource->updatePipeline();
  Product *seg = new Product(fakeSource,0);
  */
  
  // This updates the visualization pipeline before initializing the slice widgets
  source->updatePipeline();
  for (SlicePlane plane = SLICE_PLANE_FIRST; plane <= SLICE_PLANE_LAST; plane=SlicePlane(plane+1))
  {
    m_planes[plane]->setBackground(stack);
    //m_planes[plane]->addSegmentation(seg);
    //m_3d->setPlane(m_planes[plane],plane);
    connect(m_planes[plane],SIGNAL(updated()),m_3d,SLOT(updateScene()));
    connect(m_productManager,SIGNAL(sliceRender(IRenderable*)),
	    m_planes[plane],SLOT(addSegmentation(IRenderable *)));
  }
  m_productManager->registerProduct(stack);
  //m_productManager->registerProduct(seg);
}

//-----------------------------------------------------------------------------
void EspinaMainWindow::toggleVisibility(bool visible)
{
	this->Internals->toggleVisibility->setIcon(
			visible?QIcon(":/espina/show_all.svg"):QIcon(":/espina/hide_all.svg")
			);
	
  for (SlicePlane plane = SLICE_PLANE_FIRST; plane <= SLICE_PLANE_LAST; plane=SlicePlane(plane+1))
  {
	if (m_planes[plane])
	  m_planes[plane]->setBlending(visible);
  }
}


//-----------------------------------------------------------------------------
void EspinaMainWindow::buildFileMenu(QMenu &menu)
{
	QIcon icon = qApp->style()->standardIcon(QStyle::SP_DialogOpenButton);
	QAction *openAction = new QAction(icon,tr("Open"),this);
	pqLoadDataReaction * loadReaction = new pqLoadDataReaction(openAction);
	QObject::connect(loadReaction, SIGNAL(loadedData(pqPipelineSource *)),
		this, SLOT( loadData(pqPipelineSource *)));
	menu.addAction(openAction);
}


void EspinaMainWindow::buildTaxonomy()
{
  m_taxonomies = new TaxonomyNode("FEM");
  TaxonomyNode *newNode;
  newNode = m_taxonomies->addElement("Synapse","FEM");
  newNode->setColor(QColor(255,0,0));
  m_taxonomies->addElement("Vesicles","FEM");
  m_taxonomies->addElement("Symetric","Synapse");
  newNode = m_taxonomies->addElement("Asymetric","Synapse");
  newNode->setColor(QColor(Qt::yellow));
  /*
  m_taxonomies->addElement("A","Vesicles");
  m_taxonomies->addElement("B","Vesicles");
  m_taxonomies->addElement("B1","B");
  m_taxonomies->addElement("A1","A");
  m_taxonomies->addElement("A2","A");
  m_taxonomies->addElement("B2","B");
  m_taxonomies->addElement("B3","B");
  */
  //IOTaxonomy::writeXMLTaxonomy(*m_taxonomies,"TaxonomyFIB.xml");
  m_taxonomies->print();
  
}
