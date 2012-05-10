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


//----------------------------------------------------------------------------
// File:    EspinaView.h
// Purpose: Group different views and the way they are displayed
//          (i.e. main window widget, dock widgets, independent widget, etc)
//----------------------------------------------------------------------------

#ifndef ESPINAVIEW_H
#define ESPINAVIEW_H

#include <QAbstractItemView>
#include <QDockWidget>
#include "common/widgets/RectangularSelection.h"
#include "common/gui/SliceView.h"

// Forward-declaration
class ColorEngine;
class QMainWindow;

class EspinaView 
: public QAbstractItemView
{
  Q_OBJECT
public:
  explicit EspinaView(QMainWindow * parent, const QString activity = QString());
  virtual ~EspinaView(){}

  virtual void createViewMenu(QMenu* menu) = 0;
  virtual void restoreLayout() = 0;
  virtual void saveLayout() = 0;

  virtual void forceRender() = 0;
  virtual void resetCamera() = 0;

  /// QAbstractItemView Interface
  virtual QModelIndex indexAt(const QPoint& point) const {return QModelIndex();}
  virtual void scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint = EnsureVisible) {}
  virtual QRect visualRect(const QModelIndex& index) const {return QRect();}

  virtual void addWidget(EspinaWidget *widget) = 0;
  virtual void removeWidget(EspinaWidget *widget) = 0;

  virtual void addRepresentation(pqOutputPort *oport, QColor color) = 0;
  virtual void removeRepresentation(pqOutputPort *oport) = 0;

  virtual void gridSize(double size[3]) = 0;
  virtual void setGridSize(double size[3]) = 0;
  virtual void setColorEngine(ColorEngine *engine) {};
public slots:
  virtual void setShowSegmentations(bool visibility) = 0;
  virtual void setCenter(double x, double y, double z) = 0;
  virtual void setSliceSelectors(SliceView::SliceSelectors selectors) = 0;

signals:
  void statusMsg(QString);

  void selectedFromSlice(double, vtkPVSliceView::VIEW_PLANE);
  void selectedToSlice(double, vtkPVSliceView::VIEW_PLANE);

protected:
  // AbstractItemView Interfacec
  virtual QRegion visualRegionForSelection(const QItemSelection& selection) const {return QRegion();}
  // TODO: Convert QRect to Region and use ISelectable::setSelection
  virtual void setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command) {}
  virtual bool isIndexHidden(const QModelIndex& index) const {return true;}
  virtual int verticalOffset() const {return 0;}
  virtual int horizontalOffset() const {return 0;}
  virtual QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers){return QModelIndex();}

protected:
  QString      m_activity;
  QMainWindow *m_window;
};


#endif // ESPINAVIEW_H
