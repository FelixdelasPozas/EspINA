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

// EspINA
#include "GUI/View/RenderView.h"
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>

// VTK
#include <vtkSmartPointer.h>

//Forward declaration
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

namespace EspINA
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
    static const double SEGMENTATION_SHIFT;
    static const double WIDGET_SHIFT;

    enum SliceSelector
    {
      None=0x0, From = 0x1, To = 0x2
    };
    Q_DECLARE_FLAGS(SliceSelectors, SliceSelector)

  public:
    explicit View2D(Plane plane = Plane::XY, QWidget* parent = 0);
    virtual ~View2D();

    void setFitToSlices(bool value);

    bool fitToSlices() const
    { return m_fitToSlices; }

    void setInvertWheel(bool value)
    { m_invertWheel = value; }

    bool invertWheel() const
    { return m_invertWheel; }

    void setInvertSliceOrder(bool value);

    bool invertSliceOrder() const
    { return m_invertSliceOrder; }

    void setRenderers(RendererSList renderers);

    RendererSList renderers() const;

    Plane plane() const
    { return m_plane; }

    virtual void reset();

    virtual void resetCamera();

    double segmentationDepth() const
    {
      return Plane::XY == m_plane ? -View2D::SEGMENTATION_SHIFT : View2D::SEGMENTATION_SHIFT;
    }

    /** \brief Set the distance between two consecutive slices when displacement is set to SLICES
     *
     */
    void setSlicingStep(const NmVector3& steps);

    NmVector3 slicingStep() const;

    Nm slicingPosition() const;

    virtual void centerViewOn(const NmVector3& point, bool force = false);

    void centerViewOnPosition(const NmVector3& center); // this does not change slice positions

    void setCrosshairColors(const QColor& hColor, const QColor& vColor);

    void setThumbnailVisibility(bool visible);

    virtual void addActor   (vtkProp *actor);
    virtual void removeActor(vtkProp *actor);

    virtual Bounds previewBounds(bool cropToSceneBounds = true) const;

    virtual void setCrosshairVisibility(bool show);

    void updateCrosshairPoint(const Plane plane, const Nm slicePos);

    virtual RepresentationSPtr cloneRepresentation(ViewItemAdapterPtr item, Representation::Type representation);

    void activateRender(const QString &rendererName);
    void deactivateRender(const QString &rendererName);

    virtual void setVisualState(struct RenderView::VisualState);
    virtual struct RenderView::VisualState visualState();

    /* \brief Implements RenderView::select(flags, SCREEN x, SCREEN y)
     *
     */
    Selector::Selection select(const Selector::SelectionFlags flags, const int x, const int y, bool multiselection = true) const;

  public slots:
    /// Show/Hide Preprocessing
    void setShowPreprocessing(bool visible);

    /// Show/Hide the ruler
    void setRulerVisibility(bool visible);

//     /// Set Slice Selection flags to all registered Slice Views
//     void addSliceSelectors(SliceSelectorWidget* widget,
//                            SliceSelector selector);
// 
//     /// Unset Slice Selection flags to all registered Slice Views
//     void removeSliceSelectors(SliceSelectorWidget* widget);

    virtual void updateSceneBounds();

    virtual void updateSelection(){};

    virtual void updateView();

  signals:
    void centerChanged(NmVector3);

    void focusChanged(NmVector3);

    void channelSelected(ChannelAdapterPtr);

    void segmentationSelected(SegmentationAdapterPtr, bool);

    void sliceChanged(Plane, Nm);

  protected slots:
    void sliceViewCenterChanged(NmVector3 center);

    void scrollValueChanged(int value);

    void spinValueChanged(double value);

    void selectFromSlice();

    void selectToSlice();

    virtual void updateChannelsOpactity();

  protected:
    /// Update GUI controls
    void setSlicingBounds(const Bounds& bounds);

    virtual bool eventFilter(QObject* caller, QEvent* e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

    void centerCrosshairOnMousePosition();

    void centerViewOnMousePosition();

    ViewItemAdapterList pickChannels(double vx, double vy, bool repeatable = true);
    ViewItemAdapterList pickSegmentations(double vx, double vy, bool repeatable = true);

    void selectPickedItems(bool append);

  private:
    void addRendererControls(RendererSPtr renderer);
    void removeRendererControls(const QString name);
    void updateRuler();
    void updateThumbnail();
    Nm  voxelBottom(const int sliceIndex, const Plane plane) const;
    Nm  voxelBottom(const Nm  position,   const Plane plane) const;
    Nm  voxelCenter(const int sliceIndex, const Plane plane) const;
    Nm  voxelCenter(const Nm  position,   const Plane plane) const;
    Nm  voxelTop   (const int sliceIndex, const Plane plane) const;
    Nm  voxelTop   (const Nm  position,   const Plane plane) const;
    int voxelSlice (const Nm position,    const Plane plane) const;
    void buildCrosshairs();
    void setupUI();
    void initBorders(vtkPolyData* data, vtkActor *actor);
    void updateBorder(vtkPolyData *data,
                      double left, double right,
                      double upper, double lower);

    /* \brief Returns the distance in world coordinates of the height
     * of the view. Auxiliary method to take a "View State", dependent
     * on the m_plane value.
     *
     */
    double viewHeightLength();

  private slots:
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
    QPushButton    *m_renderConfig;

    // VTK View
    vtkSmartPointer<vtkRenderer>    m_thumbnail;
    vtkSmartPointer<vtkAxisActor2D> m_ruler;

    // View State
    NmVector3 m_slicingStep;

    std::unique_ptr<State> m_state;

    bool m_showThumbnail;

    // Slice Selectors
    //QPair<SliceSelectorWidget *, SliceSelectorWidget *> m_sliceSelector;

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

    friend class Representation;
  };

  inline bool isView2D(RenderView* view)
  { return dynamic_cast<View2D *>(view) != nullptr; }

  inline View2D * view2D_cast(RenderView* view)
  { return dynamic_cast<View2D *>(view); }

  Q_DECLARE_OPERATORS_FOR_FLAGS(View2D::SliceSelectors)

} // namespace EspINA

#endif // ESPINA_VIEW_2D_H
