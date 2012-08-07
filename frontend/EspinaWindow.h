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
#include <gui/DynamicWidget.h>
#include <pluginInterfaces/IDynamicMenu.h>

#include "EspinaConfig.h"

#include "RecentDocuments.h"

class QAction;
class EspinaModel;
class EspinaView;
class MainToolBar;
class QFrame;
class QUndoStack;

#ifdef TEST_ESPINA_MODELS
class ModelTest;
#endif

class EspinaWindow : public QMainWindow, public DynamicWidget
{
  Q_OBJECT
public:
    explicit EspinaWindow();
    virtual ~EspinaWindow();

public slots:
  virtual void increaseLOD(){}
  virtual void decreaseLOD(){}
  virtual void setActivity(QString activity);
  virtual void setLOD(){}

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

  void openState() {m_menuState = OPEN_STATE;}
  void addState()  {m_menuState = ADD_STATE;}

protected:
  void createActivityMenu();
  void createDynamicMenu(MenuEntry entry);
  void createLODMenu();
  virtual void closeEvent(QCloseEvent* );

  void loadPlugins();

private:
  QSharedPointer<EspinaModel> m_model;
  MainToolBar                *m_mainToolBar;
  QMenu                      *m_viewMenu;
  QMenu                      *m_addMenu;
  bool                        m_busy;

  QSharedPointer<QUndoStack>  m_undoStack;
  QString                     m_currentActivity;
  EspinaView                 *m_view;
  RecentDocuments             m_recentDocuments1;
  RecentDocuments             m_recentDocuments2; // fixes duplicated actions warning in some systems
#ifdef TEST_ESPINA_MODELS
  QSharedPointer<ModelTest>   m_modelTester;
#endif
  enum MenuState {OPEN_STATE, ADD_STATE};
  MenuState m_menuState;

  struct DynamicMenuNode
  {
    QMenu *menu;
    QList<DynamicMenuNode *> submenus;
  };
  DynamicMenuNode *m_dynamicMenuRoot;
};

#endif // ESPinaModelWINDOW_H
