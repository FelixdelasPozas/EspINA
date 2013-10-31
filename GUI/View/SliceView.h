/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_SLICEVIEW_H
#define ESPINA_SLICEVIEW_H

#include "GUI/View/RenderView.h"
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>

// EspINA

// VTK
#include <vtkSmartPointer.h>

//Forward declaration
class vtkImageResliceToColors;
class vtkImageReslice;
class vtkImageMapToColors;
class vtkImageShiftScale;
class vtkImageActor;
class vtkInteractorStyleEspinaSlice;
class vtkRenderWindow;
class vtkPolyData;
class vtkAxisActor2D;
class vtkPropPicker;
class vtkRenderer;
class vtkView;
class QVTKWidget;
class QToolButton;

// GUI
class QLabel;
class QScrollBar;
class QDoubleSpinBox;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;

namespace EspINA
{

class SliceWidget;
  class ColorEngine;
  class Representation;
  class EspinaWidget;

  /// Slice View Widget
  /// Display channels and segmentations as slices
  class EspinaGUI_EXPORT SliceView
  : public RenderView
  {
    Q_OBJECT
    class State;
    class AxialState;
    class SagittalState;
    class CoronalState;

  public:
    static const double SEGMENTATION_SHIFT;

    enum SliceSelector
    {
      None=0x0, From = 0x1, To = 0x2
    };
    Q_DECLARE_FLAGS(SliceSelectors, SliceSelector)

  public:
    explicit SliceView(Plane plane = Plane::XY, QWidget* parent = 0);
    virtual ~SliceView();

    void setInvertWheel(bool value)
    { m_invertWheel = value; }

    bool invertWheel() const
    { return m_invertWheel; }

    void setInvertSliceOrder(bool value)
    { m_invertSliceOrder = value; }

    bool invertSliceOrder() const;

    void setShowAxis(bool value);

    bool showAxis() const
    { return m_ShowAxis; }

    Plane plane() const
    { return m_plane; }

    virtual void reset();

    double segmentationDepth() const
    {
      return Plane::XY == m_plane ? -SliceView::SEGMENTATION_SHIFT : SliceView::SEGMENTATION_SHIFT;
    }

    /** \brief Set the distance between two consecutive slices when displacement is set to SLICES
     *
     */
    void setSlicingStep(const NmVector3 steps);

    NmVector3 slicingStep() const;

    Nm slicingPosition() const;

    virtual void centerViewOn(const NmVector3& point, bool force = false);

    void centerViewOnPosition(const NmVector3& center); // this does not change slice positions

    void setCrosshairColors(double hcolor[3], double vcolor[3]);

    void setThumbnailVisibility(bool visible);

    virtual void addWidget   (EspinaWidget* widget);
    virtual void removeWidget(EspinaWidget* eWidget);

    virtual void addActor   (vtkProp *actor);
    virtual void removeActor(vtkProp *actor);

    virtual Bounds previewBounds(bool cropToSceneBounds = true) const;

    virtual Selector::SelectionList pick(Selector::SelectionFlags filter, Selector::DisplayRegionList regions);

    virtual Selector::Selection select(Selector::SelectionFlags flags, Selector::SelectionMask mask){/*TODO*/}

    virtual void updateView();

    virtual void resetCamera();

    virtual void showCrosshairs(bool show);

    void updateCrosshairPoint(Plane plane, Nm slicePos);

    void addRendererControls(RendererSPtr renderer);

    void removeRendererControls(const QString name);

    virtual RepresentationSPtr cloneRepresentation(RepresentationSPtr prototype);

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

  protected slots:
    void sliceViewCenterChanged(NmVector3 center);

    void scrollValueChanged(int value);

    void spinValueChanged(double value);

    void selectFromSlice();

    void selectToSlice();

    void updateWidgetVisibility();

    virtual void updateChannelsOpactity();

  private slots:
    void onTakeSnapshot();

  signals:
    void centerChanged(NmVector3);

    void focusChanged(NmVector3);

    void channelSelected(ChannelAdapterPtr);

    void segmentationSelected(SegmentationAdapterPtr, bool);

    void sliceChanged(Plane, Nm);

  protected:
    /// Update GUI controls
    void setSlicingBounds(const Bounds& bounds);

    /// Perform a picking operation at (x,y) in picker.
    /// Picked position is returned via pickPos parameter
    /// If no item was picked return false, and therefore pickPos values
    /// are invalid
    bool pick(vtkPropPicker *picker, int x, int y, Nm pickPos[3]);

    virtual bool eventFilter(QObject* caller, QEvent* e);

    void centerCrosshairOnMousePosition();

    void centerViewOnMousePosition();

    SelectableView::Selection pickChannels(double vx, double vy, bool repeatable = true);
    SelectableView::Selection pickSegmentations(double vx, double vy, bool repeatable = true);

    void selectPickedItems(bool append);

    /// Converts point from Display coordinates to World coordinates
    Selector::WorldRegion worldRegion(const Selector::DisplayRegion &region, ViewItemAdapterPtr item);

  private:
    void updateRuler();

    void updateThumbnail();

    void initBorder(vtkPolyData* data, vtkActor* actor);

    void updateBorder(vtkPolyData *data,
                      double left, double right,
                      double upper, double lower);

    Nm  voxelBottom(int sliceIndex, Plane plane) const;

    Nm  voxelBottom(Nm  position,   Plane plane) const;

    Nm  voxelCenter(int sliceIndex, Plane plane) const;

    Nm  voxelCenter(Nm  position,   Plane plane) const;

    Nm  voxelTop   (int sliceIndex, Plane plane) const;

    Nm  voxelTop   (Nm  position,   Plane plane) const;

    int voxelSlice (Nm position,    Plane plane) const;

    void buildCrosshairs();

    void setupUI();

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

    // VTK View
    vtkSmartPointer<vtkRenderer>    m_thumbnail;
    vtkSmartPointer<vtkAxisActor2D> m_ruler;
    bool                            m_rulerVisibility;
    bool                            m_fitToSlices;

    // View State
    NmVector3 m_crosshairPoint;
    NmVector3 m_slicingStep;

    vtkMatrix4x4* m_slicingMatrix;
    State* m_state;

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

    bool          m_sceneReady;
    RendererSList m_renderers;

    // Representations
    QMap<EspinaWidget *, SliceWidget *> m_widgets;

    Plane m_plane;
    bool  m_invertWheel;
    bool  m_invertSliceOrder;
    bool  m_ShowAxis;

    friend class Representation;
  };

  Q_DECLARE_OPERATORS_FOR_FLAGS(SliceView::SliceSelectors)

} // namespace EspINA
#endif // ESPINA_SLICEVIEW_H
