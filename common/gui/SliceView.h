/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.es>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//----------------------------------------------------------------------------
// File:    SliceView.h
// Purpose: Display channels and segmentations using slices
//----------------------------------------------------------------------------
#ifndef SLICEVIEW_H
#define SLICEVIEW_H

#include <QWidget>

#include <vtkSmartPointer.h>
#include <common/views/vtkPVSliceView.h>
#include "common/settings/ISettingsPanel.h"
#include "common/selection/SelectableView.h"
#include <common/widgets/EspinaWidget.h>

class Representation;
class vtkPropPicker;
class Channel;
class ColorEngine;
class QPushButton;
class pq3DWidget;
class vtkSMRepresentationProxy;
class pqDataRepresentation;

//Forward declaration
class Channel;
class Segmentation;
class pqOutputPort;
class pqPipelineSource;
class pqSliceView;
class vtkInteractorStyleEspinaSlice;

// GUI
class QLabel;
class QScrollBar;
class QSpinBox;
class QVBoxLayout;
class QHBoxLayout;

/// Slice View Widget
/// Displays a unique slice of a channel or segmentation
class SliceView
: public QWidget
, public SelectableView
{
  Q_OBJECT
public:
  enum SliceSelector
  {
    NoSelector = 0x0,
    From       = 0x1,
    To         = 0x2
  };
  Q_DECLARE_FLAGS(SliceSelectors, SliceSelector)

  class Settings;
  typedef QSharedPointer<Settings> SettingsPtr;

public:
  SliceView(vtkPVSliceView::VIEW_PLANE plane = vtkPVSliceView::AXIAL, QWidget* parent = 0);
  virtual ~SliceView();

  inline QString title() const;
  void setTitle(const QString &title);


  void setGridSize(double size[3]);
  void setRanges(double ranges[6]/*nm*/);
  void setFitToGrid(bool value);
  void centerViewOn(double center[3]/*nm*/);
  void setCrossHairColors(double hcolor[3], double vcolor[3]);
  void resetCamera();

  // Interface of SelectableView
  bool pickChannel(int x, int y, double pickPos[3]);
  virtual void eventPosition(int &x, int &y);
  virtual SelectionHandler::MultiSelection select(SelectionHandler::SelectionFilters filters, SelectionHandler::ViewRegions regions);
  virtual pqRenderViewBase *view();

  void addChannelRepresentation(Channel *channel);
  void removeChannelRepresentation(Channel *channel);
  bool updateChannelRepresentation(Channel *channel);

  void addSegmentationRepresentation(Segmentation *seg);
  void removeSegmentationRepresentation(Segmentation *seg);
  bool updateSegmentationRepresentation(Segmentation* seg);

  void addRepresentation(pqOutputPort *oport, QColor color);
  void removeRepresentation(pqOutputPort *oport);

  virtual void addPreview(Filter* filter);
  virtual void removePreview(Filter* filter);
  virtual void previewExtent(int VOI[6]);
  void addPreview(pqOutputPort *preview);
  void removePreview(pqOutputPort *preview);

  void addWidget(SliceWidget *sWidget);
  void removeWidget(SliceWidget *sWidget);

  void setColorEngine(ColorEngine *engine){m_colorEngine = engine;}
  SettingsPtr settings() {return m_settings;}

public slots:
  // Espina has been connected to a new server
  void onConnect();
  // Espina has been disconnected from server
  void onDisconnect();

  /// Show/Hide segmentations
  void setSegmentationVisibility(bool visible);
  /// Show/Hide Preprocessing
  void setShowPreprocessing(bool visible);
  /// Show/Hide the ruler
  void setRulerVisibility(bool visible);
  /// Show/Hide Slice Selector
  void setSliceSelectors(SliceSelectors selectors);

  void forceRender();

protected slots:
  void close();
  void maximize();
  void minimize();
  void undock();

  void sliceViewCenterChanged(double x, double y, double z);
  void scrollValueChanged(int value);
  void selectFromSlice();
  void selectToSlice();

  void updateWidgetVisibility();

signals:
  void centerChanged(double, double, double);
  void focusChanged (double[3]);
  // Notify the windows manager how to display the view
  void closeRequest();
  void maximizeRequest();
  void minimizeRequest();
  void undockRequest();

  void channelSelected(Channel *);
  void segmentationSelected(Segmentation *, bool);
  void selectedFromSlice(double, vtkPVSliceView::VIEW_PLANE);
  void selectedToSlice(double, vtkPVSliceView::VIEW_PLANE);

protected:
  virtual bool eventFilter(QObject* caller, QEvent* e);
  void centerCrosshairOnMousePosition();
  void centerViewOnMousePosition();
  QList<Channel *> pickChannels(double vx, double vy, vtkRenderer *renderer, bool repeatable=true);
  QList<Segmentation *> pickSegmentations(double vx, double vy, vtkRenderer *renderer, bool repeatable=true);
  void selectPickedItems(bool append);

  double suggestedChannelOpacity();

  double sliceValue() const;

  /// Converts point from Display coordinates to World coordinates
  SelectionHandler::VtkRegion display2vtk(const QPolygonF &region);

  void buildTitle();
  void buildControls();
private:
  struct RepInfo
  {
    pqOutputPort *outport;
    vtkSMRepresentationProxy *proxy;
    bool visible;
    bool selected;
    QColor color;
    double pos[3];
  };

  vtkPVSliceView::VIEW_PLANE m_plane;

  pqSliceView *m_view;

  // GUI
  QHBoxLayout *m_titleLayout;
  QLabel      *m_title;
  QVBoxLayout *m_mainLayout;
  QHBoxLayout *m_controlLayout;
  QWidget     *m_viewWidget;
  QScrollBar  *m_scrollBar;
  QPushButton *m_fromSlice;
  QSpinBox    *m_spinBox;
  QPushButton *m_toSlice;

  SettingsPtr m_settings;

  bool m_fitToGrid;
  double m_gridSize[3];
  double m_range[6];
  double m_center[3];
  ColorEngine *m_colorEngine;

  bool m_inThumbnail;
  QMap<Channel *, RepInfo>      m_channels;
  QMap<Segmentation *, RepInfo> m_segmentations;
  QMap<pqOutputPort *, RepInfo> m_representations;

  QList<SliceWidget *>      m_widgets;
  Filter *m_preview;
};

class SliceView::Settings 
{
  const QString INVERT_SLICE_ORDER;
  const QString INVERT_WHEEL;
  const QString SHOW_AXIS;

public:
  explicit Settings(vtkPVSliceView::VIEW_PLANE plane, const QString prefix=QString());

  void setInvertWheel(bool value);
  bool invertWheel() const {return m_InvertWheel;}

  void setInvertSliceOrder(bool value);
  bool invertSliceOrder() const {return m_InvertSliceOrder;}

  void setShowAxis(bool value);
  bool showAxis() const {return m_ShowAxis;}

  vtkPVSliceView::VIEW_PLANE plane() const {return m_plane;}

private:
  static const QString view(vtkPVSliceView::VIEW_PLANE plane);

private:
  bool m_InvertWheel;
  bool m_InvertSliceOrder;
  bool m_ShowAxis;

private:
  vtkPVSliceView::VIEW_PLANE m_plane;
  QString viewSettings;
};


Q_DECLARE_OPERATORS_FOR_FLAGS(SliceView::SliceSelectors)

#endif // SLICEVIEW_H
