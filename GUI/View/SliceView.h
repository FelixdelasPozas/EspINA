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

//----------------------------------------------------------------------------
// File:    SliceView.h
// Purpose: Display channels and segmentations using slices
//----------------------------------------------------------------------------
#ifndef SLICEVIEW_H
#define SLICEVIEW_H

#include "EspinaGUI_Export.h"

// EspINA
#include "EspinaRenderView.h"
#include "GUI/ViewManager.h"
#include "GUI/ISettingsPanel.h"
#include "GUI/vtkWidgets/EspinaWidget.h"
#include <Core/Model/HierarchyItem.h>
#include <Core/Model/EspinaFactory.h>

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
  class ColorEngine;
  class Representation;

  /// Slice View Widget
  /// Display channels and segmentations as slices
  class EspinaGUI_EXPORT SliceView
  : public EspinaRenderView
  {
    Q_OBJECT
    class State;
    class AxialState;
    class SagittalState;
    class CoronalState;

  public:
    class Settings;
    typedef boost::shared_ptr<Settings> SettingsSPtr;
    static const double SEGMENTATION_SHIFT;

  public:
    explicit SliceView(EspinaFactoryPtr factory, ViewManager *vm, PlaneType plane = AXIAL, QWidget* parent = 0);
    virtual ~SliceView();

    virtual void reset();

    inline QString title() const;
    void setTitle(const QString &title);

    ViewManager *viewManager() const        { return m_viewManager; }
    PlaneType plane() const                 { return m_plane; }

    double segmentationDepth() const
    {
      return AXIAL == m_plane ? -SliceView::SEGMENTATION_SHIFT : SliceView::SEGMENTATION_SHIFT;
    }

    void slicingStep(Nm steps[3]);
    /// Set the distance between two consecutive slices when
    /// displacement is set to SLICES
    void setSlicingStep(const Nm steps[3]);
    Nm slicingPosition() const;

    void centerViewOn(Nm center[3], bool force = false);//WARNING Esta enmascarando un metodo de la clase base
    void centerViewOnPosition(Nm center[3]); // this does not change slice positions
    void setCrosshairColors(double hcolor[3], double vcolor[3]);
    void setThumbnailVisibility(bool visible);

    virtual void addWidget   (EspinaWidget* widget);
    virtual void removeWidget(EspinaWidget* eWidget);

    virtual void addActor   (vtkProp *actor);
    virtual void removeActor(vtkProp *actor);

    virtual void previewBounds(Nm bounds[6], bool cropToSceneBounds = true);

    virtual ISelector::PickList pick(ISelector::PickableItems filter,
                                   ISelector::DisplayRegionList regions);
    virtual void worldCoordinates(const QPoint& displayPos,
                                  double worldPos[3]);

    virtual void setSelectionEnabled(bool enabe);

    virtual void updateView();
    virtual void resetCamera();
    virtual void showCrosshairs(bool);

    SettingsSPtr settings() { return m_settings; }

    void updateCrosshairPoint(PlaneType plane, Nm slicepos);

    void addRendererControls(IRendererSPtr renderer);
    void removeRendererControls(const QString name);

    virtual GraphicalRepresentationSPtr cloneRepresentation(GraphicalRepresentationSPtr prototype);

  public slots:
    /// Show/Hide Preprocessing
    void setShowPreprocessing(bool visible);
    /// Show/Hide the ruler
    void setRulerVisibility(bool visible);
    /// Set Slice Selection flags to all registered Slice Views
    void addSliceSelectors(SliceSelectorWidget *widget,
                           ViewManager::SliceSelectors selectors);
    /// Unset Slice Selection flags to all registered Slice Views
    void removeSliceSelectors(SliceSelectorWidget *widget);

    /// Update Selected Items
    virtual void updateSceneBounds();

    virtual void updateSelection() {};

  protected slots:
    void sliceViewCenterChanged(Nm x, Nm y, Nm z);
    void scrollValueChanged(int value);
    void spinValueChanged(double value);
    void selectFromSlice();
    void selectToSlice();

    void updateWidgetVisibility();

    virtual void updateChannelsOpactity();

  private slots:
    void onTakeSnapshot();

  signals:
    void centerChanged(Nm, Nm, Nm);
    void focusChanged(const Nm[3]);

    void channelSelected(ChannelPtr);
    void segmentationSelected(SegmentationPtr, bool);
    void sliceChanged(PlaneType, Nm);

  protected:
    /// Update GUI controls
    void setSlicingBounds(Nm bounds[6]);

    /// Perform a picking operation at (x,y) in picker.
    /// Picked position is returned via pickPos parameter
    /// If no item was picked return false, and therefore pickPos values
    /// are invalid
    bool pick(vtkPropPicker *picker, int x, int y, Nm pickPos[3]);

    virtual bool eventFilter(QObject* caller, QEvent* e);
    void centerCrosshairOnMousePosition();
    void centerViewOnMousePosition();

    ViewManager::Selection pickChannels(double vx, double vy, bool repeatable = true);
    ViewManager::Selection pickSegmentations(double vx, double vy, bool repeatable = true);

    void selectPickedItems(bool append);

    /// Converts point from Display coordinates to World coordinates
    ISelector::WorldRegion worldRegion(const ISelector::DisplayRegion &region, PickableItemPtr item);

  private:
    void updateRuler();
    void updateThumbnail();

    void initBorder(vtkPolyData* data, vtkActor* actor);
    void updateBorder(vtkPolyData *data,
                      double left, double right,
                      double upper, double lower);

    Nm  voxelBottom(int sliceIndex, PlaneType plane) const;
    Nm  voxelBottom(Nm  position,   PlaneType plane) const;
    Nm  voxelCenter(int sliceIndex, PlaneType plane) const;
    Nm  voxelCenter(Nm  position,   PlaneType plane) const;
    Nm  voxelTop   (int sliceIndex, PlaneType plane) const;
    Nm  voxelTop   (Nm  position,   PlaneType plane) const;
    int voxelSlice (Nm position,    PlaneType plane) const;

    void buildCrosshairs();
    void buildTitle();
    void setupUI();

  private:
    // GUI
    QHBoxLayout    *m_titleLayout;
    QLabel         *m_title;
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
    Nm m_crosshairPoint[3];
    Nm m_slicingStep[3];

    vtkMatrix4x4 *m_slicingMatrix;
    State *m_state;
    bool m_selectionEnabled;
    bool m_showThumbnail;
    SettingsSPtr m_settings;

    // Slice Selectors
    QPair<SliceSelectorWidget *, SliceSelectorWidget *> m_sliceSelector;

    // Crosshairs
    vtkSmartPointer<vtkPolyData> m_HCrossLineData, m_VCrossLineData;
    vtkSmartPointer<vtkActor>    m_HCrossLine, m_VCrossLine;
    double                       m_HCrossLineColor[3];
    double                       m_VCrossLineColor[3];

    // Thumbnail
    bool m_inThumbnail;
    vtkSmartPointer<vtkPolyData> m_channelBorderData, m_viewportBorderData;
    vtkSmartPointer<vtkActor>    m_channelBorder, m_viewportBorder;

    bool           m_sceneReady;
    IRendererSList m_itemRenderers;

    // Representations
    QMap<EspinaWidget *, SliceWidget *>      m_widgets;

    friend class GraphicalRepresentation;
  };

  class EspinaGUI_EXPORT SliceView::Settings
  {
    const QString INVERT_SLICE_ORDER;
    const QString INVERT_WHEEL;
    const QString SHOW_AXIS;
    const QString RENDERERS;

  public:
    explicit Settings(const EspinaFactoryPtr factory, SliceView* parent, PlaneType plane, const QString prefix = QString());
    virtual ~Settings()
    { m_renderers.clear(); };

    void setInvertWheel(bool value);
    bool invertWheel() const
    {
      return m_InvertWheel;
    }

    void setInvertSliceOrder(bool value);
    bool invertSliceOrder() const
    {
      return m_InvertSliceOrder;
    }

    void setShowAxis(bool value);
    bool showAxis() const
    {
      return m_ShowAxis;
    }

    PlaneType plane() const
    {
      return m_plane;
    }

    void setRenderers(QList<IRenderer *> values);
    QList<IRenderer *> renderers() { return m_renderers; }

  private:
    static const QString view(PlaneType plane);

  private:
    bool           m_InvertWheel;
    bool           m_InvertSliceOrder;
    bool           m_ShowAxis;
    SliceView     *m_parent;
    QList<IRenderer *> m_renderers;

  private:
    PlaneType m_plane;
    QString viewSettings;
  };

} // namespace EspINA

#endif // SLICEVIEW_H
