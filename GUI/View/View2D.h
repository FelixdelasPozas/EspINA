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
    /** \brief View2D class constructor.
     * \param[in] plane view's orientation plane.
     * \param[in] parent raw pointer of the QWidget parent of this one.
     */
    explicit View2D(GUI::View::ViewState &state, Plane plane = Plane::XY);

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
    { return m_scaleValue; }

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

    virtual Bounds previewBounds(bool cropToSceneBounds = true) const override;

    virtual void setCameraState(CameraState state) override;

    virtual CameraState cameraState() override;

    virtual vtkRenderer *mainRenderer() const override;

    virtual void showEvent(QShowEvent *event) override;

  public slots:
    /** \brief Sets the scale visibility.
     * \param[in] visibile true to set the ruler visible, false otherwise.
     *
     */
    void setScaleVisibility(bool visible);

  signals:
    void channelSelected(ChannelAdapterPtr);

    void segmentationSelected(SegmentationAdapterPtr, bool);

  protected:
    virtual void resetImplementation() override;

    virtual bool eventFilter(QObject* caller, QEvent* e) override;

    /** \brief Updates the value of the crosshair to the mouse position and signals the change().
     *
     */
    void centerCrosshairOnMousePosition(int x, int y);

  private:
    virtual void addActor   (vtkProp *actor) override;

    virtual void removeActor(vtkProp *actor) override;

    virtual void updateViewActions(GUI::Representations::RepresentationManager::ManagerFlags flags) override;

    virtual void resetCameraImplementation() override;

    virtual bool isCrosshairPointVisible() const override;

    virtual void refreshViewImplementation() override;

    virtual Selector::Selection pickImplementation(const Selector::SelectionFlags flags, const int x, const int y, bool multiselection = true) const override;

    virtual void configureManager(GUI::Representations::RepresentationManagerSPtr manager) override;

    virtual void normalizeWorldPosition(NmVector3 &point) const override;

    NmVector3 toNormalizeWorldPosition(vtkRenderer *renderer, int x, int y) const;

    vtkSmartPointer<vtkRenderer> rendererUnderCursor() const;

    /** \brief Updates the scale widget.
     *
     */
    void updateScale();

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

    void updateScaleValue();

    void updateThumbnailBounds(const Bounds &bounds);

    void updateWidgetLimits(const Bounds &bounds);

    void updateSpinBoxLimits(int min, int max);

    void updateScrollBarLimits(int min, int max);

    inline bool fitToSlices() const;

    inline Nm  voxelCenter(const int slice, const Plane plane) const;

    inline Nm  voxelCenter(const Nm position, const Plane plane) const;

    inline int voxelSlice(const Nm position, const Plane plane) const;

  private slots:
    virtual void onCrosshairChanged(const GUI::Representations::FrameCSPtr frame) override;

    /** \brief Centers view camera on the given point.
     * \param[in] center point to center camera on.
     *
     */
    virtual void moveCamera(const NmVector3 &point) override;

    virtual void onSceneResolutionChanged(const NmVector3 &reslotuion) override;

    virtual void onSceneBoundsChanged(const Bounds &bounds) override;

    virtual void addSliceSelectors(SliceSelectorSPtr selector, SliceSelectionType type) override;

    virtual void removeSliceSelectors(SliceSelectorSPtr selector) override;

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

    virtual const QString viewName() const override;
  private:
    // GUI
    QVBoxLayout    *m_mainLayout;
    QHBoxLayout    *m_controlLayout;
    QHBoxLayout    *m_fromLayout;
    QHBoxLayout    *m_toLayout;
    QScrollBar     *m_scrollBar;
    QDoubleSpinBox *m_spinBox;
    QPushButton    *m_cameraReset;
    QPushButton    *m_snapshot;

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
    double                           m_scaleValue;
    bool                             m_scaleVisibility;
    vtkSmartPointer<vtkAxisActor2D>  m_scale;

    Plane  m_plane;
    int    m_normalCoord;

    bool  m_invertWheel;
    bool  m_invertSliceOrder;
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
} // namespace ESPINA

#endif // ESPINA_VIEW_2D_H
