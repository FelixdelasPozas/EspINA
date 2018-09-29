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
#include <Core/Factory/FilterFactory.h>
#include <Core/IO/ErrorHandler.h>
#include <Extensions/LibraryExtensionFactory.h>
#include <Extensions/Issues/Issues.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/ModelFactory.h>
#include <GUI/Widgets/SchedulerProgress.h>
#include <Support/Plugin.h>
#include <Support/Readers/ChannelReader.h>
#include <Support/Settings/Settings.h>
#include <Support/Settings/SettingsPanel.h>
#include <Support/Widgets/Panel.h>
#include <Support/Widgets/ColorEngineSwitch.h>
#include <Support/Context.h>
#include <App/EspinaErrorHandler.h>
#include <App/Views/DefaultView.h>
#include <App/ToolGroups/Restrict/RestrictToolGroup.h>
#include <App/ToolGroups/Edit/EditToolGroup.h>
#include <App/ToolGroups/Analyze/AnalyzeToolGroup.h>
#include <App/AutoSave.h>

// Qt
#include <QApplication>
#include <QMainWindow>
#include <QShortcut>

// C++
#include <cstdint>

class QLabel;
class QPluginLoader;
class QAction;
class QFrame;
class QUndoStack;

namespace ESPINA
{
  /** \class EspinaApplication
   * \brief QApplication subclass to catch exceptions from slots/signals.
   *  Bugged in Qt 4.x but apparently works in Qt 5.x kept here for future releases.
   */
  class EspinaApplication: public QApplication
  {
    public:
      /** \brief EspinaApplication class constructor.
       * \param[in] argc Number of parameters including application name and path.
       * \param[in] argv Parameter buffers.
       *
       */
      explicit EspinaApplication(int &argc, char **argv)
      : QApplication{argc, argv}
      {};

      virtual bool notify(QObject *receiver, QEvent *e) override
      {
        try
        {
          return QApplication::notify(receiver, e);
        }
        catch(ESPINA::Core::Utils::EspinaException &e)
        {
          std::cout << "ESPINA EXCEPTION IN SLOT/SIGNAL" << std::endl;
          std::cout << e.what() << std::endl;
          std::cout << e.details() << std::endl;
          std::cout << std::flush;
        }
        catch(std::exception& e)
        {
          std::cout << "C++ EXCEPTION IN SLOT/SIGNAL" << std::endl;
          std::cout << e.what() << std::endl;
          std::cout << std::flush;
        }
        catch(...)
        {
          std::cout << "UNKNOWN EXCEPTION IN SLOT/SIGNAL" << std::endl;
          std::cout << std::flush;
        }

        return true;
      }
  };

  class SeedGrowSegmentationSettings;
  class ROISettings;
  class FileSaveTool;
  class FileOpenTool;
  class CheckAnalysis;

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

    /** \brief Close current analysis.
     *
     */
    bool closeCurrentAnalysis();

    /** \brief Opens a list of seg files.
     * \param[in] filenames list of SEG file filenames.
     *
     */
    void openAnalysis(QStringList filenames);

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
    /** \brief Replace current session analysis with the loaded one
     *
     */
    void onAnalysisLoaded(AnalysisSPtr analysis);

    /** \brief Merge loaded analysis to current session analysis
     *
     */
    void onAnalysisImported(AnalysisSPtr analysis);

    /** \brief Saves tools settings just before saving a session.
     *
     */
    void onAboutToSaveSession();

    /** \brief Updates the application after a session has been saved.
     * \param[in] filename name of the saved session file.
     * \param[in] success true if the save was successful and false otherwise.
     *
     */
    void onSessionSaved(const QString &filename, bool success);


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

    /** \brief Changes the current group to the session one when saving a file.
     *
     */
    void onAutoSave(const QString &file);

    /** \brief Runs a series of test on the analysis to check for issues.
     *
     */
    void checkAnalysisConsistency();

    /** \brief Stops the analysis check task if its running.
     *
     */
    void stopAnalysisConsistencyCheck();

    /** \brief List of actions to do after the main window have been shown on screen.
     *
     */
    void delayedInitActions();

  private:
    /** \brief Helper method to initialize the available color engines.
     *
     */
    void initColorEngines();

    /** \brief Helper method to create and register the given color engine with the given icon.
     * \param[in] engine Color engine object.
     * \param[in] icon Qicon object.
     *
     */
    void createColorEngine(GUI::ColorEngines::ColorEngineSPtr engine, const QString& icon);

    /** \brief Registers the given color engine in the current context and adds it switch to the UI.
     * \param[in] colorEngineSwitch Color engine switch button.
     *
     */
    void registerColorEngine(Support::Widgets::ColorEngineSwitchSPtr colorEngineSwitch);

    /** \brief Helper method to initialize and register the available representations.
     *
     */
    void initRepresentations();

    /** \brief Helper method to create the GUI toolbars.
     *
     */
    void createToolbars();

    /** \brief Helper method to create the GUI tool groups.
     *
     */
    void createToolGroups();

    /** \brief Helper method to register the tool shorcuts.
     *
     */
    void createToolShortcuts();

    /** \brief Helper method to create the Session tool group.
     *
     */
    void createSessionToolGroup();

    /** \brief Helper method to create the Explore tool group.
     *
     */
    void createExploreToolGroup();

    /** \brief Helper method to create the Restrict tool group.
     *
     */
    void createRestrictToolGroup();

    /** \brief Helper method to create the Segment tool group.
     *
     */
    void createSegmentToolGroup();

    /** \brief Helper method to create the Edit tool group.
     *
     */
    void createEditToolGroup();

    /** \brief Helper method to create the Visualize tool group.
     *
     */
    void createVisualizeToolGroup();

    /** \brief Helper method to create the Analyze tool group.
     *
     */
    void createAnalyzeToolGroup();

    /** \brief Helper method to create a ToolGroup object.
     * \param[in] icon Toolgroup icon.
     * \param[in] title Toolgroup name.
     *
     */
    ToolGroupPtr createToolGroup(const QString &icon, const QString &title);

    /** \brief Helper method to create the application default panels.
     *
     */
    void createDefaultPanels();

    /** \brief Helper method to register the switches of a representation.
     * \param[in] representation Representation struct.
     *
     */
    void registerRepresentationSwitches(const Representation &representation);

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

    /** \brief Asks the user if the autosave must be loaded.
     *
     */
    void checkAutoSavedAnalysis();

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

    /** \brief Updates application status bar.
     * \param[in] msg message to show.
     *
     */
    void updateStatus(QString msg);

    /** \brief Updates the value of the undo stack index backup.
     *
     */
    void updateUndoStackIndex();

    /** \brief Assigns the active stack for segmentation operations.
     *
     */
    void assignActiveStack();

    /** \brief Launches the stack edges analyzer.
     *
     */
    void analyzeStackEdges();

    /** \brief Updates the configuration of all the tools.
     *
     */
    void updateToolsSettings();

    /** \brief Saves the current tool settings to the session settings in the analysis.
     *
     */
    void saveToolsSettings();

    /** \brief Helper method that returns the list of toolgroups in the UI.
     *
     */
    const QList<ToolGroupPtr> toolGroups() const;

    /** \brief Returns the list of available tools.
     *
     */
    Support::Widgets::ToolSList availableTools() const;

    /** \brief Helper method to initialize the crosshair.
     *
     */
    void initializeCrosshair();

  private:
    // ESPINA
    bool m_minimizedStatus;

    Support::Context              m_context;
    Support::FilterRefinerFactory m_filterRefiners;
    AnalysisSPtr                  m_analysis;

    AutoSave m_autoSave;
    EspinaErrorHandlerSPtr m_errorHandler;

    FilterFactorySPtr  m_filterFactory;
    ChannelReaderSPtr  m_channelReader;
    AnalysisReaderSPtr m_segFileReader;

    Support::GeneralSettingsSPtr  m_settings;
    ROISettings*                  m_roiSettings;
    SeedGrowSegmentationSettings *m_sgsSettings;

    QShortcut          m_cancelShortcut;
    QList<QShortcut *> m_toolShortcuts;

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
    ToolGroup          *m_visualizeToolGroup;
    AnalyzeToolGroup   *m_analyzeToolGroup;

    std::shared_ptr<FileOpenTool> m_openFileTool;
    std::shared_ptr<FileSaveTool> m_saveTool;
    std::shared_ptr<FileSaveTool> m_saveAsTool;

    std::shared_ptr<Support::Widgets::ProgressTool> m_checkTool;

    Support::Settings::SettingsPanelSList m_availableSettingsPanels;

    DefaultViewSPtr       m_view;
    SchedulerProgressSPtr m_schedulerProgress;

    QList<QPluginLoader *> m_plugins;

    MenuState m_menuState;

    bool m_busy;

    int m_savedUndoStackIndex;
    std::shared_ptr<CheckAnalysis> m_checkTask; /** analysis check task. */
  };

} // namespace ESPINA

#endif // ESPINA_MAIN_WINDOW_H
