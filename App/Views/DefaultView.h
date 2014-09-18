/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include <Core/EspinaTypes.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>
#include <Support/ViewManager.h>
#include <Support/Settings/SettingsPanel.h>

// Qt
#include <QAbstractItemView>

class QMainWindow;
class QDockWidget;
class QUndoStack;

namespace ESPINA
{
  class RenderersMenu;
  class CamerasMenu;

  class DefaultView
  : public QAbstractItemView
  {
    Q_OBJECT
  public:
    /* \brief DefaultView class constructor.
     * \param[in] model, model adapter smart pointer.
     * \param[in] viewManager, view manager smart pointer.
     * \param[in] undoStack, QUndoStack object raw pointer.
     * \param[in] parent, raw pointer to parent object.
     */
    explicit DefaultView(ModelAdapterSPtr    model,
                         ViewManagerSPtr     viewManager,
                         QUndoStack         *undoStack,
                         QMainWindow        *parent = nullptr);

    /* \brief DefaultView class virtual destructor.
     *
     */
    virtual ~DefaultView();

    /* \brief Sets the crosshair colors of the view.
     * \param[in] plane, plane of the crosshair line.
     * \param[in] color, color of the crosshair line.
     *
     */
    void setCrosshairColor(const Plane plane, const QColor& color);

    /* \brief Fill the view menu.
     * \param[inout] menu, menu to modify.
     *
     */
    virtual void createViewMenu(QMenu *menu);

    /* \brief Implements QAbstractItemView::indexAt().
     *
     */
    virtual QModelIndex indexAt(const QPoint& point) const
    { return QModelIndex(); }

    /* \brief Implements QAbstractItemView::scrollTo().
     *
     */
    virtual void scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint = EnsureVisible){}

    /* \brief Implements QAbstractItemView::visualRect().
     *
     */
    virtual QRect visualRect(const QModelIndex& index) const
    { return QRect(); }

    /* \brief Overrides QAbstractItemView::setModel().
     *
     */
    virtual void setModel(QAbstractItemModel *model) override;

    /* \brief Returns the view's settings panel.
     *
     */
    SettingsPanelSPtr settingsPanel();

    /* \brief Loads view settings from storage.
     * \param[in] storate, temporal storage containing the settings file.
     */
    void loadSessionSettings(TemporalStorageSPtr storage);

    /* \brief Saves view settings from storage.
     * \param[in] storate, temporal storage to save settings file.
     */
    void saveSessionSettings(TemporalStorageSPtr storage);

  protected:
    /* \brief Implements QAbstractItemView::visualRegionForSelection().
     *
     */
    virtual QRegion visualRegionForSelection(const QItemSelection& selection) const
    {return QRegion();}

    /* \brief Implements QAbstractItemView::setSelection().
     *
     */
    virtual void setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command)
    {}

    /* \brief Implements QAbstractItemView::isIndexHidden().
     *
     */
    virtual bool isIndexHidden(const QModelIndex& index) const
    {return true;}

    /* \brief Implements QAbstractItemView::verticalOffset().
     *
     */
    virtual int verticalOffset() const
    {return 0;}

    /* \brief Implements QAbstractItemView::horizontalOffset().
     *
     */
    virtual int horizontalOffset() const
    {return 0;}

    /* \brief Implements QAbstractItemView::moveCursor().
     *
     */
    virtual QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
    {return QModelIndex();}

    /* \brief Adds a channel to the view.
     * \param[in] channel, channel adapter raw pointer of the element to add.
     *
     */
    void add(ChannelAdapterPtr channel);

    /* \brief Adds a segmentation to the view.
     * \param[in] segmentation, segmentation adapter raw pointer of the element to add.
     *
     */
    void add(SegmentationAdapterPtr segmentation);

    /* \brief Removes a channel from the view.
     * \param[in] channel, channel adapter raw pointer of the element to remove.
     *
     */
    void remove(ChannelAdapterPtr channel);

    /* \brief Removes a segmentation from the view.
     * \param[in] segmentation, segmentation adapter raw pointer of the element to remove.
     *
     */
    void remove(SegmentationAdapterPtr segmentation);

    /* \brief Updates the representation of the given channel.
     * \param[in] channel, channel adapter raw pointer of the element to update representations.
     *
     */
    bool updateRepresentation(ChannelAdapterPtr channel);

    /* \brief Updates the representation of the given segmentation.
     * \param[in] segmentation, segmentation adapter raw pointer of the element to update representations.
     *
     */
    bool updateRepresentation(SegmentationAdapterPtr segmentation);

  protected slots:
  	/* \brief Resets all the views.
  	 *
  	 */
    void sourceModelReset();

    /* \brief Shows/hides the view's crosshair.
     * \param[in] visible, true to show the crosshair, false to hide.
     */
    void showCrosshair(bool visible);

    /* \brief Shows/hides the view's ruler.
     * \param[in] visible, true to show the ruler, false to hide.
     */
    void setRulerVisibility(bool visible);

    /* \brief Shows/hides the segmentations.
     * \param[in] visible, true to show the segmentations, false to hide.
     */
    void showSegmentations(bool visible);

    /* \brief Shows/hides the thumbnail in 2D views.
     * \param[in] visible, true to show the thumbnail, false to hide.
     */
    void showThumbnail(bool visible);

    /* \brief Switches visibility between channels.
     *
     */
    void switchPreprocessing();

    /* \brief Toggles "fit to slices" boolean value.
     * \param[in] unused, unused value.
     */
    void setFitToSlices(bool unused);

    /* \brief Sets the crosshair point in the views.
     * \param[in] point, point to set the crosshair.
     * \param[in] force, force centering the view in the point.
     */
    virtual void setCrosshairPoint(const NmVector3& point, bool force = false);

    /* \brief Changes a plane position.
     * \param[in] plane, plane to change.
     * \param[in] pos, position to set the plane.
     */
    virtual void changePlanePosition(Plane plane, Nm pos);

    // virtual void setSliceSelectors(SliceView::SliceSelectors selectors);
  protected:
    /* \brief Overrides QAbstractItemView::rowsInserted().
     *
     */
    virtual void rowsInserted(const QModelIndex& parent, int start, int end);

    /* \brief Overrides QAbstractItemView::rowsAboutToBeRemoved().
     *
     */
    virtual void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);

    /* \brief Updates XY, YZ, XZ and 3D views.
     *
     */
    void updateViews();

    /* \brief Initializes a 2D view.
     * \pararm[in] view, View2D raw pointer of the view to initialize.
     */
    void initView2D(View2D *view);

    /* \brief Clones a renderer.
     * \param[in] name, name of the renderer to clone.
     */
    RendererSPtr renderer(const QString& name) const;

    // void selectFromSlice(double slice, PlaneType plane);
    // void selectToSlice(double slice, PlaneType plane);
  private:
    ModelAdapterSPtr m_model;
    ViewManagerSPtr  m_viewManager;

    bool m_showProcessing;
    bool m_showSegmentations;

    NmVector3 m_slicingStep;

    QColor m_xLine, m_yLine, m_zLine;

    View2D *m_viewXY, *m_viewYZ, *m_viewXZ;
    View3D *m_view3D;

    QDockWidget *dock3D, *dockYZ, *dockXZ;
    QAction     *m_showRuler, *m_showThumbnail;

    RenderersMenu      *m_renderersMenu;
    CamerasMenu        *m_camerasMenu;
  };


  using DefaultViewSPtr = std::shared_ptr<DefaultView>;

} // namespace ESPINA

#endif // ESPINA_DEFAULT_VIEW_H
