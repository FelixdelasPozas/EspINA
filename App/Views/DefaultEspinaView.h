/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef DEFAULTESPINAVIEW_H
#define DEFAULTESPINAVIEW_H

#include <QAbstractItemView>

#include <Core/EspinaTypes.h>
#include <Core/Model/EspinaFactory.h>
#include <GUI/QtWidget/SliceView.h>
#include <GUI/QtWidget/VolumeView.h>
#include <GUI/QtWidget/SegmentationContextualMenu.h>

// Forward-declaration
class QMainWindow;
class QDockWidget;

namespace EspINA
{
  class EspinaFactory;
  class SliceViewSettingsPanel;
  class ColorEngine;
  class ViewManager;
  class VolumeViewSettingsPanel;

  class DefaultEspinaView
  : public QAbstractItemView
  {
    Q_OBJECT
    class SettingsPanel;

  public:
    explicit DefaultEspinaView(EspinaModel *model,
                               QUndoStack  *undoStack,
                               ViewManager *viewManager,
                               QMainWindow *parent=0
    );
    virtual ~DefaultEspinaView();

    virtual void createViewMenu(QMenu* menu);

    virtual ISettingsPanelPtr settingsPanel();

    virtual QModelIndex indexAt(const QPoint& point) const
    { return QModelIndex(); }
    virtual void scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint = EnsureVisible){}
    virtual QRect visualRect(const QModelIndex& index) const
    { return QRect(); }

    virtual void setModel(QAbstractItemModel *model);

  protected:
    // AbstractItemView Interface
    virtual QRegion visualRegionForSelection(const QItemSelection& selection) const {return QRegion();}
    virtual void setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command) {}
    virtual bool isIndexHidden(const QModelIndex& index) const {return true;}
    virtual int verticalOffset() const {return 0;}
    virtual int horizontalOffset() const {return 0;}
    virtual QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers){return QModelIndex();}

    void addChannel   (ChannelPtr channel);
    void removeChannel(ChannelPtr channel);
    bool updateChannel(ChannelPtr channel);

    void addSegmentation   (SegmentationPtr seg);
    void removeSegmentation(SegmentationPtr seg);
    bool updateSegmentation(SegmentationPtr seg);

  protected slots:
    void sourceModelReset();

    void showCrosshair(bool visible);
    void setRulerVisibility(bool visible);
    void showSegmentations(bool visible);
    void showThumbnail(bool visible);
    void switchPreprocessing();
    void setFitToSlices(bool unused);

    //  virtual void setCameraFocus(const Nm focus[3]);
    //
    virtual void setCrosshairPoint(Nm x, Nm y, Nm z, bool force = false);
    //  virtual void setSliceSelectors(SliceView::SliceSelectors selectors);
    virtual void changePlanePosition(PlaneType, Nm);

  protected:
    virtual void rowsInserted(const QModelIndex& parent, int start, int end);
    virtual void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);

    /// Update XY, YZ, XZ and 3D View
    void updateViews();

    //   void selectFromSlice(double slice, PlaneType plane);
    //   void selectToSlice(double slice, PlaneType plane);

    void initSliceView(SliceView *view);

  private:
    EspinaModel *m_model;
    ViewManager *m_viewManager;

    bool m_showProcessing;
    bool m_showSegmentations;
    Nm   m_slicingStep[3];

    SliceView  *xyView, *yzView, *xzView;
    VolumeView *volView;
    QDockWidget *volDock, *yzDock, *xzDock;
    QAction     *m_showRuler, *m_showThumbnail;

    ISettingsPanelPrototype m_settingsPanel;
    SegmentationContextualMenuSPtr m_contextMenu;
  };

  class DefaultEspinaView::SettingsPanel
  : public ISettingsPanel
  {
  public:
    explicit SettingsPanel(SliceView::SettingsSPtr xy,
                           SliceView::SettingsSPtr yz,
                           SliceView::SettingsSPtr xz,
                           VolumeView::SettingsPtr vol,
                           EspinaFactoryPtr factory);

    virtual const QString shortDescription() {return tr("View");}
    virtual const QString longDescription() {return tr("%1").arg(shortDescription());}
    virtual const QIcon icon() {return QIcon(":/espina/show_all.svg");}

    virtual void acceptChanges();
    virtual void rejectChanges();
    virtual bool modified() const;

    virtual ISettingsPanelPtr clone();

  private:
    SliceView::SettingsSPtr m_xy, m_yz, m_xz;
    EspinaFactory *m_factory;

    Nm m_slicingStep;

    SliceViewSettingsPanel *m_xyPanel, *m_yzPanel, *m_xzPanel;
    VolumeView::SettingsPtr m_vol;
    VolumeViewSettingsPanel *m_volPanel;
  };

} // namespace EspINA

#endif // DEFAULTESPINAVIEW_H
