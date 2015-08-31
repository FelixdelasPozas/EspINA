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

#include "EspinaMainWindow.h"

// ESPINA
#include <Core/Analysis/Channel.h>
#include "Dialogs/About/AboutDialog.h"
#include "Dialogs/Settings/GeneralSettingsDialog.h"
#include "Dialogs/RawInformation/RawInformationDialog.h"
#include <Dialogs/View3DDialog/3DDialog.h>
#include "Panels/StackExplorer/StackExplorer.h"
#include "Panels/SegmentationExplorer/SegmentationExplorer.h"
#include "Panels/SegmentationProperties/SegmentationProperties.h"
#include "IO/SegFileReader.h"
#include "Menus/ColorEngineMenu.h"
#include "Settings/GeneralSettings/GeneralSettingsPanel.h"
#include <App/Settings/ROI/ROISettings.h>
#include <App/Settings/ROI/ROISettingsPanel.h>
#include <App/Settings/SeedGrowSegmentation/SeedGrowSegmentationSettingsPanel.h>
#include <App/Settings/Utils.h>
#include <Core/IO/ClassificationXML.h>
#include <Core/IO/SegFile.h>
#include <Core/MultiTasking/Scheduler.h>
#include <Core/Utils/AnalysisUtils.h>
#include <Core/Utils/TemporalStorage.h>
#include <Core/Utils/ListUtils.hxx>
#include <Dialogs/IssueList/CheckAnalysis.h>
#include "ToolGroups/ToolGroup.h"
#include <ToolGroups/Visualize/Representations/ChannelRepresentationFactory.h>
#include <ToolGroups/Visualize/Representations/CrosshairRepresentationFactory.h>
#include <ToolGroups/Visualize/Representations/SegmentationRepresentationFactory.h>
#include "ToolGroups/Visualize/ColorEngines/InformationColorEngineSwitch.h"
#include "ToolGroups/Visualize/GenericTogglableTool.h"
#include <ToolGroups/Segment/SeedGrowSegmentation/SeedGrowSegmentationSettings.h>
#include <ToolGroups/Segment/SeedGrowSegmentation/SeedGrowSegmentationTool.h>
#include <ToolGroups/Segment/Manual/ManualSegmentTool.h>
#include <ToolGroups/Explore/ResetViewTool.h>
#include <ToolGroups/Explore/ZoomRegionTool.h>
#include <ToolGroups/Explore/PositionMarksTool.h>
#include <ToolGroups/File/FileOpenTool.h>
#include <ToolGroups/File/FileSaveTool.h>
#include <ToolGroups/File/UndoRedoTools.h>
#include <Extensions/EdgeDistances/ChannelEdges.h>
#include <GUI/ColorEngines/CategoryColorEngine.h>
#include <GUI/ColorEngines/NumberColorEngine.h>
#include <GUI/ColorEngines/UserColorEngine.h>
#include <GUI/ColorEngines/InformationColorEngine.h>
#include <GUI/Utils/DefaultIcons.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/ModelFactory.h>
#include <GUI/Widgets/Styles.h>
#include <Support/Factory/DefaultSegmentationExtensionFactory.h>
#include <Support/Readers/ChannelReader.h>
#include <Support/Settings/EspinaSettings.h>
#include <Support/Utils/FactoryUtils.h>
#include <Support/Widgets/PanelSwitch.h>

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
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Support;
using namespace ESPINA::Support::Widgets;

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

//------------------------------------------------------------------------
EspinaMainWindow::EspinaMainWindow(QList< QObject* >& plugins)
: QMainWindow()
, m_minimizedStatus(false)
, m_context(this, &m_minimizedStatus)
, m_analysis(new Analysis())
, m_channelReader{new ChannelReader()}
, m_segFileReader{new SegFileReader()}
, m_settings     {new GeneralSettings()}
, m_roiSettings  {new ROISettings()}
, m_sgsSettings  {new SeedGrowSegmentationSettings()}
, m_cancelShortcut{Qt::Key_Escape, this, SLOT(cancelOperation()), SLOT(cancelOperation()), Qt::ApplicationShortcut}
, m_mainBarGroup {this}
, m_activeToolGroup{nullptr}
, m_view(new DefaultView(m_context, this))
, m_schedulerProgress{new SchedulerProgress(m_context.scheduler(), this)}
, m_busy{false}
, m_dynamicMenuRoot{new DynamicMenuNode()}
, m_errorHandler(new EspinaErrorHandler(this))
{
  m_dynamicMenuRoot->menu = nullptr;

  updateUndoStackIndex();

  auto factory = m_context.factory();
  factory->registerAnalysisReader(m_channelReader);
  factory->registerAnalysisReader(m_segFileReader);
  factory->registerFilterFactory (m_channelReader);

  auto defaultExtensions = std::make_shared<DefaultSegmentationExtensionFactory>();
  factory->registerExtensionFactory(defaultExtensions);

  m_availableSettingsPanels << std::make_shared<SeedGrowSegmentationsSettingsPanel>(m_sgsSettings);
  m_availableSettingsPanels << std::make_shared<ROISettingsPanel>(m_roiSettings, m_context);
#if USE_METADONA
  m_availableSettingsPanels << std::make_shared<MetaDataSettingsPanel>();
#endif

  setContextMenuPolicy(Qt::NoContextMenu);

  createToolbars();

  createToolGroups();

  createDefaultPanels();

  initRepresentations();

  initColorEngines();

  loadPlugins(plugins);

  //m_colorEngineMenu->restoreUserSettings();

  createToolShortcuts();

  closeCurrentAnalysis();

  restoreGeometry();

  statusBar()->addPermanentWidget(m_schedulerProgress.get());
  statusBar()->clearMessage();

  m_sessionToolGroup->setChecked(true);
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

  delete m_exploreToolGroup;
  delete m_restrictToolGroup;
  delete m_segmentToolGroup;
  delete m_refineToolGroup;
  delete m_visualizeToolGroup;
  delete m_analyzeToolGroup;
  delete m_roiSettings;
  //delete m_colorEngineMenu;
  delete m_dynamicMenuRoot;
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
          case ToolCategory::EXPLORE:
            m_exploreToolGroup->addTool(tool.second);
            break;
          case ToolCategory::RESTRICT:
            m_restrictToolGroup->addTool(tool.second);
            break;
          case ToolCategory::SEGMENT:
            m_segmentToolGroup->addTool(tool.second);
            break;
          case ToolCategory::REFINE:
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
        qDebug() << plugin << "- Filter Factory  ...... OK";
        factory->registerFilterFactory(filterFactory);
      }

      for(auto reader: validPlugin->analysisReaders())
      {
        qDebug() << plugin << "- Analysis Reader  ...... OK";
        factory->registerAnalysisReader(reader);
      }

      for (auto extensionFactory : validPlugin->segmentationExtensionFactories())
      {
//        qDebug() << plugin << "- Segmentation Extension Factory  ...... OK";
        factory->registerExtensionFactory(extensionFactory);
      }

      for (auto report : validPlugin->reports())
      {
//        qDebug() << plugin << "- Register Reprot" << report->name() << " ...... OK";
        m_analyzeToolGroup->registerReport(report);
      }

      for (auto settings : validPlugin->settingsPanels())
      {
//        qDebug() << plugin << "- Settings Panel " << settings->windowTitle() << " ...... OK";
        m_availableSettingsPanels << settings;
      }

      for (auto factory : validPlugin->representationFactories())
      {
//        qDebug() << plugin << "- Renderers " << renderer->name() << " ...... OK";
        registerRepresentationFactory(factory);
      }

      for(auto entry: validPlugin->menuEntries())
      {
//        qDebug() << plugin << "- Menu Entries " << entry;
        createDynamicMenu(entry);
      }
    }
  }
}

//------------------------------------------------------------------------
bool EspinaMainWindow::isModelModified()
{
  return m_context.undoStack()->index() != m_undoStackSavedIndex;
}

//------------------------------------------------------------------------
void EspinaMainWindow::enableWidgets(bool value)
{
  for (auto dock : findChildren<QDockWidget *>())
  {
    dock->setEnabled(value);
  }

  auto sessionTools = m_sessionToolGroup->groupedTools();
  sessionTools[0][1]->setEnabled(value);
  sessionTools[1][1]->setEnabled(value);

  for(auto action: m_mainBar->actions())
  {
    if(action == m_mainBar->actions().first()) continue;

    action->setEnabled(value);
  }

  centralWidget()->setEnabled(value);
}

//------------------------------------------------------------------------
void EspinaMainWindow::createDynamicMenu(MenuEntry entry)
{
  auto node = m_dynamicMenuRoot;

  for(int i=0; i < entry.first.size(); i++)
  {
    auto entryName = entry.first[i];

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
      auto subnode = new DynamicMenuNode();
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
void EspinaMainWindow::registerToolGroup(ToolGroupPtr toolGroup)
{
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
    QMessageBox warning;
    warning.setWindowTitle(tr("ESPINA"));
    warning.setText(tr("ESPINA has pending actions. Do you really want to quit anyway?"));
    if (QMessageBox::Ok != warning.exec())
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

  removeTemporalDirectory();
}

//------------------------------------------------------------------------
bool EspinaMainWindow::closeCurrentAnalysis()
{
  if (isModelModified())
  {
    QMessageBox warning;
    warning.setWindowTitle(windowTitle());
    warning.setText(tr("Current session has not been saved. Do you want to save it now?"));
    warning.setStandardButtons(QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
    int res = warning.exec();

    switch(res)
    {
      case QMessageBox::Yes:
        m_saveAsTool->trigger();
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

  m_saveTool->updateUndoStackIndex();
  m_saveTool->setSessionFile(QString());

  m_context.model()->clear();
  m_analysis.reset();

  enableWidgets(false);
  enableToolShortcuts(false);

  updateSceneState(m_context.viewState(), ViewItemAdapterSList());
  m_context.viewState().resetCamera();
  m_context.viewState().refresh();

  setWindowTitle("ESPINA Interactive Neuron Analyzer");

  emit analysisClosed();

  return true;
}

//------------------------------------------------------------------------
void EspinaMainWindow::onAnalysisLoaded(AnalysisSPtr analysis)
{
  if(!closeCurrentAnalysis()) return;

  Q_ASSERT(analysis);

  if(!analysis->classification())
  {
    QFileInfo defaultClassification(":/espina/defaultClassification.xml");
    auto classification = IO::ClassificationXML::load(defaultClassification);
    analysis->setClassification(classification);
  }

  auto model = m_context.model();

  model->setAnalysis(analysis, m_context.factory());

  m_analysis = analysis;

  updateSceneState(m_context.viewState(), toViewItemSList(model->channels()));
  m_context.viewState().resetCamera();

  initializeCrosshair();

  updateToolsSettings();

  enableWidgets(true);

  enableToolShortcuts(true);

  assignActiveChannel();

  analyzeChannelEdges();

  auto tool = qobject_cast<FileOpenTool *>(sender());
  Q_ASSERT(tool);
  auto files = tool->files();

  setWindowTitle(files.first());

  m_saveTool->setSessionFile(files.first());
  m_saveAsTool->setSessionFile(files.first());
  auto saveEnabled = files.size() == 1 && files.first().endsWith(".seg", Qt::CaseInsensitive);
  m_saveTool->setEnabled(saveEnabled);

  if(!m_context.model()->isEmpty())
  {
    checkAnalysisConsistency();
  }

  emit analysisChanged();
}

////------------------------------------------------------------------------
//void EspinaMainWindow::openAnalysis()
//{
//  auto title = tr("Start New Analysis From File");
//  auto selectedFiles = DefaultDialogs::OpenFiles(title, m_context.factory()->supportedFileExtensions());
//
//  if (!selectedFiles.isEmpty())
//  {
//    openAnalysis(selectedFiles);
//  }
//}
//
////------------------------------------------------------------------------
//void EspinaMainWindow::openAnalysis(const QStringList files)
//{
//  if (!closeCurrentAnalysis()) return;
//
//  QElapsedTimer timer;
//  timer.start();
//
//  auto mergedAnalysis = loadedAnalysis(files);
//
//  if (mergedAnalysis)
//  {
//    if (!mergedAnalysis->classification())
//    {
//      QFileInfo defaultClassification(":/espina/defaultClassification.xml");
//      auto classification = IO::ClassificationXML::load(defaultClassification);
//      mergedAnalysis->setClassification(classification);
//    }
//
//    auto model = m_context.model();
//
//    model->setAnalysis(mergedAnalysis, m_context.factory());
//
//    m_analysis = mergedAnalysis;
//
//    updateSceneState(m_context.viewState(), toViewItemSList(model->channels()));
//    m_context.viewState().resetCamera();
//
//    initializeCrosshair();
//
//    int secs = timer.elapsed()/1000.0;
//    int mins = 0;
//    if (secs > 60)
//    {
//      mins = secs / 60;
//      secs = secs % 60;
//    }
//
//    m_mainBar->setEnabled(true);
//    m_contextualBar->setEnabled(true);
//
//    updateStatus(tr("File Loaded in %1m%2s").arg(mins).arg(secs));
//
//    updateToolsSettings();
//
//    enableWidgets(true);
//
//    enableToolShortcuts(true);
//
//    assignActiveChannel();
//
//    analyzeChannelEdges();
//
//    setWindowTitle(files.first());
//
//    m_sessionFile = files.first();
//
//    // We need to override the default state of the save session analysis entry
//    bool enableSave = files.size() == 1 && m_sessionFile.suffix().toLower() == QString("seg");
//    m_saveSessionAnalysis->setEnabled(enableSave);
//
//    if (!m_context.model()->isEmpty())
//    {
//      checkAnalysisConsistency();
//    }
//  }
//
//  emit analysisChanged();
//}

//------------------------------------------------------------------------
void EspinaMainWindow::showIssuesDialog(IssueList issues) const
{
  IssueListDialog dialog(issues);

  dialog.exec();
}

//------------------------------------------------------------------------
void EspinaMainWindow::openRecentAnalysis()
{
  // TODO: recent files
//  QAction *action = qobject_cast<QAction *>(sender());
//
//  if (action && !action->data().isNull())
//  {
//    QStringList files(action->data().toString());
//    if (MenuState::OPEN_STATE == m_menuState)
//    {
//      openAnalysis(files);
//    }
//    else
//    {
//      addToAnalysis(files);
//    }
//  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::addToAnalysis()
{
  auto title = tr("Add Data To Analysis");
  auto selectedFiles = DefaultDialogs::OpenFiles(title, m_context.factory()->supportedFileExtensions(), QDir::currentPath());

  if (!selectedFiles.isEmpty())
  {
    addToAnalysis(selectedFiles);
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::addRecentToAnalysis()
{
  auto action = qobject_cast<QAction *>(sender());

  if (!action || action->data().isNull()) return;

  QStringList files(action->data().toString());

  addToAnalysis(files);
}

//------------------------------------------------------------------------
void EspinaMainWindow::addToAnalysis(const QStringList files)
{
  QElapsedTimer timer;
  timer.start();

  auto model = m_context.model();

  emit abortOperation();
 
  auto newAnalyses    = loadedAnalysis(files);
  auto mergedAnalysis = merge(m_analysis, newAnalyses);

  model->setAnalysis(mergedAnalysis, m_context.factory());

  m_analysis = mergedAnalysis;

  assignActiveChannel();

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

  auto factory = m_context.factory();
  Styles::WaitingCursor cursor;
  for(auto file : files)
  {
    m_errorHandler->setDefaultDir(QFileInfo(file).dir());

    auto readers = factory->readers(file);

    if (readers.isEmpty())
    {
      Styles::DefaultCursor cursor;

      QMessageBox::warning(this, tr("File Extension is not supported"), file);
      continue;
    }

    auto reader = readers.first();

    if (readers.size() > 1)
    {
      //TODO 2015-04-20: show reader selection dialog
    }

    try
    {
      analyses << factory->read(reader, file, m_errorHandler);

      if (file != m_settings->autosavePath().absoluteFilePath(GeneralSettings::AUTOSAVE_FILE))
      {
        m_recentDocuments1.addDocument(file);
        m_recentDocuments2.updateDocumentList();
      }
    }
    catch (...)
    {
      Styles::DefaultCursor cursor;

      if(file != m_settings->autosavePath().absoluteFilePath(GeneralSettings::AUTOSAVE_FILE))
      {
        auto message = tr("File \"%1\" could not be loaded.\n"
                          "Do you want to remove it from recent documents list?")
                          .arg(file);

        if (DefaultDialogs::UserConfirmation(windowTitle(), message))
        {
          m_recentDocuments1.removeDocument(file);
          m_recentDocuments2.updateDocumentList();
        }
      }
      else
      {
        auto message = tr("The autosave file could not be loaded.\n");
        DefaultDialogs::InformationMessage(windowTitle(), message);
      }
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

  return mergedAnalysis;
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

  dialog.registerPanel(std::make_shared<GeneralSettingsPanel>(m_settings));
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
void EspinaMainWindow::cancelOperation()
{
  m_context.viewState().setEventHandler(nullptr);
  m_context.viewState().refresh();
}

//------------------------------------------------------------------------
void EspinaMainWindow::onColorEngineModified()
{
  auto  segmentations   = m_context.model()->segmentations();
  auto  invalidateItems = toRawList<ViewItemAdapter>(segmentations);
  auto &invalidator     = m_context.representationInvalidator();

  invalidator.invalidateRepresentationColors(invalidateItems);
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

//         auto separator = new QWidget();
//         separator->setMinimumWidth(8);
//         m_contextualBar->addWidget(separator);
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
  auto colorEngine  = std::dynamic_pointer_cast<MultiColorEngine>(m_context.colorEngine());
  connect(colorEngine.get(), SIGNAL(modified()),
          this,              SLOT(onColorEngineModified()));

  createColorEngine(std::make_shared<NumberColorEngine>(), "number");

  auto categoryColorEngine = std::make_shared<CategoryColorEngine>();
  categoryColorEngine->setActive(true);
  createColorEngine(categoryColorEngine, "category");

  registerColorEngine(std::make_shared<InformationColorEngineSwitch>(m_context));
  //registerColorEngine(tr("User"), std::make_shared<UserColorEngine>());
}


//------------------------------------------------------------------------
void EspinaMainWindow::registerColorEngine(ColorEngineSwitchSPtr colorEngineSwitch)
{
  auto colorEngine = colorEngineSwitch->colorEngine();
  //m_colorEngineMenu->addColorEngine(colorEngine->tooltip(), colorEngine);
  m_context.colorEngine()->add(colorEngine);

  m_visualizeToolGroup->addTool(colorEngineSwitch);
}

//------------------------------------------------------------------------
void EspinaMainWindow::initRepresentations()
{
  registerRepresentationFactory(std::make_shared<CrosshairRepresentationFactory>());
  registerRepresentationFactory(std::make_shared<ChannelRepresentationFactory>());
  registerRepresentationFactory(std::make_shared<SegmentationRepresentationFactory>());
}

//------------------------------------------------------------------------
void EspinaMainWindow::createFileMenu()
{
  auto fileMenu = new QMenu(tr("File"), this);

  auto openMenu = fileMenu->addMenu(tr("&Open"));
  openMenu->setIcon(DefaultIcons::Load());
  openMenu->setToolTip(tr("Open A Stack Or A New Analysis"));

  auto openAction = new QAction(DefaultIcons::File(), tr("&File"),this);

  openMenu->addAction(openAction);
  openMenu->addSeparator();
  openMenu->addActions(m_recentDocuments1.list());

  for (int i = 0; i < m_recentDocuments1.list().size(); i++)
  {
    connect(m_recentDocuments1.list()[i], SIGNAL(triggered()),
            this,                         SLOT(openRecentAnalysis()));
  }

  connect(openMenu, SIGNAL(aboutToShow()),
          this,     SLOT(openState()));
  connect(openMenu, SIGNAL(hovered(QAction*)),
          this,     SLOT(updateTooltip(QAction*)));
  connect(openAction, SIGNAL(triggered(bool)),
          this,       SLOT(openAnalysis()));

  m_addMenu = fileMenu->addMenu(tr("&Add"));
  m_addMenu->setIcon(QIcon(":espina/add.svg"));
  m_addMenu->setToolTip(tr("Add File to Analysis"));
  m_addMenu->setEnabled(false);

  auto addAction = new QAction(DefaultIcons::File(), tr("&File"),this);

  m_addMenu->addAction(addAction);
  m_addMenu->addSeparator();
  m_addMenu->addActions(m_recentDocuments2.list());

  for (int i = 0; i < m_recentDocuments2.list().size(); i++)
  {
    connect(m_recentDocuments2.list()[i], SIGNAL(triggered()),
            this,                         SLOT(openRecentAnalysis()));
  }

  connect(m_addMenu, SIGNAL(aboutToShow()),
          this,      SLOT(addState()));
  connect(addAction, SIGNAL(triggered(bool)),
          this,      SLOT(addToAnalysis()));

  m_saveSessionAnalysis = fileMenu->addAction(DefaultIcons::Save(), tr("&Save"));
  m_saveSessionAnalysis->setEnabled(false);

  connect(m_saveSessionAnalysis, SIGNAL(triggered(bool)),
          this,                  SLOT(saveSessionAnalysis()));

  m_saveAnalysisAs = fileMenu->addAction(DefaultIcons::Save(), tr("Save &As..."));
  m_saveAnalysisAs->setEnabled(false);

  connect(m_saveAnalysisAs, SIGNAL(triggered(bool)),
          this,             SLOT(saveAnalysisAs()));


  m_closeAnalysis = fileMenu->addAction(tr("&Close"));
  m_closeAnalysis->setEnabled(false);
  connect(m_closeAnalysis, SIGNAL(triggered(bool)),
          this,            SLOT(closeCurrentAnalysis()));

  auto exit = fileMenu->addAction(tr("&Exit"));
  connect(exit, SIGNAL(triggered(bool)),
          this, SLOT(close()));

  connect(fileMenu, SIGNAL(triggered(QAction*)),
          this,     SLOT(openRecentAnalysis()));

  menuBar()->addMenu(fileMenu);
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
  m_sessionToolGroup = createToolGroup(":/espina/toolgroup_file.svg", tr("Session"));

  auto open = std::make_shared<FileOpenTool>(m_context, m_errorHandler, m_settings);
  open->setShortcut(Qt::CTRL+Qt::Key_O);
  open->setOrder("0-0", "1-Session");

  connect(open.get(), SIGNAL(analysisLoaded(AnalysisSPtr)),
          this,       SLOT(onAnalysisLoaded(AnalysisSPtr)));

  auto add = std::make_shared<ProgressTool>("FileAdd", DefaultIcons::File(), tr("Add A Stack Or A Previous Session To The Current Session"), m_context);
  add->setShortcut(Qt::CTRL+Qt::Key_A);
  add->setOrder("0-1", "1-Session");

  connect(add.get(), SIGNAL(triggered(bool)),
          this,       SLOT(addToAnalysis()));

  m_saveTool = std::make_shared<FileSaveTool>(m_context, m_analysis, m_errorHandler, m_settings);
  m_saveTool->setOrder("1-0", "1_FileGroup");
  m_saveTool->setShortcut(Qt::CTRL+Qt::Key_S);

  connect(m_saveTool.get(), SIGNAL(aboutToSaveSession()),
          this,             SLOT(onAboutToSaveSession()));

  connect(m_saveTool.get(), SIGNAL(sessionSaved(const QString &)),
          this,             SLOT(onSessionSaved(const QString &)));

  m_saveAsTool = std::make_shared<FileSaveAsTool>(m_context, m_analysis, m_errorHandler, m_settings);
  m_saveAsTool->setOrder("1-1", "1_FileGroup");

  connect(m_saveAsTool.get(), SIGNAL(aboutToSaveSession()),
          this,               SLOT(onAboutToSaveSession()));

  connect(m_saveAsTool.get(), SIGNAL(sessionSaved(const QString &)),
          this,               SLOT(onSessionSaved(const QString &)));

  m_sessionToolGroup->addTool(open);
  m_sessionToolGroup->addTool(add);
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

  auto settings = std::make_shared<ProgressTool>("FileSettings", ":/espina/settings.svg", tr("Settings"), m_context);
  settings->setOrder("3-0", "3-Settings");

  connect(settings.get(), SIGNAL(triggered(bool)),
          this,           SLOT(showPreferencesDialog()));

  m_sessionToolGroup->addTool(settings);

  auto about = std::make_shared<ProgressTool>("FileAbout", ":/espina/espina.svg", tr("About ESPINA"), m_context);
  about->setOrder("4-0", "3-Settings");

  connect(about.get(), SIGNAL(triggered(bool)),
          this,        SLOT(showAboutDialog()));

  m_sessionToolGroup->addTool(about);

  auto exit = std::make_shared<ProgressTool>("ExitApplication", ":/espina/exit-app.svg", tr("Exit ESPINA"), m_context);
  exit->setOrder("5-0", "3-Settings");
  exit->setShortcut(Qt::CTRL+Qt::Key_Q);

  connect(exit.get(), SIGNAL(triggered(bool)),
          this,       SLOT(close()));

  //m_sessionToolGroup->addTool(exit);

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
                                                           new StackExplorer(m_context),
                                                           ":espina/display_stack_explorer.svg",
                                                           tr("Stack Explorer"),
                                                           m_context);
  stackExplorerSwitch->setOrder("0-0"); // Explorers-Channels

  auto segmentationExplorerSwitch = std::make_shared<PanelSwitch>("SegmentationExplorer",
                                                                  new SegmentationExplorer(m_filterRefiners, m_context),
                                                                  ":espina/display_segmentation_explorer.svg",
                                                                  tr("Segmentation Explorer"),
                                                                  m_context);
  segmentationExplorerSwitch->setOrder("0-1"); // Explorers-Segmetnations

  auto zoomRegion = std::make_shared<ZoomRegionTool>(m_context);
  auto resetView  = std::make_shared<ResetViewTool>(m_context);

  zoomRegion->setOrder("1-0");
  resetView->setOrder("1-1");

  auto bookmarksTool = std::make_shared<PositionMarksTool>(m_context, m_view->renderviews());

  bookmarksTool->setOrder("2");

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
  m_restrictToolGroup = new RestrictToolGroup(m_roiSettings, m_context);

  registerToolGroup(m_restrictToolGroup);
}

//------------------------------------------------------------------------
void EspinaMainWindow::createSegmentToolGroup()
{
  m_segmentToolGroup = createToolGroup(":/espina/toolgroup_segment.svg", tr("Segment"));

  m_segmentToolGroup->setToolTip(tr("Create New Segmentations"));

  auto manualSegment = std::make_shared<ManualSegmentTool>(m_context);
  auto sgsSegment    = std::make_shared<SeedGrowSegmentationTool>(m_sgsSettings, m_filterRefiners, m_context);

  m_segmentToolGroup->addTool(manualSegment);
  m_segmentToolGroup->addTool(sgsSegment);

  registerToolGroup(m_segmentToolGroup);
}

//------------------------------------------------------------------------
void EspinaMainWindow::createEditToolGroup()
{
  m_refineToolGroup = new EditToolGroup(m_filterRefiners, m_context);

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
//   0: Thumbnail
//   1: Scalebar
// 1: Widgets
// ##################
// 3-Views
//------------------------------------------------------------------------
void EspinaMainWindow::createVisualizeToolGroup()
{
  m_visualizeToolGroup = new VisualizeToolGroup(m_context, this);

  auto thumbnail = std::make_shared<GenericTogglableTool>("Thumbnail",
                                                          ":/espina/display_view_thumbnail.svg",
                                                          tr("Display Thumbnail"),
                                                          m_context);
  connect(thumbnail.get(), SIGNAL(toggled(bool)),
          m_view.get(), SLOT(showThumbnail(bool)));

  thumbnail->setOrder("0-0", "2-Display");
  thumbnail->setChecked(true);

  m_visualizeToolGroup->addTool(thumbnail);

  auto scalebar = std::make_shared<GenericTogglableTool>("Scalebar",
                                                         ":/espina/display_view_scalebar.svg",
                                                         tr("Display Scalebar"),
                                                         m_context);

  connect(scalebar.get(), SIGNAL(toggled(bool)),
          m_view.get(),   SLOT(setRulerVisibility(bool)));

  scalebar->setOrder("0-1", "2-Display");
  scalebar->setChecked(true);

  m_visualizeToolGroup->addTool(scalebar);


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

  auto dialogSwith3D = m_view->dialog3D()->tool();
  dialogSwith3D->setOrder("2","3-Views");

  m_visualizeToolGroup->addTool(dialogSwith3D);

  registerToolGroup(m_visualizeToolGroup);
}

//------------------------------------------------------------------------
void EspinaMainWindow::createAnalyzeToolGroup()
{
  m_analyzeToolGroup = new AnalyzeToolGroup(m_context);

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

  auto segmentationProperties       = new SegmentationProperties(m_filterRefiners, m_context);
  auto segmentationPropertiesSwitch = std::make_shared<PanelSwitch>("SegmentationProperties",
                                                                    segmentationProperties,
                                                                    ":espina/display_segmentation_properties.svg",
                                                                    tr("Segmentation Properties"),
                                                                    m_context);
  segmentationPropertiesSwitch->setOrder("0");

  m_refineToolGroup->addTool(segmentationPropertiesSwitch);
}

//------------------------------------------------------------------------
void EspinaMainWindow::createToolShortcuts()
{
  QList<QKeySequence> alreadyUsed;
  alreadyUsed << Qt::Key_Escape;

  for (auto tool : availableTools())
  {
    auto sequence = tool->shortcut();
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

  // Instead of using save/restoreGeometry resice+move
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
  auto representation = factory->createRepresentation(m_context, ViewType::VIEW_2D|ViewType::VIEW_3D);

  for (auto repSwitch : representation.Switches)
  {
    if(repSwitch->supportedViews().testFlag(ViewType::VIEW_2D))
    {
      m_visualizeToolGroup->addRepresentationSwitch(repSwitch);
    }
  }

  m_view->addRepresentation(representation);

  m_context.availableRepresentations() << factory;
}

//------------------------------------------------------------------------
void EspinaMainWindow::updateUndoStackIndex()
{
  m_undoStackSavedIndex = m_context.undoStack()->index();
}

//------------------------------------------------------------------------
void EspinaMainWindow::checkAnalysisConsistency()
{
  auto checkerTask = std::make_shared<CheckAnalysis>(m_context.scheduler(), m_context.model());

  connect(checkerTask.get(), SIGNAL(issuesFound(Extensions::IssueList)),
          this,              SLOT(showIssuesDialog(Extensions::IssueList)));

  checkerTask->submit(checkerTask);
}

//------------------------------------------------------------------------
void EspinaMainWindow::assignActiveChannel()
{
  auto model = m_context.model();

  if (!model->channels().isEmpty())
  {
    auto channel = model->channels().first().get();

    getSelection(m_context)->setActiveChannel(channel);
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::analyzeChannelEdges()
{
  for (auto channel : m_context.model()->channels())
  {
    auto extensions = channel->extensions();

    if (!extensions->hasExtension(ChannelEdges::TYPE))
    {
      extensions->add(std::make_shared<ChannelEdges>(m_context.scheduler()));
    }
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
void EspinaMainWindow::onSessionSaved(const QString &fileName)
{
  if (fileName != m_settings->autosavePath().absoluteFilePath(GeneralSettings::AUTOSAVE_FILE))
  {
    updateStatus(tr("File Saved Successfully in %1").arg(fileName));

    m_recentDocuments1.addDocument(fileName);
    m_recentDocuments2.updateDocumentList();

    QStringList fileParts = fileName.split(QDir::separator());
    setWindowTitle(fileParts[fileParts.size()-1]);

    m_saveTool->setSessionFile(fileName);
  }

  m_busy = false;
}

//------------------------------------------------------------------------
void EspinaMainWindow::saveToolsSettings()
{
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
