/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include <QObject>
#include <QAction>
#include "interfaces.h"

class pqTwoDRenderView;
class QMouseEvent;

//! Interface to handle selections
class ISelectionHandler : public QAction
{
  Q_OBJECT
  
public:
  explicit ISelectionHandler(QObject* parent) : QAction(parent) {}
  virtual ~ISelectionHandler(){};

  virtual void onMouseDown(QMouseEvent *event, pqTwoDRenderView *view) = 0;
  virtual void onMouseMove(QMouseEvent *event, pqTwoDRenderView *view) = 0;
  virtual void onMouseUp(QMouseEvent *event, pqTwoDRenderView *view) = 0;
  
  virtual void abortSelection() = 0;
};


//! Singleton instance to coordinate selections through different
//! components such as views and plugins
class SelectionManager : public QObject
{
  Q_OBJECT

public:
  SelectionManager();
  ~SelectionManager(){}
  
  void onMouseDown(QMouseEvent *event, pqTwoDRenderView *view) { if (m_sh) m_sh->onMouseDown(event, view);}
  void onMouseMove(QMouseEvent *event, pqTwoDRenderView *view) { if (m_sh) m_sh->onMouseMove(event, view);}
  void onMouseUp(QMouseEvent *event, pqTwoDRenderView *view) { if (m_sh) m_sh->onMouseUp(event, view);}
  
public slots:
  //! Register @sh as active Selection Handler
  void setSelectionHandler(ISelectionHandler *sh);
  
  //! Returns a SelectionManager singleton
  static SelectionManager *instance(){return m_singleton;}
  
private:
  ISelectionHandler *m_sh;
  static SelectionManager *m_singleton;
};

#endif // SELECTIONMANAGER_H
