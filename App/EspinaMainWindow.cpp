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
#include "Dialogs/GeneralSettingsDialog.h"
#include "Dialogs/Connectomics/ConnectomicsDialog.h"
#include "Dialogs/AdaptiveEdges/AdaptiveEdgesDialog.h"
#include "Docks/ChannelExplorer/ChannelExplorer.h"
#include "Docks/FilterInspectorDock/FilterInspectorDock.h"
#include "Docks/SegmentationExplorer/SegmentationExplorer.h"
#include "Dialogs/TabularReport/TabularReport.h"
#include "Dialogs/TabularReport/RawInformationDialog.h"
#include "Docks/TaxonomyExplorer/TaxonomyExplorer.h"
#include "Menus/ColorEngineMenu.h"
#include "Settings/GeneralSettings.h"
#include "Toolbars/Editor/EditorToolBar.h"
#include "Toolbars/Main/MainToolBar.h"
#include "Toolbars/Segmentation/SegmentationToolBar.h"
#include "Toolbars/VOI/VolumeOfInterest.h"
#include "Toolbars/Zoom/ZoomToolBar.h"
#include "Toolbars/Composition/CompositionToolBar.h"
#include "Views/DefaultEspinaView.h"

// EspINA
#include <Core/ColorEngines/NumberColorEngine.h>
#include <Core/ColorEngines/TaxonomyColorEngine.h>
#include <Core/ColorEngines/UserColorEngine.h>
#include <Core/EspinaSettings.h>
#include <Core/IO/IOErrorHandler.h>
#include <Core/Interfaces/IColorEngineProvider.h>
#include <Core/Interfaces/IDockWidget.h>
#include <Core/Interfaces/IFactoryExtension.h>
#include <Core/Interfaces/IFileReader.h>
#include <Core/Interfaces/IToolBar.h>
#include <Core/Interfaces/IToolBarFactory.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Extensions/EdgeDistances/AdaptiveEdges.h>
#include <GUI/QtWidget/IEspinaView.h>
#include <GUI/Renderers/CrosshairRenderer.h>
#include <GUI/Renderers/MeshRenderer.h>
#include <GUI/Renderers/SmoothedMeshRenderer.h>
#include <GUI/Renderers/VolumetricGPURenderer.h>
#include <GUI/Renderers/VolumetricRenderer.h>
#include <GUI/Renderers/SliceRenderer.h>
#include <GUI/Renderers/ContourRenderer.h>
#include <GUI/ViewManager.h>
#include <Core/Filters/ChannelReader.h>
#include <App/Undo/UndoableEspinaModel.h>
#include <GUI/IO/EspinaIO.h>
#include <GUI/Representations/BasicGraphicalRepresentationFactory.h>

#ifdef TEST_ESPINA_MODELS
  #include <Core/Model/ModelTest.h>
#endif

// Std
#include <sstream>

// Qt
#include <QtGui>

using namespace EspINA;

const QString AUTOSAVE_FILE = "espina-autosave.seg";

//------------------------------------------------------------------------
class EspinaIOErrorHandler
: public IOErrorHandler
  {
    public:
      EspinaIOErrorHandler(QWidget *parent = NULL)
      : m_parent(parent) {};

      void setDefaultDir(const QDir &dir) {m_defaultDir = dir;}
      void warning(const QString &msg)
      {
        QMessageBox::warning(m_parent, "EspINA", msg);
      }
      void error(const QString &msg)
      {
        QMessageBox::warning(m_parent, "EspINA", msg);
      }

      QFileInfo fileNotFound(const QFileInfo &file,
                             QDir dir = QDir(),
                             const QString &nameFilters = QString(),
                             const QString &hint = QString())
      {
        QFileInfo resfile;
        QString title = (hint.isEmpty()) ? QObject::tr("Select file for %1:").arg(file.fileName()) : hint;
        QDir directory = (dir == QDir()) ? m_defaultDir : dir;
        QString filters = (nameFilters.isEmpty()) ? QObject::tr("%1 files (*.%1)").arg(file.suffix()) : nameFilters;

        QList<QUrl> urls;
        urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation))
             << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation))
             << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation))
             << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DataLocation));

        QFileDialog fileDialog(m_parent);
        fileDialog.setFileMode(QFileDialog::ExistingFiles);
        fileDialog.setWindowTitle(title);
        fileDialog.setFilter(filters);
        fileDialog.setDirectory(directory);
        fileDialog.setOption(QFileDialog::DontUseNativeDialog, false);
        fileDialog.setViewMode(QFileDialog::Detail);
        fileDialog.resize(800, 480);
        fileDialog.setSidebarUrls(urls);

        QApplication::setOverrideCursor(Qt::ArrowCursor);
        if (fileDialog.exec())
          resfile = QFileInfo(fileDialog.selectedFiles().first());
        QApplication::restoreOverrideCursor();

        return resfile;
    }

  private:
    QWidget *m_parent;
    QDir m_defaultDir;
};


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
, m_scheduler{5000}//5ms
, m_settings     (new GeneralSettings())
, m_settingsPanel(new GeneralSettingsPanel(m_model, m_settings))
, m_view(NULL)
, m_busy(false)
, m_undoStackSavedIndex(0)
, m_traceableStatus(new QLabel(statusBar()))
, m_errorHandler(new EspinaIOErrorHandler(this))
{
#ifdef TEST_ESPINA_MODELS
  m_modelTester = boost::shared_ptr<ModelTest>(new ModelTest(m_model));
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

  // TODO: Pass smartpointers direcrtly to factory
  m_defaultRenderers << IRendererSPtr(new CrosshairRenderer());
  m_defaultRenderers << IRendererSPtr(new VolumetricRenderer());
  m_defaultRenderers << IRendererSPtr(new VolumetricGPURenderer());
  m_defaultRenderers << IRendererSPtr(new MeshRenderer());
  m_defaultRenderers << IRendererSPtr(new SmoothedMeshRenderer());
  m_defaultRenderers << IRendererSPtr(new SliceRenderer());
  m_defaultRenderers << IRendererSPtr(new ContourRenderer());

  foreach(IRendererSPtr renderer, m_defaultRenderers)
    factory->registerRenderer(renderer.get());

  factory->registerFilter(this, ChannelReader::TYPE);

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
  m_colorEngines->addColorEngine(tr("Number"), ColorEnginePtr(new NumberColorEngine()));
  TaxonomyColorEnginePtr taxonomyEngine(new TaxonomyColorEngine());
  m_colorEngines->addColorEngine(tr("Taxonomy"),taxonomyEngine);
  m_colorEngines->addColorEngine(tr("User"), ColorEnginePtr(new UserColorEngine()));

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

  m_mainToolBar = new MainToolBar(m_model, m_undoStack, m_viewManager);
  registerToolBar(m_mainToolBar);

  ZoomToolBar *zoomToolBar = new ZoomToolBar(m_viewManager);
  registerToolBar(zoomToolBar);

  VolumeOfInterest *voiToolBar = new VolumeOfInterest(m_model, m_viewManager);
  registerToolBar(voiToolBar);

  SegmentationToolBar *seedToolBar = new SegmentationToolBar(m_model, m_undoStack, m_viewManager);
  registerToolBar(seedToolBar);

  EditorToolBar *editorToolBar = new EditorToolBar(m_model, m_undoStack, m_viewManager);
  registerToolBar(editorToolBar);

  CompositionToolBar *compositionBar = new CompositionToolBar(m_model, m_undoStack, m_viewManager);
  registerToolBar(compositionBar);

  ChannelExplorer *channelExplorer = new ChannelExplorer(m_model, m_undoStack, m_viewManager, this);
  registerDockWidget(Qt::LeftDockWidgetArea, channelExplorer);

  SegmentationExplorer *segExplorer = new SegmentationExplorer(m_model, m_undoStack, m_viewManager, this);
  registerDockWidget(Qt::LeftDockWidgetArea, segExplorer);

  FilterInspectorDock *filterInspectorDock = new FilterInspectorDock(m_undoStack, m_viewManager, this);
  registerDockWidget(Qt::LeftDockWidgetArea, filterInspectorDock);

//   QAction *connectomicsAction = new QAction(tr("Connectomics Information"), this);
//   m_dynamicMenuRoot->submenus[0]->menu->addAction(connectomicsAction);
//   connect(connectomicsAction, SIGNAL(triggered()), this, SLOT(showConnectomicsInformation()));

  QAction *rawInformationAction = m_dynamicMenuRoot->submenus[0]->menu->addAction(tr("Raw Information"));
  connect(rawInformationAction, SIGNAL(triggered(bool)),
          this, SLOT(showRawInformation()));


  loadPlugins(plugins);

  m_colorEngines->restoreUserSettings();
  m_viewMenu->addMenu(m_dockMenu);
  m_viewMenu->addSeparator();

  DefaultEspinaView *defaultView = new DefaultEspinaView(m_model, m_undoStack, m_viewManager, this);

  statusBar()->clearMessage();

  defaultView->createViewMenu(m_viewMenu);
  connect(m_mainToolBar, SIGNAL(showSegmentations(bool)),
          defaultView, SLOT(showSegmentations(bool)));
  m_view = defaultView;

  QSettings settings(CESVIMA, ESPINA);

  /**
   * Instead of ussing save/restoreGeometry resice+move
   * Works better in Unity when espina is closed while is maximized
   */
  settings.beginGroup("MainWindow");
  resize(settings.value("size", QSize(800, 600)).toSize());
  move(settings.value("pos", QPoint(200, 200)).toPoint());
  settings.endGroup();
  restoreState(settings.value("state").toByteArray());

  m_autosave.setInterval(m_settings->autosaveInterval()*60*1000);
  m_autosave.start();
  connect(&m_autosave, SIGNAL(timeout()),
          this, SLOT(autosave()));

  cancel = new QShortcut(Qt::Key_Escape, this, SLOT(cancelOperation()));

  setWindowTitle(QString("EspINA Interactive Neuron Analyzer"));

  checkAutosave();

  statusBar()->addPermanentWidget(m_traceableStatus);
  updateTraceabilityStatus();
}

//------------------------------------------------------------------------
EspinaMainWindow::~EspinaMainWindow()
{
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying Main Window";
//   qDebug() << "********************************************************";

  foreach(IRendererSPtr renderer, m_defaultRenderers)
    m_model->factory()->unregisterRenderer(renderer.get());

  delete m_settings;
  delete m_undoStack;
  delete m_dynamicMenuRoot;
}

//------------------------------------------------------------------------
FilterSPtr EspinaMainWindow::createFilter(const QString& filter,
                                          const Filter::NamedInputs& inputs,
                                          const ModelItem::Arguments& args)
{
  if (ChannelReader::TYPE == filter)
  {
    FilterSPtr reader(new ChannelReader(inputs, args, ChannelReader::TYPE, m_errorHandler));
    SetBasicGraphicalRepresentationFactory(reader);
    return reader;
  }

  Q_ASSERT(false);
  return FilterSPtr();
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

    IToolBarFactory *toolbarFactory = qobject_cast<IToolBarFactory *>(plugin);
    if (toolbarFactory)
    {
      qDebug() << plugin << "- ToolBar ... OK";
      toolbarFactory->initToolBarFactory(m_model, m_undoStack, m_viewManager);
      foreach(IToolBar *toolbar, toolbarFactory->toolBars())
      {
        registerToolBar(toolbar);
      }
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
          toolbar, SLOT(resetToolbar()));
  connect(this, SIGNAL(abortOperation()),
          toolbar, SLOT(abortOperation()));
  addToolBar(toolbar);
}


//------------------------------------------------------------------------
void EspinaMainWindow::updateTraceabilityStatus()
{
  if (m_model->isTraceable())
  {
    m_traceableStatus->setPixmap(QPixmap(":/espina/traceable.png").scaled(22, 22));
    m_traceableStatus->setToolTip(tr("Current Session is traceable"));
  }
  else
  {
    m_traceableStatus->setPixmap(QPixmap(":/espina/non_traceable.png").scaled(22, 22));
    m_traceableStatus->setToolTip(tr("Current Session is NOT traceable"));
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

  if (m_model->hasChanged() || m_undoStack->index() != m_undoStackSavedIndex)
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

  m_viewManager->setActiveChannel(ChannelPtr());
  m_viewManager->setActiveTaxonomy(TaxonomyElementPtr());
  m_viewManager->unsetActiveVOI();
  m_viewManager->unsetActiveTool();
  m_viewManager->clearSelection();
  m_undoStack->clear();
  m_undoStackSavedIndex = m_undoStack->index();
  m_model->reset();

  // resets slice views matrices to avoid an esthetic bug
  Nm origin[3] = { 0,0,0 };
  m_viewManager->focusViewsOn(origin);

  m_addMenu->setEnabled(false);
  m_saveAnalysis->setEnabled(false);
  m_saveSessionAnalysis->setEnabled(false);
  m_closeAnalysis->setEnabled(false);
  m_sessionFile = QFileInfo();

  setWindowTitle(QString("EspINA Interactive Neuron Analyzer"));
  m_dynamicMenuRoot->submenus.first()->menu->setEnabled(false);
  updateTraceabilityStatus();

  SegFileReader::removeTemporalDir();

  return true;
}

//------------------------------------------------------------------------
void EspinaMainWindow::openAnalysis()
{
  const QString title   = tr("Start New Analysis From File");
  const QString filters = m_model->factory()->supportedFiles().join(";;");

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
    openAnalysis(fileDialog.selectedFiles().first());
    m_model->markAsSaved();
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::openAnalysis(const QFileInfo file)
{
  QElapsedTimer timer;
  timer.start();

  QFileInfo fileInfo(file);

  IOErrorHandler::STATUS loaded;
  {
    m_errorHandler->setDefaultDir(fileInfo.dir());
    QApplication::setOverrideCursor(Qt::WaitCursor);
    closeCurrentAnalysis();

    loaded =  EspinaIO::loadFile(file,
                                 m_model,
                                 m_errorHandler);
    QApplication::restoreOverrideCursor();
  }

  if (IOErrorHandler::SUCCESS != loaded)
  {
    QMessageBox box(QMessageBox::Warning,
                    tr("EspINA"),
                    tr("File \"%1\" could not be loaded.\n"
                    "Do you want to remove it from recent documents list?")
                    .arg(fileInfo.fileName()),
                    QMessageBox::Yes|QMessageBox::No);

    if (box.exec() == QMessageBox::Yes)
    {
      m_recentDocuments1.removeDocument(file.absoluteFilePath());
      m_recentDocuments2.updateDocumentList();
    }
    // NOTE: avoid triggering the save dialog at closeCurrentAnalysis()
    // as this is an incomplete load and the model is inconsistent
    m_model->markAsSaved();

    closeCurrentAnalysis();
    return;
  }

  if (!m_model->taxonomy())
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

  QApplication::restoreOverrideCursor();

  updateTraceabilityStatus();
  m_dynamicMenuRoot->submenus.first()->menu->setEnabled(true);
  updateStatus(QString("File Loaded in %1m%2s").arg(mins).arg(secs));

  if (file != m_settings->autosavePath().absoluteFilePath(AUTOSAVE_FILE))
  {
    m_recentDocuments1.addDocument(file.absoluteFilePath());
    m_recentDocuments2.updateDocumentList();
  }

  Q_ASSERT(!m_model->channels().isEmpty());

  m_addMenu      ->setEnabled(true);
  m_saveAnalysis ->setEnabled(true);
  m_closeAnalysis->setEnabled(true);

  ChannelPtr channel = m_model->channels().first().get();
  m_viewManager->setActiveChannel(channel);

  setWindowTitle(file.fileName());

  if (file.suffix().toLower() == QString("seg"))
    m_saveSessionAnalysis->setEnabled(true);
  else
    m_saveSessionAnalysis->setEnabled(false);

  m_sessionFile = file;

  m_viewManager->updateSegmentationRepresentations();
  m_viewManager->updateViews();

  if (EspinaIO::isChannelExtension(fileInfo.suffix()))
  {
    AdaptiveEdgesDialog edgesDialog(this);
    edgesDialog.exec();
    if (edgesDialog.useAdaptiveEdges())
    {
      channel->addExtension(new AdaptiveEdges(true, edgesDialog.color(), edgesDialog.threshold()));
    }
  }

  m_model->emitChannelAdded(m_model->channels());
  m_model->emitSegmentationAdded(m_model->segmentations());
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
  QList<QUrl> urls;
  urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation))
       << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation))
       << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation))
       << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DataLocation));

  QFileDialog fileDialog(this);
  fileDialog.setWindowTitle(tr("Add Data To Analysis"));
  //fileDialog.setFilters(m_model->factory()->supportedFiles());
  QStringList channelFiles;
  channelFiles << CHANNEL_FILES;
  fileDialog.setFileMode(QFileDialog::ExistingFiles);
  fileDialog.setFilters(channelFiles);
  fileDialog.setDirectory(m_sessionFile.absoluteDir());
  fileDialog.setOption(QFileDialog::DontUseNativeDialog, false);
  fileDialog.setViewMode(QFileDialog::Detail);
  fileDialog.resize(800, 480);
  fileDialog.setSidebarUrls(urls);
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);

  if (fileDialog.exec() == QFileDialog::Accepted)
    foreach(QString fileName, fileDialog.selectedFiles())
      addFileToAnalysis(QFileInfo(fileName));
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
void EspinaMainWindow::addFileToAnalysis(const QFileInfo file)
{
  // Prevent loading seg files from recent files until multiple seg analysis is fully supported
  if (!EspinaIO::isChannelExtension(file.suffix()))
    return;

  QApplication::setOverrideCursor(Qt::WaitCursor);
  QElapsedTimer timer;
  timer.start();

  ChannelSList existingChannels = m_model->channels();
  SegmentationSList existingSegmentations = m_model->segmentations();

  UndoableEspinaModel undoableModel(m_model, m_undoStack);
  m_undoStack->beginMacro(tr("Add %1 to analysis").arg(file.fileName()));
  if (IOErrorHandler::SUCCESS == EspinaIO::loadFile(file, &undoableModel))
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
    m_recentDocuments1.addDocument(file.absoluteFilePath());
    m_recentDocuments2.updateDocumentList();

    if (EspinaIO::isChannelExtension(file.suffix()))
    {
      ChannelSPtr channel;
      int i = 0;
      while (!channel && i < m_model->channels().size())
      {
        ChannelSPtr iChannel = m_model->channels()[i];
        if (file.fileName() == iChannel->data().toString())
          channel = iChannel;
        ++i;
      }

      AdaptiveEdgesDialog edgesDialog(this);
      edgesDialog.exec();
      if (edgesDialog.useAdaptiveEdges())
      {
        channel->addExtension(new AdaptiveEdges(true, edgesDialog.color(), edgesDialog.threshold()));
      }
    }

    ChannelSList newChannels;
    SegmentationSList newSegmentations;
    foreach(ChannelSPtr channel, m_model->channels())
      if (!existingChannels.contains(channel))
        newChannels << channel;

    foreach(SegmentationSPtr segmentation, m_model->segmentations())
      if (!existingSegmentations.contains(segmentation))
        newSegmentations << segmentation;

    m_model->emitChannelAdded(newChannels);
    m_model->emitSegmentationAdded(newSegmentations);

  }
  m_undoStack->endMacro();
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
  fileDialog.setFilter(SEG_FILES);
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

  SegFileReader::saveSegFile(analysisFile, m_model);

  QApplication::restoreOverrideCursor();
  updateStatus(tr("File Saved Successfully in %1").arg(analysisFile));
  m_busy = false;

  m_recentDocuments1.addDocument(analysisFile);
  m_recentDocuments2.updateDocumentList();

  m_undoStackSavedIndex = m_undoStack->index();
  m_model->markAsSaved();

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

  SegFileReader::saveSegFile(m_sessionFile, m_model);

  QApplication::restoreOverrideCursor();
  updateStatus(tr("File Saved Successfuly in %1").arg(m_sessionFile.fileName()));
  m_busy = false;

  m_recentDocuments1.addDocument(m_sessionFile.absoluteFilePath());
  m_recentDocuments2.updateDocumentList();

  m_undoStackSavedIndex = m_undoStack->index();
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
  GeneralSettingsDialog dialog;

  dialog.registerPanel(m_settingsPanel.get());
  dialog.registerPanel(m_view->settingsPanel());
  dialog.resize(800, 600);

  foreach(ISettingsPanelPtr panel, m_model->factory()->settingsPanels())
  {
    dialog.registerPanel(panel);
  }

  dialog.exec();
  updateTraceabilityStatus();
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

  SegFileReader::saveSegFile(analysisFile, m_model);

  updateStatus(tr("Analysis autosaved at %1").arg(QTime::currentTime().toString()));
  m_busy = false;
  m_autosave.setInterval(m_settings->autosaveInterval()*60*1000);
}

//------------------------------------------------------------------------
void EspinaMainWindow::showConnectomicsInformation()
{
  ConnectomicsDialog *dialog = new ConnectomicsDialog(m_model, m_viewManager, this);
  dialog->show();
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
  m_undoAction->setText(tr("Undo") + text);
}

//------------------------------------------------------------------------
void EspinaMainWindow::redoTextChanged(QString text)
{
  m_redoAction->setText(tr("Redo") + text);
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
}

//------------------------------------------------------------------------
void EspinaMainWindow::redoAction(bool unused)
{
  emit abortOperation();
  m_undoStack->redo();
}
