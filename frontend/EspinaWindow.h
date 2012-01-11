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


#ifndef ESPINAWINDOW_H
#define ESPINAWINDOW_H

#include <QMainWindow>
#include <gui/DynamicWidget.h>

class MainToolBar;
class EspINA;
class pqView;
class pqPipelineSource;

#define DEBUG

#ifdef DEBUG
class ModelTest;
#endif

class EspinaWindow : public QMainWindow, public DynamicWidget
{
  Q_OBJECT
public:
    explicit EspinaWindow();
    virtual ~EspinaWindow();

public slots:
  void onConnect();
  void loadSource(pqPipelineSource *source);

  virtual void increaseLOD(){}
  virtual void decreaseLOD(){}
  virtual void setActivity(QString activity);
  virtual void setLOD(){}
  
  /// Open a new file for analysis after unloading Previous data.
  void openFile();
  /// Load new data for analysis
  void loadFile();

protected:
  void createActivityMenu();
  void createLODMenu();
  virtual void closeEvent(QCloseEvent* );
  
  void loadParaviewBehavior();


private:
  QSharedPointer<EspINA> m_espina;
  MainToolBar *m_mainToolBar;
  
  QString m_currentActivity;
#ifdef DEBUG
  QSharedPointer<ModelTest> m_modelTester;
#endif
};

#endif // ESPINAWINDOW_H
