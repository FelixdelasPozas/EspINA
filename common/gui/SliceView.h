/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.es>

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
// File:    SliceView.h
// Purpose: Generate slices for model's elements
//----------------------------------------------------------------------------
#ifndef SLICEVIEW_H
#define SLICEVIEW_H

#include <QAbstractItemView>

#include <vtkSmartPointer.h>
#include <../Views/vtkPVSliceView.h>
#include "IPreferencePanel.h"

class QLabel;
class pqOutputPort;
class pqPipelineSource;// #include <QMutex>
// #include <QMap>
// 
// #include "selectionManager.h"//TODO: Forward declare?
// 
// #include "../frontend/IPreferencePanel.h"
// 
// 
// namespace CrosshairExtension
// {
// class SampleRepresentation;
// }
// 
//Forward declaration
// class CrosshairRepresentation;
// class vtkCamera;
// class vtkSMRenderViewProxy;
// class Sample;
class pqSliceView;
class vtkInteractorStyleEspinaSlice;

// GUI
class QScrollBar;
class QSpinBox;
class QVBoxLayout;
class QHBoxLayout;
// class pqRenderView;
// class vtkRenderWindowInteractor;

class SliceViewPreferences;

class SliceViewPreferencesPanel : public QWidget
{
  Q_OBJECT

public:
  explicit SliceViewPreferencesPanel(SliceViewPreferences *preferences);

public slots:
  void setInvertWheel(bool value);
  void setInvertNormal(bool value);
  void setShowAxis(bool value);
private:
  SliceViewPreferences *m_pref;
};

class SliceViewPreferences : public IPreferencePanel
{
public:
  explicit SliceViewPreferences(vtkPVSliceView::VIEW_PLANE plane);

  virtual const QString shortDescription();
  virtual const QString longDescription() {return shortDescription();}
  virtual const QIcon icon() {return QIcon();}

  virtual QWidget* widget() {return new SliceViewPreferencesPanel(this);}

  void setInvertWheel(bool value);
  bool invertWheel(){return m_InvertWheel;}
  void setInvertSliceOrder(bool value);
  bool invertSliceOrder() {return m_InvertSliceOrder;}
  void setShowAxis(bool value);
  bool showAxis() {return m_ShowAxis;}

private:
  bool m_InvertWheel;
  bool m_InvertSliceOrder;
  bool m_ShowAxis;

private:
  vtkPVSliceView::VIEW_PLANE m_plane;
  QString viewSettings;
};


// //! Displays a unique slice of a sample
// //! If segmentations are visible, then their slices are
// //! blended over the sample slice
/// Slice View Widget
class SliceView
: public QAbstractItemView
// , public ISelectableView
{
  Q_OBJECT
public:
  SliceView(vtkPVSliceView::VIEW_PLANE plane = vtkPVSliceView::AXIAL, QWidget* parent = 0);
  virtual ~SliceView();

//   IPreferencePanel *preferences() {return m_preferences;}
  inline QString title() const;
  void setTitle(const QString &title);

  /// QAbstractItemView Interface
  virtual QModelIndex indexAt(const QPoint& point) const {return QModelIndex();}
  virtual void scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint = EnsureVisible) {}
  virtual QRect visualRect(const QModelIndex& index) const {return QRect();}

  void setCrossHairColors(double hcolor[3], double vcolor[3]);
//   //void focusOnSample(Sample *sample);
//
//   //! Interface of ISelectableView
//   void setSelection(SelectionFilters &filters, ViewRegions &regions);
//
//   QList<Segmentation *> pickSegmentationsAt(int x, int y, int z);
//   QList<Segmentation *> pickSegmentationsAt(ISelectionHandler::VtkRegion region);
//   void selectSegmentations(int x, int y, int z);
//

public slots:
  // Espina has been connected to a new server
  void onConnect();
  // Espina has been disconnected from server
  void onDisconnect();

  void loadTestImage();

  //! Show/Hide segmentations
  void setShowSegmentations(bool value);

//   //! Slicer configuration methods:
//   void setPlane(ViewType plane);
//
//   //! Selections
//   void vtkWidgetMouseEvent(QMouseEvent *event);
//
//   void updateScene();
//
//   void beginRender();
//   void endRender();
//
// protected slots:
//   void setSlice(int slice);
//   virtual void setVOI(IVOI *voi);
//   void updateVOIVisibility();
protected slots:
  void close();
  void maximize();
  void minimize();
  void undock();

signals:
  // Notify the windows manager how to display the view
  void closeRequest();
  void maximizeRequest();
  void minimizeRequest();
  void undockRequest();
//   void sliceChanged();
// //  void pointSelected(int, int, int);

protected:
  // AbstractItemView Interfacec
  virtual QRegion visualRegionForSelection(const QItemSelection& selection) const {return QRegion();}
  // TODO: Convert QRect to Region and use ISelectable::setSelection
  virtual void setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command) {}
  virtual bool isIndexHidden(const QModelIndex& index) const {return true;}
  virtual int verticalOffset() const {return 0;}
  virtual int horizontalOffset() const {return 0;}
  virtual QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers){return QModelIndex();}
//   // Updating model changes: This determines how the view should response to changes from the model
//   virtual void rowsInserted(const QModelIndex& parent, int start, int end);
//   virtual void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
//   virtual void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

  virtual bool eventFilter(QObject* obj, QEvent* event);
//   virtual pqRenderView* view();

//   void centerViewOn(int x, int y, int z);
//   //! Converts point from Display coordinates to World coordinates
//   ISelectionHandler::VtkRegion display2vtk(const QPolygonF &region);
  void addChannelRepresentation(pqOutputPort *oport);
  void addSegmentationRepresentation(pqOutputPort *oport);


  void buildTitle();
  void buildControls();
private:
//   bool m_showSegmentations;
  vtkPVSliceView::VIEW_PLANE m_plane;

  bool first;
  pqSliceView *m_view;
//   vtkSMRenderViewProxy *m_viewProxy;
//   vtkRenderWindowInteractor *m_rwi;
//   vtkCamera *m_cam;

  // GUI
  QHBoxLayout *m_titleLayout;
  QLabel      *m_title;
  QVBoxLayout *m_mainLayout;
  QHBoxLayout *m_controlLayout;
  QWidget     *m_viewWidget;
  QScrollBar  *m_scrollBar;
  QSpinBox    *m_spinBox;

//   vtkSmartPointer<vtkInteractorStyleEspinaSlice> m_style;

  SliceViewPreferences *m_preferences;
};

#endif // SLICEVIEW_H
