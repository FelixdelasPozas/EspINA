/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_VIEW_2D_H
#define ESPINA_VIEW_2D_H

// ESPINA
#include "GUI/View/RenderView.h"
#include <GUI/Widgets/SliceSelector.h>

// VTK
#include <vtkSmartPointer.h>

//Forward declaration
class vtkActor;
class vtkPolyData;
class vtkAxisActor2D;
class vtkPropPicker;
class vtkRenderer;
class QVTKWidget;
class QToolButton;

// GUI
class QScrollBar;
class QDoubleSpinBox;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;

namespace ESPINA
{
  class Representation;
  class EspinaWidget;
  class ViewRendererMenu;

  class EspinaGUI_EXPORT View2D
  : public RenderView
  {
    Q_OBJECT
    class PlanarBehaviour;
    class AxialBehaviour;
    class SagittalBehaviour;
    class CoronalBehaviour;

  public:
    enum SliceSelectionTypes
    {
      None=0x0, From = 0x1, To = 0x2
    };
    Q_DECLARE_FLAGS(SliceSelectionType, SliceSelectionTypes)

  public:
    /** \brief View2D class constructor.
     * \param[in] plane view's orientation plane.
     * \param[in] parent raw pointer of the QWidget parent of this one.
     */
    explicit View2D(GUI::View::ViewState &state, SelectionSPtr selection, Plane plane = Plane::XY);

    /** \brief View2D class virtual destructor.
     *
     */
    virtual ~View2D();

    /** \brief Reverses the efect of the mouse wheel on the view.
     * \param[in] value true to reverse the movement of the wheel, false otherwise.
     *
     */
    void setInvertWheel(bool value)
    { m_invertWheel = value; }

    /** \brief Returns true if the wheel movement is reversed in the view.
     *
     */
    bool invertWheel() const
    { return m_invertWheel; }

    /** \brief Sets the inversion of the slices in the view.
     * \param[in] value true show the slices in inverse order, false otherwise.
     *
     */
    void setInvertSliceOrder(bool value);

    /** \brief Returns true if the slices are shown in the inverse order in the view.
     *
     */
    bool invertSliceOrder() const
    { return m_invertSliceOrder; }

    /** \brief Returns the orientation plane of the view.
     *
     */
    Plane plane() const
    { return m_plane; }

    /** \brief Returns scale of the view given by world position / display pixels
     *
     */
    double scale() const
    { return m_scale; }

    /** \brief Helper method that returns the depth value required in the view to put representations above the channels' representations.
     *
     */
    double segmentationDepth() const;

    /** \brief Helper method that returns the depth value required in the view to put widgets above the channels and segmentation's representations.
     *
     */
    double widgetDepth() const;

    /** \brief Returns the value of the actual slice of the view.
     *
     */
    Nm slicingPosition() const;

    /** \brief Enables/disables the visibility of the thumbnail.
     * \param[in] visible true to make the thumbnail visible, false otherwise.
     *
     */
    void setThumbnailVisibility(bool visible);

    virtual Bounds previewBounds(bool cropToSceneBounds = true) const;

    virtual void setCameraState(CameraState state);

    virtual CameraState cameraState();


    virtual void reset();

    virtual vtkRenderer *mainRenderer() const;

  public slots:
    /** \brief Sets the ruler visibility.
     * \param[in] visibile true to set the ruler visible, false otherwise.
     *
     */
    void setRulerVisibility(bool visible);

    /// Set Slice Selection flags to all registered Slice Views
    void addSliceSelectors(SliceSelectorSPtr widget,
                           SliceSelectionType selector);

    /// Unset Slice Selection flags to all registered Slice Views
    void removeSliceSelectors(SliceSelectorSPtr widget);

  signals:
    void channelSelected(ChannelAdapterPtr);

    void segmentationSelected(SegmentationAdapterPtr, bool);

  protected:
    virtual bool eventFilter(QObject* caller, QEvent* e) override;

    virtual void keyPressEvent(QKeyEvent *e) override;

    void keyReleaseEvent(QKeyEvent *e);

    /** \brief Updates the value of the crosshair to the mouse position and signals the change().
     *
     */
    void centerCrosshairOnMousePosition(int x, int y);

    /** \brief Updates the selection of items.
     * \param[in] append if true the elements picked will be merged with the ones currently
     *  selected, if false the elements picked will be the new selection.
     *
     *  If an item is selected and also is on the picked list the merge will deselect the item.
     *
     */
    void selectPickedItems(int x, int y, bool append);

  private:
    virtual void addActor   (vtkProp *actor) override;

    virtual void removeActor(vtkProp *actor) override;

    virtual void updateViewActions(RepresentationManager::Flags flags) override;

    virtual void resetCameraImplementation();

    virtual void refreshViewImplementation();

    virtual Selector::Selection pickImplementation(const Selector::SelectionFlags flags, const int x, const int y, bool multiselection = true) const override;

    virtual void configureManager(RepresentationManagerSPtr manager);

    virtual void normalizeWorldPosition(NmVector3 &point) const;

    NmVector3 toNormalizeWorldPosition(vtkRenderer *renderer, int x, int y) const;

    vtkSmartPointer<vtkRenderer> rendererUnderCuror() const;

    /** \brief Shows tool tip for segmentations at position (x, y)
     * \param[in] x display coordinate
     * \param[in] y display coordinate
     *
     */
    void showSegmentationTooltip(const int x, const int y);

    /** \brief Updates the ruler widget.
     *
     */
    void updateRuler();

    /** \brief Updates the thumbnail.
     *
     */
    void updateThumbnail();

    /** \brief Centers the view of the camera on the mouse position.
     *
     */
    void centerViewOnMousePosition(int x, int y);

    /** \brief Helper method to setup the UI elements.
     *
     */
    void setupUI();

    /** \brief Helper method to initialize the borders data and actors.
     * \param[in] data border vtkPolyData raw pointer.
     * \param[in] actor border vtkActor raw pointer.
     *
     */
    void initBorders(vtkPolyData* data, vtkActor *actor);

    /** \brief Updates the values of the border with the given values.
     *
     */
    void updateBorder(vtkPolyData *data,
                      double left, double right,
                      double upper, double lower);

    /** \brief Returns the distance in world coordinates of the height
     * of the view. Auxiliary method to take a "View State", dependent
     * on the m_plane value.
     *
     */
    double viewHeightLength();

    bool isCrosshairPointVisible() const;

    void updateScale();

    void updateThumbnailBounds(const Bounds &bounds);

    void updateWidgetLimits(const Bounds &bounds);

    void updateSpinBoxLimits(int min, int max);

    void updateScrollBarLimits(int min, int max);

    inline bool fitToSlices() const;

    inline Nm  voxelCenter(const int slice, const Plane plane) const;

    inline Nm  voxelCenter(const Nm position, const Plane plane) const;

    inline int voxelSlice(const Nm position, const Plane plane) const;

    bool eventHandlerFilterEvent(QEvent *event);

    EventHandlerSPtr eventHandler() const;

  private slots:
    virtual void onCrosshairChanged(const NmVector3 &point);

    /** \brief Centers view camera on the given point.
     * \param[in] center point to center camera on.
     *
     */
    virtual void moveCamera(const NmVector3 &point);

    virtual void onSceneResolutionChanged(const NmVector3 &reslotuion);

    virtual void onSceneBoundsChanged(const Bounds &bounds);

    /** \brief Updates the view when the scroll widget changes its value.
     * \param[in] value new value.
     *
     */
    void scrollValueChanged(int value);

    /** \brief Updates the view when the spinbox widget changes its value.
     * \param[in] value new value.
     *
     */
    void spinValueChanged(double value);

    /** \brief Takes an image of the view and saves it to disk.
     *
     */
    void onTakeSnapshot();

    virtual const QString viewName() const;
  private:
    // GUI
    QVBoxLayout    *m_mainLayout;
    QHBoxLayout    *m_controlLayout;
    QHBoxLayout    *m_fromLayout;
    QHBoxLayout    *m_toLayout;
    QScrollBar     *m_scrollBar;
    QDoubleSpinBox *m_spinBox;
    QPushButton    *m_zoomButton;
    QPushButton    *m_snapshot;
    QPushButton    *m_repManagerMenu;


    // VTK View
    vtkSmartPointer<vtkRenderer>     m_renderer;
    vtkSmartPointer<vtkRenderer>     m_thumbnail;
    std::unique_ptr<PlanarBehaviour> m_state2D;


    // Slice Selectors
    using SliceSelectorPair = QPair<SliceSelectorSPtr, SliceSelectorSPtr>;
    QList<SliceSelectorPair> m_sliceSelectors;

    // Thumbnail
    bool m_showThumbnail;
    bool m_inThumbnail;
    bool m_inThumbnailClick;
    vtkSmartPointer<vtkPolyData> m_channelBorderData, m_viewportBorderData;
    vtkSmartPointer<vtkActor>    m_channelBorder, m_viewportBorder;

    // Ruler
    double m_scale;
    bool   m_rulerVisibility;
    vtkSmartPointer<vtkAxisActor2D>  m_ruler;

    Plane  m_plane;
    int    m_normalCoord;

    bool  m_invertSliceOrder;
    bool  m_invertWheel;
  };

  /** \brief Returns the 2D view raw pointer given a RenderView raw pointer.
   * \param[in] view RenderView raw pointer.
   *
   */
  inline View2D * view2D_cast(RenderView* view)
  { return dynamic_cast<View2D *>(view); }

  /** \brief Returns true if the view is a 2D view.
   * \param[in] view RenderView raw pointer.
   *
   */
  inline bool isView2D(RenderView* view)
  { return view2D_cast(view) != nullptr; }

  using View2DSPtr = std::shared_ptr<View2D>;

  Q_DECLARE_OPERATORS_FOR_FLAGS(View2D::SliceSelectionType)

} // namespace ESPINA

#endif // ESPINA_VIEW_2D_H
