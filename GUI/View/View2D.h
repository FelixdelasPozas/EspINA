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
    class State;
    class AxialState;
    class SagittalState;
    class CoronalState;

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
    explicit View2D(Plane plane = Plane::XY, QWidget* parent = nullptr);

    /** \brief View2D class virtual destructor.
     *
     */
    virtual ~View2D();

    /** \brief Enables/disables the "fit to slices" flag.
     * \param[in] value true to enable false otherwise.
     *
     * If fit to slices is enabled the movement between slices is the resolution of the scene.
     *
     */
    void setFitToSlices(bool value);

    /** \brief Returns the value of the "fit to slices" flag.
     *
     */
    bool fitToSlices() const
    { return m_fitToSlices; }

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

    virtual void reset();

    virtual void resetCamera();

    /** \brief Helper method that returns the depth value required in the view to put representations above the channels' representations.
     *
     */
    double segmentationDepth() const;

    /** \brief Helper method that returns the depth value required in the view to put widgets above the channels and segmentation's representations.
     *
     */
    double widgetDepth() const;

    /** \brief Set the distance between two consecutive slices when displacement is set to SLICES.
     * \param[in] steps
     *
     */
    void setSlicingStep(const NmVector3& steps);

    /** \brief Returns the slicing step of the view.
     *
     */
    NmVector3 slicingStep() const;

    /** \brief Returns the value of the actual slice of the view.
     *
     */
    Nm slicingPosition() const;


    /** \brief Sets the crosshair colors.
     * \param[in] hColor color of the horizontal line.
     * \param[in] vColor color of the vertical line.
     *
     */
    void setCrosshairColors(const QColor& hColor, const QColor& vColor);

    /** \brief Enables/disables the visibility of the thumbnail.
     * \param[in] visible true to make the thumbnail visible, false otherwise.
     *
     */
    void setThumbnailVisibility(bool visible);

    virtual void addActor   (vtkProp *actor) override;

    virtual void removeActor(vtkProp *actor) override;

    virtual Bounds previewBounds(bool cropToSceneBounds = true) const;

    /** \brief Sets the visibility of the crosshair lines.
     * \param[in] show true to set visible, false otherwise.
     *
     */
    virtual void setCrosshairVisibility(bool show);

    virtual void setVisualState(struct RenderView::VisualState);

    virtual struct RenderView::VisualState visualState();

    Selector::Selection select(const Selector::SelectionFlags flags, const int x, const int y, bool multiselection = true) const;

  public slots:
    /** \brief Alternate the visibility between the processed and unprocessed channels.
     * \param[in] visible true to show the first channel and not the second, false to reverse situation.
     *
     */
    void setShowPreprocessing(bool visible);

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


    virtual void updateView() override;

  signals:
    void channelSelected(ChannelAdapterPtr);
    void segmentationSelected(SegmentationAdapterPtr, bool);

  protected slots:
    virtual void updateSceneBounds() override;

  protected:
    virtual bool eventFilter(QObject* caller, QEvent* e) override;

    virtual void keyPressEvent(QKeyEvent *e) override;

    void keyReleaseEvent(QKeyEvent *e);

    /** \brief Updates the value of the crosshair to the mouse position and signals the change().
     *
     */
    void centerCrosshairOnMousePosition();

    /** \brief Picks and returns the channels under given position.
     * \param[in] vx x display coordinate.
     * \param[in] vy y display coordinate.
     * \param[in] repeteable if true returns the list of items, if false returns the first (if any).
     *
     */
    ViewItemAdapterList pickChannels(double vx, double vy, bool repeatable = true);

    /** \brief Picks and returns the segmentations under given position.
     * \param[in] vx x display coordinate.
     * \param[in] vy y display coordinate.
     * \param[in] repeteable if true returns the list of items, if false returns the first (if any).
     *
     */
    ViewItemAdapterList pickSegmentations(double vx, double vy, bool repeatable = true);

    /** \brief Updates the selection of items.
     * \param[in] append if true the elements picked will be merged with the ones currently
     *  selected, if false the elements picked will be the new selection.
     *
     *  If an item is selected and also is on the picked list the merge will deselect the item.
     *
     */
    void selectPickedItems(bool append);

  private:
    void addRepresentationManagerMenu(RepresentationManagerSPtr manager);

    void removeRepresentationManagerMenu(RepresentationManagerSPtr manager);

    virtual void configureManager(RepresentationManagerSPtr manager);

    /** \brief Updates the ruler widget.
     *
     */
    void updateRuler();

    /** \brief Updates the thumbnail.
     *
     */
    void updateThumbnail();

    /** \brief Changes the scroll and spinbox limit values based on the new scene bounds.
     * \param[in] bounds new scene bounds.
     *
     */
    void setSlicingBounds(const Bounds& bounds);

    /** \brief Centers the view of the camera on the mouse position.
     *
     */
    void centerViewOnMousePosition();


    /** \brief Returns the bottom value in Nm of the voxel in the given slice index and plane.
     * \param[in] sliceIndex integer slice index.
     * \param[in] plane orientation plane.
     *
     */
    Nm  voxelBottom(const int sliceIndex, const Plane plane) const;

    /** \brief Returns the bottom value in Nm of the voxel in the given Z position and plane.
     * \param[in] position Z position of the voxel.
     * \param[in] plane orientation plane.
     *
     */
    Nm  voxelBottom(const Nm position, const Plane plane) const;

    /** \brief Returns the center value in Nm of the voxel in the given slice index and plane.
     * \param[in] sliceIndex integer slice index.
     * \param[in] plane orientation plane.
     *
     */
    Nm  voxelCenter(const int sliceIndex, const Plane plane) const;

    /** \brief Returns the center value in Nm of the voxel in the given Z position and plane.
     * \param[in] position Z position of the voxel.
     * \param[in] plane orientation plane.
     *
     */
    Nm  voxelCenter(const Nm position, const Plane plane) const;

    /** \brief Returns the top value in Nm of the voxel in the given slice index and plane.
     * \param[in] sliceIndex integer slice index.
     * \param[in] plane orientation plane.
     *
     */
    Nm  voxelTop(const int sliceIndex, const Plane plane) const;

    /** \brief Returns the top value in Nm of the voxel in the given Z position and plane.
     * \param[in] position Z position of the voxel.
     * \param[in] plane orientation plane.
     *
     */
    Nm  voxelTop(const Nm  position, const Plane plane) const;

    /** \brief Returns the numerical index of the slice given the slice position and plane.
     * \param[in] position slice position.
     * \param[in] plane orientation plane.
     *
     */
    int voxelSlice (const Nm position, const Plane plane) const;

    /** \brief Helper method to build the crosshairs actors.
     *
     */
    void buildCrosshairs();

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

    bool isCrosshairVisible() const;

  private slots:
    virtual void onCrosshairChanged(const NmVector3 &point);

    /** \brief Centers view camera on the given point.
     * \param[in] center point to center camera on.
     *
     */
    virtual void moveCamera(const NmVector3 &point);

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
    vtkSmartPointer<vtkRenderer>    m_thumbnail;
    vtkSmartPointer<vtkAxisActor2D> m_ruler;

    // View State
    NmVector3 m_slicingStep;

    std::unique_ptr<State> m_state2D;

    bool m_showThumbnail;

    // Slice Selectors
    using SliceSelectorPair = QPair<SliceSelectorSPtr, SliceSelectorSPtr>;
    QList<SliceSelectorPair> m_sliceSelectors;

    // Crosshairs
    vtkSmartPointer<vtkPolyData> m_HCrossLineData, m_VCrossLineData;
    vtkSmartPointer<vtkActor>    m_HCrossLine, m_VCrossLine;
    double                       m_HCrossLineColor[3];
    double                       m_VCrossLineColor[3];

    // Thumbnail
    bool m_inThumbnail;
    vtkSmartPointer<vtkPolyData> m_channelBorderData, m_viewportBorderData;
    vtkSmartPointer<vtkActor>    m_channelBorder, m_viewportBorder;

    bool  m_sceneReady;
    Plane m_plane;
    int   m_normalCoord;

    bool  m_fitToSlices;
    bool  m_invertSliceOrder;
    bool  m_invertWheel;
    bool  m_rulerVisibility;
    bool  m_inThumbnailClick;
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


  Q_DECLARE_OPERATORS_FOR_FLAGS(View2D::SliceSelectionType)

} // namespace ESPINA

#endif // ESPINA_VIEW_2D_H
