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
#include "filter.h"

#include <QVector3D>
#include <QPolygonF>

class pqProxy;
class EspinaProduct;
class pqRenderView;
class QMouseEvent;
class IVOI;
class pq3DWidget;

#include <assert.h>
#include <QStringList>
#include <QPair>

//! List of taxonomy ids which can be selected
//! WARNING: Special EspINA_Sample taxonomy is used to refer to the sample itself
typedef QStringList SelectionFilters;
typedef QList<QPolygon> ViewRegions;

class vtkSMProxy;
class pq3DWidget;

//! Interface for Views where user can select products
class ISelectableView
{
public:
  ISelectableView() : m_VOIWidget(NULL){}
  //! Set a selection to all elements which belong to regions
  //! and pass the filtering criteria
  virtual void setSelection(SelectionFilters &filters, ViewRegions &regions) = 0;
  
  virtual pqRenderView *view() = 0;
  
  
protected:
  virtual void setVOI(IVOI *voi) = 0;
  
protected:
  pq3DWidget *m_VOIWidget;
  IVOI *m_voi;
};

class Product;

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
  typedef QPair<VtkRegion, EspinaProduct *> SelElement;
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

class IVOI
{
public:
  virtual ~IVOI(){}

  virtual EspinaFilter *applyVOI(vtkProduct *product) = 0;
  virtual EspinaFilter *restoreVOITransormation(vtkProduct* product) = 0;
  
  virtual vtkSMProxy * getProxy() = 0;
  virtual pq3DWidget *newWidget() = 0;
  virtual void deleteWidget(pq3DWidget *&widget) = 0;
  
  virtual bool contains(ISelectionHandler::VtkRegion region) = 0;
  
  virtual void cancelVOI() = 0;
  
  virtual void setSource(Sample *product) { m_product  = product;}
  
protected:
  Sample *m_product;
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
  void setVOI(IVOI *voi);
  IVOI *voi() {return m_voi;}
  //! Applies active VOI to product
  //IFilter *applyVOI(vtkProduct *product);
  //! Restores VOI transformations
  //IFilter *restoreVOITransformation(vtkProduct *product);
  
public slots:
  //! Register @sh as active Selection Handler
  void setSelectionHandler(ISelectionHandler *sh);
  
signals:
  void VOIChanged(IVOI *voi);
  
public:
  //! Returns a SelectionManager singleton
  static SelectionManager *instance(){return m_singleton;}
  
private:
  ISelectionHandler *m_handler;
  IVOI *m_voi;
  static SelectionManager *m_singleton;
};

#endif // SELECTIONMANAGER_H
