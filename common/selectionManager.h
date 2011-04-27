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
#include <QPolygonF>

class Product;
class pqTwoDRenderView;
class QMouseEvent;

#include <assert.h>
#include <QStringList>
#include <QPair>

//! List of taxonomy ids which can be selected
//! WARNING: Special EspINA_Sample taxonomy is used to refer to the sample itself
typedef QStringList SelectionFilters;
typedef QList<QPolygon> ViewRegions;


//! Interface for Views where user can select products
class ISelectableView
{
public:
  //! Set a selection to all elements which belong to regions
  //! and pass the filtering criteria
  virtual void setSelection(SelectionFilters &filters, ViewRegions &regions) = 0;
  
  virtual pqTwoDRenderView *view() = 0;
};


//! Interface to handle selections
//! Plugin that implement this interface have to specify
//! which selection method has to be used and which type of
//! products must be selected
class ISelectionHandler
: public QObject
{
  Q_OBJECT
public:
  typedef QList<Point> VtkRegion;
  typedef QList<VtkRegion> VtkRegions;
  typedef QPair<VtkRegion, Product *> SelElement;
  typedef QList<SelElement> Selection;
  
public:
  explicit ISelectionHandler()
  : filters()
  , multiSelection(false)
  {}
  virtual ~ISelectionHandler(){};
  
  virtual void onMouseDown(QPoint &pos, ISelectableView *view) = 0;
  virtual void onMouseMove(QPoint &pos, ISelectableView *view) = 0;
  virtual void onMouseUp(QPoint &pos, ISelectableView *view) = 0;
  
  void setSelection(ISelectionHandler::Selection sel);
  void abortSelection();
  
  //! The types of products which are requested for selection
  SelectionFilters filters;
  //! Determines if multiple products can be selected or not
  bool multiSelection;
  
signals:
  void selectionChanged(ISelectionHandler::Selection);
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
  void onMouseDown(QPoint &pos, ISelectableView *view) { if (m_handler) m_handler->onMouseDown(pos, view);}
  void onMouseMove(QPoint &pos, ISelectableView *view) { if (m_handler) m_handler->onMouseMove(pos, view);}
  void onMouseUp(QPoint &pos, ISelectableView *view) { if (m_handler) m_handler->onMouseUp(pos, view);}
  
  void setSelection(ISelectionHandler::Selection sel) {if (m_handler) m_handler->setSelection(sel);}
  
public slots:
  //! Register @sh as active Selection Handler
  void setSelectionHandler(ISelectionHandler *sh);
  
  //! Returns a SelectionManager singleton
  static SelectionManager *instance(){return m_singleton;}
  
private:
  ISelectionHandler *m_handler;
  static SelectionManager *m_singleton;
};

#endif // SELECTIONMANAGER_H
