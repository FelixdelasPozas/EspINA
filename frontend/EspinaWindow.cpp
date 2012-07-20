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
#include "docks/TaxonomyExplorer.h"
#include "docks/ModifyFilter/ModifyFilterPanel.h"
#include "toolbar/MainToolBar.h"
#include <model/Channel.h>
#include <model/Taxonomy.h>
#include "undo/AddChannel.h"
#include "undo/AddSample.h"
#include <undo/AddRelation.h>

#include <QtGui>

#include <sstream>

#include "SettingsDialog.h"
#include "docks/ChannelExplorer.h"
#include "docks/DataView/DataViewPanel.h"
#include "toolbar/LODToolBar.h"
#include "views/DefaultEspinaView.h"
#include <model/EspinaFactory.h>
#include <renderers/VolumetricRenderer.h>
#include <renderers/CrosshairRenderer.h>
#include <renderers/MeshRenderer.h>
#include <selection/SelectionManager.h>
#include "toolbar/editor/EditorToolBar.h"
#include "toolbar/seedgrow/SeedGrowSegmentation.h"
#include "toolbar/voi/VolumeOfInterest.h"
#include <IO/FilePack.h>
#include <pluginInterfaces/ToolBarInterface.h>

#define DEBUG

//------------------------------------------------------------------------
EspinaWindow::EspinaWindow()
: m_model(EspinaCore::instance()->model())
, m_busy(false)
, m_undoStack(EspinaCore::instance()->undoStack())
, m_currentActivity("NONE")
, m_view(NULL)
{
#ifdef DEBUG
  m_modelTester = QSharedPointer<ModelTest>(new ModelTest(m_model.data()));
#endif

  QIcon addIcon = QIcon(":espina/add.svg");
  QIcon fileIcon = qApp->style()->standardIcon(QStyle::SP_FileIcon);
  QIcon openIcon = qApp->style()->standardIcon(QStyle::SP_DialogOpenButton);
  QIcon saveIcon = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);

//  EspinaFactory::instance()->registerRenderer(new CrosshairRenderer());
  EspinaFactory::instance()->registerRenderer(new VolumetricRenderer());
  EspinaFactory::instance()->registerRenderer(new MeshRenderer());

  /*** FILE MENU ***/
  QMenu *fileMenu = new QMenu(tr("File"));
  {
    QMenu *openMenu = new QMenu(tr("&Open"));
    {
      openMenu->setIcon(openIcon);
      openMenu->setToolTip(tr("Open New Analysis"));

      QAction *openAction = new QAction(fileIcon, tr("&File"),this);

      openMenu->addAction(openAction);
      openMenu->addSeparator();
      openMenu->addActions(m_recentDocuments.list());

      connect(openMenu, SIGNAL(aboutToShow()),
	      this, SLOT(openState()));
      connect(openAction, SIGNAL(triggered(bool)),
	      this, SLOT(openAnalysis()));
    }

    m_addMenu = new QMenu(tr("&Add"),this);
    {
      m_addMenu->setIcon(addIcon);
      m_addMenu->setToolTip(tr("Add File to Analysis"));
      m_addMenu->setEnabled(false);

      QAction *addAction = new QAction(fileIcon, tr("&File"),this);

      m_addMenu->addAction(addAction);
      m_addMenu->addSeparator();
      m_addMenu->addActions(m_recentDocuments.list());

      connect(m_addMenu, SIGNAL(aboutToShow()),
	      this, SLOT(addState()));
      connect(addAction, SIGNAL(triggered(bool)),
	      this, SLOT(addToAnalysis()));
    }

    QAction *saveAnalysis = new QAction(saveIcon, tr("&Save"),this);
    connect(saveAnalysis,SIGNAL(triggered(bool)),
	    this,SLOT(saveAnalysis()));

    QAction *exit = new QAction(tr("&Exit"), this);
    connect(exit, SIGNAL(triggered(bool)),
	    QApplication::instance(), SLOT(quit()));

    fileMenu->addMenu(openMenu);
    fileMenu->addMenu(m_addMenu);
    fileMenu->addAction(saveAnalysis);
    fileMenu->addAction(exit);
  }
  connect(fileMenu, SIGNAL(triggered(QAction*)),
	  this, SLOT(openRecentAnalysis(QAction*)));
  menuBar()->addMenu(fileMenu);

  /*** EDIT MENU ***/
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

  /*** VIEW MENU ***/
  m_viewMenu = new QMenu(tr("View"));

  menuBar()->addMenu(m_viewMenu);

//   createActivityMenu();
//   createLODMenu();

  /*** Settings MENU ***/
  QMenu *settings = new QMenu(tr("&Settings"));
  {
    QAction *configure = new QAction(tr("&Configure EspINA"), this);
    connect(configure, SIGNAL(triggered(bool)),
	    this, SLOT(showPreferencesDialog()));
    settings->addAction(configure);
  }
  menuBar()->addMenu(settings);

  m_mainToolBar = new MainToolBar(m_model);
//   m_mainToolBar->setMovable(false);
  addToolBar(m_mainToolBar);
  addToolBar(new VolumeOfInterest());
  addToolBar(new SeedGrowSegmentation());
  addToolBar(new EditorToolBar());

  loadPlugins();
//   QToolBar *lod = new LODToolBar();
// //   lod->setMovable(false);
//   addToolBar(lod);

  ChannelExplorer *channelExplorer = new ChannelExplorer(m_model, this);
  addDockWidget(Qt::LeftDockWidgetArea, channelExplorer);

  SegmentationExplorer *segExplorer = new SegmentationExplorer(m_model, this);
  addDockWidget(Qt::LeftDockWidgetArea, segExplorer);

  TaxonomyExplorer *taxExplorer = new TaxonomyExplorer(m_model, this);
  addDockWidget(Qt::LeftDockWidgetArea, taxExplorer);

  ModifyFilterPanel *filterPanel = new ModifyFilterPanel(this);
  addDockWidget(Qt::LeftDockWidgetArea, filterPanel);

  DataViewPanel *dataView = new DataViewPanel(this);
  addDockWidget(Qt::BottomDockWidgetArea, dataView);

  setActivity("segmentate");
//   QSettings settings("CeSViMa", "EspinaModel");
//   restoreGeometry(settings.value("geometry").toByteArray());
//   restoreState(settings.value("state").toByteArray(),0);
  statusBar()->clearMessage();
}

//------------------------------------------------------------------------
EspinaWindow::~EspinaWindow()
{
}

//------------------------------------------------------------------------
void EspinaWindow::loadPlugins()
{
  QDir pluginsDir = QDir(qApp->applicationDirPath());

  #if defined(Q_OS_WIN)
  if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
    pluginsDir.cdUp();
  #elif defined(Q_OS_MAC)
  if (pluginsDir.dirName() == "MacOS")
  {
    pluginsDir.cdUp();
    pluginsDir.cdUp();
    pluginsDir.cdUp();
  }
  #endif

  pluginsDir.cd("plugins");

  qDebug() << "Loading Plugins: ";
  foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
    QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
    QObject *plugin = loader.instance();
    if (plugin)
    {
      qDebug() << " -" << fileName;
      ToolBarInterface *toolbar = qobject_cast<ToolBarInterface*>(plugin);
      if (toolbar)
        addToolBar(toolbar);
    }
  }
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
    QMessageBox warning;
    warning.setWindowTitle(tr("EspINA"));
    warning.setText(tr("EspINA has pending actions. Do you really want to quit anyway?"));
    if (warning.exec() != QMessageBox::Ok)
    {
      event->ignore();
      return;
    }
  }

  if (m_model->hasChanged())
  {
    QMessageBox warning;
    warning.setWindowTitle(tr("EspINA"));
    warning.setText(tr("Current session has not been saved. Do you want to save it now?"));
    warning.setStandardButtons(QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
    int res = warning.exec();
    if (QMessageBox::Yes == res)
    {
      saveAnalysis();
    } else if (QMessageBox::Cancel == res)
    {
      event->ignore();
      return;
    }
  }

  if (m_view)
    m_view->saveLayout();

  event->accept();
//   closeCurrentAnalysis();
//   QSettings settings("CeSViMa", "EspinaModel");

//   settings.setValue(m_currentActivity+"/geometry", saveGeometry());
//   settings.setValue(m_currentActivity+"/state", saveState());
  exit(0);
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
  m_viewMenu->addMenu(EspinaCore::instance()->colorSettings().availableEngines());
  m_viewMenu->addSeparator();

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
    m_view->setColorEngine(EspinaCore::instance()->colorSettings().engine());
    vm->setCurrentView(m_view);
    connect(m_view, SIGNAL(statusMsg(QString)),
	    this, SLOT(updateStatus(QString)));
    m_view->createViewMenu(m_viewMenu);
    m_view->setModel(m_model.data());
  }

  if (m_view)
    connect(m_mainToolBar, SIGNAL(showSegmentations(bool)),
	    m_view, SLOT(showSegmentations(bool)));

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
  SelectionManager::instance()->setSelectionHandler(NULL);
  //SelectionManager::instance()->setVOI(NULL);
}

static const QString CHANNEL_FILES = QObject::tr("Channel Files (*.mha *.mhd *.tif *.tiff)");
static const QString SEG_FILES     = QObject::tr("Espina Analysis (*.seg)");
static const QString ESPINA_FILES  = QObject::tr("Espina Files (*.mha *.mhd *.tif *.tiff *.seg)");

//------------------------------------------------------------------------
void EspinaWindow::openAnalysis()
{
  QStringList filters;
  filters << ESPINA_FILES;
  filters << CHANNEL_FILES;
  filters << SEG_FILES;

  QFileDialog fileDialog(this,
			tr("Start Analysis from:"));
  fileDialog.setObjectName("OpenAnalysisFileDialog");
  fileDialog.setFileMode(QFileDialog::ExistingFiles);
  fileDialog.setFilters(filters);
  fileDialog.setWindowTitle("Analyse Data");
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
  if (fileDialog.exec() != QDialog::Accepted)
    return;

  if (fileDialog.selectedFiles().size() != 1)
  {
    QMessageBox::warning(this,
			tr("EspinaModel"),
			tr("Loading multiple files at a time is not supported"));
    return; //Multi-channels is not supported
  }
  const QString file = fileDialog.selectedFiles().first();
  openAnalysis(file);
  m_model->markAsSaved();
}

//------------------------------------------------------------------------
void EspinaWindow::openAnalysis(const QString file)
{
  QElapsedTimer timer;
  timer.start();

  QFileInfo fileInfo(file);

  QApplication::setOverrideCursor(Qt::WaitCursor);
  closeCurrentAnalysis();
  QApplication::restoreOverrideCursor();

  if (!EspinaCore::instance()->loadFile(file))
  {
    QApplication::setOverrideCursor(Qt::ArrowCursor);
    QMessageBox box(QMessageBox::Warning,
		    tr("Espina"),
	            tr("File %1 could not be loaded.\nDo you want to remove it from recent documents?")
		    .arg(fileInfo.fileName()),
		    QMessageBox::Yes|QMessageBox::No);

    if (box.exec() == QMessageBox::Yes)
      m_recentDocuments.removeDocument(file);
    QApplication::restoreOverrideCursor();
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  if (!m_model->taxonomy())
  {
    Taxonomy *defaultTaxonomy = IOTaxonomy::openXMLTaxonomy(":/espina/defaultTaxonomy.xml");
    //defaultTaxonomy->print();
    m_model->setTaxonomy(defaultTaxonomy);
  }

  m_view->resetCamera();
  m_addMenu->setEnabled(true);

  int secs = timer.elapsed()/1000.0;
  int mins = 0;
  if (secs > 60)
  {
    mins = secs / 60;
    secs = secs % 60;
  }

  updateStatus(QString("File Loaded in %1m%2s").arg(mins).arg(secs));
  QApplication::restoreOverrideCursor();
  m_recentDocuments.addDocument(file);
  setWindowTitle(EspinaCore::instance()->sample()->data(Qt::DisplayRole).toString());
}

//------------------------------------------------------------------------
void EspinaWindow::openRecentAnalysis(QAction *action)
{
  if (action && !action->data().isNull())
  {
    if (OPEN_STATE == m_menuState)
    {
      openAnalysis(action->data().toString());
      m_model->markAsSaved();
    }
    else
      addToAnalysis(action->data().toString());
  }
}

//------------------------------------------------------------------------
void EspinaWindow::addToAnalysis()
{
  QStringList filters;
  filters << ESPINA_FILES;
  filters << CHANNEL_FILES;
  filters << SEG_FILES;

  QFileDialog fileDialog(this,
			tr("Analyse:"));
  fileDialog.setObjectName("AddToAnalysisFileDialog");
  fileDialog.setFileMode(QFileDialog::ExistingFiles);
  fileDialog.setFilters(filters);
  fileDialog.setWindowTitle("Add data to Analysis");
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
  if (fileDialog.exec() != QDialog::Accepted)
    return;

  if (fileDialog.selectedFiles().size() != 1)
  {
    QMessageBox::warning(this,
			 tr("EspinaModel"),
			 tr("Loading multiple files at a time is not supported"));
    return; //Multi-channels is not supported
  }
  const QString file = fileDialog.selectedFiles().first();
  addToAnalysis(file);
}

//------------------------------------------------------------------------
void EspinaWindow::addToAnalysis(const QString file)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QElapsedTimer timer;
  timer.start();

  EspinaCore::instance()->loadFile(file);

  int secs = timer.elapsed()/1000.0;
  int mins = 0;
  if (secs > 60)
  {
    mins = secs / 60;
    secs = secs % 60;
  }

  updateStatus(QString("File Loaded in %1m%2s").arg(mins).arg(secs));
  QApplication::restoreOverrideCursor();
  m_recentDocuments.addDocument(file);
}


//------------------------------------------------------------------------
void EspinaWindow::saveAnalysis()
{
  QString filters(SEG_FILES);

  QFileDialog fileDialog(this, tr("Save Analysis:"), QString(), filters);
  fileDialog.setObjectName("SaveAnalysisFileDialog");
  fileDialog.setWindowTitle("Save Espina Analysis");
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  fileDialog.setDefaultSuffix(QString(tr("seg")));
  fileDialog.setFileMode(QFileDialog::AnyFile);
  fileDialog.selectFile("");

  if (fileDialog.exec() == QDialog::Accepted)
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_busy = true;

    const QString analysisFile = fileDialog.selectedFiles().first();

    IOEspinaFile::saveFile(analysisFile,
                           m_model);

    QApplication::restoreOverrideCursor();
    updateStatus(QString("File Saved Successfuly in %1").arg(analysisFile));
    m_busy = false;

    m_recentDocuments.addDocument(analysisFile);
  }
  m_model->markAsSaved();
}

//------------------------------------------------------------------------
void EspinaWindow::updateStatus(QString msg)
{
  if (msg.isEmpty())
    statusBar()->clearMessage();
  else
    statusBar()->showMessage(msg);
}

//------------------------------------------------------------------------
void EspinaWindow::showPreferencesDialog()
{
  SettingsDialog dialog;

  dialog.addPanel(m_view->settingsPanel());

  foreach(ISettingsPanel *panel, EspinaFactory::instance()->settingsPanels())
  {
    dialog.addPanel(panel);
  }

  dialog.exec();
}
