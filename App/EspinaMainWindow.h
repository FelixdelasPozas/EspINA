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
#include "RecentDocuments.h"

#include <Core/Interfaces/IDynamicMenu.h>
#include <Core/EspinaTypes.h>
#include <Core/Model/EspinaModel.h>

#include <QTimer>

class QAction;
class QFrame;
class QUndoStack;
class QShortcut;

#ifdef TEST_ESPINA_MODELS
class ModelTest;
#endif

namespace EspINA
{
  class ColorEngineMenu;
  class DefaultEspinaView;
  class SelectionManager;
  class GeneralSettings;
  class MainToolBar;
  class ViewManager;

  class EspinaMainWindow
  : public QMainWindow
  {
    Q_OBJECT
  public:
    explicit EspinaMainWindow(ViewManager *viewManager);
    virtual ~EspinaMainWindow();

  public slots:
    void closeCurrentAnalysis();

    void openRecentAnalysis();
    /// Close former analysis and load a new one
    void openAnalysis();
    void openAnalysis(const QString& file);
    /// Add new data from file to current analysis
    void addToAnalysis();
    void addRecentToAnalysis();
    void addFileToAnalysis(const QString &file);
    /// Save Current Analysis
    void saveAnalysis();

  protected slots:
    void updateStatus(QString msg);
    void updateTooltip(QAction *action);
    void showPreferencesDialog();
    void showAboutDialog();
    void showConnectomicsInformation();

    void openState() {m_menuState = OPEN_STATE;}
    void addState()  {m_menuState = ADD_STATE;}

    void autosave();
    void cancelOperation() {emit analysisClosed(); }

  signals:
    void analysisClosed();

  protected:
    void createActivityMenu();
    void createDynamicMenu(MenuEntry entry);
    void createLODMenu();

    void checkAutosave();

    virtual void closeEvent(QCloseEvent* );

    void loadPlugins();

  private:
    // EspINA
    EspinaFactoryPtr m_factory;
    EspinaModelSPtr   m_model;
    QUndoStack       *m_undoStack;
    ViewManager      *m_viewManager;
    GeneralSettings  *m_settings;

    // GUI
    QMenu           *m_addMenu;
    QAction         *m_saveAnalysis;
    QAction         *m_closeAnalysis;
    QMenu           *m_viewMenu;
    ColorEngineMenu *m_colorEngines;
    QMenu           *m_dockMenu;

    MainToolBar *m_mainToolBar;
    DefaultEspinaView *m_view;

    RecentDocuments m_recentDocuments1;
    RecentDocuments m_recentDocuments2; // fixes duplicated actions warning in some systems

    #ifdef TEST_ESPINA_MODELS
    QSharedPointer<ModelTest>   m_modelTester;
    #endif

    enum MenuState {OPEN_STATE, ADD_STATE};
    MenuState m_menuState;

    bool m_busy;
    QShortcut *cancel;

    struct DynamicMenuNode
    {
      QMenu *menu;
      QList<DynamicMenuNode *> submenus;
    };
    DynamicMenuNode *m_dynamicMenuRoot;

    QTimer m_autosave;
  };

} // namespace EspINA

#endif // ESPINA_MAIN_WINDOW_H