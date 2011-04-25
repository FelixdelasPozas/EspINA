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

#include "interfaces.h"

#include <QVector3D>
#include <QPolygon>

class Product;
class pqTwoDRenderView;
class QMouseEvent;

#include <assert.h>

typedef QList<QPolygon> ViewRegions;

typedef int ProductType;

enum ProductTypes //TODO: Need to be dynamic or use taxonomy value
{
    NONE   = 0
  , SAMPLE = 1
  , SEGMENTATION = 2
};

//! Interface for Views where user can select products
class ISelectableView
{
public:
  //! Set a selection to all elements which belong to regions
  //! and pass the filtering criteria
  virtual void setSelection(ViewRegions *regions) = 0;
  
protected:
  virtual pqTwoDRenderView *view() = 0;
};


//! Interface to manage selections on views
class IViewSelector
{
public:
  virtual void onMouseDown(QMouseEvent *event, ISelectableView *view) = 0;
  virtual void onMouseMove(QMouseEvent *event, ISelectableView *view) = 0;
  virtual void onMouseUp(QMouseEvent *event, ISelectableView *view) = 0;
};

//! Interface to handle selections
//! Plugin that implement this interface have to specify
//! which selection method has to be used and which type of
//! products must be selected
class SelectionHandler
: public QObject
{
  Q_OBJECT
  
public:
  typedef QList<Point> VtkRegion;
  typedef QList<VtkRegion> VtkRegions;
  typedef QList<Product *> Selection;
  
public:
  explicit SelectionHandler()
  : productType(0)
  , selectionMethod(NULL)
  , multiSelection(false)
  {}
  
  virtual ~SelectionHandler(){};
  
  void setSelection(SelectionHandler::Selection sel, SelectionHandler::VtkRegions region);
  void abortSelection();
  
  //! The types of products which are requested for selection
  ProductType productType;
  IViewSelector *selectionMethod;
  //! Determines if multiple products can be selected or not
  bool multiSelection;
  
signals:
  void selectionChanged(SelectionHandler::Selection, SelectionHandler::VtkRegions);
  void selectionAborted();
};


//! Singleton instance to coordinate selections through different
//! components such as views and plugins
class SelectionManager : public QObject
{
  Q_OBJECT

public:
  SelectionManager();
  ~SelectionManager(){}
  
  //! Delegates calls on active SelectionHandler
  void onMouseDown(QMouseEvent *event, ISelectableView *view) { if (m_handler) m_handler->selectionMethod->onMouseDown(event, view);}
  void onMouseMove(QMouseEvent *event, ISelectableView *view) { if (m_handler) m_handler->selectionMethod->onMouseMove(event, view);}
  void onMouseUp(QMouseEvent *event, ISelectableView *view) { if (m_handler) m_handler->selectionMethod->onMouseUp(event, view);}
  
  void setSelection(SelectionHandler::Selection sel, SelectionHandler::VtkRegions regions) {if (m_handler) m_handler->setSelection(sel, regions);}
  
public slots:
  //! Register @sh as active Selection Handler
  void setSelectionHandler(SelectionHandler *sh);
  
  //! Returns a SelectionManager singleton
  static SelectionManager *instance(){return m_singleton;}
  
private:
  SelectionHandler *m_handler;
  static SelectionManager *m_singleton;
};

#endif // SELECTIONMANAGER_H
