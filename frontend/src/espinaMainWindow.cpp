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
#include "emSegmentation.h"
#include "sliceWidget.h"
#include "slicer.h"
#include "volumeWidget.h"

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
  pqParaViewMenuBuilders::buildViewMenu(*this->Internals->menu_View, *this);

  //// Setup the help menu.
  pqParaViewMenuBuilders::buildHelpMenu(*this->Internals->menu_Help);

  // ParaView Server
  pqServerManagerObserver *server = pqApplicationCore::instance()->getServerManagerObserver();

  //Create ESPINA
  m_segmentation = new EMSegmentation();
  for (SlicePlane plane = SLICE_PLANE_FIRST; plane <= SLICE_PLANE_LAST; plane=SlicePlane(plane+1))
	  m_planes[SlicePlane(plane)] = new SliceBlender(SlicePlane(plane));

  //Create ESPINA views
  m_xy = new SliceWidget(m_planes[SLICE_PLANE_XY]);
  this->setCentralWidget(m_xy);
  connect(server,SIGNAL(connectionCreated(vtkIdType)),m_xy,SLOT(connectToServer()));
  connect(server,SIGNAL(connectionClosed(vtkIdType)),m_xy,SLOT(disconnectFromServer()));
  m_yz = new SliceWidget(m_planes[SLICE_PLANE_YZ]);
  this->Internals->yzSliceDock->setWidget(m_yz);
  connect(server,SIGNAL(connectionCreated(vtkIdType)),m_yz,SLOT(connectToServer()));
  connect(server,SIGNAL(connectionClosed(vtkIdType)),m_yz,SLOT(disconnectFromServer()));
  m_xz = new SliceWidget(m_planes[SLICE_PLANE_XZ]);
  this->Internals->xzSliceDock->setWidget(m_xz);
  connect(server,SIGNAL(connectionCreated(vtkIdType)),m_xz,SLOT(connectToServer()));
  connect(server,SIGNAL(connectionClosed(vtkIdType)),m_xz,SLOT(disconnectFromServer()));
  m_3d = new VolumeWidget();
  this->Internals->volumeDock->setWidget(m_3d);
  connect(server,SIGNAL(connectionCreated(vtkIdType)),m_3d,SLOT(connectToServer()));
  connect(server,SIGNAL(connectionClosed(vtkIdType)),m_3d,SLOT(disconnectFromServer()));
  
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
void EspinaMainWindow::setWorkingStack(pqPipelineSource *source)
{
	//TODO: Deal with multiple representations inside the same view
	//		At the moment, we only display the first one
	m_segmentation->setStack(source);

	//pqActiveObjects& activeObjects = pqActiveObjects::instance();
	//activeObjects.setActiveSource(source);
	
	//Set new stack and display it
	m_3d->showSource(m_segmentation->visualizationStack()->getOutputPort(0),true);
	for (SlicePlane plane = SLICE_PLANE_FIRST; plane <= SLICE_PLANE_LAST; plane=SlicePlane(plane+1))
	{
		m_planes[plane]->addInput(m_segmentation->visualizationStack());
		m_3d->showSource(m_planes[plane]->getOutput(),true);
		connect(m_planes[plane],SIGNAL(updated()),m_3d,SLOT(updateRepresentation()));
	}
}

//-----------------------------------------------------------------------------
void EspinaMainWindow::toggleVisibility(bool visible)
{
	this->Internals->toggleVisibility->setIcon(
			visible?QIcon(":/espina/ojo_abierto.svg"):QIcon(":/espina/ojo_cerrado.svg")
			);
	//TODO: Modificar blenders para redirigir la salida
}


//-----------------------------------------------------------------------------
void EspinaMainWindow::buildFileMenu(QMenu &menu)
{
	QIcon icon = qApp->style()->standardIcon(QStyle::SP_DialogOpenButton);
	QAction *openAction = new QAction(icon,tr("Open"),this);
	pqLoadDataReaction * loadReaction = new pqLoadDataReaction(openAction);
	QObject::connect(loadReaction, SIGNAL(loadedData(pqPipelineSource *)),
		this, SLOT(setWorkingStack(pqPipelineSource *)));
	menu.addAction(openAction);
}
