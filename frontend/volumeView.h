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

class VolumeView : public QAbstractItemView
{
  Q_OBJECT
public:
    VolumeView(QWidget* parent = 0);

public slots:
  void connectToServer();
  void disconnectFromServer();
  
  //! Show/Hide scene actors
  void showActors(bool value);
    
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
    
protected slots:
  //! Select Mesh Rendering
  void setMeshRenderer(); 
  //! Select Volume Rendering
  void setVolumeRenderer(); 

  void updateScene();
  void render(const QModelIndex &index);
  
private:
  bool m_init;
  bool m_showPlanes;
  bool m_showActors;
  IRenderer *m_renderer;
  
  // GUI
  pqRenderView *m_view;
  QWidget *m_viewWidget;
  QToolButton *m_togglePlanes;
  QToolButton *m_toggleActors;
  QVBoxLayout *m_mainLayout;
  QHBoxLayout *m_controlLayout;
};

#endif // VOLUMEVIEW_H