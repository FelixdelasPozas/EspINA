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
#include <gui/EspinaView.h>


EspinaWindow::EspinaWindow()
: m_view(NULL)
, m_currentActivity("NONE")
{
  m_espina = QSharedPointer<EspINA>(new EspINA());
#ifdef DEBUG
  m_modelTester = QSharedPointer<ModelTest>(new ModelTest(m_espina.data()));
#endif

  QMenu *fileMenu = new QMenu("File");
  pqParaViewMenuBuilders::buildFileMenu(*fileMenu);
  menuBar()->addMenu(fileMenu);
  
  QMenu *toolsMenu = new QMenu("Tools");
  //NOTE: This method causes maxViewWindowSizeSet connection fail warning
  pqParaViewMenuBuilders::buildToolsMenu(*toolsMenu);
  menuBar()->addMenu(toolsMenu);
  
  m_viewMenu = new QMenu(tr("View"));
  menuBar()->addMenu(m_viewMenu);
  createActivityMenu();
  createLODMenu();


//   pqServerManagerObserver *server = pqApplicationCore::instance()->getServerManagerObserver();
//   connect(server,SIGNAL(connectionCreated(vtkIdType)),this,SLOT(onConnect()));
//   QAction *action = new QAction(tr("Open - ParaView mode"),this);
//   action->setShortcut(tr("Ctrl+O"));
//   fileMenu->addAction(action);
//   pqLoadDataReaction * loadReaction = new pqLoadDataReaction(action);
//   QObject::connect(loadReaction, SIGNAL(loadedData(pqPipelineSource *)),
// 		    this, SLOT( loadSource(pqPipelineSource *)));


  m_mainToolBar = new MainToolBar(m_espina);
  m_mainToolBar->setMovable(false);
  addToolBar(m_mainToolBar);
  
  SegmentationExplorer *segExplorer = new SegmentationExplorer(m_espina, this);
  addDockWidget(Qt::LeftDockWidgetArea,segExplorer);


  loadParaviewBehavior();

  setActivity("segmentate");
//   QSettings settings("CeSViMa", "EspINA");
//   
//   restoreGeometry(settings.value("geometry").toByteArray());
//   restoreState(settings.value("state").toByteArray(),0);
//   
  statusBar()->clearMessage();
}

EspinaWindow::~EspinaWindow()
{
}

void EspinaWindow::createActivityMenu()
{
  QSignalMapper *sigMapper = new QSignalMapper(this);

  QMenu *activityMenu = new QMenu(tr("Activity"));
  menuBar()->addMenu(activityMenu);

  QAction *analyse = new QAction(tr("Analyse"),activityMenu);
  activityMenu->addAction(analyse);
  sigMapper->setMapping(analyse,QString("analyse"));
  connect(analyse,SIGNAL(triggered(bool)), sigMapper, SLOT(map()));
  
  QAction *reload = new QAction(tr("Reload"),activityMenu);
  activityMenu->addAction(reload);
  sigMapper->setMapping(reload,QString("Reload"));
  connect(reload,SIGNAL(triggered(bool)), sigMapper, SLOT(map()));

  QAction *segmentate = new QAction(tr("Segmentate"),activityMenu);
  activityMenu->addAction(segmentate);
  sigMapper->setMapping(segmentate,QString("segmentate"));
  connect(segmentate,SIGNAL(triggered(bool)), sigMapper, SLOT(map()));

  connect(sigMapper,SIGNAL(mapped(QString)),this, SLOT(setActivity(QString)));
}

void EspinaWindow::createLODMenu()
{
}


void EspinaWindow::closeEvent(QCloseEvent* event)
{
  if (m_view)
    m_view->saveLayout();
//   QSettings settings("CeSViMa", "EspINA");

//   settings.setValue(m_currentActivity+"/geometry", saveGeometry());
//   settings.setValue(m_currentActivity+"/state", saveState());
  QMainWindow::closeEvent(event);
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
  // Crashes while loading pqPipelineSource after a re-connecting
  // the server and loading the same source
  // new pqPVNewSourceBehavior(this);
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

void EspinaWindow::setActivity(QString activity)
{
  if (activity == m_currentActivity)
    return;
  // Changing the central widget desrtoys the previous one
//   QSettings settings("CeSViMa", "EspINA");

//   settings.setValue(m_currentActivity+"/geometry", saveGeometry());
//   settings.setValue(m_currentActivity+"/state", saveState());
  if (m_view)
    m_view->saveLayout();

  m_viewMenu->clear();

  if (activity == "analyse")
  {
    qDebug() << "Switch to Analyse Activity";
    QSharedPointer<ViewManager> vm = ViewManager::instance();
    m_view = vm->createLayout(this, "squared");
  }
  else if (activity == "segmentate")
  {
    qDebug() << "Switch to Segmentate Activity";
    QSharedPointer<ViewManager> vm = ViewManager::instance();
    m_view = vm->createLayout(this);
    connect(m_view, SIGNAL(statusMsg(QString)),
	    this, SLOT(updateStatus(QString)));
    m_view->createViewMenu(m_viewMenu);
  }

  if (m_view)
    connect(m_mainToolBar, SIGNAL(showSegmentations(bool)),
	    m_view, SLOT(setShowSegmentations(bool)));

  if (m_view)
  {
    m_view->restoreLayout();
  }

  
  m_currentActivity = activity;

//   restoreGeometry(settings.value(m_currentActivity+"/geometry").toByteArray());
//   restoreState(settings.value(m_currentActivity+"/state").toByteArray());
}


void EspinaWindow::openFile()
{
//   m_espina
}

void EspinaWindow::loadFile()
{
}

void EspinaWindow::updateStatus(QString msg)
{
  if (msg.isEmpty())
    statusBar()->clearMessage();
  else
    statusBar()->showMessage(msg);
}



