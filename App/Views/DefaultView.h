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


#ifndef ESPINA_DEFAULT_VIEW_H
#define ESPINA_DEFAULT_VIEW_H

#include <QAbstractItemView>

#include <Core/EspinaTypes.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>
#include <Support/ViewManager.h>
#include <Support/Settings/SettingsPanel.h>

// Forward-declaration
class QMainWindow;
class QDockWidget;
class QUndoStack;

namespace EspINA
{
  class DefaultView
  : public QAbstractItemView
  {
    Q_OBJECT
  public:
    explicit DefaultView(ModelAdapterSPtr    model,
                         ViewManagerSPtr     viewManager,
                         QUndoStack         *undoStack,
                         const RendererSList &renderers,
                         QMainWindow         *parent=0
    );
    virtual ~DefaultView();

    void setCrosshairColor(const Plane plane, const QColor& color);

    virtual void createViewMenu(QMenu *menu);

    virtual QModelIndex indexAt(const QPoint& point) const
    { return QModelIndex(); }

    virtual void scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint = EnsureVisible){}

    virtual QRect visualRect(const QModelIndex& index) const
    { return QRect(); }

    virtual void setModel(QAbstractItemModel *model);

    SettingsPanelSPtr settingsPanel();

  protected:
    // AbstractItemView Interface
    virtual QRegion visualRegionForSelection(const QItemSelection& selection) const
    {return QRegion();}

    virtual void setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command)
    {}

    virtual bool isIndexHidden(const QModelIndex& index) const
    {return true;}

    virtual int verticalOffset() const
    {return 0;}

    virtual int horizontalOffset() const
    {return 0;}

    virtual QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
    {return QModelIndex();}

    void add(ChannelAdapterPtr      channel);
    void add(SegmentationAdapterPtr segmentation);

    void remove(ChannelAdapterPtr      channel);
    void remove(SegmentationAdapterPtr segmentation);

    bool updateRepresentation(ChannelAdapterPtr      channel);
    bool updateRepresentation(SegmentationAdapterPtr segmentation);

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
    virtual void setCrosshairPoint(const NmVector3& point, bool force = false);
    //  virtual void setSliceSelectors(SliceView::SliceSelectors selectors);
    virtual void changePlanePosition(Plane, Nm);

  protected:
    virtual void rowsInserted(const QModelIndex& parent, int start, int end);
    virtual void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);

    /// Update XY, YZ, XZ and 3D View
    void updateViews();

    //   void selectFromSlice(double slice, PlaneType plane);
    //   void selectToSlice(double slice, PlaneType plane);

    void initView2D(View2D *view);

  private:
    ModelAdapterSPtr m_model;
    ViewManagerSPtr  m_viewManager;

    bool m_showProcessing;
    bool m_showSegmentations;

    NmVector3 m_slicingStep;

    QColor m_xLine, m_yLine, m_zLine;

    View2D *m_viewXY, *m_viewYZ, *m_viewXZ;
    View3D *m_view3D;

    const RendererSList &m_renderers;

    QDockWidget *dock3D, *dockYZ, *dockXZ;
    QAction     *m_showRuler, *m_showThumbnail;

    ContextualMenuSPtr m_contextMenu;
  };


  using DefaultViewSPtr = std::shared_ptr<DefaultView>;

} // namespace EspINA

#endif // ESPINA_DEFAULT_VIEW_H
