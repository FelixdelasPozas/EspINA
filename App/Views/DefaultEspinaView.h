/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef DEFAULTESPINAVIEW_H
#define DEFAULTESPINAVIEW_H

#include <QAbstractItemView>

#include <Core/EspinaTypes.h>
#include <GUI/QtWidget/SliceView.h>
#include <GUI/QtWidget/VolumeView.h>
#include <GUI/QtWidget/SegmentationContextualMenu.h>

class EspinaFactory;
// Forward-declaration
class SliceViewSettingsPanel;
class ColorEngine;
class QMainWindow;
class QDockWidget;
class Segmentation;
class ViewManager;
class VolumeViewSettingsPanel;

class DefaultEspinaView
: public QAbstractItemView
{
  Q_OBJECT
  class SettingsPanel;

public:
  explicit DefaultEspinaView(EspinaModel *model,
                             ViewManager *vm,
                             QMainWindow *parent=0
                            );
  virtual ~DefaultEspinaView();

  virtual void createViewMenu(QMenu* menu);

  virtual ISettingsPanel* settingsPanel();

  virtual QModelIndex indexAt(const QPoint& point) const
  { return QModelIndex(); }
  virtual void scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint = EnsureVisible){}
  virtual QRect visualRect(const QModelIndex& index) const
  { return QRect(); }

protected:
  // AbstractItemView Interface
  virtual QRegion visualRegionForSelection(const QItemSelection& selection) const {return QRegion();}
  virtual void setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command) {}
  virtual bool isIndexHidden(const QModelIndex& index) const {return true;}
  virtual int verticalOffset() const {return 0;}
  virtual int horizontalOffset() const {return 0;}
  virtual QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers){return QModelIndex();}

  void addChannel(Channel *channel);
  void removeChannel(Channel *channel);
  bool updateChannel(Channel *channel);

  void addSegmentation(Segmentation *seg);
  void removeSegmentation(Segmentation *seg);
  bool updateSegmentation(Segmentation *seg);

protected slots:
  virtual void rowsInserted(const QModelIndex& parent, int start, int end);
  virtual void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
  virtual void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

  void showCrosshair(bool visible);
  void setRulerVisibility(bool visible);
  void showSegmentations(bool visible);
  void showThumbnail(bool visible);
  void switchPreprocessing();
  void setFitToSlices(bool fit);

//  virtual void setCameraFocus(const Nm focus[3]);
//
 virtual void setCrosshairPoint(Nm x, Nm y, Nm z, bool force = false);
//  virtual void setSliceSelectors(SliceView::SliceSelectors selectors);
 virtual void changePlanePosition(PlaneType, Nm);

protected:
  /// Update XY, YZ, XZ and 3D View
  void updateViews();

//   void selectFromSlice(double slice, PlaneType plane);
//   void selectToSlice(double slice, PlaneType plane);

  void initSliceView(SliceView *view);

private:
  EspinaModel *m_model;

  bool m_showProcessing;
  bool m_showSegmentations;
  Nm   m_slicingStep[3];

  SliceView  *xyView, *yzView, *xzView;
  VolumeView *volView;
  QDockWidget *volDock, *yzDock, *xzDock;
  QAction     *m_showRuler, *m_showThumbnail;

  QSharedPointer<SegmentationContextualMenu> m_contextMenu;
};

class DefaultEspinaView::SettingsPanel
: public ISettingsPanel
{
public:
  explicit SettingsPanel(SliceView::SettingsPtr xy,
                         SliceView::SettingsPtr yz,
                         SliceView::SettingsPtr xz,
                         VolumeView::SettingsPtr vol,
                         EspinaFactory *factory);

  virtual const QString shortDescription() {return tr("View");}
  virtual const QString longDescription() {return tr("%1 Settings").arg(shortDescription());}
  virtual const QIcon icon() {return QIcon(":/espina/show_all.svg");}

  virtual void acceptChanges();

  virtual bool modified() const;

  virtual ISettingsPanel* clone();

private:
  SliceView::SettingsPtr m_xy, m_yz, m_xz;
  EspinaFactory *m_factory;

  Nm m_slicingStep;

  SliceViewSettingsPanel *m_xyPanel, *m_yzPanel, *m_xzPanel;
  VolumeView::SettingsPtr m_vol;
  VolumeViewSettingsPanel *m_volPanel;
};

#endif // DEFAULTESPINAVIEW_H
