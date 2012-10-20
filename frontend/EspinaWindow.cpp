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

// EspINA
#include "common/IO/EspinaIO.h"
#include "common/model/EspinaModel.h"
#include "common/model/EspinaFactory.h"
#include "common/gui/IEspinaView.h"
#include "common/gui/ViewManager.h"
#include "common/pluginInterfaces/IToolBar.h"
#include "common/pluginInterfaces/IDockWidget.h"
#include "common/pluginInterfaces/IFactoryExtension.h"
#include "common/pluginInterfaces/IFileReader.h"
#include <pluginInterfaces/IColorEngineProvider.h>
#include "common/renderers/VolumetricRenderer.h"
#include "common/renderers/CrosshairRenderer.h"
#include "common/renderers/MeshRenderer.h"
#include "common/settings/GeneralSettings.h"
#include "common/settings/EspinaSettings.h"
#include "common/colorEngines/TaxonomyColorEngine.h"
#include "common/colorEngines/NumberColorEngine.h"
#include "common/colorEngines/UserColorEngine.h"
#include "frontend/docks/ChannelExplorer.h"
#include "frontend/docks/DataView/DataViewPanel.h"
#include "frontend/docks/SegmentationExplorer.h"
#include "frontend/docks/TaxonomyExplorer.h"
#include "frontend/docks/FilterInspector/FilterInspector.h"
#include "frontend/toolbar/editor/EditorToolBar.h"
#include "frontend/toolbar/main/MainToolBar.h"
#include "frontend/toolbar/seedgrow/SeedGrowSegmentation.h"
#include "frontend/toolbar/voi/VolumeOfInterest.h"
#include "frontend/SettingsDialog.h"
#include "frontend/AboutDialog.h"
#include "frontend/views/DefaultEspinaView.h"
#include "frontend/ColorEngineMenu.h"

#ifdef TEST_ESPINA_MODELS
  #include "common/model/ModelTest.h"
#endif

// Std
#include <sstream>

// Qt
#include <QtGui>

//------------------------------------------------------------------------
EspinaWindow::EspinaWindow()
: m_factory    (new EspinaFactory())
, m_model      (new EspinaModel(m_factory))
, m_undoStack  (new QUndoStack())
, m_viewManager(new ViewManager())
, m_settings   (new GeneralSettings())
, m_busy(false)
, m_view(NULL)
{
#ifdef TEST_ESPINA_MODELS
  m_modelTester = QSharedPointer<ModelTest>(new ModelTest(m_model.data()));
#endif

  m_dynamicMenuRoot = new DynamicMenuNode();
  m_dynamicMenuRoot->menu = NULL;

  connect(m_undoStack, SIGNAL(indexChanged(int)),
          m_viewManager, SLOT(updateViews()));

  QIcon addIcon = QIcon(":espina/add.svg");
  QIcon fileIcon = qApp->style()->standardIcon(QStyle::SP_FileIcon);
  QIcon openIcon = qApp->style()->standardIcon(QStyle::SP_DialogOpenButton);
  QIcon saveIcon = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);

  m_factory->registerRenderer(new CrosshairRenderer());
  m_factory->registerRenderer(new VolumetricRenderer(m_viewManager));
  m_factory->registerRenderer(new MeshRenderer(m_viewManager));

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
      openMenu->addActions(m_recentDocuments1.list());

      for (int i = 0; i < m_recentDocuments1.list().size(); i++)
        connect(m_recentDocuments1.list()[i], SIGNAL(triggered()), this, SLOT(openRecentAnalysis()));

      connect(openMenu, SIGNAL(aboutToShow()), this, SLOT(openState()));
      connect(openMenu, SIGNAL(hovered(QAction*)), this, SLOT(updateTooltip(QAction*)));
      connect(openAction, SIGNAL(triggered(bool)), this, SLOT(openAnalysis()));

    }

    m_addMenu = new QMenu(tr("&Add"),this);
    {
      m_addMenu->setIcon(addIcon);
      m_addMenu->setToolTip(tr("Add File to Analysis"));
      m_addMenu->setEnabled(false);

      QAction *addAction = new QAction(fileIcon, tr("&File"),this);

      m_addMenu->addAction(addAction);
      m_addMenu->addSeparator();
      m_addMenu->addActions(m_recentDocuments2.list());

      for (int i = 0; i < m_recentDocuments2.list().size(); i++)
        connect(m_recentDocuments2.list()[i], SIGNAL(triggered()), this, SLOT(openRecentAnalysis()));

      connect(m_addMenu, SIGNAL(aboutToShow()), this, SLOT(addState()));
      connect(addAction, SIGNAL(triggered(bool)), this, SLOT(addToAnalysis()));
    }

    QAction *saveAnalysis = new QAction(saveIcon, tr("&Save"),this);
    connect(saveAnalysis,SIGNAL(triggered(bool)), this,SLOT(saveAnalysis()));

    QAction *exit = new QAction(tr("&Exit"), this);
    connect(exit, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));

    fileMenu->addMenu(openMenu);
    fileMenu->addMenu(m_addMenu);
    fileMenu->addAction(saveAnalysis);
    fileMenu->addAction(exit);
  }

  connect(fileMenu, SIGNAL(triggered(QAction*)), this, SLOT(openRecentAnalysis()));
  menuBar()->addMenu(fileMenu);

  /*** ANALYSIS MENU ***/
  DynamicMenuNode *subnode = new DynamicMenuNode();
  subnode->menu = menuBar()->addMenu(tr("Analysis"));
  m_dynamicMenuRoot->submenus << subnode;

  /*** EDIT MENU ***/
  QMenu *editMenu = new QMenu(tr("Edit"));
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

  m_colorEngines = new ColorEngineMenu(m_viewManager, tr("Color By"));
  m_colorEngines->addColorEngine(tr("Number"), ColorEnginePtr(new NumberColorEngine()));
  TaxonomyColorEnginePtr taxonomyEngine(new TaxonomyColorEngine());
  m_colorEngines->addColorEngine(tr("Taxonomy"),taxonomyEngine);
  m_colorEngines->addColorEngine(tr("User"), ColorEnginePtr(new UserColorEngine()));

  m_dockMenu = new QMenu(tr("Panels"));

  menuBar()->addMenu(m_viewMenu);
  m_viewMenu->addMenu(m_colorEngines);

  /*** Settings MENU ***/
  QMenu *settingsMenu = new QMenu(tr("&Settings"));
  {
    QAction *configure = new QAction(tr("&Configure EspINA"), this);
    connect(configure, SIGNAL(triggered(bool)),
            this, SLOT(showPreferencesDialog()));
    settingsMenu->addAction(configure);

    QAction *about = new QAction(tr("About"), this);
    connect(about, SIGNAL(triggered(bool)),
            this, SLOT(showAboutDialog()));
    settingsMenu->addAction(about);
  }
  menuBar()->addMenu(settingsMenu);

  m_mainToolBar = new MainToolBar(m_model, m_undoStack, m_viewManager);
//   m_mainToolBar->setMovable(false);
  addToolBar(m_mainToolBar);
  addToolBar(new VolumeOfInterest(m_viewManager));
  addToolBar(new SeedGrowSegmentation(m_model, m_undoStack, m_viewManager));
  addToolBar(new EditorToolBar(m_model, m_undoStack, m_viewManager));

  ChannelExplorer *channelExplorer = new ChannelExplorer(m_model, m_viewManager, this);
  addDockWidget(Qt::LeftDockWidgetArea, channelExplorer);
  m_dockMenu->addAction(channelExplorer->toggleViewAction());

  DataViewPanel *dataView = new DataViewPanel(m_model, m_viewManager, this);
  addDockWidget(Qt::BottomDockWidgetArea, dataView);
  m_dynamicMenuRoot->submenus[0]->menu->addAction(dataView->toggleViewAction());

  FilterInspector *filterInspector = new FilterInspector(m_undoStack, m_viewManager, this);
  addDockWidget(Qt::LeftDockWidgetArea, filterInspector);
  m_dockMenu->addAction(filterInspector->toggleViewAction());

  SegmentationExplorer *segExplorer = new SegmentationExplorer(m_model, m_undoStack, m_viewManager, this);
  addDockWidget(Qt::LeftDockWidgetArea, segExplorer);
  m_dockMenu->addAction(segExplorer->toggleViewAction());

  TaxonomyExplorer *taxExplorer = new TaxonomyExplorer(m_model, m_viewManager, taxonomyEngine, this);
  addDockWidget(Qt::LeftDockWidgetArea, taxExplorer);
  m_dockMenu->addAction(taxExplorer->toggleViewAction());

  loadPlugins();

  m_colorEngines->restoreUserSettings();
  m_viewMenu->addMenu(m_dockMenu);
  m_viewMenu->addSeparator();

  DefaultEspinaView *defaultView = new DefaultEspinaView(m_model, m_viewManager, this);
  /*TODO 2012-10-05 m_view->setColorEngine(EspinaCore::instance()->colorSettings().engine());
  /connect(m_view, SIGNAL(statusMsg(QString)),
          this, SLOT(updateStatus(QString)));
          */
  statusBar()->clearMessage();

  defaultView->createViewMenu(m_viewMenu);
  connect(m_mainToolBar, SIGNAL(showSegmentations(bool)),
          defaultView, SLOT(showSegmentations(bool)));
  m_view = defaultView;

  QSettings settings(CESVIMA, ESPINA);

  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("state").toByteArray());

  m_autosave.setInterval(m_settings->autosaveInterval()*60*1000);
  m_autosave.start();
  connect(&m_autosave, SIGNAL(timeout()),
          this, SLOT(autosave()));
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
  foreach (QString fileName, pluginsDir.entryList(QDir::Files))
  {
    QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
    QObject *plugin = loader.instance();

    if (plugin)
    {
      qDebug() << "Found plugin " << fileName;;
      IFactoryExtension *factoryExtension = qobject_cast<IFactoryExtension *>(plugin);
      if (factoryExtension)
      {
        qDebug() << "- Factory Extension...... OK";
        factoryExtension->initFactoryExtension(m_factory);
      }

      IToolBar *toolbar = qobject_cast<IToolBar *>(plugin);
      if (toolbar)
      {
        qDebug() << "- ToolBar ... OK";
        addToolBar(toolbar);
        toolbar->initToolBar(m_model, m_undoStack, m_viewManager);
        connect(this, SIGNAL(analysisClosed()), toolbar, SLOT(resetState()));
      }

      IDynamicMenu *menu = qobject_cast<IDynamicMenu *>(plugin);
      if (menu)
      {
        qDebug() << "- Menus ..... OK";
        foreach(MenuEntry entry, menu->menuEntries())
          createDynamicMenu(entry);
      }

      IColorEngineProvider *provider = qobject_cast<IColorEngineProvider *>(plugin);
      if (provider)
      {
        qDebug() << "- Color Engine Provider ..... OK";
        foreach(IColorEngineProvider::Engine engine, provider->colorEngines())
          m_colorEngines->addColorEngine(engine.first, engine.second);
      }

      IDockWidget *dock = qobject_cast<IDockWidget *>(plugin);
      if (dock)
      {
        qDebug() << "- Dock ...... OK";
        addDockWidget(Qt::LeftDockWidgetArea, dock);
        m_dockMenu->addAction(dock->toggleViewAction());
        dock->initDockWidget(m_model, m_undoStack, m_viewManager);
        connect(this, SIGNAL(analysisClosed()), dock, SLOT(resetState()));
      }

      IFileReader *fileReader = qobject_cast<IFileReader *>(plugin);
      if (fileReader)
      {
        qDebug() << "- File Reader ...... OK";
        fileReader->initFileReader(m_model, m_undoStack, m_viewManager);
      }
    }
  }
}


//------------------------------------------------------------------------
void EspinaWindow::createActivityMenu()
{
  QSignalMapper *sigMapper = new QSignalMapper(this);

  QMenu *activityMenu = new QMenu(tr("acceptmodeActivity"));
  menuBar()->addMenu(activityMenu);

  QAction *analyse = new QAction(tr("Analyze"),activityMenu);
  activityMenu->addAction(analyse);
  sigMapper->setMapping(analyse,QString("analyze"));
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
void EspinaWindow::createDynamicMenu(MenuEntry entry)
{
  DynamicMenuNode *node = m_dynamicMenuRoot;
  for(int i=0; i<entry.first.size(); i++)
  {
    QString entryName = entry.first[i];

    int index = -1;
    for(int m=0; m<node->submenus.size(); m++)
    {
      if (node->submenus[m]->menu->title() == entryName)
      {
        index = m;
        node = node->submenus[m];
        break;
      }
    }
    if (-1 == index)
    {
      DynamicMenuNode *subnode = new DynamicMenuNode();
      if (node == m_dynamicMenuRoot)
        subnode->menu = menuBar()->addMenu(entryName);
      else
        subnode->menu = node->menu->addMenu(entryName);
      node->submenus << subnode;
      node = subnode;
    }
  }
  node->menu->addAction(entry.second);
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

  QSettings settings(CESVIMA, ESPINA);

  settings.setValue("geometry", saveGeometry());
  settings.setValue("state", saveState());
  settings.sync();
  event->accept();

  m_model->reset();

  exit(0);
}

//------------------------------------------------------------------------
void EspinaWindow::closeCurrentAnalysis()
{
  emit analysisClosed();
  m_viewManager->setActiveChannel(NULL);
  m_viewManager->setActiveTaxonomy(NULL);
  m_viewManager->unsetActiveTool();
  m_model->reset();
  m_undoStack->clear();
}

//------------------------------------------------------------------------
void EspinaWindow::openAnalysis()
{
  QFileDialog fileDialog(this,
                         tr("Start Analysis from:"));

  fileDialog.setObjectName("OpenAnalysisFileDialog");
  fileDialog.setFileMode(QFileDialog::ExistingFiles);
  fileDialog.setFilters(m_factory->supportedFiles());
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

  if (EspinaIO::SUCCESS != EspinaIO::loadFile(file,
                                              m_model,
                                              m_undoStack,
                                              m_settings->autosavePath()))
  {
    QApplication::setOverrideCursor(Qt::ArrowCursor);
    QMessageBox box(QMessageBox::Warning,
                    tr("Espina"),
                    tr("File %1 could not be loaded.\n"
                    "Do you want to remove it from recent documents?")
                    .arg(fileInfo.fileName()),
                    QMessageBox::Yes|QMessageBox::No);

    if (box.exec() == QMessageBox::Yes)
    {
      m_recentDocuments1.removeDocument(file);
      m_recentDocuments2.updateDocumentList();
    }
    closeCurrentAnalysis();
    QApplication::restoreOverrideCursor();
    return;
  }

  if (!m_model->taxonomy())
  {
    Taxonomy *defaultTaxonomy = IOTaxonomy::openXMLTaxonomy(":/espina/defaultTaxonomy.xml");
    //defaultTaxonomy->print();
    m_model->setTaxonomy(defaultTaxonomy);
  }

  m_viewManager->resetViewCameras();
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
  m_recentDocuments1.addDocument(file);
  m_recentDocuments2.updateDocumentList();

  Q_ASSERT(!m_model->channels().isEmpty());
  m_viewManager->setActiveChannel(m_model->channels().first());
  // TODO 2012-10-05 Set proper title
  setWindowTitle(file);
  m_viewManager->updateSegmentationRepresentations();
  m_viewManager->updateViews();
}

//------------------------------------------------------------------------
void EspinaWindow::openRecentAnalysis()
{
  QAction *action = qobject_cast<QAction *>(sender());

  if (action && !action->data().isNull())
  {
    if (OPEN_STATE == m_menuState)
    {
      openAnalysis(action->data().toString());
      m_model->markAsSaved();
    }
    else
      addFileToAnalysis(action->data().toString());
  }
}

//------------------------------------------------------------------------
void EspinaWindow::addToAnalysis()
{
  QFileDialog fileDialog(this, tr("Analyse:"));
  fileDialog.setObjectName("AddToAnalysisFileDialog");
  fileDialog.setFileMode(QFileDialog::ExistingFiles);
  fileDialog.setFilters(m_factory->supportedFiles());
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
  addFileToAnalysis(file);
}

//------------------------------------------------------------------------
void EspinaWindow::addRecentToAnalysis()
{
  QAction *action = qobject_cast<QAction *>(sender());

  if (!action || action->data().isNull())
    return;

  addFileToAnalysis(action->data().toString());
}

//------------------------------------------------------------------------
void EspinaWindow::addFileToAnalysis(const QString file)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QElapsedTimer timer;
  timer.start();

  if (EspinaIO::SUCCESS == EspinaIO::loadFile(file,
                                              m_model,
                                              m_undoStack,
                                              m_settings->autosavePath()))
  {
    int secs = timer.elapsed()/1000.0;
    int mins = 0;
    if (secs > 60)
    {
      mins = secs / 60;
      secs = secs % 60;
    }

    updateStatus(QString("File Loaded in %1m%2s").arg(mins).arg(secs));
    QApplication::restoreOverrideCursor();
    m_recentDocuments1.addDocument(file);
    m_recentDocuments2.updateDocumentList();
  }
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

    EspinaIO::saveFile(analysisFile, m_model);

    QApplication::restoreOverrideCursor();
    updateStatus(QString("File Saved Successfuly in %1").arg(analysisFile));
    m_busy = false;

    m_recentDocuments1.addDocument(analysisFile);
    m_recentDocuments2.updateDocumentList();
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
void EspinaWindow::updateTooltip(QAction* action)
{
  QMenu *menu = dynamic_cast<QMenu *>(sender());
  menu->setToolTip(action->toolTip());
}

//------------------------------------------------------------------------
void EspinaWindow::showPreferencesDialog()
{
  SettingsDialog dialog;

  GeneralSettingsPanel *settingsPanel = new GeneralSettingsPanel(m_settings);
  dialog.addPanel(settingsPanel);
  dialog.addPanel(m_view->settingsPanel());

  foreach(ISettingsPanel *panel, m_factory->settingsPanels())
  {
    dialog.addPanel(panel);
  }

  dialog.exec();
  delete settingsPanel;
}

//------------------------------------------------------------------------
void EspinaWindow::showAboutDialog()
{
  AboutDialog dialog;

  dialog.exec();
}

//------------------------------------------------------------------------
void EspinaWindow::autosave()
{
  if (!m_model->hasChanged())
    return;

  m_busy = true;

  QDir autosavePath = m_settings->autosavePath();
  if (!autosavePath.exists())
    autosavePath.mkpath(autosavePath.absolutePath());

  const QFileInfo analysisFile = autosavePath.absoluteFilePath("espina-autosave.seg");

  EspinaIO::saveFile(analysisFile, m_model);

  updateStatus(QString("Analysis autosaved at %1").arg(QTime::currentTime().toString()));
  m_busy = false;
  m_autosave.setInterval(m_settings->autosaveInterval()*60*1000);
}
