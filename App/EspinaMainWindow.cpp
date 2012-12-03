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

#include "EspinaMainWindow.h"

#include "Dialogs/AboutDialog.h"
#include "Dialogs/SettingsDialog.h"
#include "Dialogs/Connectomics/ConnectomicsDialog.h"
#include "Docks/ChannelExplorer/ChannelExplorer.h"
#include "Docks/FilterInspector/FilterInspector.h"
#include "Docks/SegmentationExplorer/SegmentationExplorer.h"
#include "Docks/TabularReport/DataView.h"
#include "Docks/TabularReport/DataViewPanel.h"
#include "Docks/TaxonomyExplorer/TaxonomyExplorer.h"
#include "Menus/ColorEngineMenu.h"
#include "Settings/GeneralSettings.h"
#include "Toolbars/Editor/EditorToolBar.h"
#include "Toolbars/Main/MainToolBar.h"
#include "Toolbars/SeedGrowSegmentation/SeedGrowSegmentation.h"
#include "Toolbars/VOI/VolumeOfInterest.h"
#include "Toolbars/Zoom/ZoomToolBar.h"
#include "Views/DefaultEspinaView.h"

// EspINA
#include <Core/ColorEngines/NumberColorEngine.h>
#include <Core/ColorEngines/TaxonomyColorEngine.h>
#include <Core/ColorEngines/UserColorEngine.h>
#include <Core/EspinaSettings.h>
#include <Core/IO/EspinaIO.h>
#include <Core/Interfaces/IColorEngineProvider.h>
#include <Core/Interfaces/IDockWidget.h>
#include <Core/Interfaces/IFactoryExtension.h>
#include <Core/Interfaces/IFileReader.h>
#include <Core/Interfaces/IToolBar.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/EspinaModel.h>
#include <GUI/QtWidget/IEspinaView.h>
#include <GUI/Renderers/CrosshairRenderer.h>
#include <GUI/Renderers/MeshRenderer.h>
#include <GUI/Renderers/VolumetricRenderer.h>
#include <GUI/ViewManager.h>


#ifdef TEST_ESPINA_MODELS
  #include <Core/Model/ModelTest.h>
#endif

// Std
#include <sstream>

// Qt
#include <QtGui>

const QString AUTOSAVE_FILE = "espina-autosave.seg";

//------------------------------------------------------------------------
EspinaMainWindow::EspinaMainWindow()
: m_view(NULL)
, m_factory    (new EspinaFactory())
, m_model      (new EspinaModel(m_factory))
, m_undoStack  (new QUndoStack())
, m_viewManager(new ViewManager())
, m_settings   (new GeneralSettings())
, m_busy(false)
{
#ifdef TEST_ESPINA_MODELS
  m_modelTester = QSharedPointer<ModelTest>(new ModelTest(m_model));
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
  addToolBar(m_mainToolBar);
  VolumeOfInterest *voiBar = new VolumeOfInterest(m_model, m_viewManager);
  addToolBar(voiBar);
  SeedGrowSegmentation *seedBar = new SeedGrowSegmentation(m_model, m_undoStack, m_viewManager);
  connect(this, SIGNAL(analysisClosed()), seedBar, SLOT(cancelSegmentationOperation()));
  addToolBar(seedBar);
  EditorToolBar *editorBar = new EditorToolBar(m_model, m_undoStack, m_viewManager);
  connect(this, SIGNAL(analysisClosed()), editorBar, SLOT(resetState()));
  addToolBar(editorBar);
  ZoomToolBar *zoomToolBar = new ZoomToolBar(m_viewManager);
  connect(this, SIGNAL(analysisClosed()), zoomToolBar, SLOT(resetState()));
  addToolBar(zoomToolBar);

  ChannelExplorer *channelExplorer = new ChannelExplorer(m_model, m_viewManager, this);
  addDockWidget(Qt::LeftDockWidgetArea, channelExplorer);
  m_dockMenu->addAction(channelExplorer->toggleViewAction());

  DataViewPanel *dataView = new DataViewPanel(m_model, m_viewManager, this);
  addDockWidget(Qt::BottomDockWidgetArea, dataView);
  m_dynamicMenuRoot->submenus[0]->menu->addAction(dataView->toggleViewAction());

  QAction *connectomicsAction = new QAction(tr("Connectomics Information"), this);
  m_dynamicMenuRoot->submenus[0]->menu->addAction(connectomicsAction);
  connect(connectomicsAction, SIGNAL(triggered()), this, SLOT(showConnectomicsInformation()));

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

  cancel = new QShortcut(Qt::Key_Escape, this, SLOT(cancelOperation()));

  checkAutosave();
}

//------------------------------------------------------------------------
EspinaMainWindow::~EspinaMainWindow()
{
}

//------------------------------------------------------------------------
void EspinaMainWindow::loadPlugins()
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

    // for debuggin plugins
    // qDebug() << loader.errorString();

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
void EspinaMainWindow::createActivityMenu()
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
void EspinaMainWindow::createDynamicMenu(MenuEntry entry)
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
void EspinaMainWindow::createLODMenu()
{
}


//------------------------------------------------------------------------
void EspinaMainWindow::checkAutosave()
{
  QDir autosavePath = m_settings->autosavePath();
  if (autosavePath.exists(AUTOSAVE_FILE))
  {
    QMessageBox info;
    info.setWindowTitle(tr("EspINA"));
    info.setText(tr("Previous working session closed unexpectedly. "
                    "Do you want to load auto-saved session?"));
    info.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    if (QMessageBox::Yes == info.exec())
    {
      openAnalysis(autosavePath.absoluteFilePath(AUTOSAVE_FILE));
    }
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::closeEvent(QCloseEvent* event)
{
  if (m_busy)
  {
    QMessageBox warning;
    warning.setWindowTitle(tr("EspINA"));
    warning.setText(tr("EspINA has pending actions. Do you really want to quit anyway?"));
    if (QMessageBox::Ok != warning.exec())
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

  QDir autosavePath = m_settings->autosavePath();
  autosavePath.remove(AUTOSAVE_FILE);

  exit(0);
}

//------------------------------------------------------------------------
void EspinaMainWindow::closeCurrentAnalysis()
{
  emit analysisClosed();
  m_viewManager->setActiveChannel(NULL);
  m_viewManager->setActiveTaxonomy(NULL);
  m_viewManager->setVOI(NULL);
  m_viewManager->unsetActiveTool();
  m_model->reset();
  m_undoStack->clear();
}

//------------------------------------------------------------------------
void EspinaMainWindow::openAnalysis()
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
void EspinaMainWindow::openAnalysis(const QString file)
{
  QElapsedTimer timer;
  timer.start();

  QFileInfo fileInfo(file);

  EspinaIO::STATUS loaded;
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    closeCurrentAnalysis();

    loaded =  EspinaIO::loadFile(file,
                                 m_model,
                                 m_undoStack,
                                 m_settings->autosavePath());
    QApplication::restoreOverrideCursor();
  }

  if (EspinaIO::SUCCESS != loaded)
  {
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
  if (file != m_settings->autosavePath().absoluteFilePath(AUTOSAVE_FILE))
  {
    m_recentDocuments1.addDocument(file);
    m_recentDocuments2.updateDocumentList();
  }

  Q_ASSERT(!m_model->channels().isEmpty());
  m_viewManager->setActiveChannel(m_model->channels().first());
  setWindowTitle(m_viewManager->activeChannel()->data().toString());
  m_viewManager->updateSegmentationRepresentations();
  m_viewManager->updateViews();
}

//------------------------------------------------------------------------
void EspinaMainWindow::openRecentAnalysis()
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
void EspinaMainWindow::addToAnalysis()
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
void EspinaMainWindow::addRecentToAnalysis()
{
  QAction *action = qobject_cast<QAction *>(sender());

  if (!action || action->data().isNull())
    return;

  addFileToAnalysis(action->data().toString());
}

//------------------------------------------------------------------------
void EspinaMainWindow::addFileToAnalysis(const QString file)
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
void EspinaMainWindow::saveAnalysis()
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

    EspinaIO::saveSegFile(analysisFile, m_model);

    QApplication::restoreOverrideCursor();
    updateStatus(tr("File Saved Successfuly in %1").arg(analysisFile));
    m_busy = false;

    m_recentDocuments1.addDocument(analysisFile);
    m_recentDocuments2.updateDocumentList();
  }
  m_model->markAsSaved();
}

//------------------------------------------------------------------------
void EspinaMainWindow::updateStatus(QString msg)
{
  if (msg.isEmpty())
    statusBar()->clearMessage();
  else
    statusBar()->showMessage(msg);
}

//------------------------------------------------------------------------
void EspinaMainWindow::updateTooltip(QAction* action)
{
  QMenu *menu = dynamic_cast<QMenu *>(sender());
  menu->setToolTip(action->toolTip());
}

//------------------------------------------------------------------------
void EspinaMainWindow::showPreferencesDialog()
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
void EspinaMainWindow::showAboutDialog()
{
  AboutDialog dialog;

  dialog.exec();
}

//------------------------------------------------------------------------
void EspinaMainWindow::autosave()
{
  if (!m_model->hasChanged())
    return;

  m_busy = true;

  QDir autosavePath = m_settings->autosavePath();
  if (!autosavePath.exists())
    autosavePath.mkpath(autosavePath.absolutePath());

  const QFileInfo analysisFile = autosavePath.absoluteFilePath(AUTOSAVE_FILE);

  EspinaIO::saveSegFile(analysisFile, m_model);

  updateStatus(tr("Analysis autosaved at %1").arg(QTime::currentTime().toString()));
  m_busy = false;
  m_autosave.setInterval(m_settings->autosaveInterval()*60*1000);
}

//------------------------------------------------------------------------
void EspinaMainWindow::showConnectomicsInformation()
{
  ConnectomicsDialog *dialog = new ConnectomicsDialog(m_model, m_viewManager, this);
  dialog->exec();
}
