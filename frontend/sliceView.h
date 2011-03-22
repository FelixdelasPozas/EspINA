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
class vtkSMProxy;
class IModelItem;

#include "selectionManager.h"//TODO: Forward declare?
class Blender
{
public:
  static Blender *instance();
  
  pqPipelineSource *source(){  return m_imageBlender;}
  
  //! Focus on a new sample, if previous segmentation were shown
  //! their memory is freed.
  void focusOnSample(Sample *sample);
  //! Blends seg into the focused sample
  void blendSegmentation(Segmentation *seg);
  //! Unblends seg into the focused sample
  void unblendSegmentation(Segmentation *seg);
  
  void updateImageBlenderInput();
  
private:
  Blender() : m_sampleMapper(NULL), m_imageBlender(NULL) {}
  static Blender *m_blender;
  pqPipelineSource *m_sampleMapper;
  pqPipelineSource *m_imageBlender;
  QMap<IModelItem *,pqPipelineSource *> m_blendingMappers;
};


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
  virtual void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
  virtual void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

public:
  virtual QModelIndex indexAt(const QPoint& point) const;
  virtual void scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint = EnsureVisible);
  virtual QRect visualRect(const QModelIndex& index) const;

  void focusOnSample(Sample *sample);
  
  pqPipelineSource **output(){return &m_slicer;}

public slots:
  //! Slicer configuration methods:
  void setPlane(SlicePlane plane);
  void setSlice(int value);
  
  //! Selections
  void vtkWidgetMouseEvent(QMouseEvent *event);

signals:
  void sliceChanged();

protected:
  void updateScene();

private:
  pqPipelineSource *blender();
  void slice(pqPipelineSource *source);

private:
  bool m_init;
  bool m_showSegmentations;
  vtkSMImageSliceRepresentationProxy *m_rep;
  SlicePlane m_plane;
  vtkSMIntVectorProperty *m_slice;
  pqPipelineSource *m_slicer;

  //TODO: Reasign when reconecting to server
  Sample *s_focusedSample; // The sample which is being currently displayed
  static Blender *s_blender; // A blending filter

  // GUI
  QWidget *m_viewWidget;
  pqTwoDRenderView *m_view;
  QScrollBar *m_scrollBar;
  QSpinBox *m_spinBox;
  QVBoxLayout *m_mainLayout;
  QHBoxLayout *m_controlLayout;
};

#endif // SLICEVIEW_H
