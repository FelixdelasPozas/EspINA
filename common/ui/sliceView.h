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

#include <QMutex>
#include <QMap>

#include "selectionManager.h"//TODO: Forward declare?

class CrosshairRepresentation;
class vtkInteractorStyleEspina;
class vtkCamera;
class vtkSMRenderViewProxy;
//Forward declaration
class Sample;
class Segmentation;
class IModelItem;
class pqPipelineSource;

/*
//! Blends Segmentations in a given Sample
class Blender
{
public:
  Blender() : m_currentSample(NULL), m_bgMapper(NULL), m_imageBlender(NULL) {}
  pqPipelineSource *source(){  return m_imageBlender;}
  
  //! Focus on a new sample, if previous segmentation were shown
  //! their memory is freed.
  void setBackground(Sample *product);
  //! Blends seg into the focused sample
  void blend(Segmentation *seg);
  //! Unblends seg from the focused sample
  void unblend(Segmentation *seg);
  
  // Free all internal memory and paraview proxies
  void clear();
  
  void updateImageBlenderInput();
private:
  Sample *m_currentSample;
  pqPipelineSource *m_bgMapper;
  pqPipelineSource *m_imageBlender;
  QMap<IModelItem *,ISegmentationRepresentation *> m_blendingMappers;
  QMutex m_mutex;
};
*/

#include <QAbstractItemView>

// Interface
class QWidget;
class QScrollBar;
class QSpinBox;
class QVBoxLayout;
class QHBoxLayout;
class pqOutputPort;
class IRenderer;
class vtkSMProxy;
class pqRenderView;
class vtkSMImageSliceRepresentationProxy;
class vtkSMIntVectorProperty;
class vtkRenderWindowInteractor;


//! Displays a unique slice of a sample
//! If segmentations are visible, then their slices are
//! blended over the sample slice
class SliceView 
: public QAbstractItemView
, public ISelectableView
{
  Q_OBJECT
public:
  SliceView(QWidget* parent = 0);
  virtual ~SliceView();

  //! AbstractItemView Interface
  virtual QModelIndex indexAt(const QPoint& point) const;
  virtual void scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint = EnsureVisible);
  virtual QRect visualRect(const QModelIndex& index) const;
  
  //void focusOnSample(Sample *sample);
  
  //! Interface of ISelectableView
  void setSelection(SelectionFilters &filters, ViewRegions &regions);

  virtual bool eventFilter(QObject* obj, QEvent* event);

public slots:
  void connectToServer();
  void disconnectFromServer();

  //! Show/Hide segmentations
  void showSegmentations(bool value);
  
  //! Slicer configuration methods:
  void setPlane(ViewType plane);
  void setSlice(int value);
  
  void centerViewOn(int x, int y, int z);
  
  //! Selections
  void vtkWidgetMouseEvent(QMouseEvent *event);

  void updateScene();
  
protected slots:
  virtual void setVOI(IVOI *voi);
  
signals:
  void sliceChanged();
//  void pointSelected(int, int, int);

protected:
  //! AbstractItemView Interfacec
  virtual QRegion visualRegionForSelection(const QItemSelection& selection) const;
  // TODO: Convert QRect to Region and use ISelectable::setSelection
  virtual void setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command);
  virtual bool isIndexHidden(const QModelIndex& index) const;
  virtual int verticalOffset() const;
  virtual int horizontalOffset() const;
  virtual QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
  // Updating model changes: This determines how the view should response to changes from the model
  virtual void rowsInserted(const QModelIndex& parent, int start, int end);
  virtual void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
  virtual void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

  virtual pqRenderView* view();
  
  //! Converts point from Display coordinates to World coordinates
  ISelectionHandler::VtkRegion display2vtk(const QPolygonF &region);
  
private:
  bool m_showSegmentations;
  ViewType m_plane;
  CrosshairRepresentation *m_sampleRep;
  
  Sample *m_focusedSample; // The sample which is being currently displayed
  
  pqRenderView *m_view;
  vtkSMRenderViewProxy *m_viewProxy;
  vtkRenderWindowInteractor *m_rwi;
  vtkCamera *m_cam;

  // GUI
  QWidget *m_viewWidget;
  QScrollBar *m_scrollBar;
  QSpinBox *m_spinBox;
  QVBoxLayout *m_mainLayout;
  QHBoxLayout *m_controlLayout;
  vtkInteractorStyleEspina *m_style;
};

#endif // SLICEVIEW_H
