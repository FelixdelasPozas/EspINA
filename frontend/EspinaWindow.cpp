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
#include "EspinaCore.h"
#include "common/model/ModelTest.h"
#include "common/model/EspinaModel.h"

#include <gui/EspinaView.h>
#include <model/Sample.h>
#include "common/gui/ViewManager.h"
#include "docks/SegmentationExplorer.h"
#include "docks/TaxonomyInspector.h"
#include "toolbar/MainToolBar.h"
#include <model/Channel.h>
#include <model/Taxonomy.h>
#include "undo/AddChannel.h"
#include "undo/AddSample.h"
#include <undo/AddRelation.h>

#include <QtGui>

#include <sstream>

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
#include <pqObjectBuilder.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxy.h>
#include <vtkSMStringVectorProperty.h>
#include <pqPipelineFilter.h>
#include "toolbar/LODToolBar.h"
#include "views/DefaultEspinaView.h"
#include "docks/DataViewPanel.h"
#include <model/EspinaFactory.h>
#include <pqServer.h>
#include <vtkSMProxyManager.h>
#include <vtkSMReaderFactory.h>


static const QString CHANNEL_FILES = QObject::tr("Channel (*.mha; *.segmha)");
static const QString SEG_FILES     = QObject::tr("Espina Analysis (*.seg)");

//------------------------------------------------------------------------
EspinaWindow::EspinaWindow()
: m_view(NULL)
, m_busy(false)
, m_model(EspinaCore::instance()->model())
, m_undoStack(EspinaCore::instance()->undoStack())
, m_currentActivity("NONE")
{
#ifdef DEBUG
  m_modelTester = QSharedPointer<ModelTest>(new ModelTest(m_model.data()));
#endif

  QMenu *fileMenu = new QMenu("File");
  pqParaViewMenuBuilders::buildFileMenu(*fileMenu);
  QAction *openAnalysis = new QAction(tr("Open"),this);
  connect(openAnalysis,SIGNAL(triggered(bool)),
	  this,SLOT(openAnalysis()));
  m_addToAnalysis = new QAction(tr("Add"),this);
  m_addToAnalysis->setEnabled(false);
  connect(m_addToAnalysis, SIGNAL(triggered(bool)),
	  this, SLOT(addToAnalysis()));
  QAction *saveAnalysis = new QAction(tr("Save"),this);
  connect(saveAnalysis,SIGNAL(triggered(bool)),
	  this,SLOT(saveAnalysis()));
  fileMenu->addAction(openAnalysis);
  fileMenu->addAction(m_addToAnalysis);
  fileMenu->addAction(saveAnalysis);
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


  m_mainToolBar = new MainToolBar(m_model);
//   m_mainToolBar->setMovable(false);
  addToolBar(m_mainToolBar);

  QToolBar *lod = new LODToolBar();
//   lod->setMovable(false);
  addToolBar(lod);
  

  SegmentationExplorer *segExplorer = new SegmentationExplorer(m_model, this);
  addDockWidget(Qt::LeftDockWidgetArea,segExplorer);

  TaxonomyInspector *taxInspector = new TaxonomyInspector(m_model, this);
  addDockWidget(Qt::LeftDockWidgetArea,taxInspector);

  DataViewPanel *dataView = new DataViewPanel(this);
  addDockWidget(Qt::BottomDockWidgetArea, dataView);

  loadParaviewBehavior();

  setActivity("segmentate");
//   QSettings settings("CeSViMa", "EspinaModel");
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
  if (m_busy)
  {
    QMessageBox::warning(this, tr("EspINA"),
				  tr("EspINA has pending actions. Do you really want to quit anyway?"));
    return;
  }
    
  if (m_view)
    m_view->saveLayout();

  closeCurrentAnalysis();
//   QSettings settings("CeSViMa", "EspinaModel");

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
  m_model->reset();
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
//   QSettings settings("CeSViMa", "EspinaModel");

//   settings.setValue(m_currentActivity+"/geometry", saveGeometry());
//   settings.setValue(m_currentActivity+"/state", saveState());
  if (m_view)
    m_view->saveLayout();

  m_viewMenu->clear();

  QSharedPointer<ViewManager> vm = EspinaCore::instance()->viewManger();
  if (activity == "analyse")
  {
    qDebug() << "Switch to Analyse Activity";
    m_view = vm->createView(this, "squared");
  }
  else if (activity == "segmentate")
  {
    qDebug() << "Switch to Segmentate Activity";
    m_view = new DefaultEspinaView(this);
    vm->setCurrentView(m_view);
    connect(m_view, SIGNAL(statusMsg(QString)),
	    this, SLOT(updateStatus(QString)));
    m_view->createViewMenu(m_viewMenu);
    m_view->setModel(m_model.data());
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

//------------------------------------------------------------------------
void EspinaWindow::closeCurrentAnalysis()
{
  EspinaCore::instance()->closeCurrentAnalysis();
}

//------------------------------------------------------------------------
void EspinaWindow::openAnalysis()
{
  pqServer *server = pqActiveObjects::instance().activeServer();
  vtkSMReaderFactory *readerFactory =
    vtkSMProxyManager::GetProxyManager()->GetReaderFactory();
  QString filters = readerFactory->GetSupportedFileTypes(server->session());
  filters.replace("Meta Image Files", "Channel Files");

  pqFileDialog fileDialog(server,
    this,
    tr("Start Analysis from:"), QString(), filters);
  fileDialog.setObjectName("OpenAnalysisFileDialog");
  fileDialog.setFileMode(pqFileDialog::ExistingFiles);
  fileDialog.setWindowTitle("Analyse Data");
  if (fileDialog.exec() != QDialog::Accepted)
    return;

  if (fileDialog.getSelectedFiles().size() != 1)
  {
    QMessageBox::warning(this, tr("EspinaModel"),
				  tr("Loading multiple files at a time is not supported"));
    return; //Multi-channels is not supported
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  QElapsedTimer timer;
  timer.start();

  closeCurrentAnalysis();

  const QString file = fileDialog.getSelectedFiles().first();

  EspinaCore::instance()->loadFile(file);

  if (m_model->taxonomy().isNull())
  {
    TaxonomyPtr defaultTaxonomy = IOTaxonomy::openXMLTaxonomy(":/espina/defaultTaxonomy.xml");
    m_model->setTaxonomy(defaultTaxonomy);
  }

  m_view->resetCamera();
  m_addToAnalysis->setEnabled(true);
  updateStatus(QString("File Loaded in %1 s").arg(timer.elapsed()/1000.0));
  QApplication::restoreOverrideCursor();
}

//------------------------------------------------------------------------
void EspinaWindow::addToAnalysis()
{
  pqServer *server = pqActiveObjects::instance().activeServer();
  vtkSMReaderFactory *readerFactory =
    vtkSMProxyManager::GetProxyManager()->GetReaderFactory();
  QString filters = readerFactory->GetSupportedFileTypes(server->session());
  filters.replace("Meta Image Files", "Channel Files");

  pqFileDialog fileDialog(server,
    this,
    tr("Analyse:"), QString(), filters);
  fileDialog.setObjectName("AddToAnalysisFileDialog");
  fileDialog.setFileMode(pqFileDialog::ExistingFiles);
  fileDialog.setWindowTitle("Add data to Analysis");
  if (fileDialog.exec() != QDialog::Accepted)
    return;

  if (fileDialog.getSelectedFiles().size() != 1)
  {
    QMessageBox::warning(this,
			 tr("EspinaModel"),
			 tr("Loading multiple files at a time is not supported"));
    return; //Multi-channels is not supported
  }
  
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QElapsedTimer timer;
  timer.start();

  const QString file = fileDialog.getSelectedFiles().first();

  EspinaCore::instance()->loadFile(file);
  updateStatus(QString("File Loaded in %1 s").arg(timer.elapsed()/1000.0));
  QApplication::restoreOverrideCursor();
}

//------------------------------------------------------------------------
void EspinaWindow::saveAnalysis()
{
//   closeCurrentAnalysis();
//   return;
  pqServer* server = pqActiveObjects::instance().activeServer();
  QString filters(SEG_FILES);
  pqFileDialog fileDialog(server, 
    this,
    tr("Save Analysis:"), QString(), filters);
  fileDialog.setObjectName("SaveAnalysisFileDialog");
  fileDialog.setFileMode(pqFileDialog::AnyFile);
  fileDialog.setWindowTitle("Save Espina Analysis");
  if (fileDialog.exec() == QDialog::Accepted)
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_busy = true;

    const QString analysisFile = fileDialog.getSelectedFiles().first();
//     const QStrin analysisName = fileNameWithExtension(analysisFile);
    Q_ASSERT(server);
    pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
    pqPipelineSource* writer = ob->createFilter("filters", "EspinaWriter",
                   QMap<QString, QList< pqOutputPort*> >(),
                   pqApplicationCore::instance()->getActiveServer() );

    // Set the file name
    vtkSMPropertyHelper(writer->getProxy(), "FileName").Set(analysisFile.toStdString().c_str());

    // Set Taxonomy
    QString taxonomySerialization;
    IOTaxonomy::writeXMLTaxonomy(m_model->taxonomy(), taxonomySerialization);
    vtkSMPropertyHelper(writer->getProxy(), "Taxonomy").Set(taxonomySerialization.toStdString().c_str());

    // Set Trace
    std::ostringstream relationsSerialization;
    m_model->serializeRelations(relationsSerialization);

    vtkSMPropertyHelper(writer->getProxy(), "Trace").Set(relationsSerialization.str().c_str());

    // Save the segmentations in different files
    QString filePath = analysisFile;
    filePath.remove(QRegExp("\\..*$"));
    QDir tmpDir(filePath);
    SegmentationPtr seg;
    foreach(seg, m_model->segmentations())
    {
      QString tmpfilePath(seg->volume().source()->id() + ".pvd");
      tmpfilePath = tmpDir.filePath(tmpfilePath);
//       pqActiveObjects::instance().setActivePort(seg->outputPort());
      qDebug() << "EspINA::saveSegementation" << tmpfilePath;
      pqPipelineSource *segWriter = ob->createFilter("writers","XMLPVDWriter", seg->volume().pipelineSource(), seg->volume().portNumber());
      vtkSMPropertyHelper(segWriter->getProxy(), "FileName").Set(tmpfilePath.toStdString().c_str());
      segWriter->getProxy()->UpdateVTKObjects();
      segWriter->updatePipeline();
//       EspinaSaveDataReaction::saveActiveData(tmpfilePath);
    }

    //Update the pipeline to obtain the content of the file
    writer->getProxy()->UpdateVTKObjects();
    writer->updatePipeline();
    // Destroy de segFileWriter object
    ob->destroy(writer);

    QApplication::restoreOverrideCursor();
    updateStatus(QString("File Saved Successfuly in %1").arg(analysisFile));
    m_busy = false;
  }
}

//------------------------------------------------------------------------
void EspinaWindow::updateStatus(QString msg)
{
  if (msg.isEmpty())
    statusBar()->clearMessage();
  else
    statusBar()->showMessage(msg);
}



