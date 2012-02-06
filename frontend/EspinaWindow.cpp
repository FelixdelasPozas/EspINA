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

#include <gui/EspinaView.h>
#include <model/Sample.h>
#include "common/gui/ViewManager.h"
#include "SegmentationExplorer.h"
#include "TaxonomyInspector.h"
#include "MainToolBar.h"
#include <cache/CachedObjectBuilder.h>
#include <model/Channel.h>
#include <model/Taxonomy.h>
#include "undo/AddChannel.h"
#include "undo/AddSample.h"

#include <QtGui>

#include <pqActiveObjects.h>
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
#include <pqFileDialog.h>
#include <pqFixPathsInStateFilesBehavior.h>
#include <pqCommandLineOptionsBehavior.h>
#include <pqPersistentMainWindowStateBehavior.h>
#include <pqObjectPickingBehavior.h>
#include <pqInterfaceTracker.h>
#include <pqStandardViewModules.h>
#include <pqPipelineSource.h>
#include <pqLoadDataReaction.h>
#include <processing/pqFilter.h>
#include <processing/pqData.h>


//------------------------------------------------------------------------
EspinaWindow::EspinaWindow()
: m_view(NULL)
, m_undoStack(new QUndoStack(this))
, m_currentActivity("NONE")
, m_espina(EspINA::instance())
{
#ifdef DEBUG
  m_modelTester = QSharedPointer<ModelTest>(new ModelTest(m_espina.data()));
#endif

  QMenu *fileMenu = new QMenu("File");
  pqParaViewMenuBuilders::buildFileMenu(*fileMenu);
  QAction *newAnalysis = new QAction(tr("New"),this);
  connect(newAnalysis,SIGNAL(triggered(bool)),
	  this,SLOT(newAnalysis()));
  QAction *openAnalysis = new QAction(tr("Open"),this);
  connect(openAnalysis,SIGNAL(triggered(bool)),
	  this,SLOT(openAnalysis()));
  QAction *addToAnalysis = new QAction(tr("Add"),this);
  addToAnalysis->setEnabled(false);
  fileMenu->addAction(newAnalysis);
  fileMenu->addAction(openAnalysis);
  fileMenu->addAction(addToAnalysis);
  menuBar()->addMenu(fileMenu);

  QMenu *editMenu = new QMenu("Edit");
  QAction *undo = m_undoStack->createUndoAction(editMenu);
  undo->setShortcut(QString("Ctrl+Z"));
  undo->setIcon(QIcon(":espina/edit-undo.svg"));
  editMenu->addAction(undo);
  QAction *redo = m_undoStack->createRedoAction(editMenu);
  redo->setShortcut(QString("Ctrl+Shift+Z"));
  redo->setIcon(QIcon(":espina/edit-redo.svg"));
  editMenu->addAction(redo);
  menuBar()->addMenu(editMenu);

  QMenu *toolsMenu = new QMenu("Tools");
  //NOTE: This method causes maxViewWindowSizeSet connection fail warning
  pqParaViewMenuBuilders::buildToolsMenu(*toolsMenu);
  menuBar()->addMenu(toolsMenu);

  m_viewMenu = new QMenu(tr("View"));
  menuBar()->addMenu(m_viewMenu);
  createActivityMenu();
  createLODMenu();


  pqServerManagerObserver *server = pqApplicationCore::instance()->getServerManagerObserver();
  connect(server,SIGNAL(connectionClosed(vtkIdType)),
	  this,SLOT(onConnect()));
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

  TaxonomyInspector *taxInspector = new TaxonomyInspector(m_espina, this);
  addDockWidget(Qt::LeftDockWidgetArea,taxInspector);


  loadParaviewBehavior();

  setActivity("segmentate");
//   QSettings settings("CeSViMa", "EspINA");
//   
//   restoreGeometry(settings.value("geometry").toByteArray());
//   restoreState(settings.value("state").toByteArray(),0);
//   
  statusBar()->clearMessage();
}

//------------------------------------------------------------------------
EspinaWindow::~EspinaWindow()
{
}

//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
void EspinaWindow::createLODMenu()
{
}


//------------------------------------------------------------------------
void EspinaWindow::closeEvent(QCloseEvent* event)
{
  if (m_view)
    m_view->saveLayout();
//   QSettings settings("CeSViMa", "EspINA");

//   settings.setValue(m_currentActivity+"/geometry", saveGeometry());
//   settings.setValue(m_currentActivity+"/state", saveState());
  QMainWindow::closeEvent(event);
}


//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
void EspinaWindow::onConnect()
{
  m_espina->clear();
  m_undoStack->clear();
//   pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
//   pqServer * server = pqActiveObjects::instance().activeServer();
// 
//   //EspinaView *view = qobject_cast<EspinaView*>(
//     m_view = ob->createView( "EspinaView", server);
// //   m_view = view;
//   setCentralWidget(m_view->getWidget());
//     qDebug() << "Connected";
}

//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
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
    m_view->setModel(m_espina.data());
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

QString extension(const QString path)
{
  return path.section('.',-1);
}

QString fileName(const QString path)
{
  return path.section('/',-1).section('.',0,-2);
}

QString fileNameWithExtension(const QString path)
{
  return path.section('/',-1);
}

//------------------------------------------------------------------------
void EspinaWindow::newAnalysis()
{
  pqServer* server = pqActiveObjects::instance().activeServer();
  QString filters(tr("Channel (*.mha; *.mhd)"));
  pqFileDialog fileDialog(server, 
    this,
    tr("Channel:"), QString(), filters);
  fileDialog.setObjectName("NewAnalysisFileDialog");
  fileDialog.setFileMode(pqFileDialog::ExistingFiles);
  fileDialog.setWindowTitle("Analyse Data");
  if (fileDialog.exec() == QDialog::Accepted)
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_espina->clear();

    Taxonomy *defaultTaxonomy = IOTaxonomy::openXMLTaxonomy(":/espina/defaultTaxonomy.xml");
    m_espina->setTaxonomy(defaultTaxonomy);
//     defaultTaxonomy->print();

    if (fileDialog.getSelectedFiles().size() != 1)
    {
      QMessageBox::warning(this, tr("EspINA"),
			   tr("Loading multiple files at a time is not supported"));
      return; //Multi-channels is not supported
    }

    // TODO: Check for channel sample
    const QString channelFile = fileDialog.getSelectedFiles().first();
    const QString SampleName  = fileName(channelFile);
    const QString channelName = fileNameWithExtension(channelFile);


    m_undoStack->beginMacro("New Analysis");
    QSharedPointer<Sample> sample = QSharedPointer<Sample>(new Sample(SampleName));
//     Sample *sample = new Sample(SampleName);
    m_undoStack->push(new AddSample(m_espina, sample));
    m_undoStack->push(new AddChannel(m_espina, sample, channelFile));

    m_undoStack->endMacro();
    QApplication::restoreOverrideCursor();
  }
}

//------------------------------------------------------------------------
void EspinaWindow::openAnalysis()
{
  m_espina->clear();
  m_undoStack->clear();
//   pqServer* server = pqActiveObjects::instance().activeServer();
//   QString filters(tr("EspINA Segmentation (*.seg)"));
// //   filters += "All files (*)";
//   pqFileDialog fileDialog(server, 
//     this,
//     tr("Open:"), QString(), filters);
//   fileDialog.setObjectName("OpenAnalysisFileDialog");
//   fileDialog.setFileMode(pqFileDialog::ExistingFiles);
//   fileDialog.setWindowTitle("Analyse Data");
//   if (fileDialog.exec() == QDialog::Accepted)
//   {
// //     Internals->dataDock->setHidden(true);
//     this->update();
//     this->repaint();
//     QApplication::setOverrideCursor(Qt::WaitCursor);
// //     m_espina->loadFile(fileDialog.getSelectedFiles()[0], method);
//     QApplication::restoreOverrideCursor();
//   }
}

//------------------------------------------------------------------------
void EspinaWindow::addToAnalysis()
{

}



//------------------------------------------------------------------------
void EspinaWindow::updateStatus(QString msg)
{
  if (msg.isEmpty())
    statusBar()->clearMessage();
  else
    statusBar()->showMessage(msg);
}



