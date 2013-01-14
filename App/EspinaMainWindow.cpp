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
#include "Toolbars/Composition/CompositionToolBar.h"
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
#include <Core/Model/Channel.h>
#endif

// Std
#include <sstream>

// Qt
#include <QtGui>

using namespace EspINA;

const QString AUTOSAVE_FILE = "espina-autosave.seg";

//------------------------------------------------------------------------
EspinaMainWindow::DynamicMenuNode::DynamicMenuNode()
: menu(NULL)
{

}

//------------------------------------------------------------------------
EspinaMainWindow::DynamicMenuNode::~DynamicMenuNode()
{
  foreach(DynamicMenuNode *node, submenus)
  {
    delete node;
  }
}

//------------------------------------------------------------------------
EspinaMainWindow::EspinaMainWindow(EspinaModel      *model,
                                   ViewManager      *viewManager,
                                   QList<QObject *> &plugins)
: QMainWindow()
, m_model      (model)
, m_undoStack  (new QUndoStack())
, m_viewManager(viewManager)
, m_settings     (new GeneralSettings())
, m_settingsPanel(new GeneralSettingsPanel(m_settings))
, m_view(NULL)
, m_busy(false)
{
#ifdef TEST_ESPINA_MODELS
  m_modelTester = QSharedPointer<ModelTest>(new ModelTest(m_model));
#endif

  m_dynamicMenuRoot = new DynamicMenuNode();
  m_dynamicMenuRoot->menu = NULL;

  connect(m_undoStack,   SIGNAL(indexChanged(int)),
          m_viewManager, SLOT  (updateViews()));

  QIcon addIcon = QIcon(":espina/add.svg");
  QIcon fileIcon = qApp->style()->standardIcon(QStyle::SP_FileIcon);
  QIcon openIcon = qApp->style()->standardIcon(QStyle::SP_DialogOpenButton);
  QIcon saveIcon = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);

  EspinaFactory *factory = m_model->factory();

  m_defaultRenderers << IRendererSPtr(new CrosshairRenderer());
  m_defaultRenderers << IRendererSPtr(new VolumetricRenderer(m_viewManager));
  m_defaultRenderers << IRendererSPtr(new MeshRenderer(m_viewManager));

  foreach(IRendererSPtr renderer, m_defaultRenderers)
    factory->registerRenderer(renderer.data());

  /*** FILE MENU ***/
  QMenu *fileMenu = new QMenu(tr("File"), this);
  {
    QMenu *openMenu = fileMenu->addMenu(tr("&Open"));
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

    m_addMenu = fileMenu->addMenu(tr("&Add"));
    {
      m_addMenu->setIcon(addIcon);
      m_addMenu->setToolTip(tr("Add File to Analysis"));
      m_addMenu->setEnabled(false);

      QAction *addAction = new QAction(fileIcon, tr("&File"),this);

      m_addMenu->addAction(addAction);
      m_addMenu->addSeparator();
      m_addMenu->addActions(m_recentDocuments2.list());

      for (int i = 0; i < m_recentDocuments2.list().size(); i++)
      {
        connect(m_recentDocuments2.list()[i], SIGNAL(triggered()),
                this, SLOT(openRecentAnalysis()));
      }

      connect(m_addMenu, SIGNAL(aboutToShow()),
              this, SLOT(addState()));
      connect(addAction, SIGNAL(triggered(bool)),
              this, SLOT(addToAnalysis()));
    }

    m_saveAnalysis = fileMenu->addAction(saveIcon, tr("&Save"));
    m_saveAnalysis->setEnabled(false);
    connect(m_saveAnalysis, SIGNAL(triggered(bool)),
            this,SLOT(saveAnalysis()));

    m_closeAnalysis = fileMenu->addAction(tr("&Close"));
    m_closeAnalysis->setEnabled(false);
    connect(m_closeAnalysis, SIGNAL(triggered(bool)),
            this, SLOT(closeCurrentAnalysis()));

    QAction *exit = fileMenu->addAction(tr("&Exit"));
    connect(exit, SIGNAL(triggered(bool)),
            this, SLOT(close()));
  }

  connect(fileMenu, SIGNAL(triggered(QAction*)), this, SLOT(openRecentAnalysis()));
  menuBar()->addMenu(fileMenu);

  /*** ANALYSIS MENU ***/
  DynamicMenuNode *subnode = new DynamicMenuNode();
  subnode->menu = menuBar()->addMenu(tr("Analysis"));
  m_dynamicMenuRoot->submenus << subnode;

  /*** EDIT MENU ***/
  QMenu *editMenu = new QMenu(tr("Edit"), this);
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
  registerToolBar(m_mainToolBar);

  ZoomToolBar *zoomToolBar = new ZoomToolBar(m_viewManager);
  registerToolBar(zoomToolBar);

  VolumeOfInterest *voiToolBar = new VolumeOfInterest(m_model, m_viewManager);
  registerToolBar(voiToolBar);

  SeedGrowSegmentation *seedToolBar = new SeedGrowSegmentation(m_model, m_undoStack, m_viewManager);
  registerToolBar(seedToolBar);

  EditorToolBar *editorToolBar = new EditorToolBar(m_model, m_undoStack, m_viewManager);
  registerToolBar(editorToolBar);

  CompositionToolBar *compositionBar = new CompositionToolBar(m_model, m_undoStack, m_viewManager);
  registerToolBar(compositionBar);

  ChannelExplorer *channelExplorer = new ChannelExplorer(m_model, m_viewManager, this);
  registerDockWidget(Qt::LeftDockWidgetArea, channelExplorer);

  SegmentationExplorer *segExplorer = new SegmentationExplorer(m_model, m_undoStack, m_viewManager, this);
  registerDockWidget(Qt::LeftDockWidgetArea, segExplorer);

  FilterInspector *filterInspector = new FilterInspector(m_undoStack, m_viewManager, this);
  registerDockWidget(Qt::LeftDockWidgetArea, filterInspector);

  DataViewPanel *dataView = new DataViewPanel(m_model, m_viewManager, this);
  registerDockWidget(Qt::BottomDockWidgetArea, dataView);

//   QAction *connectomicsAction = new QAction(tr("Connectomics Information"), this);
//   m_dynamicMenuRoot->submenus[0]->menu->addAction(connectomicsAction);
//   connect(connectomicsAction, SIGNAL(triggered()), this, SLOT(showConnectomicsInformation()));
// 
//   TaxonomyExplorer *taxExplorer = new TaxonomyExplorer(m_model, m_viewManager, taxonomyEngine, this);
//   addDockWidget(Qt::LeftDockWidgetArea, taxExplorer);
//   m_dockMenu->addAction(taxExplorer->toggleViewAction());

  loadPlugins(plugins);

  m_colorEngines->restoreUserSettings();
  m_viewMenu->addMenu(m_dockMenu);
  m_viewMenu->addSeparator();

  DefaultEspinaView *defaultView = new DefaultEspinaView(m_model, m_undoStack, m_viewManager, this);

  statusBar()->clearMessage();

  defaultView->createViewMenu(m_viewMenu);
//   connect(m_mainToolBar, SIGNAL(showSegmentations(bool)),
//           defaultView, SLOT(showSegmentations(bool)));
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
  qDebug() << "********************************************************";
  qDebug() << "              Destroying Main Window";
  qDebug() << "********************************************************";

  foreach(IRendererSPtr renderer, m_defaultRenderers)
    m_model->factory()->unregisterRenderer(renderer.data());

  delete m_settings;
  delete m_undoStack;
  delete m_dynamicMenuRoot;
}

//------------------------------------------------------------------------
void EspinaMainWindow::loadPlugins(QList<QObject *> &plugins)
{
  foreach (QObject *plugin, plugins)
  {
    IFactoryExtension *factoryExtension = qobject_cast<IFactoryExtension *>(plugin);
    if (factoryExtension)
    {
      qDebug() << plugin << "- Factory Extension...... OK";
      factoryExtension->initFactoryExtension(m_model->factory());
    }

    IToolBar *toolbar = qobject_cast<IToolBar *>(plugin);
    if (toolbar)
    {
      qDebug() << plugin << "- ToolBar ... OK";
      toolbar->initToolBar(m_model, m_undoStack, m_viewManager);
      registerToolBar(toolbar);
    }

    IDynamicMenu *menu = qobject_cast<IDynamicMenu *>(plugin);
    if (menu)
    {
      qDebug() << plugin << "- Menus ..... OK";
      foreach(MenuEntry entry, menu->menuEntries())
        createDynamicMenu(entry);
    }

    IColorEngineProvider *provider = qobject_cast<IColorEngineProvider *>(plugin);
    if (provider)
    {
      qDebug() << plugin << "- Color Engine Provider ..... OK";
      foreach(IColorEngineProvider::Engine engine, provider->colorEngines())
        m_colorEngines->addColorEngine(engine.first, engine.second);
    }

    IDockWidget *dock = qobject_cast<IDockWidget *>(plugin);
    if (dock)
    {
      qDebug() << plugin << "- Dock ...... OK";
      registerDockWidget(Qt::LeftDockWidgetArea, dock);
      dock->initDockWidget(m_model, m_undoStack, m_viewManager);
    }

    IFileReader *fileReader = qobject_cast<IFileReader *>(plugin);
    if (fileReader)
    {
      qDebug() << plugin << "- File Reader ...... OK";
      fileReader->initFileReader(m_model, m_undoStack, m_viewManager);
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
void EspinaMainWindow::registerDockWidget(Qt::DockWidgetArea area, IDockWidget* dock)
{
  connect(this, SIGNAL(analysisClosed()),
          dock, SLOT(reset()));

  m_dockMenu->addAction(dock->toggleViewAction());
  addDockWidget(area, dock);
}

//------------------------------------------------------------------------
void EspinaMainWindow::registerToolBar(IToolBar* toolbar)
{
  connect(this, SIGNAL(analysisClosed()),
          toolbar, SLOT(reset()));
  addToolBar(toolbar);
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

  closeCurrentAnalysis();

  QDir autosavePath = m_settings->autosavePath();
  autosavePath.remove(AUTOSAVE_FILE);
}

//------------------------------------------------------------------------
void EspinaMainWindow::closeCurrentAnalysis()
{
  m_viewManager->setActiveChannel(ChannelPtr());
  m_viewManager->setActiveTaxonomy(TaxonomyElementPtr());
  m_viewManager->unsetActiveVOI();
  m_viewManager->unsetActiveTool();
  m_viewManager->clearSelection();
  m_undoStack->clear();
  m_model->reset();

  m_addMenu      ->setEnabled(false);
  m_saveAnalysis ->setEnabled(false);
  m_closeAnalysis->setEnabled(false);

  emit analysisClosed();
}

//------------------------------------------------------------------------
void EspinaMainWindow::openAnalysis()
{
  QFileDialog fileDialog(this,
                         tr("Start Analysis from:"));

  fileDialog.setObjectName("OpenAnalysisFileDialog");
  fileDialog.setFileMode(QFileDialog::ExistingFiles);
  fileDialog.setFilters(m_model->factory()->supportedFiles());
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
void EspinaMainWindow::openAnalysis(const QString &file)
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

  if (m_model->taxonomy().isNull())
  {
    TaxonomySPtr defaultTaxonomy = IOTaxonomy::openXMLTaxonomy(":/espina/defaultTaxonomy.xml");
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

  m_addMenu      ->setEnabled(true);
  m_saveAnalysis ->setEnabled(true);
  m_closeAnalysis->setEnabled(true);

  m_viewManager->setActiveChannel(m_model->channels().first().data());
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
  fileDialog.setFilters(m_model->factory()->supportedFiles());
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
void EspinaMainWindow::addFileToAnalysis(const QString &file)
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

  dialog.registerPanel(m_settingsPanel.data());
  dialog.registerPanel(m_view->settingsPanel());

  foreach(ISettingsPanelPtr panel, m_model->factory()->settingsPanels())
  {
    dialog.registerPanel(panel);
  }

  dialog.exec();
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
