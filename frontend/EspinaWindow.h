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


#ifndef ESPinaModelWINDOW_H
#define ESPinaModelWINDOW_H

#include <QMainWindow>

#include "EspinaConfig.h"
#include <pluginInterfaces/IDynamicMenu.h>
#include "RecentDocuments.h"

#include <QTimer>

class ColorEngineMenu;
class DefaultEspinaView;
class SelectionManager;
class GeneralSettings;
class EspinaFactory;
class QAction;
class EspinaModel;
class MainToolBar;
class QFrame;
class QUndoStack;
class ViewManager;

#ifdef TEST_ESPINA_MODELS
class ModelTest;
#endif

class EspinaWindow
: public QMainWindow
{
  Q_OBJECT
public:
    explicit EspinaWindow();
    virtual ~EspinaWindow();

public slots:
  void closeCurrentAnalysis();

  void openRecentAnalysis();
  /// Close former analysis and load a new one
  void openAnalysis();
  void openAnalysis(const QString file);
  /// Add new data from file to current analysis
  void addToAnalysis();
  void addRecentToAnalysis();
  void addFileToAnalysis(const QString file);
  /// Save Current Analysis
  void saveAnalysis();

protected slots:
  void updateStatus(QString msg);
  void updateTooltip(QAction *action);
  void showPreferencesDialog();
  void showAboutDialog();

  void openState() {m_menuState = OPEN_STATE;}
  void addState()  {m_menuState = ADD_STATE;}

  void autosave();

signals:
  void analysisClosed();

protected:
  void createActivityMenu();
  void createDynamicMenu(MenuEntry entry);
  void createLODMenu();
  virtual void closeEvent(QCloseEvent* );

  void loadPlugins();

private:
  QMenu           *m_viewMenu;
  ColorEngineMenu *m_colorEngines;
  QMenu           *m_dockMenu;
  QMenu           *m_addMenu;

  MainToolBar *m_mainToolBar;
  DefaultEspinaView *m_view;

  EspinaFactory    *m_factory;
  EspinaModel      *m_model;
  QUndoStack       *m_undoStack;
  ViewManager      *m_viewManager;
  GeneralSettings  *m_settings;

  RecentDocuments m_recentDocuments1;
  RecentDocuments m_recentDocuments2; // fixes duplicated actions warning in some systems

#ifdef TEST_ESPINA_MODELS
  QSharedPointer<ModelTest>   m_modelTester;
#endif

  enum MenuState {OPEN_STATE, ADD_STATE};
  MenuState m_menuState;

  bool m_busy;

  struct DynamicMenuNode
  {
    QMenu *menu;
    QList<DynamicMenuNode *> submenus;
  };
  DynamicMenuNode *m_dynamicMenuRoot;
  QTimer m_autosave;
};

#endif // ESPinaModelWINDOW_H