/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>

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

#include "SelectionHandler.h"
#include <QCursor>

class QPoint;
class SelectableView;

/// Singleton instance to coordinate selections through different
/// components such as views and plugins
class SelectionManager
: public QObject
{
  Q_OBJECT
public:
  ~SelectionManager(){}

  // Delegates calls on active SelectionHandler
  void onMouseDown(const QPoint &pos, SelectableView *view) const;
  void onMouseMove(const QPoint &pos, SelectableView *view) const;
  void onMouseUp  (const QPoint &pos, SelectableView *view) const;
  bool filterEvent(QEvent *e, SelectableView *view=NULL) const;

  void setSelection(SelectionHandler::MultiSelection sel) const;
//   void setVOI(IVOI *voi);
//   IVOI *voi() {return m_voi;}
//   //! Applies active VOI to product
  //IFilter *applyVOI(vtkProduct *product);
//   //! Restores VOI transformations
  //IFilter *restoreVOITransformation(vtkProduct *product);
  QCursor cursor() const;

public slots:
  /// Register @sh as active Selection Handler
  void setSelectionHandler(SelectionHandler *sh);
  /// Unregister @sh as active Selection Handler
  void unsetSelectionHandler(SelectionHandler *sh);

signals:
//   void VOIChanged(IVOI *voi);

public:
  /// Returns a SelectionManager singleton
  static SelectionManager *instance();

private:
  explicit SelectionManager();

private:
  SelectionHandler *m_handler;
//   IVOI *m_voi;

  static SelectionManager *m_singleton;
};


//class vtkSMProxy;
//class pq3DWidget;



// class Product;

/*
class IVOI : public QObject
{
  Q_OBJECT
public:
  virtual ~IVOI(){}

  virtual EspinaFilter *applyVOI(vtkProduct *product) = 0;
  virtual EspinaFilter *restoreVOITransormation(vtkProduct* product) = 0;
  virtual void setDefaultBounds(double bounds[6]) = 0;
  virtual void resizeToDefaultSize() = 0;
  void defaultBounds(double bounds[6])  {memcpy(bounds,m_bounds,6*sizeof(double));}
  virtual void bounds(double bounds[6]) = 0;
  
  virtual vtkSMProxy * getProxy() = 0;
  virtual pq3DWidget *newWidget(ViewType viewType) = 0;
  virtual void deleteWidget(pq3DWidget *&widget) = 0;
  
  virtual bool contains(ISelectionHandler::VtkRegion region) = 0;
  virtual bool intersectPlane(ViewType plane, int slice) = 0;
  
  virtual void setEnabled(bool value) = 0;
  
  virtual void cancelVOI() = 0;
  
  virtual void setSource(Sample *product) { m_product  = product;}
  
  virtual void setFromSlice(int value) = 0;

  virtual void setToSlice(int value) = 0;

signals:
  void voiModified();
  void voiCancelled();

  
protected:
  Sample *m_product;
  double m_bounds[6];
};*/



#endif // SELECTIONMANAGER_H
