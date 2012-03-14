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
// File:    VolumeView.h
// Purpose: Display 3D representations for model's elements
//----------------------------------------------------------------------------

#ifndef VOLUMEVIEW_H
#define VOLUMEVIEW_H

#include <QAbstractItemView>
#include <QPushButton>

class pqPipelineRepresentation;
class pq3DWidget;
class Sample;
class Sample;
class Segmentation;
//Forward declaration
class SliceBlender;
class pqRenderView;
class QWidget;
class QToolButton;
class QVBoxLayout;
class QHBoxLayout;
class pqOutputPort;
class Segmentation;
class IRenderer;
class IViewWidget;

/// VolumeView
class VolumeView
: public QAbstractItemView
{
  Q_OBJECT
public:
  explicit VolumeView(QWidget* parent = 0);
  virtual ~VolumeView(){}

  /// QAbstractItemView Interface
  virtual QModelIndex indexAt(const QPoint& point) const {return QModelIndex();}
  virtual void scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint = EnsureVisible) {}
  virtual QRect visualRect(const QModelIndex& index) const {return QRect();}

  void addSegmentationRepresentation(Segmentation *seg);
  void removeSegmentationRepresentation(Segmentation *seg);
  
  void addRepresentation(pqOutputPort *oport);

  void addWidget(pq3DWidget *widget);

public slots:
  void onConnect();
  void onDisconnect();

  void forceRender();

protected:
  /// QAbstractItemView Interface
  virtual QRegion visualRegionForSelection(const QItemSelection& selection) const {return QRegion();}
  virtual void setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command) {}
  virtual bool isIndexHidden(const QModelIndex& index) const {return true;}
  virtual int verticalOffset() const {return 0;}
  virtual int horizontalOffset() const {return 0;}
  virtual QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers) {return QModelIndex();}

  // Updating model changes
//   virtual void rowsInserted(const QModelIndex& parent, int start, int end);
//   virtual void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
//   virtual void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

private:
  void selectSegmentations(int x, int y, int z);
  
protected slots:
//   /// Selections
//   void vtkWidgetMouseEvent(QMouseEvent *event);
  
//   void updateScene();
//   void render(const QModelIndex &index);
  void exportScene();
  void takeSnapshot();

  void buildControls();
private:
  pqRenderView *m_view;

  // GUI
  QVBoxLayout *m_mainLayout;
  QHBoxLayout *m_controlLayout;
  QWidget *m_viewWidget;
  QPushButton m_snapshot;
  QPushButton m_export;

  QMap<Segmentation *, pqPipelineRepresentation *> m_segmentations;
};

#endif // VOLUMEVIEW_H