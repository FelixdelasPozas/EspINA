/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef ESPINA_MAIN_WINDOW_H
#define ESPINA_MAIN_WINDOW_H

#include <QMainWindow>

#include "EspinaConfig.h"
#include "EspinaErrorHandler.h"
#include "RecentDocuments.h"
#include "Settings/GeneralSettings/GeneralSettings.h"
#include "Views/DefaultView.h"
#include <Core/Factory/FilterFactory.h>
#include <Core/IO/ErrorHandler.h>
#include <Dialogs/ProblemList/ProblemListDialog.h>
#include <Extensions/ExtensionFactory.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/ModelFactory.h>
#include <GUI/Widgets/SchedulerProgress.h>
#include <Support/DockWidget.h>
#include <Support/DynamicMenu.h>
#include <Support/Readers/ChannelReader.h>
#include <Support/Settings/SettingsPanel.h>
#include <Support/ViewManager.h>

#include <QTimer>

class QLabel;
class QPluginLoader;
class QAction;
class QFrame;
class QUndoStack;
class QShortcut;

namespace EspINA
{

class MainToolBar;

  class ColorEngineMenu;

  class EspinaMainWindow
  : public QMainWindow
  {
    Q_OBJECT
    class FilterFactory;

    enum class MenuState 
    { OPEN_STATE,
      ADD_STATE
    };

  public:
    explicit EspinaMainWindow(QList<QObject *> &plugins);
    virtual ~EspinaMainWindow();

  public slots:
    bool closeCurrentAnalysis();

    void openRecentAnalysis();
    /// Close former analysis and load a new one
    void openAnalysis();

    void openAnalysis(const QStringList files);
    /// Add new data from file to current analysis

    void addToAnalysis();

    void addRecentToAnalysis();

    void addToAnalysis(const QStringList files);

    AnalysisSPtr loadedAnalysis(const QStringList files);

    /// Save Current Analysis
    void saveAnalysis();

    void saveSessionAnalysis();

  private slots:
    void updateStatus(QString msg);

    void updateTooltip(QAction *action);

    void showPreferencesDialog();

    void showAboutDialog();

    void showConnectomicsInformation();

    void showRawInformation();

    void openState() {m_menuState = MenuState::OPEN_STATE;}

    void addState()  {m_menuState = MenuState::ADD_STATE;}

    void autosave();

    void cancelOperation() {emit analysisClosed(); }

    /// undo slots
    void undoTextChanged(QString);

    void redoTextChanged(QString);

    void canRedoChanged(bool);

    void canUndoChanged(bool);

    void undoAction(bool);

    void redoAction(bool);

  signals:
    void analysisChanged();

    void analysisClosed();

    void abortOperation();

  protected:
    virtual void closeEvent(QCloseEvent* );

  private:
    ProblemList checkAnalysisConsistency();

    void createActivityMenu();

    void createDynamicMenu(MenuEntry entry);

    void checkAutosave();

    void registerDockWidget(Qt::DockWidgetArea area, DockWidget *dock);

    void registerToolGroup(ToolGroupPtr tools);

    void loadPlugins(QList<QObject *> &plugins);

    bool isModelModified();

  private:
    // EspINA
    SchedulerSPtr    m_scheduler;
    ModelFactorySPtr m_factory;
    AnalysisSPtr     m_analysis;
    ModelAdapterSPtr m_model;
    ViewManagerSPtr  m_viewManager;
    QUndoStack      *m_undoStack;

    FilterFactorySPtr  m_filterFactory;
    ChannelReaderSPtr  m_channelReader;
    AnalysisReaderSPtr m_segFileReader;

    GeneralSettingsSPtr m_settings;

    // GUI
    QMenu           *m_addMenu;
    QAction         *m_saveAnalysis;
    QAction         *m_saveSessionAnalysis;
    QAction         *m_closeAnalysis;
    QMenu           *m_viewMenu;
    ColorEngineMenu *m_colorEngines;
    QMenu           *m_dockMenu;

    QToolBar *m_mainBar;
    QToolBar *m_contextualBar;

    // UNDO
    QAction         *m_undoAction;
    QAction         *m_redoAction;

    ExtensionFactorySList m_extensionFactories;
    RendererSList         m_availableRenderers;
    SettingsPanelSList    m_availableSettingsPanels;

    MainToolBar     *m_mainToolBar;
    DefaultViewSPtr  m_view;
    SchedulerProgressSPtr m_schedulerProgress;

    RecentDocuments m_recentDocuments1;
    RecentDocuments m_recentDocuments2; // fixes duplicated actions warning in some systems

    QList<QPluginLoader *> m_plugins;

    MenuState m_menuState;

    bool m_busy;
    QShortcut *cancel;

    struct DynamicMenuNode
    {
      explicit DynamicMenuNode();
      ~DynamicMenuNode();

      QMenu *menu;
      QList<DynamicMenuNode *> submenus;
    };
    DynamicMenuNode *m_dynamicMenuRoot;

    int       m_undoStackSavedIndex;
    QTimer    m_autosave;
    QFileInfo m_sessionFile;

    // Status Bar
    EspinaErrorHandlerSPtr m_errorHandler;
  };

} // namespace EspINA

#endif // ESPINA_MAIN_WINDOW_H
