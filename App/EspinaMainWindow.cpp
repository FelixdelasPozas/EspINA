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

// EspINA
#include "Dialogs/About/AboutDialog.h"
#include "Dialogs/AdaptiveEdges/AdaptiveEdgesDialog.h"
#include "Dialogs/Settings/GeneralSettingsDialog.h"
#include "Dialogs/TabularReport/RawInformationDialog.h"
#include "Docks/ChannelExplorer/ChannelExplorer.h"
#include "Docks/SegmentationExplorer/SegmentationExplorer.h"
#include "IO/ChannelReader.h"
#include "IO/SegFileReader.h"
#include "Menus/ColorEngineMenu.h"
#include "ToolGroups/Editor/EditionTools.h"
#include "ToolGroups/Segmentation/SegmentationTools.h"
#include "ToolGroups/ViewState/ViewTools.h"
#include "Settings/GeneralSettings/GeneralSettingsPanel.h"
#include <Core/IO/ClassificationXML.h>
#include <Core/IO/SegFile.h>
#include <Core/MultiTasking/Scheduler.h>
#include <Core/Utils/AnalysisUtils.h>
#include <Extensions/EdgeDistances/AdaptiveEdges.h>
#include <GUI/ColorEngines/CategoryColorEngine.h>
#include <GUI/ColorEngines/NumberColorEngine.h>
#include <GUI/ColorEngines/UserColorEngine.h>
#include <GUI/Model/Utils/ModelAdapterUtils.h>
#include <GUI/Representations/BasicRepresentationFactory.h>
#include <GUI/Representations/Renderers/CrosshairRenderer.h>
#include <GUI/Representations/Renderers/ContourRenderer.h>
#include <GUI/Representations/Renderers/MeshRenderer.h>
#include <GUI/Representations/Renderers/SliceRenderer.h>
#include <GUI/Representations/Renderers/SmoothedMeshRenderer.h>
#include <GUI/Representations/Renderers/VolumetricGPURenderer.h>
#include <GUI/Representations/Renderers/VolumetricRenderer.h>
#include <Support/Plugin.h>
#include <Support/Settings/EspinaSettings.h>
#include <Support/Utils/FactoryUtils.h>

// Std
#include <sstream>

// Qt
#include <QtGui>

using namespace EspINA;

const QString AUTOSAVE_FILE = "espina-autosave.seg";

//------------------------------------------------------------------------
EspinaMainWindow::DynamicMenuNode::DynamicMenuNode()
: menu(nullptr)
{

}

//------------------------------------------------------------------------
EspinaMainWindow::DynamicMenuNode::~DynamicMenuNode()
{
  for(DynamicMenuNode *node : submenus)
  {
    delete node;
  }
}

const int PERIOD_NS = 1000000;

//------------------------------------------------------------------------
EspinaMainWindow::EspinaMainWindow(QList< QObject* >& plugins)
: QMainWindow()
, m_scheduler(new Scheduler(PERIOD_NS))
, m_factory(new ModelFactory(espinaCoreFactory(m_scheduler)))
, m_analysis(new Analysis())
, m_model(new ModelAdapter())
, m_viewManager(new ViewManager())
, m_undoStack(new QUndoStack())
//, m_filterFactory(new EspinaMainWindow::FilterFactory())
, m_channelReader{new ChannelReader()}
, m_segFileReader{new SegFileReader()}
, m_settings     (new GeneralSettings())
, m_schedulerProgress(new SchedulerProgress(m_scheduler, this))
, m_busy(false)
, m_undoStackSavedIndex(0)
, m_errorHandler(new EspinaErrorHandler(this))
{
  m_dynamicMenuRoot = new DynamicMenuNode();
  m_dynamicMenuRoot->menu = nullptr;

  connect(m_undoStack,         SIGNAL(indexChanged(int)),
          m_viewManager.get(), SLOT  (updateViews()));

  QIcon addIcon = QIcon(":espina/add.svg");
  QIcon fileIcon = qApp->style()->standardIcon(QStyle::SP_FileIcon);
  QIcon openIcon = qApp->style()->standardIcon(QStyle::SP_DialogOpenButton);
  QIcon saveIcon = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);

  m_factory->registerAnalysisReader(m_channelReader.get());
  m_factory->registerAnalysisReader(m_segFileReader.get());
  m_factory->registerFilterFactory (m_channelReader);
  m_factory->registerChannelRepresentationFactory(RepresentationFactorySPtr{new BasicChannelRepresentationFactory()});
  m_factory->registerSegmentationRepresentationFactory(RepresentationFactorySPtr{new BasicSegmentationRepresentationFactory()});

  m_availableRenderers << RendererSPtr(new CrosshairRenderer());
  m_availableRenderers << RendererSPtr(new MeshRenderer());
  m_availableRenderers << RendererSPtr(new SmoothedMeshRenderer());
  m_availableRenderers << RendererSPtr(new SliceRenderer());
  m_availableRenderers << RendererSPtr(new VolumetricRenderer<itkVolumeType>());
  m_availableRenderers << RendererSPtr(new VolumetricGPURenderer<itkVolumeType>());
  m_availableRenderers << RendererSPtr(new ContourRenderer());

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

    m_saveSessionAnalysis = fileMenu->addAction(saveIcon, tr("&Save"));
    m_saveSessionAnalysis->setEnabled(false);
    m_saveSessionAnalysis->setShortcut(Qt::CTRL+Qt::Key_S);
    connect(m_saveSessionAnalysis, SIGNAL(triggered(bool)),
            this,SLOT(saveSessionAnalysis()));

    m_saveAnalysis = fileMenu->addAction(saveIcon, tr("Save &As..."));
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
  subnode->menu->setEnabled(false);
  m_dynamicMenuRoot->submenus << subnode;

  /*** EDIT MENU ***/
  QMenu *editMenu = new QMenu(tr("Edit"), this);
  m_undoAction = new QAction(QIcon(":espina/edit-undo.svg"), tr("Undo"), this);
  m_undoAction->setEnabled(false);
  m_undoAction->setCheckable(false);
  m_undoAction->setShortcut(QString("Ctrl+Z"));
  editMenu->addAction(m_undoAction);

  connect(m_undoAction, SIGNAL(triggered(bool)), this, SLOT(undoAction(bool)));

  m_redoAction = new QAction(QIcon(":espina/edit-redo.svg"), tr("Redo"), this);
  m_redoAction->setEnabled(false);
  m_redoAction->setCheckable(false);
  m_redoAction->setShortcut(QString("Ctrl+Shift+Z"));
  editMenu->addAction(m_redoAction);

  connect(m_redoAction, SIGNAL(triggered(bool)), this, SLOT(redoAction(bool)));

  menuBar()->addMenu(editMenu);

  // undo connection with menu actions
  connect(m_undoStack, SIGNAL(canRedoChanged(bool)), this, SLOT(canRedoChanged(bool)));
  connect(m_undoStack, SIGNAL(canUndoChanged(bool)), this, SLOT(canUndoChanged(bool)));
  connect(m_undoStack, SIGNAL(redoTextChanged(QString)), this, SLOT(redoTextChanged(QString)));
  connect(m_undoStack, SIGNAL(undoTextChanged(QString)), this, SLOT(undoTextChanged(QString)));

  /*** VIEW MENU ***/
  m_viewMenu = new QMenu(tr("View"));

  m_colorEngines = new ColorEngineMenu(m_viewManager, tr("Color By"));
  m_colorEngines->addColorEngine(tr("Number"),  ColorEngineSPtr{new NumberColorEngine()});
  m_colorEngines->addColorEngine(tr("Category"),ColorEngineSPtr{new CategoryColorEngine()});
  m_colorEngines->addColorEngine(tr("User"),    ColorEngineSPtr{new UserColorEngine()});

  m_dockMenu = new QMenu(tr("Pannels"));

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

  m_mainBar = addToolBar("Main ToolBar");
  m_mainBar->setMovable(false);
  m_mainBar->setObjectName("Main ToolBar");
  addToolBarBreak();
  m_contextualBar = addToolBar("Contextual ToolBar");
  m_contextualBar->setMovable(true);
  m_contextualBar->setObjectName("Contextual ToolBar");
  m_contextualBar->setMinimumHeight(44);
  m_contextualBar->setMaximumHeight(44);
  m_viewManager->setContextualBar(m_contextualBar);

  auto defaultActiveTool = new ViewTools(m_viewManager, this);
  registerToolGroup(defaultActiveTool);
// 
//   VolumeOfInterest *voiToolBar = new VolumeOfInterest(m_model, m_viewManager);
//   registerToolBar(voiToolBar);
// 
  auto segmentationTools = new SegmentationTools(m_model, m_factory, m_viewManager, m_undoStack, this);
  registerToolGroup(segmentationTools);

  auto editionTools = new EditionTools(m_model, m_factory, m_viewManager, m_undoStack, this);
  registerToolGroup(editionTools);
// 
//   CompositionToolBar *compositionBar = new CompositionToolBar(m_model, m_undoStack, m_viewManager);
//   registerToolBar(compositionBar);
// 
  auto channelExplorer = new ChannelExplorer(m_model, m_viewManager, m_scheduler, m_undoStack, this);
  registerDockWidget(Qt::LeftDockWidgetArea, channelExplorer);

  auto segmentationExplorer = new SegmentationExplorer(m_model, m_viewManager, m_undoStack, this);
  m_viewManager->registerView(segmentationExplorer);
  registerDockWidget(Qt::LeftDockWidgetArea, segmentationExplorer);
// 
//   FilterInspectorDock *filterInspectorDock = new FilterInspectorDock(m_undoStack, m_viewManager, this);
//   registerDockWidget(Qt::LeftDockWidgetArea, filterInspectorDock);

//   QAction *connectomicsAction = new QAction(tr("Connectomics Information"), this);
//   m_dynamicMenuRoot->submenus[0]->menu->addAction(connectomicsAction);
//   connect(connectomicsAction, SIGNAL(triggered()), this, SLOT(showConnectomicsInformation()));

  defaultActiveTool->showTools(true);

  QAction *rawInformationAction = m_dynamicMenuRoot->submenus[0]->menu->addAction(tr("Raw Information"));
  connect(rawInformationAction, SIGNAL(triggered(bool)),
          this, SLOT(showRawInformation()));

  loadPlugins(plugins);

  m_colorEngines->restoreUserSettings();
  m_viewMenu->addMenu(m_dockMenu);
  m_viewMenu->addSeparator();

  statusBar()->clearMessage();

  m_view = DefaultViewSPtr{new DefaultView(m_model, m_viewManager, m_undoStack, m_availableRenderers, this)};
  m_view->createViewMenu(m_viewMenu);

  QSettings settings(CESVIMA, ESPINA);
  /**
   * Instead of ussing save/restoreGeometry resice+move
   * Works better in Unity when espina is closed while is maximized
   */
  settings.beginGroup("MainWindow");
  resize(settings.value("size", QSize (800, 600)).toSize());
  move  (settings.value("pos",  QPoint(200, 200)).toPoint());
  settings.endGroup();
  restoreState(settings.value("state").toByteArray());

  m_autosave.setInterval(m_settings->autosaveInterval()*60*1000);
  m_autosave.start();
  connect(&m_autosave, SIGNAL(timeout()),
          this, SLOT(autosave()));

  cancel = new QShortcut(Qt::Key_Escape, this, SLOT(cancelOperation()));

  setWindowTitle(QString("EspINA Interactive Neuron Analyzer"));

  checkAutosave();

  closeCurrentAnalysis();

  statusBar()->addPermanentWidget(m_schedulerProgress.get());
}

//------------------------------------------------------------------------
EspinaMainWindow::~EspinaMainWindow()
{
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying Main Window";
//   qDebug() << "********************************************************";

//   foreach(IRendererSPtr renderer, m_defaultRenderers)
//     m_model->factory()->unregisterRenderer(renderer.get());
  delete m_undoStack;
  delete m_dynamicMenuRoot;
}

//------------------------------------------------------------------------
void EspinaMainWindow::loadPlugins(QList<QObject *> &plugins)
{
  for(QObject *plugin : plugins)
  {
    Plugin *validPlugin = qobject_cast<Plugin*>(plugin);
    if (validPlugin)
    {
      validPlugin->init(m_model, m_viewManager, m_undoStack);

      for (auto colorEngine : validPlugin->colorEngines())
      {
        m_colorEngines->addColorEngine(colorEngine.first,  colorEngine.second);
      }

      for (auto dock : validPlugin->dockWidgets())
      {
        qDebug() << plugin << "- Dock " << dock->windowTitle() << " ...... OK";
        registerDockWidget(Qt::LeftDockWidgetArea, dock);
      }

      for (auto settings : validPlugin->settingsPanels())
      {
        qDebug() << plugin << "- Settings Panel " << settings->windowTitle() << " ...... OK";
        m_availableSettingsPanels << settings;
      }

      for (auto renderer : validPlugin->renderers())
      {
        qDebug() << plugin << "- Renderers " << renderer->name() << " ...... OK";
        m_availableRenderers << renderer;
      }
    }
//     IFactoryExtension *factoryExtension = qobject_cast<IFactoryExtension *>(plugin);
//     if (factoryExtension)
//     {
//       qDebug() << plugin << "- Factory Extension...... OK";
//       factoryExtension->initFactoryExtension(m_model->factory());
//     }
// 
//     IToolBarFactory *toolbarFactory = qobject_cast<IToolBarFactory *>(plugin);
//     if (toolbarFactory)
//     {
//       qDebug() << plugin << "- ToolBar ... OK";
//       toolbarFactory->initToolBarFactory(m_model, m_undoStack, m_viewManager);
//       foreach(IToolBar *toolbar, toolbarFactory->toolBars())
//       {
//         registerToolBar(toolbar);
//       }
//     }

//     DynamicMenu *menu = qobject_cast<DynamicMenu *>(plugin);
//     if (menu)
//     {
//       qDebug() << plugin << "- Menus ..... OK";
//       foreach(MenuEntry entry, menu->menuEntries())
//         createDynamicMenu(entry);
//     }

//     IColorEngineProvider *provider = qobject_cast<IColorEngineProvider *>(plugin);
//     if (provider)
//     {
//       qDebug() << plugin << "- Color Engine Provider ..... OK";
//       foreach(IColorEngineProvider::Engine engine, provider->colorEngines())
//         m_colorEngines->addColorEngine(engine.first, engine.second);
//     }


//     IFileReader *fileReader = qobject_cast<IFileReader *>(plugin);
//     if (fileReader)
//     {
//       qDebug() << plugin << "- File Reader ...... OK";
//       fileReader->initFileReader(m_model, m_undoStack, m_viewManager);
//     }
  }
}

//------------------------------------------------------------------------
bool EspinaMainWindow::isModelModified()
{
  return m_undoStack->index() != m_undoStackSavedIndex;
}

//------------------------------------------------------------------------
void EspinaMainWindow::createActivityMenu()
{
  QSignalMapper *sigMapper = new QSignalMapper(this);

  QMenu *activityMenu = new QMenu(tr("acceptmodeActivity"));
  menuBar()->addMenu(activityMenu);

  QAction *analyze = new QAction(tr("Analyze"),activityMenu);
  activityMenu->addAction(analyze);
  sigMapper->setMapping(analyze,QString("analyze"));
  connect(analyze,SIGNAL(triggered(bool)), sigMapper, SLOT(map()));

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
    info.setText(tr("EspINA closed unexpectedly. "
                    "Do you want to load autosave file?"));
    info.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    if (QMessageBox::Yes == info.exec())
    {
      QStringList files;
      files << autosavePath.absoluteFilePath(AUTOSAVE_FILE);
      openAnalysis(files);
    }
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::registerDockWidget(Qt::DockWidgetArea area, DockWidget* dock)
{
  connect(this, SIGNAL(analysisClosed()),
          dock, SLOT(reset()));

  m_dockMenu->addAction(dock->toggleViewAction());
  addDockWidget(area, dock);
}

//------------------------------------------------------------------------
void EspinaMainWindow::registerToolGroup(ToolGroupPtr tools)
{
//   connect(this,  SIGNAL(analysisClosed()),
//           tools, SLOT(resetToolbar()));
//   connect(this,  SIGNAL(abortOperation()),
//           tools, SLOT(abortOperation()));
  m_mainBar->addAction(tools);
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

  if (closeCurrentAnalysis())
  {
    QSettings settings(CESVIMA, ESPINA);

    /**
     * Instead of ussing save/restoreGeometry resice+move
     * Works better in Unity when espina is closed while is maximized
     */
    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();

    settings.setValue("state", saveState());
    settings.sync();
    event->accept();

    QDir autosavePath = m_settings->autosavePath();
    autosavePath.remove(AUTOSAVE_FILE);
  }
}

//------------------------------------------------------------------------
bool EspinaMainWindow::closeCurrentAnalysis()
{
  emit analysisClosed();

  if (isModelModified())
  {
    QMessageBox warning;
    warning.setWindowTitle(tr("EspINA"));
    warning.setText(tr("Current session has not been saved. Do you want to save it now?"));
    warning.setStandardButtons(QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
    int res = warning.exec();

    switch(res)
    {
      case QMessageBox::Yes:
        saveAnalysis();
        break;
      case QMessageBox::Cancel:
        return false;
        break;
      default:
        break;
    }
  }

  m_viewManager->setActiveChannel(ChannelAdapterPtr());
  m_viewManager->setActiveCategory(CategoryAdapterPtr());
  //m_viewManager->hideTools();
  m_viewManager->selection()->clear();
  m_undoStack->clear();
  m_undoStackSavedIndex = m_undoStack->index();

  m_model  ->reset();
  m_analysis.reset();

  // resets slice views matrices to avoid an esthetic bug
  NmVector3 origin;
  m_viewManager->focusViewsOn(origin);

  m_addMenu            ->setEnabled(false);
  m_saveAnalysis       ->setEnabled(false);
  m_saveSessionAnalysis->setEnabled(false);
  m_closeAnalysis      ->setEnabled(false);

  m_sessionFile = QFileInfo();

  setWindowTitle(QString("EspINA Interactive Neuron Analyzer"));

  m_mainBar->setEnabled(false);
  m_contextualBar->setEnabled(false);
  m_dynamicMenuRoot->submenus.first()->menu->setEnabled(false);

  return true;
}

//------------------------------------------------------------------------
void EspinaMainWindow::openAnalysis()
{
  const QString title   = tr("Start New Analysis From File");
  const QString filters = m_factory->supportedFileExtensions().join(";;");

  QList<QUrl> urls;
  urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation))
       << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation))
       << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation))
       << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DataLocation));

  QFileDialog fileDialog(this);
  fileDialog.setFileMode(QFileDialog::ExistingFiles);
  fileDialog.setWindowTitle(title);
  fileDialog.setFilter(filters);
  fileDialog.setDirectory(QDir());
  fileDialog.setOption(QFileDialog::DontUseNativeDialog, false);
  fileDialog.setViewMode(QFileDialog::Detail);
  fileDialog.resize(800, 480);
  fileDialog.setSidebarUrls(urls);
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);

  if (fileDialog.exec() == QDialog::Accepted)
  {
    openAnalysis(fileDialog.selectedFiles());
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::openAnalysis(const QStringList files)
{
  QElapsedTimer timer;
  timer.start();

  closeCurrentAnalysis();

  AnalysisSPtr mergedAnalysis = loadedAnalysis(files);

  if (mergedAnalysis) 
  {
    if (!mergedAnalysis->classification())
    {
      QFileInfo defaultClassification(":/espina/defaultTaxonomy.xml");
      auto classification = IO::ClassificationXML::load(defaultClassification);
      mergedAnalysis->setClassification(classification);
    }

    m_model->setAnalysis(mergedAnalysis, m_factory);
    m_analysis = mergedAnalysis;

    m_viewManager->resetViewCameras();
    m_viewManager->updateViews();
    m_addMenu->setEnabled(true);

    int secs = timer.elapsed()/1000.0;
    int mins = 0;
    if (secs > 60)
    {
      mins = secs / 60;
      secs = secs % 60;
    }

    m_dynamicMenuRoot->submenus.first()->menu->setEnabled(true);

    updateStatus(QString("File Loaded in %1m%2s").arg(mins).arg(secs));

    m_addMenu      ->setEnabled(true);
    m_saveAnalysis ->setEnabled(true);
    m_closeAnalysis->setEnabled(true);

    if (!m_model->channels().isEmpty())
    {
      auto activeChannel = m_model->channels().first().get();

      m_viewManager->setActiveChannel(activeChannel);

      if (m_viewManager->activeCategory() == nullptr)
      {
        m_viewManager->setActiveCategory(m_model->classification()->categories().first().get());
      }

      for (auto channel : m_model->channels())
      {
        if (!channel->hasExtension(AdaptiveEdges::TYPE))
        {
          AdaptiveEdgesDialog edgesDialog(this);
          edgesDialog.exec();

          int color     = edgesDialog.color();
          int threshold = edgesDialog.threshold();

          AdaptiveEdgesSPtr extension{new AdaptiveEdges(edgesDialog.useAdaptiveEdges(), color, threshold, m_scheduler)};
          channel->addExtension(extension);
        }
      }
    }

    setWindowTitle(files.first());

    m_sessionFile = files.first();
    bool enableSave = files.size() == 1 && m_sessionFile.suffix().toLower() == QString("seg");
    m_saveSessionAnalysis->setEnabled(enableSave);

    m_mainBar->setEnabled(true);
    m_contextualBar->setEnabled(true);

    m_viewManager->updateSegmentationRepresentations();
    m_viewManager->updateViews();

    m_model->emitChannelAdded(m_model->channels());
    m_model->emitSegmentationAdded(m_model->segmentations());

    if (!m_model->channels().isEmpty())
    {
      m_viewManager->setActiveChannel(m_model->channels().first().get());
    }
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::openRecentAnalysis()
{
  QAction *action = qobject_cast<QAction *>(sender());

  if (action && !action->data().isNull())
  {
    QStringList files(action->data().toString());
    if (MenuState::OPEN_STATE == m_menuState)
    {
      openAnalysis(files);
    }
    else
    {
      addToAnalysis(files);
    }
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::addToAnalysis()
{
  QList<QUrl> urls;
  urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation))
       << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation))
       << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation))
       << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DataLocation));

  QFileDialog fileDialog(this);
  fileDialog.setWindowTitle(tr("Add Data To Analysis"));
  //fileDialog.setFilters(m_model->factory()->supportedFiles());
  QStringList channelFiles;
  //TODO 2013-10-06: channelFiles << CHANNEL_FILES;
  fileDialog.setFileMode(QFileDialog::ExistingFiles);
  fileDialog.setFilters(channelFiles);
  fileDialog.setDirectory(m_sessionFile.absoluteDir());
  fileDialog.setOption(QFileDialog::DontUseNativeDialog, false);
  fileDialog.setViewMode(QFileDialog::Detail);
  fileDialog.resize(800, 480);
  fileDialog.setSidebarUrls(urls);
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);

  if (fileDialog.exec() == QFileDialog::Accepted)
  {
    addToAnalysis(fileDialog.selectedFiles());
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::addRecentToAnalysis()
{
  QAction *action = qobject_cast<QAction *>(sender());

  if (!action || action->data().isNull())
    return;

  QStringList files(action->data().toString());

  addToAnalysis(files);
}

//------------------------------------------------------------------------
void EspinaMainWindow::addToAnalysis(const QStringList files)
{
  QElapsedTimer timer;
  timer.start();

  AnalysisSPtr newAnalyses    = loadedAnalysis(files);
  AnalysisSPtr mergedAnalysis = merge(m_analysis, newAnalyses);
  m_model->setAnalysis(mergedAnalysis, m_factory);
  m_analysis = mergedAnalysis;

  int secs = timer.elapsed()/1000.0;
  int mins = 0;
  if (secs > 60)
  {
    mins = secs / 60;
    secs = secs % 60;
  }

  updateStatus(QString("File Loaded in %1m%2s").arg(mins).arg(secs));

  for(auto file : files)
  {
    m_recentDocuments1.addDocument(file);
  }
  m_recentDocuments2.updateDocumentList();
}

//------------------------------------------------------------------------
AnalysisSPtr EspinaMainWindow::loadedAnalysis(const QStringList files)
{
  QList<AnalysisSPtr> analyses;

  QApplication::setOverrideCursor(Qt::WaitCursor);
  for(auto file : files)
  {
    m_errorHandler->setDefaultDir(QFileInfo(file).dir());
    AnalysisReaderList readers = m_factory->readers(file);

    if (readers.isEmpty())
    {
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(this, tr("File Extension is not supported"), file);
      QApplication::setOverrideCursor(Qt::WaitCursor);
      continue;
    }

    AnalysisReaderPtr  reader  = readers.first();

    if (readers.size() > 1) 
    {
      //TODO choose reader
    }

    try {
      analyses << m_factory->read(reader, file, m_errorHandler);

      if (file != m_settings->autosavePath().absoluteFilePath(AUTOSAVE_FILE))
      {
        m_recentDocuments1.addDocument(file);
        m_recentDocuments2.updateDocumentList();
      }
    } catch (...)
    {
      QApplication::restoreOverrideCursor();
      QMessageBox box(QMessageBox::Warning,
                      tr("EspINA"),
                      tr("File \"%1\" could not be loaded.\n"
                      "Do you want to remove it from recent documents list?")
                      .arg(file),
                      QMessageBox::Yes|QMessageBox::No);

      if (box.exec() == QMessageBox::Yes)
      {
        m_recentDocuments1.removeDocument(file);
        m_recentDocuments2.updateDocumentList();
      }
      QApplication::setOverrideCursor(Qt::WaitCursor);
    }
  }

  AnalysisSPtr mergedAnalysis;
  if (!analyses.isEmpty())
  {
    mergedAnalysis = analyses.first();
    for(int i = 1; i < analyses.size(); ++i)
    {
      mergedAnalysis = merge(mergedAnalysis, analyses[i]);
    }
  }
  QApplication::restoreOverrideCursor();

  return mergedAnalysis;
}

//------------------------------------------------------------------------
void EspinaMainWindow::saveAnalysis()
{
  QString suggestedFileName;
  if (m_sessionFile.suffix().toLower() == QString("seg"))
    suggestedFileName = m_sessionFile.fileName();
  else
    suggestedFileName = m_sessionFile.baseName() + QString(".seg");

  QList<QUrl> urls;
  urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation))
       << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation))
       << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation))
       << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DataLocation));

  QFileDialog fileDialog(this);
  fileDialog.selectFile(suggestedFileName);
  fileDialog.setDefaultSuffix(QString(tr("seg")));
  fileDialog.setWindowTitle(tr("Save EspINA Analysis"));
  fileDialog.setFilter(tr("EspINA Analysis (*.seg)"));
  fileDialog.setDirectory(m_sessionFile.absoluteDir());
  fileDialog.setOption(QFileDialog::DontUseNativeDialog, false);
  fileDialog.setViewMode(QFileDialog::Detail);
  fileDialog.resize(800, 480);
  fileDialog.setSidebarUrls(urls);
  fileDialog.setConfirmOverwrite(true);
  fileDialog.setFileMode(QFileDialog::AnyFile);
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);

  QString analysisFile;
  if (fileDialog.exec() == QFileDialog::AcceptSave)
    analysisFile = fileDialog.selectedFiles().first();
  else
    return;

  Q_ASSERT(analysisFile.toLower().endsWith(QString(tr(".seg"))));

  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_busy = true;

  IO::SegFile::save(m_analysis.get(), analysisFile, m_errorHandler);

  QApplication::restoreOverrideCursor();
  updateStatus(tr("File Saved Successfully in %1").arg(analysisFile));
  m_busy = false;

  m_recentDocuments1.addDocument(analysisFile);
  m_recentDocuments2.updateDocumentList();

  m_undoStackSavedIndex = m_undoStack->index();

  QStringList fileParts = analysisFile.split(QDir::separator());
  setWindowTitle(fileParts[fileParts.size()-1]);

  m_saveSessionAnalysis->setEnabled(true);
  m_sessionFile = analysisFile;
}

//------------------------------------------------------------------------
void EspinaMainWindow::saveSessionAnalysis()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_busy = true;

  IO::SegFile::save(m_analysis.get(), m_sessionFile, nullptr);

  QApplication::restoreOverrideCursor();
  updateStatus(tr("File Saved Successfuly in %1").arg(m_sessionFile.fileName()));
  m_busy = false;

  m_recentDocuments1.addDocument(m_sessionFile.absoluteFilePath());
  m_recentDocuments2.updateDocumentList();

  m_undoStackSavedIndex = m_undoStack->index();
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
  GeneralSettingsDialog dialog;

  dialog.registerPanel(SettingsPanelSPtr(new GeneralSettingsPanel(m_settings)));
  dialog.registerPanel(m_view->settingsPanel());
  dialog.resize(800, 600);

  for(auto panel : m_availableSettingsPanels)
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
  if (!isModelModified()) return;

  m_busy = true;

  QDir autosavePath = m_settings->autosavePath();
  if (!autosavePath.exists())
    autosavePath.mkpath(autosavePath.absolutePath());

  const QFileInfo analysisFile = autosavePath.absoluteFilePath(AUTOSAVE_FILE);

  IO::SegFile::save(m_analysis.get(), analysisFile, nullptr);

  updateStatus(tr("Analysis autosaved at %1").arg(QTime::currentTime().toString()));
  m_busy = false;
  m_autosave.setInterval(m_settings->autosaveInterval()*60*1000);
}

//------------------------------------------------------------------------
void EspinaMainWindow::showConnectomicsInformation()
{
//   ConnectomicsDialog *dialog = new ConnectomicsDialog(m_model, m_viewManager, this);
//   dialog->show();
}

//------------------------------------------------------------------------
void EspinaMainWindow::showRawInformation()
{
  if (!m_model->segmentations().isEmpty())
  {
    RawInformationDialog *dialog = new RawInformationDialog(m_model, m_viewManager, this);
    connect(dialog, SIGNAL(finished(int)),
            dialog, SLOT(deleteLater()));
    dialog->show();
  }
  else
  {
    QMessageBox::warning(this, ESPINA, tr("Current analysis does not contain any segmentations"));
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::undoTextChanged(QString text)
{
  m_undoAction->setText(tr("Undo ") + text);
}

//------------------------------------------------------------------------
void EspinaMainWindow::redoTextChanged(QString text)
{
  m_redoAction->setText(tr("Redo ") + text);
}

//------------------------------------------------------------------------
void EspinaMainWindow::canRedoChanged(bool value)
{
  m_redoAction->setEnabled(value);
}

//------------------------------------------------------------------------
void EspinaMainWindow::canUndoChanged(bool value)
{
  m_undoAction->setEnabled(value);
}

//------------------------------------------------------------------------
void EspinaMainWindow::undoAction(bool unused)
{
  emit abortOperation();
  m_undoStack->undo();
  m_viewManager->updateSegmentationRepresentations();
}

//------------------------------------------------------------------------
void EspinaMainWindow::redoAction(bool unused)
{
  emit abortOperation();
  m_undoStack->redo();
  m_viewManager->updateSegmentationRepresentations();
}
