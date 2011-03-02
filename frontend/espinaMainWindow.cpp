/*=========================================================================
 *
 *   Program: Espina
 *   Module:    EspinaMainWindow.cpp
 *
 *   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
 *   All rights reserved.
 *
 *   ParaView is a free software; you can redistribute it and/or modify it
 *   under the terms of the ParaView license version 1.2.
 *
 *   See License_v1.2.txt for the full ParaView license.
 *   A copy of this license can be obtained by contacting
 *   Kitware Inc.
 *   28 Corporate Drive
 *   Clifton Park, NY 12065
 *   USA
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ========================================================================*/
// ESPINA includes
#include "espinaMainWindow.h"
#include "ui_espinaMainWindow.h"
#include "slicer.h"
#include "volumeView.h"
#include "distance.h"
#include "unitExplorer.h"
#include "selectionManager.h"
#include "traceNodes.h"
#include "cache/cache.h" //TODO delete
#include "cache/cachedObjectBuilder.h" // TODO deberia ir en traceNodes
#include "data/taxonomy.h"
#include "espina.h"

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

//QT includes
#include <QFileDialog>

//Debug includes
#include <QDebug>
#include <iostream>
#include <QPushButton>
#include <pqServerResources.h>


#include <taxonomyProxy.h>
#include <sampleProxy.h>
#include "sliceView.h"

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
  connect(this->Internals->toggleVisibility, SIGNAL(toggled(bool)), this, SLOT(toggleVisibility(bool)));

  // Set up the dock window corners to give the vertical docks more room.
  this->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
  this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

  m_espina = EspINA::instance();
  TaxonomyProxy *taxProxy = new TaxonomyProxy();
  taxProxy->setSourceModel(m_espina);
  SampleProxy *sampleProxy = new SampleProxy();
  sampleProxy->setSourceModel(m_espina);
  this->Internals->objectTreeView->setModel(taxProxy);
  this->Internals->objectTreeView->setRootIndex(taxProxy->mapFromSource(m_espina->taxonomyRoot()));
  connect(this->Internals->objectTreeView, SIGNAL(doubleClicked(const QModelIndex &)), m_espina, SLOT(setUserDefindedTaxonomy(const QModelIndex&)));

  //Create File Menu
  buildFileMenu(*this->Internals->menu_File);

  //// Populate application menus with actions.
  pqParaViewMenuBuilders::buildFileMenu(*this->Internals->menu_File);

  //// Populate filters menu.
  //pqParaViewMenuBuilders::buildFiltersMenu(*this->Internals->menuTools, this);

  //// Populate Tools menu.
  pqParaViewMenuBuilders::buildToolsMenu(*this->Internals->menuTools);

  //// setup the context menu for the pipeline browser.
  //pqParaViewMenuBuilders::buildPipelineBrowserContextMenu(
  //  *this->Internals->pipelineBrowser);

  //// Setup the View menu. This must be setup after all toolbars and dockwidgets
  //// have been created.
  m_unitExplorer = new UnitExplorer();
  connect(this->Internals->actionUnits, SIGNAL(triggered()), m_unitExplorer, SLOT(show()));
  pqParaViewMenuBuilders::buildViewMenu(*this->Internals->menu_View, *this);

  //// Setup the help menu.
  pqParaViewMenuBuilders::buildHelpMenu(*this->Internals->menu_Help);

  // ParaView Server
  pqServerManagerObserver *server = pqApplicationCore::instance()->getServerManagerObserver();

  //Create ESPINA
  m_selectionManager = SelectionManager::singleton();

  //Create ESPINA views
  m_xy = new SliceView();
  m_xy->setPlane(SliceView::SLICE_PLANE_XY);
  this->setCentralWidget(m_xy);
  connect(server, SIGNAL(connectionCreated(vtkIdType)), m_xy, SLOT(connectToServer()));
  connect(server, SIGNAL(connectionClosed(vtkIdType)), m_xy, SLOT(disconnectFromServer()));
  connect(m_xy, SIGNAL(pointSelected(const Point)), m_selectionManager, SLOT(pointSelected(const Point)));
  m_xy->setModel(sampleProxy);
  m_xy->setRootIndex(sampleProxy->mapFromSource(m_espina->sampleRoot()));

  /*
  m_yz = new SliceWidget(m_planes[SLICE_PLANE_XY]);
  this->Internals->yzSliceDock->setWidget(m_yz);
  connect(server, SIGNAL(connectionCreated(vtkIdType)), m_yz, SLOT(connectToServer()));
  connect(server, SIGNAL(connectionClosed(vtkIdType)), m_yz, SLOT(disconnectFromServer()));
  connect(m_yz, SIGNAL(pointSelected(const Point)), m_selectionManager, SLOT(pointSelected(const Point)));

  m_xz = new SliceWidget(m_planes[SLICE_PLANE_YZ]);
  this->Internals->xzSliceDock->setWidget(m_xz);
  connect(server, SIGNAL(connectionCreated(vtkIdType)), m_xz, SLOT(connectToServer()));
  connect(server, SIGNAL(connectionClosed(vtkIdType)), m_xz, SLOT(disconnectFromServer()));
  */
  
  m_3d = new VolumeView();
  m_3d->setModel(sampleProxy);
  m_3d->setRootIndex(sampleProxy->mapFromSource(m_espina->sampleRoot()));
  this->Internals->volumeDock->setWidget(m_3d);
  connect(server, SIGNAL(connectionCreated(vtkIdType)), m_3d, SLOT(connectToServer()));
  connect(server, SIGNAL(connectionClosed(vtkIdType)), m_3d, SLOT(disconnectFromServer()));

  // m_3d->setSelectionModel(this->Internals->objectTreeView->selectionModel());
  
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

void EspinaMainWindow::loadTrace()
{
  QString filePath = QFileDialog::getOpenFileName(this, tr("Open trace"), "", tr("Trace Files (*.trace)"));
  
  if( !filePath.isEmpty() )
  {
    qDebug() << "Loading Trace: " << filePath;
  }
}

void EspinaMainWindow::loadFile()
{
  // GUI 
  QString filePath = QFileDialog::getOpenFileName(this, tr("Open"), "", 
						  tr("Espina old files (*.mha);;Trace Files (*.trace)"));
  // en cache
  /*
  pqPipelineSource* stack = pqLoadDataReaction::loadData( QStringList(filePath) );
  if( stack )
    emit loadData(stack);
  */
  //EspinaProxy* stack = CachedObjectBuilder::createStack( filePath );
  
  if( !filePath.isEmpty() ){
    qDebug() << "FILEPATH: " << filePath;
    
    EspinaProxy* source = CachedObjectBuilder::instance()->createStack(filePath);
    Product *stack = new Product(source,0, ""); //TO stack
    stack->name = filePath;
    stack->setVisible(false);
    
    ProcessingTrace::instance()->addNode(stack);
    
   
  }
}

//-----------------------------------------------------------------------------
void EspinaMainWindow::loadData(pqPipelineSource *source)
{
  Sample *stack = new Sample(source, 0);
  stack->name = "/home/jorge/Stacks/peque.mha";
  stack->setVisible(false);

  m_espina->addSample(stack);
}

//-----------------------------------------------------------------------------
void EspinaMainWindow::toggleVisibility(bool visible)
{
  this->Internals->toggleVisibility->setIcon(
    visible ? QIcon(":/espina/show_all.svg") : QIcon(":/espina/hide_all.svg")
  );
  m_xy->showSegmentations(visible);
}


//-----------------------------------------------------------------------------
void EspinaMainWindow::buildFileMenu(QMenu &menu)
{
  QIcon icon = qApp->style()->standardIcon(QStyle::SP_DialogOpenButton);

  QAction *action = new QAction(icon,tr("PV Open"),this);
  pqLoadDataReaction * loadReaction = new pqLoadDataReaction(action);
  QObject::connect(loadReaction, SIGNAL(loadedData(pqPipelineSource *)),
		    this, SLOT( loadData(pqPipelineSource *)));
  menu.addAction(action);
    
  /* Load mha */
  action = new QAction(icon,tr("new Open"),this);
  QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT( loadFile()));
  menu.addAction(action);
  
  /* Load Trace */
  action = new QAction(icon,tr("Load trace"),this);
  QObject::connect(action, SIGNAL(triggered()),
		    this, SLOT(loadTrace()));
  menu.addAction(action);
  
  /* Save Trace */
  action = new QAction(qApp->style()->standardIcon(QStyle::SP_DialogSaveButton),
			tr("Save trace"),this);
  menu.addAction(action);
}
