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

#include <QWidget>

#include "common/settings/ISettingsPanel.h"
#include "common/selection/SelectableView.h"
#include "common/widgets/EspinaWidget.h"

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkCellPicker.h>
#include <vtkPolyData.h>
#include <itkImageToVTKImageFilter.h>

class vtkImageResliceToColors;
class vtkImageActor;
class QVTKWidget;
class vtkRenderWindow;
class vtkView;

//Forward declaration
class Channel;
class ColorEngine;
class Representation;
class Segmentation;

class vtkInteractorStyleEspinaSlice;

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
: public QWidget
, public SelectableView
{
  Q_OBJECT

  class State;
  class AxialState;
  class SagittalState;
  class CoronalState;

public:
  enum SliceSelector
  {
    NoSelector = 0x0, From = 0x1, To = 0x2
  };Q_DECLARE_FLAGS(SliceSelectors, SliceSelector)

  class Settings;
  typedef QSharedPointer<Settings> SettingsPtr;

public:
  SliceView(PlaneType plane = AXIAL, QWidget* parent = 0);
  virtual ~SliceView();

  inline QString title() const;
  void setTitle(const QString &title);

  /// Set the distance between two consecutive slices when
  /// displacement is set to SLICES
  void setSlicingStep(Nm steps[3]);
  /// Set ranges in which slices are contained
  void setSlicingRanges(Nm ranges[6]);
  void centerViewOn(Nm center[3], bool force = false);
  void setCrosshairColors(double hcolor[3], double vcolor[3]);
  void setCrosshairVisibility(bool visible);
  void setThumbnailVisibility(bool visible);
  void resetCamera();

  // Interface of SelectableView
  bool pickChannel(int x, int y, Nm pickPos[3]);
  virtual void eventPosition(int &x, int &y);
  virtual SelectionHandler::MultiSelection select(SelectionHandler::SelectionFilters filters, SelectionHandler::ViewRegions regions);
  virtual vtkRenderWindow *renderWindow();
  virtual QVTKWidget *view();

  void addChannelRepresentation(Channel *channel);
  void removeChannelRepresentation(Channel *channel);
  bool updateChannelRepresentation(Channel *channel);

  void addSegmentationRepresentation(Segmentation *seg);
  void removeSegmentationRepresentation(Segmentation *seg);
  bool updateSegmentationRepresentation(Segmentation* seg);

  //void addRepresentation(pqOutputPort *oport, QColor color);
  //void removeRepresentation(pqOutputPort *oport);

  virtual void addPreview(Filter* filter);
  virtual void removePreview(Filter* filter);
  virtual void previewExtent(int VOI[6]);

  void addWidget(SliceWidget *sWidget);
  void removeWidget(SliceWidget *sWidget);

  void setColorEngine(ColorEngine *engine)
  {
    m_colorEngine = engine;
  }

  SettingsPtr settings()
  {
    return m_settings;
  }

public slots:
  /// Show/Hide segmentations
  void setSegmentationVisibility(bool visible);
  /// Show/Hide Preprocessing
  void setShowPreprocessing(bool visible);
  /// Show/Hide the ruler
  void setRulerVisibility(bool visible);
  /// Show/Hide Slice Selector
  void setSliceSelectors(SliceSelectors selectors);

  /// Update Selected Items
  virtual void updateSelection(SelectionHandler::MultiSelection msel);

  void forceRender();

protected slots:
  void close();
  void maximize();
  void minimize();
  void undock();

  void sliceViewCenterChanged(Nm x, Nm y, Nm z);
  void scrollValueChanged(int value);
  void selectFromSlice();
  void selectToSlice();

  void updateWidgetVisibility();

signals:
  void centerChanged(Nm, Nm, Nm);
  void showCrosshairs(bool);
  void focusChanged(const Nm[3]);

  // Notify the windows manager how to display the view
  void closeRequest();
  void maximizeRequest();
  void minimizeRequest();
  void undockRequest();

  void channelSelected(Channel *);
  void segmentationSelected(Segmentation *, bool);
  void selectedFromSlice(double, PlaneType);
  void selectedToSlice(double, PlaneType);
  void sliceChanged(PlaneType, Nm);

protected:
  virtual bool eventFilter(QObject* caller, QEvent* e);
  void centerCrosshairOnMousePosition();
  void centerViewOnMousePosition();
  QList<Channel *> pickChannels(double vx, double vy, vtkRenderer *renderer, bool repeatable = true);
  QList<Segmentation *> pickSegmentations(double vx, double vy, vtkRenderer *renderer, bool repeatable = true);
  void selectPickedItems(bool append);

  double suggestedChannelOpacity();

  Nm slicingPosition() const;

  /// Convenience funtion to get vtkProp3D's chanenl
  Channel *property3DChannel(vtkProp3D *prop);
  /// Convenience funtion to get vtkProp3D's segmentation
  Segmentation *property3DSegmentation(vtkProp3D *prop);

  /// Converts point from Display coordinates to World coordinates
  SelectionHandler::VtkRegion display2vtk(const QPolygonF &region);

  void buildCrosshairs();
  void buildTitle();
  void setupUI();
private:
  struct SliceRep
  {
    vtkImageResliceToColors *resliceToColors;
    vtkImageActor *slice;
    vtkLookupTable *lut;
    bool visible;
    bool selected;
    QColor color;
    Nm pos[3];
  };

  // GUI
  QHBoxLayout *m_titleLayout;
  QLabel      *m_title;
  QVBoxLayout *m_mainLayout;
  QHBoxLayout *m_controlLayout;
  QVTKWidget  *m_view;
  QScrollBar  *m_scrollBar;
  QPushButton *m_fromSlice;
  QSpinBox    *m_spinBox;
  QPushButton *m_toSlice;

  // VTK View
  vtkSmartPointer<vtkRenderer>   m_renderer;
  vtkSmartPointer<vtkRenderer>   m_thumbnail;
  vtkSmartPointer<vtkCellPicker> m_channelPicker;
  vtkSmartPointer<vtkCellPicker> m_segmentationPicker;

  // View State
  Nm m_crosshairPoint[3];
  Nm m_slicingRanges[6];
  Nm m_slicingStep[3];
  ColorEngine *m_colorEngine;

  PlaneType m_plane;
  vtkMatrix4x4 *m_slicingMatrix;
  State *m_state;
  bool m_showSegmentations;
  SettingsPtr m_settings;

  // Crosshairs
  vtkSmartPointer<vtkPolyData> m_HCrossLineData, m_VCrossLineData;
  vtkSmartPointer<vtkActor>    m_HCrossLine, m_VCrossLine;
  double                       m_HCrossLineColor[3];
  double                       m_VCrossLineColor[3];

  // Thumbnail
  bool m_inThumbnail;

  // Representations
  QMap<Channel *,      SliceRep> m_channels;
  QMap<Segmentation *, SliceRep> m_segmentations;
  //QMap<pqOutputPort *, RepInfo> m_representations;
  QList<SliceWidget *> m_widgets;
  Filter *m_preview;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SliceView::SliceSelectors)

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
