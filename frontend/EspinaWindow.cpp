/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "EspinaWindow.h"

// Data Model
#include "common/model/EspINA.h"
#include "common/model/ModelTest.h"

#include "common/gui/ViewManager.h"
#include "SegmentationExplorer.h"

#include <QtGui>

#include <pqApplicationCore.h>
#include <pqParaViewBehaviors.h>
#include <pqParaViewMenuBuilders.h>
#include <pqServerManagerObserver.h>
#include <pqDataTimeStepBehavior.h>
#include <pqAlwaysConnectedBehavior.h>
#include <pqPVNewSourceBehavior.h>
#include <pqDeleteBehavior.h>
#include <pqAutoLoadPluginXMLBehavior.h>
#include <pqPluginDockWidgetsBehavior.h>
#include <pqVerifyRequiredPluginBehavior.h>
#include <pqPluginActionGroupBehavior.h>
#include <pqFixPathsInStateFilesBehavior.h>
#include <pqCommandLineOptionsBehavior.h>
#include <pqPersistentMainWindowStateBehavior.h>
#include <pqObjectPickingBehavior.h>
#include <pqInterfaceTracker.h>
#include <pqStandardViewModules.h>
#include "MainToolBar.h"


EspinaWindow::EspinaWindow()
{
  QSharedPointer<ViewManager> vm = ViewManager::instance();
  setCentralWidget(vm->createLayout());

  QMenu *fileMenu = new QMenu("File");
  menuBar()->addMenu(fileMenu);
  QMenu *toolsMenu = new QMenu("Tools");
  menuBar()->addMenu(toolsMenu);
  
  pqParaViewMenuBuilders::buildFileMenu(*fileMenu);
  //NOTE: This method causes maxViewWindowSizeSet connection fail warning 
  pqParaViewMenuBuilders::buildToolsMenu(*toolsMenu);

//   pqServerManagerObserver *server = pqApplicationCore::instance()->getServerManagerObserver();
//   connect(server,SIGNAL(connectionCreated(vtkIdType)),this,SLOT(onConnect()));
//   QAction *action = new QAction(tr("Open - ParaView mode"),this);
//   action->setShortcut(tr("Ctrl+O"));
//   fileMenu->addAction(action);
//   pqLoadDataReaction * loadReaction = new pqLoadDataReaction(action);
//   QObject::connect(loadReaction, SIGNAL(loadedData(pqPipelineSource *)),
// 		    this, SLOT( loadSource(pqPipelineSource *)));


  m_espina = QSharedPointer<EspINA>(new EspINA());
#ifdef DEBUG
  m_modelTester = QSharedPointer<ModelTest>(new ModelTest(m_espina.data()));
#endif

  QToolBar *mainToolBar = new MainToolBar(m_espina);
  mainToolBar->setMovable(false);
  addToolBar(mainToolBar);
  
  SegmentationExplorer *segExplorer = new SegmentationExplorer(m_espina, this);
  addDockWidget(Qt::LeftDockWidgetArea,segExplorer);

  loadParaviewBehavior();

  statusBar()->clearMessage();
}

EspinaWindow::~EspinaWindow()
{
}

void EspinaWindow::loadParaviewBehavior()
{
  // Register ParaView interfaces.
  pqInterfaceTracker* pgm = pqApplicationCore::instance()->interfaceTracker();

  // * adds support for standard paraview views.
  pgm->addInterface(new pqStandardViewModules(pgm));

  // Load plugins distributed with application.
  pqApplicationCore::instance()->loadDistributedPlugins();

  // Define application behaviors.
  new pqDataTimeStepBehavior(this);
  new pqAlwaysConnectedBehavior(this);
  new pqPVNewSourceBehavior(this);
  new pqDeleteBehavior(this);
  new pqAutoLoadPluginXMLBehavior(this);
  new pqPluginDockWidgetsBehavior(this);
  new pqVerifyRequiredPluginBehavior(this);
  new pqPluginActionGroupBehavior(this);
  new pqFixPathsInStateFilesBehavior(this);//??
  new pqCommandLineOptionsBehavior(this);//Maybe useful
  new pqObjectPickingBehavior(this);//Maybe useful
}

void EspinaWindow::onConnect()
{
//   pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
//   pqServer * server = pqActiveObjects::instance().activeServer();
// 
//   //EspinaView *view = qobject_cast<EspinaView*>(
//     m_view = ob->createView( "EspinaView", server);
// //   m_view = view;
//   setCentralWidget(m_view->getWidget());
//     qDebug() << "Connected";
}

void EspinaWindow::loadSource(pqPipelineSource * source)
{
  
//   pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
//   dp->createPreferredRepresentation(source->getOutputPort(0),pqActiveObjects::instance().activeView(),true);
//   pqView * view=pqActiveObjects::instance().activeView();
//   vtkSMEspinaViewProxy *ep =  vtkSMEspinaViewProxy::SafeDownCast(view->getViewProxy());
//   assert(ep);
//   vtkSMProxy* reprProxy = 0;
//   reprProxy = view->getViewProxy()->CreateDefaultRepresentation(source->getProxy(),0);
//   vtkSMProxy* viewModuleProxy = view->getProxy();
//     // Set the reprProxy's input.
//   pqSMAdaptor::setInputProperty(reprProxy->GetProperty("Input"), source->getProxy(),0);
//   reprProxy->UpdateVTKObjects();
//       // Add the reprProxy to render module.
//   pqSMAdaptor::addProxyProperty(
//     viewModuleProxy->GetProperty("Representations"), reprProxy);
//   viewModuleProxy->UpdateVTKObjects();
//   view->resetDisplay();
//   view->render();

}

void EspinaWindow::openFile()
{
//   m_espina
}

void EspinaWindow::loadFile()
{
}


