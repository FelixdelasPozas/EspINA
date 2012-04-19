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
#include "IPreferencePanel.h"
#include "common/selection/SelectableView.h"

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

class SliceViewPreferences;

class SliceViewPreferencesPanel : public QWidget
{
  Q_OBJECT

public:
  explicit SliceViewPreferencesPanel(SliceViewPreferences *preferences);

public slots:
  void setInvertWheel(bool value);
  void setInvertNormal(bool value);
  void setShowAxis(bool value);
private:
  SliceViewPreferences *m_pref;
};

class SliceViewPreferences : public ISettingsPanel
{
public:
  explicit SliceViewPreferences(vtkPVSliceView::VIEW_PLANE plane);

  virtual const QString shortDescription();
  virtual const QString longDescription() {return shortDescription();}
  virtual const QIcon icon() {return QIcon();}

  virtual QWidget* widget() {return new SliceViewPreferencesPanel(this);}

  void setInvertWheel(bool value);
  bool invertWheel(){return m_InvertWheel;}
  void setInvertSliceOrder(bool value);
  bool invertSliceOrder() {return m_InvertSliceOrder;}
  void setShowAxis(bool value);
  bool showAxis() {return m_ShowAxis;}

private:
  bool m_InvertWheel;
  bool m_InvertSliceOrder;
  bool m_ShowAxis;

private:
  vtkPVSliceView::VIEW_PLANE m_plane;
  QString viewSettings;
};

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

public:
  SliceView(vtkPVSliceView::VIEW_PLANE plane = vtkPVSliceView::AXIAL, QWidget* parent = 0);
  virtual ~SliceView();

//   IPreferencePanel *preferences() {return m_preferences;}
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

//   QList<Segmentation *> pickSegmentationsAt(int x, int y, int z);
//   QList<Segmentation *> pickSegmentationsAt(ISelectionHandler::VtkRegion region);
//   void selectSegmentations(int x, int y, int z);
//
  void addChannelRepresentation(Channel *channel);
  void removeChannelRepresentation(Channel *channel);

  void addSegmentationRepresentation(Segmentation *seg);
  void removeSegmentationRepresentation(Segmentation *seg);
  bool updateSegmentationRepresentation(Segmentation* seg);

  void addRepresentation(pqOutputPort *oport);
  void removeRepresentation(pqOutputPort *oport);
  
  virtual void addPreview(Filter* filter);
  virtual void removePreview(Filter* filter);
  virtual void previewExtent(int VOI[6]);
  void addPreview(pqOutputPort *preview);
  void removePreview(pqOutputPort *preview);

  void addWidget(pq3DWidget *widget);
  void removeWidget(pq3DWidget *widget);

  void setColorEngine(ColorEngine *engine){m_colorEngine = engine;}

public slots:
  // Espina has been connected to a new server
  void onConnect();
  // Espina has been disconnected from server
  void onDisconnect();

  /// Show/Hide segmentations
  void setSegmentationVisibility(bool visible);
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
  // Notify the windows manager how to display the view
  void closeRequest();
  void maximizeRequest();
  void minimizeRequest();
  void undockRequest();

  void selectedFromSlice(double, vtkPVSliceView::VIEW_PLANE);
  void selectedToSlice(double, vtkPVSliceView::VIEW_PLANE);

protected:
  // AbstractItemView Interfacec
  virtual bool eventFilter(QObject* caller, QEvent* e);
  void centerCrosshairOnMousePosition();
  void centerViewOnMousePosition();
  void updateChannelOpacity();

  double sliceValue() const;

  /// Converts point from Display coordinates to World coordinates
  SelectionHandler::VtkRegion display2vtk(const QPolygonF &region);

  void buildTitle();
  void buildControls();
private:
  struct SegRep
  {
    pqOutputPort *outport;
    vtkSMRepresentationProxy *proxy;
    bool visible;
    bool selected;
    QColor color;
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

  SliceViewPreferences *m_preferences;
  bool m_fitToGrid;
  double m_gridSize[3];
  double m_range[6];
  double m_center[3];
  ColorEngine *m_colorEngine;

  bool m_inThumbnail;
  QMap<Channel *, vtkSMRepresentationProxy *> m_channels;
  QMap<Segmentation *, SegRep> m_segmentations;
  QList<pq3DWidget *>          m_widgets;
  vtkSMRepresentationProxy *prevRep;
  Filter *m_preview;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SliceView::SliceSelectors)

#endif // SLICEVIEW_H
