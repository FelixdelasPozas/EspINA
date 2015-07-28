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
#include "Docks/ChannelExplorer/ChannelExplorer.h"
#include "Docks/SegmentationExplorer/SegmentationExplorer.h"
#include "Docks/SegmentationInformation/SegmentationInformation.h"
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
#include <ToolGroups/Segment/SeedGrowSegmentation/SeedGrowSegmentationSettings.h>
#include <ToolGroups/Segment/SeedGrowSegmentation/SeedGrowSegmentationTool.h>
#include <ToolGroups/Segment/Manual/ManualSegmentTool.h>
#include <ToolGroups/Explore/ResetZoom.h>
#include <ToolGroups/Explore/ZoomTool.h>
#include <ToolGroups/Explore/VisualBookmarks.h>
#include <Extensions/EdgeDistances/ChannelEdges.h>
#include <GUI/ColorEngines/CategoryColorEngine.h>
#include <GUI/ColorEngines/NumberColorEngine.h>
#include <GUI/ColorEngines/UserColorEngine.h>
#include <GUI/ColorEngines/InformationColorEngine.h>
#include <GUI/Utils/DefaultIcons.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/ModelFactory.h>
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
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Support;
using namespace ESPINA::Support::Widgets;

const QString AUTOSAVE_FILE     = "espina-autosave.seg";
const int CONTEXTUAL_BAR_HEIGHT = 44;

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
, m_context(this)
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

  createMenus();

  createToolbars();

  createToolGroups();

  createDefaultPanels();

  initRepresentations();

  initColorEngines(m_viewMenu);

  loadPlugins(plugins);

  //m_colorEngineMenu->restoreUserSettings();

  createToolShortcuts();

  closeCurrentAnalysis();

  restoreGeometry();

  configureAutoSave();

  statusBar()->addPermanentWidget(m_schedulerProgress.get());
  statusBar()->clearMessage();

  activateToolGroup(m_exploreToolGroup);
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

  for(QObject *plugin : plugins)
  {
    Plugin *validPlugin = qobject_cast<Plugin*>(plugin);
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
  m_addMenu            ->setEnabled(value);
  m_saveAnalysisAs       ->setEnabled(value);
  m_saveSessionAnalysis->setEnabled(value);
  m_closeAnalysis      ->setEnabled(value);
  m_dynamicMenuRoot
    ->submenus.first()
    ->menu             ->setEnabled(value);
  m_editMenu           ->setEnabled(value);
  m_viewMenu           ->setEnabled(value);

  for (auto dock : findChildren<QDockWidget *>())
  {
    dock->setEnabled(value);
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
void EspinaMainWindow::checkAutosave()
{
  QDir autosavePath = m_settings->autosavePath();
  if (autosavePath.exists(AUTOSAVE_FILE))
  {
    QMessageBox info;
    info.setWindowTitle(tr("ESPINA"));
    info.setText(tr("ESPINA closed unexpectedly. "
                    "Do you want to load autosave file?"));
    info.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    if (QMessageBox::Yes == info.exec())
    {
      QStringList files;
      files << autosavePath.absoluteFilePath(AUTOSAVE_FILE);
      openAnalysis(files);
    }
    else
    {
      autosavePath.remove(AUTOSAVE_FILE);
    }
  }
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

  connect(this,      SIGNAL(analysisAboutToBeClosed()),
          toolGroup, SLOT(abortOperations()));
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

    QDir autosavePath = m_settings->autosavePath();
    autosavePath.remove(AUTOSAVE_FILE);
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
    warning.setWindowTitle(tr("ESPINA"));
    warning.setText(tr("Current session has not been saved. Do you want to save it now?"));
    warning.setStandardButtons(QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
    int res = warning.exec();

    switch(res)
    {
      case QMessageBox::Yes:
        saveAnalysisAs();
        break;
      case QMessageBox::Cancel:
        return false;
        break;
      default:
        break;
    }
  }

  emit analysisAboutToBeClosed();

  getSelection(m_context)->clear();
  m_context.undoStack()->clear();
  updateUndoStackIndex();

  m_context.model()->clear();
  m_analysis.reset();

  enableWidgets(false);
  enableToolShortcuts(false);

  updateSceneState(m_context.viewState(), ViewItemAdapterSList());
  m_context.viewState().resetCamera();
  m_context.viewState().refresh();

  m_sessionFile = QFileInfo();

  setWindowTitle(QString("ESPINA Interactive Neuron Analyzer"));

  m_mainBar->setEnabled(false);
  m_contextualBar->setEnabled(false);
  m_mainBar->actions().first()->setChecked(true);
  m_dynamicMenuRoot->submenus.first()->menu->setEnabled(false);

  emit analysisClosed();

  return true;
}

//------------------------------------------------------------------------
void EspinaMainWindow::openAnalysis()
{
  auto title = tr("Start New Analysis From File");
  auto selectedFiles = DefaultDialogs::OpenFiles(title, m_context.factory()->supportedFileExtensions());

  if (!selectedFiles.isEmpty())
  {
    openAnalysis(selectedFiles);
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::openAnalysis(const QStringList files)
{
  QElapsedTimer timer;
  timer.start();

  closeCurrentAnalysis();

  auto mergedAnalysis = loadedAnalysis(files);

  if (mergedAnalysis)
  {
    if (!mergedAnalysis->classification())
    {
      QFileInfo defaultClassification(":/espina/defaultClassification.xml");
      auto classification = IO::ClassificationXML::load(defaultClassification);
      mergedAnalysis->setClassification(classification);
    }

    auto model = m_context.model();

    model->setAnalysis(mergedAnalysis, m_context.factory());

    m_analysis = mergedAnalysis;

    updateSceneState(m_context.viewState(), toViewItemSList(model->channels()));
    m_context.viewState().resetCamera();

    auto bounds = m_context.viewState().coordinateSystem()->bounds();
    auto resolution = m_context.viewState().coordinateSystem()->resolution();
    NmVector3 initialCrosshair{bounds[0] + 0.5*resolution[0], bounds[2] + 0.5*resolution[1], bounds[4] + 0.5*resolution[2]};
    m_context.viewState().setCrosshair(initialCrosshair);

    int secs = timer.elapsed()/1000.0;
    int mins = 0;
    if (secs > 60)
    {
      mins = secs / 60;
      secs = secs % 60;
    }

    updateStatus(tr("File Loaded in %1m%2s").arg(mins).arg(secs));

    updateToolsSettings();

    enableWidgets(true);
    enableToolShortcuts(true);

    assignActiveChannel();

    analyzeChannelEdges();

    setWindowTitle(files.first());

    m_sessionFile = files.first();

    // We need to override the default state of the save session analysis entry
    bool enableSave = files.size() == 1 && m_sessionFile.suffix().toLower() == QString("seg");
    m_saveSessionAnalysis->setEnabled(enableSave);

    m_mainBar->setEnabled(true);
    m_contextualBar->setEnabled(true);

    if (!m_context.model()->isEmpty())
    {
      checkAnalysisConsistency();
    }
  }

  emit analysisChanged();
}

//------------------------------------------------------------------------
void EspinaMainWindow::showIssuesDialog(IssueList issues) const
{
  IssueListDialog dialog(issues);

  dialog.exec();
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
  auto title = tr("Add Data To Analysis");
  auto selectedFiles = DefaultDialogs::OpenFiles(title, m_context.factory()->supportedFileExtensions(), m_sessionFile.absoluteDir().absolutePath());

  if (!selectedFiles.isEmpty())
  {
    addToAnalysis(selectedFiles);
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

  auto model = m_context.model();

  AnalysisSPtr newAnalyses    = loadedAnalysis(files);
  AnalysisSPtr mergedAnalysis = merge(m_analysis, newAnalyses);
  model->setAnalysis(mergedAnalysis, m_context.factory());
  m_analysis = mergedAnalysis;

  int secs = timer.elapsed()/1000.0;
  int mins = 0;
  if (secs > 60)
  {
    mins = secs / 60;
    secs = secs % 60;
  }

  updateStatus(QString("File Loaded in %1m%2s").arg(mins).arg(secs));

  assignActiveChannel();

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
  QApplication::setOverrideCursor(Qt::WaitCursor);
  for(auto file : files)
  {
    m_errorHandler->setDefaultDir(QFileInfo(file).dir());

    auto readers = factory->readers(file);

    if (readers.isEmpty())
    {
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(this, tr("File Extension is not supported"), file);
      QApplication::setOverrideCursor(Qt::WaitCursor);
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

      if (file != m_settings->autosavePath().absoluteFilePath(AUTOSAVE_FILE))
      {
        m_recentDocuments1.addDocument(file);
        m_recentDocuments2.updateDocumentList();
      }
    }
    catch (...)
    {
      QApplication::restoreOverrideCursor();

      auto title   = tr("ESPINA");
      if(file != m_settings->autosavePath().absoluteFilePath(AUTOSAVE_FILE))
      {
        auto message = tr("File \"%1\" could not be loaded.\n"
                          "Do you want to remove it from recent documents list?")
                          .arg(file);

        if (DefaultDialogs::UserConfirmation(title, message))
        {
          m_recentDocuments1.removeDocument(file);
          m_recentDocuments2.updateDocumentList();
        }
      }
      else
      {
        auto message = tr("The autosave file could not be loaded.\n");
        DefaultDialogs::InformationMessage(title, message);
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
void EspinaMainWindow::saveAnalysisAs()
{
  QString suggestedFileName;
  if (m_sessionFile.suffix().toLower() == "seg")
  {
    suggestedFileName = m_sessionFile.fileName();
  }
  else
  {
    suggestedFileName = m_sessionFile.baseName() + QString(".seg");
  }

  auto analysisFile = DefaultDialogs::SaveFile(tr("Save ESPINA Analysis"),
                                               SupportedFormats().addSegFormat(),
                                               m_sessionFile.absolutePath(),
                                               "seg",
                                               suggestedFileName);

  if (analysisFile.isEmpty()) return;

  Q_ASSERT(analysisFile.toLower().endsWith(tr(".seg")));

  saveAnalysis(analysisFile);

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

  saveToolsSettings();

  IO::SegFile::save(m_analysis.get(), m_sessionFile, nullptr);

  QApplication::restoreOverrideCursor();
  updateStatus(tr("File Saved Successfuly in %1").arg(m_sessionFile.fileName()));
  m_busy = false;

  m_recentDocuments1.addDocument(m_sessionFile.absoluteFilePath());
  m_recentDocuments2.updateDocumentList();

  updateUndoStackIndex();
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
  QMenu *menu = dynamic_cast<QMenu *>(sender());
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
void EspinaMainWindow::showRawInformation()
{
  if (m_context.model()->segmentations().isEmpty())
  {
    auto title   = tr("Raw Information");
    auto message = tr("Current analysis does not contain any segmentations");

    DefaultDialogs::InformationMessage(title, message);
  }
  else
  {
    auto dialog = new RawInformationDialog(m_context);

    connect(dialog, SIGNAL(finished(int)),
            dialog, SLOT(deleteLater()));

    dialog->show();
  }
}

//------------------------------------------------------------------------
void EspinaMainWindow::cancelOperation()
{
  m_context.viewState().setEventHandler(nullptr);
  m_context.viewState().refresh();
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

  m_context.undoStack()->undo();
}

//------------------------------------------------------------------------
void EspinaMainWindow::redoAction(bool unused)
{
  emit abortOperation();

  m_context.undoStack()->redo();
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

//         auto separator = new QWidget();
//         separator->setMinimumWidth(8);
//         m_contextualBar->addWidget(separator);
        m_contextualBar->addSeparator();
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
void EspinaMainWindow::initColorEngines(QMenu *parentMenu)
{

  //m_colorEngineMenu = new ColorEngineMenu(tr("Color By"), colorEngine);

  auto colorEngine  = std::dynamic_pointer_cast<MultiColorEngine>(m_context.colorEngine());
  connect(colorEngine.get(), SIGNAL(modified()),
          this,              SLOT(onColorEngineModified()));

  //parentMenu->addMenu(m_colorEngineMenu);

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
void EspinaMainWindow::createMenus()
{
  createFileMenu();

  createReportsMenu();

  createEditMenu();

  createViewMenu();

  createSettingsMenu();
}

//------------------------------------------------------------------------
void EspinaMainWindow::createFileMenu()
{
  auto fileMenu = new QMenu(tr("File"), this);

  auto openMenu = fileMenu->addMenu(tr("&Open"));
  openMenu->setIcon(DefaultIcons::Load());
  openMenu->setToolTip(tr("Open New Analysis"));

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
  m_saveSessionAnalysis->setShortcut(Qt::CTRL+Qt::Key_S);

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
void EspinaMainWindow::createReportsMenu()
{
  auto subnode = new DynamicMenuNode();
  subnode->menu = menuBar()->addMenu(tr("Reports"));
  subnode->menu->setEnabled(false);
  m_dynamicMenuRoot->submenus << subnode;

  auto rawInformationAction = m_dynamicMenuRoot->submenus[0]->menu->addAction(tr("Raw Information"));
  connect(rawInformationAction, SIGNAL(triggered(bool)),
          this,                 SLOT(showRawInformation()));

}

//------------------------------------------------------------------------
void EspinaMainWindow::createEditMenu()
{
  m_editMenu = new QMenu(tr("Edit"), this);
  m_undoAction = new QAction(QIcon(":espina/edit-undo.svg"), tr("Undo"), this);
  m_undoAction->setEnabled(false);
  m_undoAction->setCheckable(false);
  m_undoAction->setShortcut(QString("Ctrl+Z"));
  m_editMenu->addAction(m_undoAction);

  connect(m_undoAction, SIGNAL(triggered(bool)),
          this,         SLOT(undoAction(bool)));

  m_redoAction = new QAction(QIcon(":espina/edit-redo.svg"), tr("Redo"), this);
  m_redoAction->setEnabled(false);
  m_redoAction->setCheckable(false);
  m_redoAction->setShortcut(QString("Ctrl+Shift+Z"));
  m_editMenu->addAction(m_redoAction);

  connect(m_redoAction, SIGNAL(triggered(bool)),
          this,         SLOT(redoAction(bool)));

  menuBar()->addMenu(m_editMenu);

  auto undoStack = m_context.undoStack();
  // undo connection with menu actions
  connect(undoStack, SIGNAL(canRedoChanged(bool)),
          this,      SLOT(canRedoChanged(bool)));
  connect(undoStack, SIGNAL(canUndoChanged(bool)),
          this,      SLOT(canUndoChanged(bool)));
  connect(undoStack, SIGNAL(redoTextChanged(QString)),
          this,      SLOT(redoTextChanged(QString)));
  connect(undoStack, SIGNAL(undoTextChanged(QString)),
          this,      SLOT(undoTextChanged(QString)));
}

//------------------------------------------------------------------------
void EspinaMainWindow::createViewMenu()
{
  m_viewMenu = new QMenu(tr("View"));
  m_view->createViewMenu(m_viewMenu);

  menuBar()->addMenu(m_viewMenu);

  m_viewMenu->addSeparator();
}

//------------------------------------------------------------------------
void EspinaMainWindow::createSettingsMenu()
{
  auto settingsMenu = new QMenu(tr("&Settings"));
  auto configure = new QAction(tr("&Configure ESPINA"), this);

  connect(configure, SIGNAL(triggered(bool)),
          this, SLOT(showPreferencesDialog()));
  settingsMenu->addAction(configure);

  QAction *about = new QAction(tr("About"), this);
  connect(about, SIGNAL(triggered(bool)),
          this, SLOT(showAboutDialog()));
  settingsMenu->addAction(about);

  menuBar()->addMenu(settingsMenu);
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
  m_contextualBar->setMinimumHeight(CONTEXTUAL_BAR_HEIGHT);
  m_contextualBar->setMaximumHeight(CONTEXTUAL_BAR_HEIGHT);
}

//------------------------------------------------------------------------
void EspinaMainWindow::createToolGroups()
{
  createExploreToolGroup();

  createRestrictToolGroup();

  createSegmentToolGroup();

  createRefineToolGroup();

  createVisualizeToolGroup();

  createAnalyzeToolGroup();
}

//------------------------------------------------------------------------
void EspinaMainWindow::createExploreToolGroup()
{
  m_exploreToolGroup = createToolGroup(":/espina/toolgroup_explore.svg", tr("Explore"));

  auto channelExplorerSwitch = std::make_shared<PanelSwitch>("ChannelExplorer",
                                                             new ChannelExplorer(m_context),
                                                             ":espina/channel_explorer_switch.svg",
                                                             tr("Display Channel Explorer"),
                                                             m_context);

  auto segmentationExplorerSwitch = std::make_shared<PanelSwitch>("SegmentationExplorer",
                                                                  new SegmentationExplorer(m_filterRefiners, m_context),
                                                                  ":espina/segmentation_explorer_switch.svg",
                                                                  tr("Display Segmentation Explorer"),
                                                                  m_context);

  auto zoomTool      = std::make_shared<ZoomTool>(m_context);
  auto resetZoom     = std::make_shared<ResetZoom>(m_context);
  auto bookmarksTool = std::make_shared<VisualBookmarks>(m_context, m_view->renderviews());

  connect(this,                SIGNAL(analysisClosed()),
          bookmarksTool.get(), SLOT(clear()));

  channelExplorerSwitch     ->setGroupWith("explorer");
  segmentationExplorerSwitch->setGroupWith("explorer");

  zoomTool ->setGroupWith("zoom");
  resetZoom->setGroupWith("zoom");

  bookmarksTool->setGroupWith("visual bookmarks");

  m_exploreToolGroup->addTool(channelExplorerSwitch);
  m_exploreToolGroup->addTool(segmentationExplorerSwitch);
  m_exploreToolGroup->addTool(zoomTool);
  m_exploreToolGroup->addTool(resetZoom);
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

  auto manualSegment = std::make_shared<ManualSegmentTool>(m_context);
  auto sgsSegment    = std::make_shared<SeedGrowSegmentationTool>(m_sgsSettings, m_filterRefiners, m_context);

  manualSegment->setGroupWith("manual_segment");
  sgsSegment   ->setGroupWith("sgs_segment");

  m_segmentToolGroup->addTool(manualSegment);
  m_segmentToolGroup->addTool(sgsSegment);

  registerToolGroup(m_segmentToolGroup);
}

//------------------------------------------------------------------------
void EspinaMainWindow::createRefineToolGroup()
{
  m_refineToolGroup = new RefineToolGroup(m_filterRefiners, m_context);

  registerToolGroup(m_refineToolGroup);
}

//------------------------------------------------------------------------
void EspinaMainWindow::createVisualizeToolGroup()
{
  m_visualizeToolGroup = new VisualizeToolGroup(m_context, this);

  auto panelSwitchXY = std::make_shared<PanelSwitch>("XZ",
                                                     m_view->panelXZ(),
                                                     ":espina/panel_xz.svg",
                                                     tr("Display XZ View"),
                                                     m_context);
  panelSwitchXY->setGroupWith("view_panels");
  m_visualizeToolGroup->addTool(panelSwitchXY);

  auto panelSwitchYZ = std::make_shared<PanelSwitch>("YZ",
                                                     m_view->panelYZ(),
                                                     ":espina/panel_yz.svg",
                                                     tr("Display YZ View"),
                                                     m_context);
  panelSwitchYZ->setGroupWith("view_panels");
  m_visualizeToolGroup->addTool(panelSwitchYZ);

  auto dialog3dTool = m_view->dialog3D()->tool();
  dialog3dTool->setGroupWith("view_panels");

  m_visualizeToolGroup->addTool(dialog3dTool);

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

  auto segmentationInformation       = new SegmentationInformation(m_filterRefiners, m_context);
  auto segmentationInformationSwitch = std::make_shared<PanelSwitch>("SegmentationInformation",
                                                                     segmentationInformation,
                                                                     ":espina/segmentation_information_switch.svg",
                                                                     tr("Display Segmentation Information"),
                                                                     m_context);
  m_analyzeToolGroup->addTool(segmentationInformationSwitch);
}

//------------------------------------------------------------------------
void EspinaMainWindow::createToolShortcuts()
{
  QList<QKeySequence> alreadyUsed;
  alreadyUsed << Qt::Key_Escape << Qt::CTRL+Qt::Key_S << Qt::CTRL+Qt::Key_Z << Qt::CTRL+Qt::SHIFT+Qt::Key_Z;

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
void EspinaMainWindow::configureAutoSave()
{
  m_autosave.setInterval(m_settings->autosaveInterval()*60*1000);
  m_autosave.start();

  connect(&m_autosave, SIGNAL(timeout()),
          this,        SLOT(autosave()));

  checkAutosave();
}

//------------------------------------------------------------------------
void EspinaMainWindow::registerRepresentationFactory(RepresentationFactorySPtr factory)
{
  auto representation = factory->createRepresentation(m_context, ViewType::VIEW_2D|ViewType::VIEW_3D);

  for (auto repSwitch : representation.Switches)
  {
    if(repSwitch->supportedViews().testFlag(ViewType::VIEW_2D))
    {
      m_visualizeToolGroup->addRepresentationSwitch(representation.Group, repSwitch);
    }
  }

  m_view->addRepresentation(representation);

  m_context.availableRepresentations() << factory;
}

//------------------------------------------------------------------------
void EspinaMainWindow::checkAnalysisConsistency()
{
  auto checkerTask = std::make_shared<CheckAnalysis>(m_context.scheduler(), m_context.model());

  connect(checkerTask.get(), SIGNAL(issuesFound(IssueList)),
          this,              SLOT(showIssuesDialog(IssueList)));

  checkerTask->submit(checkerTask);
}

//------------------------------------------------------------------------
void EspinaMainWindow::updateUndoStackIndex()
{
  m_undoStackSavedIndex = m_context.undoStack()->index();
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
void EspinaMainWindow::saveAnalysis(const QString &filename)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_busy = true;

  saveToolsSettings();

  IO::SegFile::save(m_analysis.get(), filename, m_errorHandler);

  QApplication::restoreOverrideCursor();
  updateStatus(tr("File Saved Successfully in %1").arg(filename));
  m_busy = false;

  m_recentDocuments1.addDocument(filename);
  m_recentDocuments2.updateDocumentList();

  updateUndoStackIndex();
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
