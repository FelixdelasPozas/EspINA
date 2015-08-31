/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "EspinaConfig.h"
#include "EspinaErrorHandler.h"
#include "RecentDocuments.h"
#include "Settings/GeneralSettings/GeneralSettings.h"
#include "Views/DefaultView.h"
#include <Core/Factory/FilterFactory.h>
#include <Core/IO/ErrorHandler.h>
#include <Dialogs/IssueList/IssueListDialog.h>
#include "ToolGroups/Visualize/VisualizeToolGroup.h"
#include "ToolGroups/Restrict/RestrictToolGroup.h"
#include "ToolGroups/Edit/EditToolGroup.h"
#include "ToolGroups/Analyze/AnalyzeToolGroup.h"
#include <Extensions/ExtensionFactory.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/ModelFactory.h>
#include <GUI/Widgets/SchedulerProgress.h>
#include <Support/Plugin.h>
#include <Support/Readers/ChannelReader.h>
#include <Support/Settings/SettingsPanel.h>
#include <Support/Widgets/Panel.h>
#include <Support/Widgets/ColorEngineSwitch.h>
#include <Support/Context.h>

// Qt
#include <QMainWindow>
#include <QTimer>
#include <QShortcut>

// C++
#include <cstdint>

class QLabel;
class QPluginLoader;
class QAction;
class QFrame;
class QUndoStack;
class QShortcut;

namespace ESPINA
{
  class SeedGrowSegmentationSettings;
  class ROISettings;
  class FileSaveTool;
  class FileSaveAsTool;

  class EspinaMainWindow
  : public QMainWindow
  {
    Q_OBJECT
    class FilterFactory;

    enum class MenuState: std::int8_t { OPEN_STATE, ADD_STATE };

  public:
    /** \brief EspinaMainWindow class constructor.
     * \param[in] plugins list of ESPINA plugins to load.
     *
     */
    explicit EspinaMainWindow(QList<QObject *> &plugins);

    /** \brief EspinaMainWindow class destructor.
     *
     */
    virtual ~EspinaMainWindow();

  signals:
    void analysisChanged();
    void analysisAboutToBeClosed();
    void analysisClosed();
    void abortOperation();

  protected:
    virtual void showEvent(QShowEvent *event) override;

    virtual void hideEvent(QHideEvent *event) override;

    virtual void closeEvent(QCloseEvent *event) override;

  private slots:
    /** \brief Updates the application when an analysis has been loaded.
     *
     */
    void onAnalysisLoaded(AnalysisSPtr analysis);

    /** \brief Saves tools settings just before saving a session.
     *
     */
    void onAboutToSaveSession();

    /** \brief Updates the application after a session has been saved.
     * \param[in] filename name of the saved session file.
     *
     */
    void onSessionSaved(const QString &filename);

    /** \brief Opens an analysis from the recent list.
     * TODO: to be moved to FileOpenTool.
     *
     */
    void openRecentAnalysis();

    /** \brief Add new data from file to current analysis.
     *
     */
    void addToAnalysis();

    /** \brief Close current analysis.
     *
     */
    bool closeCurrentAnalysis();

    /** \brief Change context bar to display tools of the selected group
     *
     */
    void activateToolGroup(ToolGroup *toolGroup);

    /** \brief Notifies tool groups that a new exclusive tool is in use
     *
     */
    void onExclusiveToolInUse(Support::Widgets::ProgressTool *tool);

    /** \brief Sets the menu state as "open".
     *
     */
    void openState()
    { m_menuState = MenuState::OPEN_STATE; }

    /** \brief Sets the menu state as "add".
     *
     */
    void addState()
    { m_menuState = MenuState::ADD_STATE; }

    /** \brief Cancels current operation.
     *
     */
    void cancelOperation();

    /** \brief Updates the tooltip of the menu.
     * \param[in] action action that contains the tooltip.
     *
     */
    void updateTooltip(QAction *action);

    /** \brief Shows the preferences dialog.
     *
     */
    void showPreferencesDialog();

    /** \brief Shows the about dialog.
     *
     */
    void showAboutDialog();

    /** \brief Invalidates the colors of the representations when the color engine changes.
     *
     */
    void onColorEngineModified();

    /** \brief Shows the issues dialog with the given issues.
     *
     */
    void showIssuesDialog(Extensions::IssueList problems) const;

  private:
    void initColorEngines();

    void createColorEngine(GUI::ColorEngines::ColorEngineSPtr engine, const QString& icon);

    void registerColorEngine(Support::Widgets::ColorEngineSwitchSPtr colorEngineSwitch);

    void initRepresentations();

    void createFileMenu(); // TODO: remove

    void createToolbars();

    void createToolGroups();

    void createToolShortcuts();

    void createSessionToolGroup();

    void createExploreToolGroup();

    void createRestrictToolGroup();

    void createSegmentToolGroup();

    void createEditToolGroup();

    void createVisualizeToolGroup();

    void createAnalyzeToolGroup();

    ToolGroupPtr createToolGroup(const QString &icon, const QString &title);

    void createDefaultPanels();

    void saveGeometry();

    void restoreGeometry();

    /** \brief Enables/disables the tool shortcuts.
     *
     */
    void enableToolShortcuts(bool value);

    /** \brief Registers representation factory
     *
     */
    void registerRepresentationFactory(RepresentationFactorySPtr factory);

    /** \brief Runs a series of test on the analysis to check for issues.
     *
     */
    void checkAnalysisConsistency();

    /** \brief Creates dynamic menu.
     * \param[in] entry pair of (QStringList, Action *) object to add.
     *
     */
    void createDynamicMenu(Support::MenuEntry entry);

    /** \brief Adds a tool group to the application.
     * \param[in] tools tool group raw pointer.
     *
     */
    void registerToolGroup(ToolGroupPtr tools);


    /** \brief Loads a list of plugins in the application.
     * \param[in] plugins list of plugins to load.
     *
     */
    void loadPlugins(QList<QObject *> &plugins);

    /** \brief Returns true if the analysis have been modified.
     *
     */
    bool isModelModified();

    /** \brief Enables/disables the application widgets.
     *
     */
    void enableWidgets(bool value);

    /** \brief Adds data from a file from the recent list to current analysis.
     *
     */
    void addRecentToAnalysis();

    /** \brief Adds a list of analysis to the current analysis.
     *      \param[in] files list of files to add.
     *
     */
    void addToAnalysis(const QStringList files);

    /** \brief Merges a list of analysis into a sigle analysis.
     * \param[in] files list of files to merge.
     *
     */
    AnalysisSPtr loadedAnalysis(const QStringList files);

    /** \brief Updates application status bar.
     * \param[in] msg message to show.
     *
     */
    void updateStatus(QString msg);

    void updateUndoStackIndex();

    void assignActiveChannel();

    void analyzeChannelEdges();

    /** \brief Updates the configuration of all the tools.
     *
     */
    void updateToolsSettings();

    /** \brief Saves the current tool settings to the session settings in the analysis.
     *
     */
    void saveToolsSettings();

    const QList<ToolGroupPtr> toolGroups() const;

    Support::Widgets::ToolSList availableTools() const;

    QString windowTitle() const
    { return tr("ESPINA"); }

    void initializeCrosshair();

  private:
    // ESPINA
    bool m_minimizedStatus;

    Support::Context               m_context;
    Support::FilterRefinerRegister m_filterRefiners;
    AnalysisSPtr                   m_analysis;

    FilterFactorySPtr  m_filterFactory;
    ChannelReaderSPtr  m_channelReader;
    AnalysisReaderSPtr m_segFileReader;

    GeneralSettingsSPtr           m_settings;
    ROISettings*                  m_roiSettings;
    SeedGrowSegmentationSettings *m_sgsSettings;

    QShortcut          m_cancelShortcut;
    QList<QShortcut *> m_toolShortcuts;

    // Menus
    QMenu           *m_addMenu;
    QAction         *m_saveAnalysisAs;
    QAction         *m_saveSessionAnalysis;
    QAction         *m_closeAnalysis;
    QMenu           *m_editMenu;

    // ToolBars
    QToolBar           *m_mainBar;
    QActionGroup        m_mainBarGroup;
    QToolBar           *m_contextualBar;
    ToolGroupPtr        m_activeToolGroup;

    ToolGroup          *m_sessionToolGroup;
    ToolGroup          *m_exploreToolGroup;
    RestrictToolGroup  *m_restrictToolGroup;
    ToolGroup          *m_segmentToolGroup;
    EditToolGroup      *m_refineToolGroup;
    VisualizeToolGroup *m_visualizeToolGroup;
    AnalyzeToolGroup   *m_analyzeToolGroup;

    ExtensionFactorySList m_extensionFactories;
    Support::Settings::SettingsPanelSList m_availableSettingsPanels;

    DefaultViewSPtr       m_view;
    SchedulerProgressSPtr m_schedulerProgress;

    std::shared_ptr<FileSaveTool> m_saveTool;
    std::shared_ptr<FileSaveAsTool> m_saveAsTool;

    RecentDocuments m_recentDocuments1;
    RecentDocuments m_recentDocuments2; // fixes duplicated actions warning in some systems

    QList<QPluginLoader *>    m_plugins;

    MenuState m_menuState;

    bool m_busy;

    struct DynamicMenuNode
    {
      explicit DynamicMenuNode();
      ~DynamicMenuNode();

      QMenu *menu;
      QList<DynamicMenuNode *> submenus;
    };
    DynamicMenuNode *m_dynamicMenuRoot;

    int       m_undoStackSavedIndex;

    EspinaErrorHandlerSPtr m_errorHandler;
  };

} // namespace ESPINA

#endif // ESPINA_MAIN_WINDOW_H
