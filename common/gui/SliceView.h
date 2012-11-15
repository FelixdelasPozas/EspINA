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

#include "EspinaRenderView.h"

#include "common/settings/ISettingsPanel.h"
#include "common/gui/ViewManager.h"
#include "common/widgets/EspinaWidget.h"

#include <itkImageToVTKImageFilter.h>

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkCellPicker.h>
#include <vtkPolyData.h>
#include <vtkAxisActor2D.h>

//Forward declaration
class Channel;
class ColorEngine;
class Representation;
class Segmentation;
class TransparencySelectionHighlighter;

class vtkImageResliceToColors;
class vtkImageActor;
class vtkInteractorStyleEspinaSlice;
class vtkRenderWindow;
class vtkView;
class QVTKWidget;

// GUI
class QLabel;
class QScrollBar;
class QSpinBox;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;

/// Slice View Widget
/// Display channels and segmentations as slices
class SliceView
: public EspinaRenderView
{
  Q_OBJECT
  class State;
  class AxialState;
  class SagittalState;
  class CoronalState;

public:
  class Settings;
  typedef QSharedPointer<Settings> SettingsPtr;

public:
  explicit SliceView(ViewManager *vm, PlaneType plane = AXIAL, QWidget* parent = 0);
  virtual ~SliceView();

  inline QString title() const;
  void setTitle(const QString &title);

  void slicingStep(Nm steps[3]);
  /// Set the distance between two consecutive slices when
  /// displacement is set to SLICES
  void setSlicingStep(Nm steps[3]);
  Nm slicingPosition() const;

  void centerViewOn(Nm center[3], bool force = false);
  void setCrosshairColors(double hcolor[3], double vcolor[3]);
  void setThumbnailVisibility(bool visible);

  virtual void addChannel(Channel* channel);
  virtual void removeChannel(Channel* channel);
  virtual bool updateChannel(Channel* channel);

  virtual void addSegmentation(Segmentation* seg);
  virtual void removeSegmentation(Segmentation* seg);
  virtual bool updateSegmentation(Segmentation* seg);

  virtual void addWidget(EspinaWidget* widget);
  virtual void removeWidget(EspinaWidget* eWidget);

  virtual void addPreview(vtkProp3D *preview);
  virtual void removePreview(vtkProp3D *preview);
  virtual void previewBounds(Nm bounds[6]);

  virtual void setCursor(const QCursor& cursor);

  virtual void eventPosition(int &x, int &y);
  virtual IPicker::PickList pick(IPicker::PickableItems filter,
                                 IPicker::DisplayRegionList regions);
  virtual void worldCoordinates(const QPoint& displayPos,
                                double worldPos[3]);

  virtual void setSelectionEnabled(bool enabe);

  virtual vtkRenderWindow *renderWindow();
  virtual vtkRenderer* mainRenderer();

  virtual void updateView();
  virtual void resetCamera();
  virtual void showCrosshairs(bool);

  SettingsPtr settings() { return m_settings; }

  void updateSegmentationRepresentations();
  void UpdateCrosshairPoint(PlaneType plane, Nm slicepos);

  virtual void updateSelection(){}
public slots:
  /// Show/Hide segmentations
  void setSegmentationVisibility(bool visible);
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
  virtual void updateSelection(ViewManager::Selection selection);
  virtual void updateSceneBounds();

protected slots:
  void sliceViewCenterChanged(Nm x, Nm y, Nm z);
  void scrollValueChanged(int value);
  void selectFromSlice();
  void selectToSlice();

  void updateWidgetVisibility();

signals:
  void centerChanged(Nm, Nm, Nm);
  void focusChanged(const Nm[3]);

  void channelSelected(Channel *);
  void segmentationSelected(Segmentation *, bool);
  void sliceChanged(PlaneType, Nm);

protected:
  /// Update GUI controls
  void setSlicingBounds(Nm bounds[6]);

  /// Perform a picking operation at (x,y) in picker.
  /// Picked position is returned via pickPos parameter
  /// If no item was picked return false, and therefore pickPos values
  /// are invalid
  bool pick(vtkPicker *picker, int x, int y, Nm pickPos[3]);

  virtual bool eventFilter(QObject* caller, QEvent* e);
  void centerCrosshairOnMousePosition();
  void centerViewOnMousePosition();
  QList<Channel *> pickChannels(double vx, double vy, vtkRenderer *renderer, bool repeatable = true);
  QList<Segmentation *> pickSegmentations(double vx, double vy, vtkRenderer *renderer, bool repeatable = true);
  void selectPickedItems(bool append);


  /// Convenience function to get vtkProp3D's channel
  Channel *property3DChannel(vtkProp3D *prop);
  /// Convenience function to get vtkProp3D's segmentation
  Segmentation *property3DSegmentation(vtkProp3D *prop);

  /// Converts point from Display coordinates to World coordinates
  IPicker::WorldRegion worldRegion(const IPicker::DisplayRegion &region, PickableItem *item);

private:
  void updateRuler();
  void updateThumbnail();

  void initBorder(vtkPolyData* data, vtkActor* actor);
  void updateBorder(vtkPolyData *data,
                    double left, double right,
                    double upper, double lower);

  void buildCrosshairs();
  void buildTitle();
  void setupUI();

  void addActor(vtkProp *actor);
  void removeActor(vtkProp *actor);

private:
  struct SliceRep
  {
    vtkSmartPointer<vtkImageResliceToColors> resliceToColors;
    vtkSmartPointer<vtkLookupTable> lut;
    vtkImageActor *slice;
    bool visible;
    bool selected;
    QColor color;
    Nm pos[3];
  };

  ViewManager *m_viewManager;

  // GUI
  QHBoxLayout *m_titleLayout;
  QLabel      *m_title;
  QVBoxLayout *m_mainLayout;
  QHBoxLayout *m_controlLayout;
  QHBoxLayout *m_fromLayout;
  QHBoxLayout *m_toLayout;
  QVTKWidget  *m_view;
  QScrollBar  *m_scrollBar;
  QSpinBox    *m_spinBox;

  // VTK View
  vtkRenderWindow                *m_renderWindow;
  vtkSmartPointer<vtkRenderer>    m_renderer;
  vtkSmartPointer<vtkRenderer>    m_thumbnail;
  vtkSmartPointer<vtkCellPicker>  m_channelPicker;
  vtkSmartPointer<vtkCellPicker>  m_segmentationPicker;
  vtkSmartPointer<vtkAxisActor2D> m_ruler;
  bool                            m_rulerVisibility;

  // View State
  Nm m_crosshairPoint[3];
  Nm m_slicingStep[3];

  PlaneType m_plane;
  vtkMatrix4x4 *m_slicingMatrix;
  State *m_state;
  bool m_selectionEnabled;
  bool m_showSegmentations;
  bool m_showThumbnail;
  SettingsPtr m_settings;

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

  bool m_sceneReady;

  // Representations
  TransparencySelectionHighlighter   *m_highlighter;
  QMap<Channel *,      SliceRep>      m_channelReps;
  QMap<Segmentation *, SliceRep>      m_segmentationReps;
  QMap<EspinaWidget *, SliceWidget *> m_widgets;
};


class SliceView::Settings
{
  const QString INVERT_SLICE_ORDER;
  const QString INVERT_WHEEL;
  const QString SHOW_AXIS;

public:
  explicit Settings(PlaneType plane, const QString prefix = QString());

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

private:
  static const QString view(PlaneType plane);

private:
  bool m_InvertWheel;
  bool m_InvertSliceOrder;
  bool m_ShowAxis;

private:
  PlaneType m_plane;
  QString viewSettings;
};

#endif // SLICEVIEW_H
