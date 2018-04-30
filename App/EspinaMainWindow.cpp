/*
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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


// ESPINA
#include "EspinaMainWindow.h"
#include <Core/IO/ClassificationXML.h>
#include <Core/IO/SegFile.h>
#include <Core/MultiTasking/Scheduler.h>
#include <Core/Utils/AnalysisUtils.h>
#include <Core/Utils/ListUtils.hxx>
#include <Core/Analysis/Channel.h>
#include <Extensions/EdgeDistances/ChannelEdges.h>
#include <GUI/ColorEngines/CategoryColorEngine.h>
#include <GUI/ColorEngines/NumberColorEngine.h>
#include <GUI/ColorEngines/UserColorEngine.h>
#include <GUI/ColorEngines/InformationColorEngine.h>
#include <GUI/Utils/DefaultIcons.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Widgets/Styles.h>
#include <Support/Readers/ChannelReader.h>
#include <Support/Settings/Settings.h>
#include <Support/Utils/FactoryUtils.h>
#include <Support/Widgets/PanelSwitch.h>
#include <App/Dialogs/About/AboutDialog.h>
#include <App/Dialogs/Settings/GeneralSettingsDialog.h>
#include <App/Dialogs/RawInformation/RawInformationDialog.h>
#include <App/Dialogs/Dialog3D/Dialog3D.h>
#include <App/Dialogs/IssueList/CheckAnalysis.h>
#include <App/Panels/StackExplorer/StackExplorer.h>
#include <App/Panels/SegmentationExplorer/SegmentationExplorer.h>
#include <App/Panels/SegmentationProperties/SegmentationProperties.h>
#include <App/Settings/GeneralSettings/GeneralSettingsPanel.h>
#include <App/Settings/ROI/ROISettings.h>
#include <App/Settings/ROI/ROISettingsPanel.h>
#include <App/Settings/SeedGrowSegmentation/SeedGrowSegmentationSettingsPanel.h>
#include <App/Settings/Utils.h>
#include <App/IO/SegFileReader.h>
#include <App/ToolGroups/ToolGroup.h>
#include <App/ToolGroups/Visualize/Representations/CrosshairRepresentationFactory.h>
#include <App/ToolGroups/Visualize/Representations/SegmentationRepresentationFactory.h>
#include <App/ToolGroups/Visualize/Representations/StackRepresentationFactory.h>
#include <App/ToolGroups/Visualize/ColorEngines/InformationColorEngineSwitch.h>
#include <App/ToolGroups/Visualize/GenericTogglableTool.h>
#include <App/ToolGroups/Visualize/FullscreenTool.h>
#include <App/ToolGroups/Segment/SeedGrowSegmentation/SeedGrowSegmentationSettings.h>
#include <App/ToolGroups/Segment/SeedGrowSegmentation/SeedGrowSegmentationTool.h>
#include <App/ToolGroups/Segment/Manual/ManualSegmentTool.h>
#include <App/ToolGroups/Segment/Skeleton/SkeletonCreationTool.h>
#include <App/ToolGroups/Explore/ResetViewTool.h>
#include <App/ToolGroups/Explore/ZoomRegionTool.h>
#include <App/ToolGroups/Explore/PositionMarksTool.h>
#include <App/ToolGroups/Session/FileOpenTool.h>
#include <App/ToolGroups/Session/FileSaveTool.h>
#include <App/ToolGroups/Session/UndoRedoTools.h>
#include <App/RecentDocuments.h>

#if USE_METADONA
  #include <App/Settings/MetaData/MetaDataSettingsPanel.h>
#endif

// C++
#include <sstream>

// Qt
#include <QtGui>

using namespace ESPINA;
using namespace ESPINA::Extensions;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Support;
using namespace ESPINA::Support::Widgets;

//------------------------------------------------------------------------
EspinaMainWindow::EspinaMainWindow(QList< QObject* >& plugins)
: QMainWindow        {nullptr, 0}
, m_minimizedStatus  {false}
, m_context          {this, &m_minimizedStatus}
, m_analysis         {new Analysis()}
, m_errorHandler     {new EspinaErrorHandler(this)}
, m_channelReader    {new ChannelReader()}
, m_segFileReader    {new SegFileReader()}
, m_settings         {new ApplicationSettings()}
, m_roiSettings      {new ROISettings()}
, m_sgsSettings      {new SeedGrowSegmentationSettings()}
, m_cancelShortcut   {Qt::Key_Escape, this, SLOT(cancelOperation()), SLOT(cancelOperation()), Qt::ApplicationShortcut}
, m_mainBarGroup     {this}
, m_activeToolGroup  {nullptr}
, m_view             {new DefaultView(m_context, this)}
, m_schedulerProgress{new SchedulerProgress(m_context.scheduler(), this)}
, m_busy             {false}
, m_checkTask        {nullptr}
{
  updateUndoStackIndex();

  auto factory = m_context.factory();
  factory->registerAnalysisReader(m_channelReader);
  factory->registerAnalysisReader(m_segFileReader);
  factory->registerFilterFactory (m_channelReader);

  m_availableSettingsPanels << std::make_shared<SeedGrowSegmentationsSettingsPanel>(m_sgsSettings);
  m_availableSettingsPanels << std::make_shared<ROISettingsPanel>(m_roiSettings, m_context);
#if USE_METADONA
  m_availableSettingsPanels << std::make_shared<MetaDataSettingsPanel>();
#endif

  setContextMenuPolicy(Qt::NoContextMenu);

  initRepresentations();

  createToolbars();

  createToolGroups();

  createDefaultPanels();

  initColorEngines();

  loadPlugins(plugins);

  createToolShortcuts();

  closeCurrentAnalysis();

  restoreGeometry();

  statusBar()->addPermanentWidget(m_schedulerProgress.get(), 1);
  statusBar()->clearMessage();

  m_sessionToolGroup->setChecked(true);

  try
  {
    checkAutoSavedAnalysis();
  }
  catch(...)
  {
    // nothing
  }
}

//------------------------------------------------------------------------
EspinaMainWindow::~EspinaMainWindow()
{
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying Main Window";
//   qDebug() << "********************************************************";

  for(auto shortcut: m_toolShortcuts)
  {
    delete shortcut;
  }
  m_toolShortcuts.clear();

  delete m_restrictToolGroup;
  delete m_exploreToolGroup;
  delete m_segmentToolGroup;
  delete m_refineToolGroup;
  delete m_visualizeToolGroup;
  delete m_analyzeToolGroup;
  delete m_roiSettings;
}

//------------------------------------------------------------------------
void EspinaMainWindow::loadPlugins(QList<QObject *> &plugins)
{
  auto factory = m_context.factory();

  for(auto plugin : plugins)
  {
    auto validPlugin = qobject_cast<Plugin*>(plugin);
    if (validPlugin)
    {
      validPlugin->init(m_context);

      connect(this,        SIGNAL(analysisChanged()),
              validPlugin, SLOT(onAnalysisChanged()));
      connect(this,        SIGNAL(analysisAboutToBeClosed()),
              validPlugin, SLOT(onAnalysisClosed()));

      for (auto colorEngine : validPlugin->colorEngines())
      {
        qDebug() << plugin << "- Color Engine " << colorEngine->colorEngine()->tooltip() << " ...... OK";
        registerColorEngine(colorEngine);
      }

      for (auto extensionFactory : validPlugin->channelExtensionFactories())
      {
        qDebug() << plugin << "- Channel Extension Factory  ...... OK";
        factory->registerExtensionFactory(extensionFactory);
      }

      for (auto tool : validPlugin->tools())
      {
        qDebug() << plugin << "- Tool " /*<< tool.second->toolTip()*/ << " ...... OK";
        switch (tool.first)
        {
          case ToolCategory::SESSION:
            m_sessionToolGroup->addTool(tool.second);
            break;
          case ToolCategory::EXPLORE:
            m_exploreToolGroup->addTool(tool.second);
            break;
          case ToolCategory::ROI:
            m_restrictToolGroup->addTool(tool.second);
            break;
          case ToolCategory::SEGMENT:
            m_segmentToolGroup->addTool(tool.second);
            break;
          case ToolCategory::EDIT:
            m_refineToolGroup->addTool(tool.second);
            break;
          case ToolCategory::VISUALIZE:
            m_visualizeToolGroup->addTool(tool.second);
            break;
          case ToolCategory::ANALYZE:
            m_analyzeToolGroup->addTool(tool.second);
            break;
        }
      }

      for(auto filterFactory: validPlugin->filterFactories())
      {
        qDebug() << plugin << "- Filter Factory  ...... OK (" << filterFactory->providedFilters() << ")";
        factory->registerFilterFactory(filterFactory);
      }

      for(auto reader: validPlugin->analysisReaders())
      {
        qDebug() << plugin << "- Analysis Reader  ...... OK (" << reader->type() << ")";
        factory->registerAnalysisReader(reader);
      }

      for (auto extensionFactory : validPlugin->segmentationExtensionFactories())
      {
        qDebug() << plugin << "- Segmentation Extension Factory  ...... OK (" << extensionFactory->providedExtensions() << ")";
        factory->registerExtensionFactory(extensionFactory);
      }

      for (auto report : validPlugin->reports())
      {
        qDebug() << plugin << "- Register Report" << report->name() << " ...... OK";
        m_analyzeToolGroup->registerReport(report);
      }

      for (auto settings : validPlugin->settingsPanels())
      {
        qDebug() << plugin << "- Settings Panel " << settings->windowTitle() << " ...... OK";
        m_availableSettingsPanels << settings;
      }

      for (auto factory : validPlugin->representationFactories())
      {
        qDebug() << plugin << "- Representation Factory  ...... OK";
        registerRepresentationFactory(factory);
        registerRepresentationSwitches(m_context.representation(factory));
      }
    }
  }
}

//------------------------------------------------------------------------
bool EspinaMainWindow::isModelModified()
{
  return m_context.undoStack()->index() != m_savedUndoStackIndex;
}

//------------------------------------------------------------------------
void EspinaMainWindow::enableWidgets(bool value)
{
  for (auto dock : findChildren<QDockWidget *>())
  {
    dock->setEnabled(value);
  }

  auto sessionTools = m_sessionToolGroup->groupedTools();
  sessionTools[0][1]->setEnabled(value); // Add
  sessionTools[1][1]->setEnabled(value); // Save as
  sessionTools[3][0]->setEnabled(value); // Check

  for(auto action: m_mainBar->actions())
  {
    if(action == m_mainBar->actions().first()) continue;

    action->setEnabled(value);
  }

  centralWidget()->setEnabled(value);
}

//------------------------------------------------------------------------
void EspinaMainWindow::registerToolGroup(ToolGroupPtr toolGroup)
{
  static int key = 0;
  QList<Qt::Key> keys{ Qt::Key_1, Qt::Key_2, Qt::Key_3, Qt::Key_4, Qt::Key_5, Qt::Key_6, Qt::Key_7, Qt::Key_8, Qt::Key_9 };

  if(key < 9)
  {
    toolGroup->setShortcut(Qt::CTRL+keys[key++]);
  }

  m_mainBarGroup.addAction(toolGroup);
  m_mainBar->addAction(toolGroup);

  connect(toolGroup, SIGNAL(activated(ToolGroup*)),
          this,      SLOT(activateToolGroup(ToolGroup *)));

  connect(toolGroup, SIGNAL(exclusiveToolInUse(Support::Widgets::ProgressTool*)),
          this,      SLOT(onExclusiveToolInUse(Support::Widgets::ProgressTool*)));

  connect(this,      SIGNAL(abortOperation()),
          toolGroup, SLOT(abortOperations()));
}


//------------------------------------------------------------------------
void EspinaMainWindow::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);

  ESPINA_SETTINGS(settings);
  settings.beginGroup("MainWindow");
  auto firstRun = !settings.childKeys().contains("size"); // absent in first run of the application.

  // if first time running adjust the docks into tabs to avoid getting a size bigger than the screen (if accumulated top-bottom)
  if(firstRun)
  {
    QList<QDockWidget *> panels;
    QDockWidget *segPanel = nullptr;

    for (auto dock : findChildren<QDockWidget *>())
    {
      if(dynamic_cast<View2D *>(dock)) continue;

      auto panel = dynamic_cast<Panel *>(dock);

      if(panel == m_view->panelXZ() || panel == m_view->panelYZ())
      {
        addDockWidget(Qt::RightDockWidgetArea, panel);
        continue;
      }

      if(panel && panel->isVisible() && (panel != m_view->panelXZ()) && (panel != m_view->panelYZ()))
      {
        if(dynamic_cast<SegmentationExplorer *>(dock))
        {
          segPanel = dock;
        }
        else
        {
          panels << dock;
        }
      }
    }

    for(auto dock: panels)
    {
      QMainWindow::tabifyDockWidget(segPanel, dock);
    }

    showMaximized();
  }

  m_minimizedStatus = false;
}

//------------------------------------------------------------------------
void EspinaMainWindow::hideEvent(QHideEvent *event)
{
  QWidget::hideEvent(event);

  m_minimizedStatus = true;
}

//------------------------------------------------------------------------
void EspinaMainWindow::closeEvent(QCloseEvent* event)
{
  if (m_busy)
  {
    auto answer = DefaultDialogs::UserQuestion(tr("ESPINA has pending actions. Do you really want to quit anyway?"),
                                               QMessageBox::Yes|QMessageBox::Cancel);
    if(answer == QMessageBox::Cancel)
    {
      event->ignore();
      return;
    }
  }

  if (closeCurrentAnalysis())
  {
    saveGeometry();

    event->accept();
  }
  else
  {
    event->ignore();
    return;
  }

  m_view.reset();

  removeTemporalDirectory(m_settings->temporalPath());

  m_context.scheduler()->abort();
}

//------------------------------------------------------------------------
bool EspinaMainWindow::closeCurrentAnalysis()
{
  // checks if the model have been modified since that last save or the file never has been saved.
  if (isModelModified() || (!m_context.model()->isEmpty() && !m_saveTool->isEnabled()))
  {
    auto message = tr("Current session has not been saved. Do you want to save it now?");
    auto buttons = QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel;
    auto title   = windowTitle();
    auto answer  = DefaultDialogs::UserQuestion(message, buttons, title);

    switch(answer)
    {
      case QMessageBox::Yes:
        m_saveAsTool->saveAnalysis();
        break;
      case QMessageBox::Cancel:
        return false;
        break;
      default:
        break;
    }
  }

  emit abortOperation();
  emit analysisAboutToBeClosed();

  getSelection(m_context)->clear();
  m_context.undoStack()->clear();
  updateUndoStackIndex();

  m_saveTool->setSaveFilename("");

  m_context.model()->clear();
  m_analysis.reset();

  enableWidgets(false);
  enableToolShortcuts(false);

  setWindowTitle(tr("ESPINA Interactive Neuron Analyzer"));

  emit analysisClosed();

  return true;
}

//------------------------------------------------------------------------
void EspinaMainWindow::openAnalysis(QStringList filenames)
{
  QStringList failedFiles, successFiles;

  for(auto filename: filenames)
  {
    QFileInfo fileInfo(filename);

    if(!fileInfo.exists())
    {
      failedFiles << filename;
    }
    else
    {
      successFiles << filename;
    }
  }

  if(closeCurrentAnalysis())
  {
    if(!successFiles.isEmpty())
    {
      m_openFileTool->load(successFiles);
    }

    if(!failedFiles.isEmpty())
    {
      QString message(tr("The following files couldn't be loaded because they do not exist:\n"));
      for(auto filename: failedFiles)
      {
        message += QString("\n") + filename;
      }

      DefaultDialogs::InformationMessage(message, tr("Error loading files"));
    }
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::onAnalysisLoaded(AnalysisSPtr analysis)
{
  Q_ASSERT(analysis);

  stopAnalysisConsistencyCheck();

  if(!analysis->classification())
  {
    QFileInfo defaultClassification(":/espina/defaultClassification.xml");

    auto classification = IO::ClassificationXML::load(defaultClassification);
    analysis->setClassification(classification);
  }

  auto model = m_context.model();

  model->setAnalysis(analysis, m_context.factory());

  m_analysis = analysis;

  updateSceneState(NmVector3{0,0,0}, m_context.viewState(), toViewItemSList(model->channels()));

  m_context.viewState().resetCamera();

  initializeCrosshair();

  updateToolsSettings();

  enableWidgets(true);

  enableToolShortcuts(true);

  assignActiveStack();

  analyzeStackEdges();

  auto files = m_openFileTool->loadedFiles();

  Q_ASSERT(!files.isEmpty());
  auto referenceFile = QFileInfo{files.first()};

  updateUndoStackIndex();

  if(!m_autoSave.isAutoSaveFile(referenceFile.absoluteFilePath()))
  {
    setWindowTitle(referenceFile.fileName());

    m_saveTool->setSaveFilename(referenceFile.absoluteFilePath());
    m_saveTool->setEnabled(files.size() == 1 && referenceFile.fileName().endsWith(".seg", Qt::CaseInsensitive));

    m_saveAsTool->setSaveFilename(referenceFile.absoluteFilePath());
  }
  else
  {
    setWindowTitle("New unsaved session");

    m_saveTool->setEnabled(false);
    m_saveTool->setSaveFilename("");

    m_saveAsTool->setSaveFilename("");
  }

  m_autoSave.resetCountDown();

  if(!m_context.model()->isEmpty() && m_settings->performAnalysisCheckOnLoad())
  {
    checkAnalysisConsistency();
  }

  emit analysisChanged();

  m_context.viewState().refresh();
}

//------------------------------------------------------------------------
void EspinaMainWindow::onAnalysisImported(AnalysisSPtr analysis)
{
  stopAnalysisConsistencyCheck();

  auto model = m_context.model();

  emit abortOperation();

  auto mergedAnalysis = merge(m_analysis, analysis);

  model->setAnalysis(mergedAnalysis, m_context.factory());

  m_analysis = mergedAnalysis;

  assignActiveStack();

  updateSceneState(m_context.viewState().crosshair(), m_context.viewState(), toViewItemSList(model->channels()));

  if(!m_context.model()->isEmpty() && m_settings->performAnalysisCheckOnLoad())
  {
    checkAnalysisConsistency();
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::showIssuesDialog(IssueList issues) const
{
  IssueListDialog dialog(issues);

  dialog.exec();
}

//------------------------------------------------------------------------
void EspinaMainWindow::onAutoSave(const QString& file)
{
  QApplication::processEvents();

  m_saveAsTool->saveAnalysis(file);

  QApplication::processEvents();
}

//------------------------------------------------------------------------
void EspinaMainWindow::updateStatus(QString msg)
{
  if (msg.isEmpty())
  {
    statusBar()->clearMessage();
  }
  else
  {
    statusBar()->showMessage(msg);
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::updateTooltip(QAction* action)
{
  auto menu = dynamic_cast<QMenu *>(sender());
  menu->setToolTip(action->toolTip());
}

//------------------------------------------------------------------------
void EspinaMainWindow::showPreferencesDialog()
{
  GeneralSettingsDialog dialog;
  dialog.registerPanel(std::make_shared<GeneralSettingsPanel>(m_autoSave, m_settings));
  dialog.resize(800, 600);

  for(auto panel : m_availableSettingsPanels)
  {
    dialog.registerPanel(panel);
  }

  auto temporalDirPath = m_settings->temporalPath();

  dialog.exec();

  if(temporalDirPath != m_settings->temporalPath())
  {
    auto dir = QDir{m_settings->temporalPath()};
    try
    {
      m_context.factory()->setTemporalDirectory(dir);
      removeTemporalDirectory(temporalDirPath);
    }
    catch(const EspinaException &e)
    {
      DefaultDialogs::InformationMessage(tr("Unable to change the temporal directory to '%1'.\n\nError: %2").arg(dir.absolutePath()).arg(QString(e.what())), "Error changing temporal directory");
    }
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::showAboutDialog()
{
  AboutDialog dialog;

  dialog.exec();
}

//------------------------------------------------------------------------
void EspinaMainWindow::cancelOperation()
{
  m_context.viewState().setEventHandler(nullptr);
  m_context.viewState().refresh();
}

//------------------------------------------------------------------------
void EspinaMainWindow::onColorEngineModified()
{
}

//------------------------------------------------------------------------
void EspinaMainWindow::activateToolGroup(ToolGroup *toolGroup)
{
  if (m_activeToolGroup != toolGroup)
  {
    if (m_activeToolGroup)
    {
      m_activeToolGroup->setChecked(false);
    }

    m_contextualBar->clear();

    if (toolGroup)
    {
      for(auto tools : toolGroup->groupedTools())
      {
        for (auto tool : tools)
        {
          tool->onToolGroupActivated();

          for(auto action : tool->actions())
          {
            m_contextualBar->addAction(action);
          }
        }

        if (tools != toolGroup->groupedTools().last())
        {
          m_contextualBar->addSeparator();
        }
      }
    }

    m_activeToolGroup = toolGroup;
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::onExclusiveToolInUse(ProgressTool* tool)
{
  for (auto toolGroup : toolGroups())
  {
    toolGroup->onExclusiveToolInUse(tool);
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::createColorEngine(ColorEngineSPtr engine, const QString &icon)
{
  registerColorEngine(std::make_shared<ColorEngineSwitch>(engine, ":espina/color_engine_switch_" + icon + ".svg", m_context));
}

//------------------------------------------------------------------------
void EspinaMainWindow::initColorEngines()
{
  createColorEngine(std::make_shared<NumberColorEngine>(), "number");

  auto categoryColorEngine = std::make_shared<CategoryColorEngine>();
  categoryColorEngine->setActive(true);
  createColorEngine(categoryColorEngine, "category");

  registerColorEngine(std::make_shared<InformationColorEngineSwitch>(m_context));
}


//------------------------------------------------------------------------
void EspinaMainWindow::registerColorEngine(ColorEngineSwitchSPtr colorEngineSwitch)
{
  auto colorEngine = colorEngineSwitch->colorEngine();
  m_context.addColorEngine(colorEngine);

  m_visualizeToolGroup->addTool(colorEngineSwitch);
}

//------------------------------------------------------------------------
void EspinaMainWindow::initRepresentations()
{
  registerRepresentationFactory(std::make_shared<CrosshairRepresentationFactory>());
  registerRepresentationFactory(std::make_shared<StackRepresentationFactory>());
  registerRepresentationFactory(std::make_shared<SegmentationRepresentationFactory>());
}


//------------------------------------------------------------------------
void EspinaMainWindow::createToolbars()
{
  m_mainBar = addToolBar("Main ToolBar");
  m_mainBar->setMovable(false);
  m_mainBar->setObjectName("Main ToolBar");
  m_mainBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  m_mainBarGroup.setExclusive(true);
  m_mainBarGroup.setEnabled(true);
  m_mainBarGroup.setVisible(true);

  m_contextualBar = addToolBar("Contextual ToolBar");
  m_contextualBar->setMovable(false);
  m_contextualBar->setObjectName("Contextual ToolBar");
  m_contextualBar->setMinimumHeight(Styles::CONTEXTUAL_BAR_HEIGHT);
  m_contextualBar->setMaximumHeight(Styles::CONTEXTUAL_BAR_HEIGHT);
}

//------------------------------------------------------------------------
void EspinaMainWindow::createToolGroups()
{
  createSessionToolGroup();

  createExploreToolGroup();

  createRestrictToolGroup();

  createSegmentToolGroup();

  createEditToolGroup();

  createVisualizeToolGroup();

  createAnalyzeToolGroup();
}

//------------------------------------------------------------------------
// Order
//------------------------------------------------------------------------
// 0: Import
//   0: Open
//   1: Add
// 1: Export
//   0: Save
//   1: Save As
// 2: Undo/Redo
//   0: Undo
//   1: Redo
// 3: Configuration
//   0: Settings
// 4: Information
//   0: About ESPINA
// 5: Exit
//   0: Quit
//------------------------------------------------------------------------
void EspinaMainWindow::createSessionToolGroup()
{
  m_sessionToolGroup = createToolGroup(":/espina/toolgroup_session.svg", tr("Session"));

  m_openFileTool = std::make_shared<FileOpenTool>("FileOpen",  ":/espina/file_open.svg", tr("Open File"), m_context, m_errorHandler);
  m_openFileTool->setShortcut(Qt::CTRL+Qt::Key_O);
  m_openFileTool->setOrder("0-0", "1-Session");
  m_openFileTool->setCloseCallback(this);

  connect(m_openFileTool.get(), SIGNAL(analysisLoaded(AnalysisSPtr)),
          this,                 SLOT(onAnalysisLoaded(AnalysisSPtr)));

  connect(&m_autoSave,          SIGNAL(restoreFromFile(QString)),
          m_openFileTool.get(), SLOT(loadAnalysis(QString)));

  auto importTool = std::make_shared<FileOpenTool>("FileAdd", ":/espina/file_add.svg", tr("Import File"), m_context, m_errorHandler);
  importTool->setShortcut(Qt::CTRL+Qt::Key_I);
  importTool->setOrder("0-1", "1-Session");

  connect(importTool.get(), SIGNAL(analysisLoaded(AnalysisSPtr)),
          this,             SLOT(onAnalysisImported(AnalysisSPtr)));

  m_saveTool = std::make_shared<FileSaveTool>("FileSave",  ":/espina/file_save.svg", tr("Save File"), m_context, m_analysis, m_errorHandler);
  m_saveTool->setOrder("1-0", "1_FileGroup");
  m_saveTool->setShortcut(Qt::CTRL+Qt::Key_S);
  m_saveTool->setEnabled(false);

  connect(&m_autoSave,      SIGNAL(saveToFile(QString)),
          this,             SLOT(onAutoSave(QString)));

  connect(m_saveTool.get(), SIGNAL(aboutToSaveSession()),
          this,             SLOT(onAboutToSaveSession()));

  connect(m_saveTool.get(), SIGNAL(sessionSaved(const QString &, bool)),
          this,             SLOT(onSessionSaved(const QString &, bool)));

  connect(this,             SIGNAL(analysisAboutToBeClosed()),
          m_saveTool.get(), SLOT(abortTask()));

  m_saveAsTool = std::make_shared<FileSaveTool>("FileSaveAs",  ":/espina/file_save_as.svg", tr("Save File As"), m_context, m_analysis, m_errorHandler);
  m_saveAsTool->setOrder("1-1", "1_FileGroup");
  m_saveAsTool->setAlwaysAskUser(true);

  connect(m_saveAsTool.get(), SIGNAL(aboutToSaveSession()),
          this,               SLOT(onAboutToSaveSession()));

  connect(m_saveAsTool.get(), SIGNAL(sessionSaved(const QString &, bool)),
          this,               SLOT(onSessionSaved(const QString &, bool)));

  m_sessionToolGroup->addTool(m_openFileTool);
  m_sessionToolGroup->addTool(importTool);
  m_sessionToolGroup->addTool(m_saveTool);
  m_sessionToolGroup->addTool(m_saveAsTool);

  auto undo = std::make_shared<UndoTool>(m_context);
  undo->setOrder("2-0", "2-RedoUndo");

  connect(undo.get(), SIGNAL(executed()),
          this,       SIGNAL(abortOperation()));

  auto redo = std::make_shared<RedoTool>(m_context);
  redo->setOrder("2-1", "2-RedoUndo");

  connect(redo.get(), SIGNAL(executed()),
          this,       SIGNAL(abortOperation()));

  m_sessionToolGroup->addTool(undo);
  m_sessionToolGroup->addTool(redo);

  m_checkTool = std::make_shared<ProgressTool>("CheckAnalysis", ":/espina/checklist.svg", tr("Check session for issues"), m_context);
  m_checkTool->setOrder("3-0", "3-Settings");

  connect(m_checkTool.get(), SIGNAL(triggered(bool)),
          this,              SLOT(checkAnalysisConsistency()));

  m_sessionToolGroup->addTool(m_checkTool);

  auto settings = std::make_shared<ProgressTool>("Settings", ":/espina/settings.svg", tr("Settings"), m_context);
  settings->setOrder("4-0", "3-Settings");

  connect(settings.get(), SIGNAL(triggered(bool)),
          this,           SLOT(showPreferencesDialog()));

  m_sessionToolGroup->addTool(settings);

  auto about = std::make_shared<ProgressTool>("About", ":/espina/info.svg", tr("About ESPINA"), m_context);
  about->setOrder("5-0", "3-Settings");

  connect(about.get(), SIGNAL(triggered(bool)),
          this,        SLOT(showAboutDialog()));

  m_sessionToolGroup->addTool(about);

  auto exit = std::make_shared<ProgressTool>("Exit", ":/espina/exit.svg", tr("Exit ESPINA"), m_context);
  exit->setOrder("6-0", "3-Settings");
  exit->setShortcut(Qt::CTRL+Qt::Key_Q);

  connect(exit.get(), SIGNAL(triggered(bool)),
          this,       SLOT(close()));

  m_sessionToolGroup->addTool(exit);

  registerToolGroup(m_sessionToolGroup);
}

//------------------------------------------------------------------------
// Order
//------------------------------------------------------------------------
// 0: Explorers
//   0: Channels
//   1: Segmentations
// 1: Camera
//   0: Zoom Region
//   1: Reset View
// 2: Positions
//------------------------------------------------------------------------
void EspinaMainWindow::createExploreToolGroup()
{
  m_exploreToolGroup = createToolGroup(":/espina/toolgroup_explore.svg", tr("Explore"));

  auto stackExplorerSwitch = std::make_shared<PanelSwitch>("StackExplorer",
                                                           new StackExplorer(m_context, this),
                                                           ":espina/display_stack_explorer.svg",
                                                           tr("Stack Explorer"),
                                                           m_context);
  stackExplorerSwitch->setOrder("0-0", "0-Panels"); // Explorers-Channels

  auto segmentationExplorerSwitch = std::make_shared<PanelSwitch>("SegmentationExplorer",
                                                                  new SegmentationExplorer(m_filterRefiners, m_context, this),
                                                                  ":espina/display_segmentation_explorer.svg",
                                                                  tr("Segmentation Explorer"),
                                                                  m_context);
  segmentationExplorerSwitch->setOrder("0-1", "0-Panels"); // Explorers-Segmentations

  auto zoomRegion = std::make_shared<ZoomRegionTool>(m_context);
  auto resetView  = std::make_shared<ResetViewTool>(m_context);

  zoomRegion->setOrder("1-0", "1-ViewTools");
  resetView->setOrder("1-1", "1-ViewTools");

  auto bookmarksTool = std::make_shared<PositionMarksTool>(m_context, m_view->renderviews());

  bookmarksTool->setOrder("2", "1-ViewTools");

  connect(this,                SIGNAL(analysisClosed()),
          bookmarksTool.get(), SLOT(clear()));

  m_exploreToolGroup->addTool(stackExplorerSwitch);
  m_exploreToolGroup->addTool(segmentationExplorerSwitch);
  m_exploreToolGroup->addTool(zoomRegion);
  m_exploreToolGroup->addTool(resetView);
  m_exploreToolGroup->addTool(bookmarksTool);

  registerToolGroup(m_exploreToolGroup);
}

//------------------------------------------------------------------------
void EspinaMainWindow::createRestrictToolGroup()
{
  m_restrictToolGroup = new RestrictToolGroup(m_roiSettings, m_context, this);

  registerToolGroup(m_restrictToolGroup);
}

//------------------------------------------------------------------------
void EspinaMainWindow::createSegmentToolGroup()
{
  m_segmentToolGroup = createToolGroup(":/espina/toolgroup_segment.svg", tr("Segment"));

  m_segmentToolGroup->setToolTip(tr("Create New Segmentations"));

  auto manualSegment = std::make_shared<ManualSegmentTool>(m_context);
  manualSegment->setOrder("1-0", "1-SEGMENT");
  auto sgsSegment    = std::make_shared<SeedGrowSegmentationTool>(m_sgsSettings, m_filterRefiners, m_context);
  sgsSegment->setOrder("1-1", "1-SEGMENT");
  auto skeleton      = std::make_shared<SkeletonCreationTool>(m_context);
  skeleton->setOrder("1-2", "1-SEGMENT");

  m_segmentToolGroup->addTool(manualSegment);
  m_segmentToolGroup->addTool(sgsSegment);
  m_segmentToolGroup->addTool(skeleton);

  registerToolGroup(m_segmentToolGroup);
}

//------------------------------------------------------------------------
void EspinaMainWindow::createEditToolGroup()
{
  m_refineToolGroup = new EditToolGroup(m_filterRefiners, m_context, this);

  registerToolGroup(m_refineToolGroup);
}

//------------------------------------------------------------------------
// Order
//------------------------------------------------------------------------
// 0-Display (Representations)
// 0: Channels
// 1: Segmentations
// ##################
// 1-Color by
// ##################
// 2-Display (widgets)
// 0: View Settings
//   0: Scalebar
//   1: Thumbnail
// 1: Widgets
// ##################
// 3-Views
//------------------------------------------------------------------------
void EspinaMainWindow::createVisualizeToolGroup()
{
  m_visualizeToolGroup = createToolGroup(":/espina/toolgroup_visualize.svg", tr("Visualize"));

  auto scalebar = std::make_shared<GenericTogglableTool>("Scalebar",
                                                         ":/espina/display_view_scalebar.svg",
                                                         tr("Display Scalebar"),
                                                         m_context);

  connect(scalebar.get(), SIGNAL(toggled(bool)),
          m_view.get(),   SLOT(setRulerVisibility(bool)));

  scalebar->setOrder("0-0", "2-Display");

  m_visualizeToolGroup->addTool(scalebar);


  auto thumbnail = std::make_shared<GenericTogglableTool>("Thumbnail",
                                                          ":/espina/display_view_thumbnail.svg",
                                                          tr("Display Thumbnail"),
                                                          m_context);
  connect(thumbnail.get(), SIGNAL(toggled(bool)),
          m_view.get(), SLOT(showThumbnail(bool)));

  thumbnail->setOrder("0-1", "2-Display");

  m_visualizeToolGroup->addTool(thumbnail);


  auto panelSwitchXY = std::make_shared<PanelSwitch>("XZ",
                                                     m_view->panelXZ(),
                                                     ":espina/panel_xz.svg",
                                                     tr("Display XZ View"),
                                                     m_context);
  panelSwitchXY->setOrder("0","3-Views");
  m_visualizeToolGroup->addTool(panelSwitchXY);

  auto panelSwitchYZ = std::make_shared<PanelSwitch>("YZ",
                                                     m_view->panelYZ(),
                                                     ":espina/panel_yz.svg",
                                                     tr("Display YZ View"),
                                                     m_context);
  panelSwitchYZ->setOrder("1","3-Views");
  m_visualizeToolGroup->addTool(panelSwitchYZ);

  auto dialogSwitch3D = m_view->dialog3D()->tool();
  dialogSwitch3D->setOrder("2","3-Views");

  m_visualizeToolGroup->addTool(dialogSwitch3D);

  auto fullscreenSwitch = std::make_shared<FullscreenTool>(m_context, *this);
  fullscreenSwitch->setOrder("0", "4-MainWindow");

  m_visualizeToolGroup->addTool(fullscreenSwitch);

  for(auto representation: m_context.representations())
  {
    registerRepresentationSwitches(representation);
  }

  registerToolGroup(m_visualizeToolGroup);
}

//------------------------------------------------------------------------
void EspinaMainWindow::createAnalyzeToolGroup()
{
  m_analyzeToolGroup = new AnalyzeToolGroup(m_context, this);

  registerToolGroup(m_analyzeToolGroup);
}

//------------------------------------------------------------------------
ToolGroupPtr EspinaMainWindow::createToolGroup(const QString &icon, const QString &title)
{
  return new ToolGroup(icon, title, this);
}

//------------------------------------------------------------------------
void EspinaMainWindow::createDefaultPanels()
{
  auto segmentationProperties = new SegmentationProperties(m_filterRefiners, m_context, this);

  connect(this,                   SIGNAL(analysisAboutToBeClosed()),
          segmentationProperties, SLOT(reset()));

  auto segmentationPropertiesSwitch = std::make_shared<PanelSwitch>("SegmentationProperties",
                                                                    segmentationProperties,
                                                                    ":espina/display_segmentation_properties.svg",
                                                                    tr("Segmentation Properties"),
                                                                    m_context);
  segmentationPropertiesSwitch->setOrder("0", "0-PROPERTIES");

  m_refineToolGroup->addTool(segmentationPropertiesSwitch);
}

//------------------------------------------------------------------------
void EspinaMainWindow::createToolShortcuts()
{
  QList<QKeySequence> alreadyUsed;
  alreadyUsed << Qt::Key_Escape;

  for (auto tool : availableTools())
  {
    auto list = tool->shortcuts();
    for(auto sequence: list)
    {
      if(!sequence.isEmpty())
      {
        if(!alreadyUsed.contains(sequence))
        {
          auto shortcut = new QShortcut(sequence, this, 0, 0, Qt::ApplicationShortcut);

          m_toolShortcuts << shortcut;

          connect(shortcut,   SIGNAL(activated()),
                  tool.get(), SLOT(trigger()));
        }
        else
        {
          qWarning() << "Tool" << tool->id() << "tried to register a shortcut already in use:" << sequence;
        }
      }
    }
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::enableToolShortcuts(bool value)
{
  for(auto shortcut: m_toolShortcuts)
  {
    shortcut->setEnabled(value);
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::saveGeometry()
{
  ESPINA_SETTINGS(settings);

  // Instead of using save/restoreGeometry resize+move
  // Works better in Ubuntu Unity when espina is closed while is maximized
  settings.beginGroup("MainWindow");
  settings.setValue("size", size());
  settings.setValue("pos", pos());
  settings.endGroup();

  settings.setValue("state", saveState());
  settings.sync();
}

//------------------------------------------------------------------------
void EspinaMainWindow::restoreGeometry()
{
  ESPINA_SETTINGS(settings);

  // Instead of using save/restoreGeometry resize+move
  // Works better in Ubuntu Unity when espina is closed while is maximized
  settings.beginGroup("MainWindow");
  resize(settings.value("size", QSize (800, 600)).toSize());
  move  (settings.value("pos",  QPoint(200, 200)).toPoint());
  settings.endGroup();
  restoreState(settings.value("state").toByteArray());

  // Add the break after restoring the previous state
  insertToolBarBreak(m_contextualBar);
}

//------------------------------------------------------------------------
void EspinaMainWindow::registerRepresentationFactory(RepresentationFactorySPtr factory)
{
  auto representation = m_context.addRepresentation(factory);

  m_view->addRepresentation(representation);
}

//------------------------------------------------------------------------
void EspinaMainWindow::registerRepresentationSwitches(const Representation &representation)
{
  for(auto repSwitch : representation.Switches)
  {
    if(repSwitch->supportedViews().testFlag(ViewType::VIEW_2D))
    {
      m_visualizeToolGroup->addTool(repSwitch);
    }
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::checkAutoSavedAnalysis()
{
  if (m_autoSave.canRestore())
  {
    auto msg = tr("ESPINA closed unexpectedly but there is an auto-save. "
                  "If you choose to restore the auto-saved version you will be forced immediately to save it in another file. "
                  "Do you want to load the auto-saved analysis?\n");
    auto dateString = m_autoSave.autoSaveDate();
    auto timeString = m_autoSave.autoSaveTime();
    if(!dateString.isEmpty() && !timeString.isEmpty())
    {
      msg += tr("\nThe last auto-save was made the day %1 at the time %2.").arg(dateString).arg(timeString);
    }

    if (QMessageBox::Yes == DefaultDialogs::UserQuestion(msg))
    {
      m_autoSave.restore();

      // force saving in another file.
      auto saved = false;

      while(!saved)
      {
        m_saveAsTool->setSaveFilename(QDir::home().filePath("EspINA restored session.seg"));
        auto fileName = m_saveAsTool->saveFilename();
        auto fileInfo = QFileInfo{fileName};

        while(m_autoSave.isAutoSaveFile(fileName) || (fileInfo.exists() && !fileInfo.isWritable()))
        {
          fileName = m_saveAsTool->saveFilename();
          fileInfo = QFileInfo{fileName};
        }

        m_saveAsTool->setSaveFilename(fileName);
        saved = m_saveAsTool->saveAnalysis(fileName);
      }
    }
    else
    {
      m_autoSave.clear();
    }
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::updateUndoStackIndex()
{
  m_savedUndoStackIndex = m_context.undoStack()->index();
}

//------------------------------------------------------------------------
void EspinaMainWindow::stopAnalysisConsistencyCheck()
{
  if(m_checkTask)
  {
    disconnect(m_checkTask.get(), SIGNAL(finished()),
               this,              SLOT(stopAnalysisConsistencyCheck()));

    disconnect(m_checkTask.get(), SIGNAL(progress(int)),
               m_checkTool.get(), SLOT(setProgress(int)));

    m_checkTool->setProgress(100);

    disconnect(m_checkTask.get(), SIGNAL(issuesFound(Extensions::IssueList)),
               this,              SLOT(showIssuesDialog(Extensions::IssueList)));

    m_checkTask->abort();
    m_checkTask = nullptr;
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::checkAnalysisConsistency()
{
  stopAnalysisConsistencyCheck();

  m_checkTask = std::make_shared<CheckAnalysis>(m_context);

  connect(m_checkTask.get(), SIGNAL(issuesFound(Extensions::IssueList)),
          this,              SLOT(showIssuesDialog(Extensions::IssueList)));

  connect(m_checkTask.get(), SIGNAL(finished()),
          this,              SLOT(stopAnalysisConsistencyCheck()));

  connect(m_checkTask.get(), SIGNAL(progress(int)),
          m_checkTool.get(), SLOT(setProgress(int)));

  m_checkTask->submit(m_checkTask);
}

//------------------------------------------------------------------------
void EspinaMainWindow::assignActiveStack()
{
  auto model = m_context.model();

  if (!model->channels().isEmpty())
  {
    auto channel = model->channels().first().get();

    getSelection(m_context)->setActiveChannel(channel);
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::analyzeStackEdges()
{
  for (auto channel : m_context.model()->channels())
  {
    retrieveOrCreateStackExtension(channel, ChannelEdges::TYPE, m_context.factory());
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::updateToolsSettings()
{
  if(m_settings->loadSEGfileSettings())
  {
    auto settings = m_analysis->storage()->sessionSettings();

    for(auto tool: availableTools())
    {
      if(!tool->id().isEmpty() && settings->childGroups().contains(tool->id()))
      {
        settings->beginGroup(tool->id());
        SettingsContainer container;
        container.copyFrom(settings);
        settings->endGroup();

        tool->restoreSettings(container.settings());
      }
    }
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::onAboutToSaveSession()
{
  saveToolsSettings();
  m_busy = true;
}

//------------------------------------------------------------------------
void EspinaMainWindow::onSessionSaved(const QString &filename, bool success)
{
  if(success)
  {
    if (!m_autoSave.isAutoSaveFile(filename))
    {
      updateStatus(tr("File saved successfully as %1").arg(filename));

      auto file = QFileInfo{filename};
      file.refresh();
      setWindowTitle(file.fileName());

      m_saveTool->setSaveFilename(filename);
      m_saveTool->setEnabled(filename.endsWith(".seg", Qt::CaseInsensitive));

      m_saveAsTool->setSaveFilename(filename);

      m_autoSave.resetCountDown();

      updateUndoStackIndex();

      RecentDocuments recent;
      recent.addDocument(filename);
    }
    else
    {
      updateStatus(tr("Auto-save succeeded at %1").arg(QTime::currentTime().toString("HH:mm:ss")));
    }
  }

  m_busy = false;
}

//------------------------------------------------------------------------
void EspinaMainWindow::saveToolsSettings()
{
  if(!m_analysis->storage()) return;

  auto settings   = m_analysis->storage()->sessionSettings();
  auto toolgroups = QList<ToolGroupPtr>{ m_exploreToolGroup, m_restrictToolGroup, m_segmentToolGroup, m_refineToolGroup, m_visualizeToolGroup, m_analyzeToolGroup};

  for(auto tool: availableTools())
  {
    if(!tool->id().isEmpty())
    {
      SettingsContainer container;
      auto toolSettings = container.settings();
      tool->saveSettings(toolSettings);

      if(!toolSettings->allKeys().isEmpty() || !toolSettings->childGroups().isEmpty())
      {
        settings->beginGroup(tool->id());
        container.copyTo(settings);
        settings->endGroup();
      }
    }
  }

  settings->sync();
}

//------------------------------------------------------------------------
const QList<ToolGroupPtr> EspinaMainWindow::toolGroups() const
{
  return QList<ToolGroupPtr>{
    m_sessionToolGroup,
    m_exploreToolGroup,
    m_restrictToolGroup,
    m_segmentToolGroup,
    m_refineToolGroup,
    m_visualizeToolGroup,
    m_analyzeToolGroup
  };
}

//------------------------------------------------------------------------
ToolSList EspinaMainWindow::availableTools() const
{
  ToolSList availableTools;

  for (auto toolGroup : toolGroups())
  {
    for (auto tools : toolGroup->groupedTools())
    {
      availableTools << tools;
    }
  }

  return availableTools;
}

//------------------------------------------------------------------------
void EspinaMainWindow::initializeCrosshair()
{
  auto coordinateSystem = m_context.viewState().coordinateSystem();
  auto bounds           = coordinateSystem->bounds();

  m_context.viewState().setCrosshair(lowerPoint(bounds));
}
