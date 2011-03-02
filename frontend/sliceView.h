/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#ifndef SLICEVIEW_H
#define SLICEVIEW_H

#include <qabstractitemview.h>

//Forward declaration
class SliceBlender;
class pqTwoDRenderView;
class vtkSMImageSliceRepresentationProxy;
class vtkSMIntVectorProperty;
class QWidget;
class QScrollBar;
class QSpinBox;
class QVBoxLayout;
class QHBoxLayout;
class pqOutputPort;
class Segmentation;
class Sample;
class IRenderer;

#include "selectionManager.h"//TODO: Forward declare?


//! Displays a unique slice of a sample
//! If segmentations are visible, then their slices are
//! blended 	over the sample slice
class SliceView : public QAbstractItemView
{
  Q_OBJECT
public:
  SliceView(QWidget* parent = 0);

  enum SlicePlane
  {
    SLICE_PLANE_FIRST = 0,
    SLICE_PLANE_XY    = 0,
    SLICE_PLANE_YZ    = 1,
    SLICE_PLANE_XZ    = 2,
    SLICE_PLANE_LAST  = 2
  };


public slots:
  void connectToServer();
  void disconnectFromServer();

  //! Show/Hide segmentations
  void showSegmentations(bool value);

signals:
  void pointSelected(const Point);

protected:
  virtual QRegion visualRegionForSelection(const QItemSelection& selection) const;
  virtual void setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command);
  virtual bool isIndexHidden(const QModelIndex& index) const;
  virtual int verticalOffset() const;
  virtual int horizontalOffset() const;
  virtual QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
  // Updating model changes
  virtual void rowsInserted(const QModelIndex& parent, int start, int end);

public:
  virtual QModelIndex indexAt(const QPoint& point) const;
  virtual void scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint = EnsureVisible);
  virtual QRect visualRect(const QModelIndex& index) const;

  void focusOnSample(Sample *sample);

public slots:
  //! Slicer configuration methods:
  void setPlane(SlicePlane plane);
  void setSlice(int value);
  
  //! Selections
  void vtkWidgetMouseEvent(QMouseEvent *event);

protected:
  void updateScene();
  void render(const QModelIndex &index);

private:
  pqPipelineSource *blender();
  void slice(pqPipelineSource *source);
  void updateBlending(Segmentation* seg);

private:
  bool m_init;
  bool m_showSegmentations;
  vtkSMImageSliceRepresentationProxy *m_rep;
  SlicePlane m_plane;
  vtkSMIntVectorProperty *m_slice;
  pqPipelineSource *m_slicer;

  //TODO: Reasign when reconecting to server
  static Sample *s_focusedSample; // The sample which is being currently displayed
  static pqPipelineSource *s_colouredSample; // A blending filter

  // GUI
  QWidget *m_viewWidget;
  pqTwoDRenderView *m_view;
  QScrollBar *m_scrollBar;
  QSpinBox *m_spinBox;
  QVBoxLayout *m_mainLayout;
  QHBoxLayout *m_controlLayout;
};

#endif // SLICEVIEW_H
