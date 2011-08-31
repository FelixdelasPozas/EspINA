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


#ifndef VOLUMEVIEW_H
#define VOLUMEVIEW_H

#include <qabstractitemview.h>
#include <selectionManager.h>

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

//! 
class VolumeView : public QAbstractItemView
{
  Q_OBJECT
public:
    VolumeView(QWidget* parent = 0);

public slots:
  void connectToServer();
  void disconnectFromServer();
  
  void setVOI(IVOI *voi);
    
protected:
  //! QAbstractItemView Interface
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
  //! QAbstractItemView Interface
    virtual void setRootIndex(const QModelIndex& index);
    virtual QModelIndex indexAt(const QPoint& point) const;
    virtual void scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint = EnsureVisible);
    virtual QRect visualRect(const QModelIndex& index) const;
    
    void addWidget(IViewWidget *widget);
    
protected slots:
  void updateScene();
  void render(const QModelIndex &index);
  void exportScene();
  void takeSnapshot();
  
private:
  bool m_init;
  double m_focus[3];
  
  // GUI
  QList<IViewWidget *> m_widgets;
  QMap<QString, IViewWidget *> m_representations;
  pqRenderView *m_view;
  QWidget *m_viewWidget;
  QVBoxLayout *m_mainLayout;
  QToolButton m_snapshot;
  QToolButton m_export;
  QHBoxLayout *m_controlLayout;
  pq3DWidget *m_VOIWidget;//Because it doesn't implement ISelectableView
};

#endif // VOLUMEVIEW_H