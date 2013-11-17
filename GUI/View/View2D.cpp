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

#include "View2D.h"

// EspINA
#include "View2DState.h"
#include "Widgets/EspinaWidget.h"
#include <GUI/View/vtkInteractorStyleEspinaSlice.h>
#include <Core/Analysis/Channel.h>

// Debug
#include <QDebug>

// Qt includes
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QSettings>
#include <QDate>
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QVector3D>
#include <QWheelEvent>
#include <QMenu>
#include <QToolButton>
#include <QVTKWidget.h>

// Boost
#include <boost/concept_check.hpp>

// VTK
#include <vtkAbstractWidget.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellPicker.h>
#include <vtkCoordinate.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkImageProperty.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkInteractorStyleImage.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkMath.h>
#include <vtkProperty.h>
#include <vtkPropPicker.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkWidgetRepresentation.h>
#include <vtkWorldPointPicker.h>
#include <vtkImageShiftScale.h>
#include <vtkSmartPointer.h>
#include <vtkAxisActor2D.h>
#include <vtkRendererCollection.h>

using namespace EspINA;

const double View2D::SEGMENTATION_SHIFT = 0.05;

//-----------------------------------------------------------------------------
// SLICE VIEW
//-----------------------------------------------------------------------------
View2D::View2D(Plane plane, QWidget* parent)
: RenderView(parent)
, m_mainLayout(new QVBoxLayout())
, m_controlLayout(new QHBoxLayout())
, m_fromLayout(new QHBoxLayout())
, m_toLayout(new QHBoxLayout())
, m_scrollBar(new QScrollBar(Qt::Horizontal))
, m_spinBox(new QDoubleSpinBox())
, m_zoomButton(new QPushButton())
, m_snapshot(new QPushButton())
, m_ruler(vtkSmartPointer<vtkAxisActor2D>::New())
, m_showThumbnail(true)
// , m_sliceSelector(QPair<SliceSelectorWidget*,SliceSelectorWidget*>(nullptr, nullptr))
, m_inThumbnail(false)
, m_sceneReady(false)
, m_slicingStep{1, 1, 1}
, m_plane{plane}
, m_fitToSlices{true}
, m_invertWheel{false}
, m_invertSliceOrder{false}
, m_rulerVisibility{true}
{
  setupUI();

  switch (m_plane)
  {
    case Plane::XY:
      m_state = std::unique_ptr<State>(new AxialState());
      break;
    case Plane::XZ:
      m_state = std::unique_ptr<State>(new CoronalState());
      break;
    case Plane::YZ:
      m_state = std::unique_ptr<State>(new SagittalState());
      break;
  };
  m_normalCoord = normalCoordinateIndex(m_plane);

  // Init Render Window
  vtkRenderWindow* renderWindow = m_view->GetRenderWindow();
  renderWindow->DoubleBufferOn();
  renderWindow->SetNumberOfLayers(2);

  // Init Renderers
  m_renderer = vtkSmartPointer<vtkRenderer>::New();
  m_renderer->GetActiveCamera()->ParallelProjectionOn();
  m_renderer->LightFollowCameraOn();
  m_renderer->SetLayer(0);
  m_thumbnail = vtkSmartPointer<vtkRenderer>::New();
  m_thumbnail->SetViewport(0.75, 0.0, 1.0, 0.25);
  m_thumbnail->SetLayer(1);
  m_thumbnail->InteractiveOff();
  m_thumbnail->DrawOff();

  // Init Ruler
  m_ruler->SetPosition(0.1, 0.1);
  m_ruler->SetPosition2(0.1, 0.1);
  m_ruler->SetPickable(false);
  m_ruler->SetLabelFactor(0.8);
  m_ruler->SetFontFactor(1);
  m_ruler->SetTitle("nm");
  m_ruler->RulerModeOff();
  m_ruler->SetLabelFormat("%.0f");
  m_ruler->SetAdjustLabels(false);
  m_ruler->SetNumberOfLabels(2);
  m_ruler->SizeFontRelativeToAxisOff();
  m_renderer->AddActor(m_ruler);

  SliceViewInteractor interactor = SliceViewInteractor::New();
  interactor->AutoAdjustCameraClippingRangeOn();
  interactor->KeyPressActivationOff();
  renderWindow->AddRenderer(m_renderer);
  renderWindow->AddRenderer(m_thumbnail);
  m_view->GetInteractor()->SetInteractorStyle(interactor);

  m_channelBorderData = vtkSmartPointer<vtkPolyData>::New();
  m_channelBorder     = vtkSmartPointer<vtkActor>::New();
  initBorder(m_channelBorderData, m_channelBorder);

  m_viewportBorderData = vtkSmartPointer<vtkPolyData>::New();
  m_viewportBorder     = vtkSmartPointer<vtkActor>::New();
  initBorder(m_viewportBorderData, m_viewportBorder);

  buildCrosshairs();

  this->setAutoFillBackground(true);
  setLayout(m_mainLayout);

}

//-----------------------------------------------------------------------------
View2D::~View2D()
{
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying Slice View" << m_plane;
//   qDebug() << "********************************************************";
  // Representation destructors may need to access slice view in their destructors
  m_channelStates.clear();
  m_segmentationStates.clear();
}

//-----------------------------------------------------------------------------
void View2D::setFitToSlices(bool value)
{
  m_fitToSlices = value;
}

//-----------------------------------------------------------------------------
void View2D::setInvertSliceOrder(bool value)
{
  m_invertSliceOrder = value;
}


//-----------------------------------------------------------------------------
void View2D::setRenderers(RendererSList renderers)
{
  foreach(RendererSPtr renderer, renderers)
  {
    if (canRender(renderer, RendererType::RENDERER_SLICEVIEW))
    {
      addRendererControls(renderer->clone());
    }
  }
}

//-----------------------------------------------------------------------------
void View2D::reset()
{
  foreach(EspinaWidget *widget, m_widgets.keys())
    removeWidget(widget);

  foreach(SegmentationAdapterPtr segmentation, m_segmentationStates.keys())
    remove(segmentation);

  foreach(ChannelAdapterPtr channel, m_channelStates.keys())
    remove(channel);
}

//-----------------------------------------------------------------------------
Nm rulerScale(Nm value)
{
  int factor = 100;

  if (value < 10)
    factor = 1;
  else if (value < 25)
    factor = 5;
  else if (value < 100)
    factor = 10;
  else if (value < 250)
    factor = 50;

  int res = int(value/factor)*factor;
  return std::max(res,1);
}

//-----------------------------------------------------------------------------
void View2D::updateRuler()
{
  if (!m_renderer || !m_view->GetRenderWindow())
    return;

  double *value;
  Nm left, right;
  vtkSmartPointer<vtkCoordinate> coords = vtkSmartPointer<vtkCoordinate>::New();
  coords->SetCoordinateSystemToNormalizedViewport();

  int c = m_plane==Plane::YZ?2:0;
  coords->SetValue(0, 0); //Viewport Lower Left Corner
  value = coords->GetComputedWorldValue(m_renderer);
  left = value[c];

  coords->SetValue(1, 0); // Viewport Lower Right Corner
  value = coords->GetComputedWorldValue(m_renderer);
  right = value[c];

  Nm rulerLength = 0.07;//viewport coordinates - Configuration file
  Nm viewWidth = fabs(left-right);

  Nm scale = rulerLength * viewWidth;
  scale = rulerScale(scale);
  rulerLength = scale / viewWidth;

  m_ruler->SetRange(0, scale);
  m_ruler->SetPoint2(0.1+rulerLength, 0.1);
  m_ruler->SetVisibility(m_rulerVisibility && (0.02 < rulerLength) && (rulerLength < 0.8));
}

//-----------------------------------------------------------------------------
void View2D::updateThumbnail()
{
  if (!m_showThumbnail || !m_sceneReady)
    return;

  double *value;
  // Position of world margins acording to the display
  // Depending on the plane being shown can refer to different
  // bound components
  double viewLeft, viewRight, viewUpper, viewLower;
  vtkSmartPointer<vtkCoordinate> coords = vtkSmartPointer<vtkCoordinate>::New();

  coords->SetViewport(m_renderer);
  coords->SetCoordinateSystemToNormalizedViewport();

  int h = m_plane==Plane::YZ?2:0;
  int v = m_plane==Plane::XZ?2:1;
  coords->SetValue(0, 0); // Viewport Lower Left Corner
  value = coords->GetComputedWorldValue(m_renderer);
  viewLower = value[v]; // Lower Margin in World Coordinates
  viewLeft  = value[h]; // Left Margin in World Coordinates

  coords->SetValue(1, 1);
  value = coords->GetComputedWorldValue(m_renderer);
  viewUpper = value[v]; // Upper Margin in World Coordinates
  viewRight = value[h]; // Right Margin in Worl Coordinates

  double sceneLeft  = m_sceneBounds[2*h];
  double sceneRight = m_sceneBounds[2*h+1];
  double sceneUpper = m_sceneBounds[2*v];
  double sceneLower = m_sceneBounds[2*v+1];

  bool leftHidden   = sceneLeft  < viewLeft;
  bool rightHidden  = sceneRight > viewRight;
  bool upperHidden  = sceneUpper < viewUpper;
  bool lowerHidden  = sceneLower > viewLower;

  if (leftHidden || rightHidden || upperHidden || lowerHidden)
  {
    m_thumbnail->DrawOn();
    updateBorder(m_channelBorderData, sceneLeft, sceneRight, sceneUpper, sceneLower);
    updateBorder(m_viewportBorderData, viewLeft, viewRight, viewUpper, viewLower);
  }
  else
    m_thumbnail->DrawOff();
}

//-----------------------------------------------------------------------------
void View2D::updateSceneBounds()
{
  RenderView::updateSceneBounds();

  if (m_spinBox->minimum() == 0 && m_spinBox->maximum() == 0)
  {
    m_crosshairPoint[0] = m_crosshairPoint[1] = m_crosshairPoint[2] = 0;
  }

  setSlicingStep(m_sceneResolution);

  // we need to update the view only if a signal has been sent
  // (the volume of a channel has been updated)
  if (sender() != nullptr)
    updateView();
}

//-----------------------------------------------------------------------------
void View2D::initBorder(vtkPolyData* data, vtkActor* actor)
{
  vtkSmartPointer<vtkPoints> corners = vtkSmartPointer<vtkPoints>::New();
  corners->InsertNextPoint(m_sceneBounds[0], m_sceneBounds[2], 0); //UL
  corners->InsertNextPoint(m_sceneBounds[0], m_sceneBounds[3], 0); //UR
  corners->InsertNextPoint(m_sceneBounds[1], m_sceneBounds[3], 0); //LR
  corners->InsertNextPoint(m_sceneBounds[1], m_sceneBounds[2], 0); //LL
  vtkSmartPointer<vtkCellArray> borders = vtkSmartPointer<vtkCellArray>::New();
  borders->EstimateSize(4, 2);
  for (int i=0; i < 4; i++)
  {
    borders->InsertNextCell (2);
    borders->InsertCellPoint(i);
    borders->InsertCellPoint((i+1)%4);
  }
  data->SetPoints(corners);
  data->SetLines(borders);
  // data->Update(); According to VTK6 update should be done to the filter producing it

  vtkSmartPointer<vtkPolyDataMapper> Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  Mapper->SetInputData(data);
  actor->SetMapper(Mapper);
  actor->GetProperty()->SetLineWidth(2);
  actor->SetPickable(false);

  m_thumbnail->AddActor(actor);
}

//-----------------------------------------------------------------------------
void View2D::updateBorder(vtkPolyData* data,
                             double left, double right,
                             double upper, double lower)
{
  vtkPoints *corners = data->GetPoints();

  switch (m_plane)
  {
    case Plane::XY:
      corners->SetPoint(0, left,  upper, -0.1); //UL
      corners->SetPoint(1, right, upper, -0.1); //UR
      corners->SetPoint(2, right, lower, -0.1); //LR
      corners->SetPoint(3, left,  lower, -0.1); //LL
      break;
    case Plane::XZ:
      corners->SetPoint(0, left,  0.1, upper); //UL
      corners->SetPoint(1, right, 0.1, upper); //UR
      corners->SetPoint(2, right, 0.1, lower); //LR
      corners->SetPoint(3, left,  0.1, lower); //LL
      break;
    case Plane::YZ:
      corners->SetPoint(0, 0.1, upper,  left); //UL
      corners->SetPoint(1, 0.1, lower,  left); //UR
      corners->SetPoint(2, 0.1, lower, right); //LR
      corners->SetPoint(3, 0.1, upper, right); //LL
      break;
  }
  data->Modified();
}

//-----------------------------------------------------------------------------
Nm View2D::voxelBottom(int sliceIndex, Plane plane) const
{
  return  m_sceneBounds[2*m_normalCoord] + sliceIndex * m_slicingStep[m_normalCoord];
}

//-----------------------------------------------------------------------------
Nm View2D::voxelBottom(Nm position, Plane plane) const
{
  return voxelBottom(voxelSlice(position, plane), plane);
}

//-----------------------------------------------------------------------------
Nm View2D::voxelCenter(int sliceIndex, Plane plane) const
{
  return m_sceneBounds[2*m_normalCoord] + (sliceIndex + 0.5) * m_slicingStep[m_normalCoord];
}

//-----------------------------------------------------------------------------
Nm View2D::voxelCenter(Nm position, Plane plane) const
{
  return voxelCenter(voxelSlice(position, plane), plane);
}


//-----------------------------------------------------------------------------
Nm View2D::voxelTop(int sliceIndex, Plane plane) const
{
  return m_sceneBounds[2*m_normalCoord] + (sliceIndex + 1.0) * m_slicingStep[m_normalCoord];
}

//-----------------------------------------------------------------------------
Nm View2D::voxelTop(Nm position, Plane plane) const
{
  return voxelTop(voxelSlice(position, plane), plane);
}


//-----------------------------------------------------------------------------
int View2D::voxelSlice(Nm position, Plane plane) const
{
  return int((position-m_sceneBounds[2*m_normalCoord])/m_slicingStep[m_normalCoord]);
}

//-----------------------------------------------------------------------------
void View2D::buildCrosshairs()
{
  vtkSmartPointer<vtkPoints> HPoints = vtkSmartPointer<vtkPoints>::New();
  HPoints->InsertNextPoint(-0.5, 0, 0);
  HPoints->InsertNextPoint(0.5, 0, 0);
  vtkSmartPointer<vtkCellArray> HLine = vtkSmartPointer<vtkCellArray>::New();
  HLine->EstimateSize(1, 2);
  HLine->InsertNextCell (2);
  HLine->InsertCellPoint(0);
  HLine->InsertCellPoint(1);
  
  m_HCrossLineData = vtkSmartPointer<vtkPolyData>::New();
  m_HCrossLineData->SetPoints(HPoints);
  m_HCrossLineData->SetLines (HLine);
  
  vtkSmartPointer<vtkPolyDataMapper> HMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  HMapper->SetInputData(m_HCrossLineData);
  
  m_HCrossLine = vtkSmartPointer<vtkActor>::New();
  m_HCrossLine->SetMapper(HMapper);
  m_HCrossLine->GetProperty()->SetLineWidth(2);
  m_HCrossLine->SetPickable(false);
  
  vtkSmartPointer<vtkPoints> VPoints = vtkSmartPointer<vtkPoints>::New();
  VPoints->InsertNextPoint(0, -0.5, 0);
  VPoints->InsertNextPoint(0, 0.5, 0);
  vtkSmartPointer<vtkCellArray> VLine = vtkSmartPointer<vtkCellArray>::New();
  VLine->EstimateSize(1, 2);
  VLine->InsertNextCell (2);
  VLine->InsertCellPoint(0);
  VLine->InsertCellPoint(1);
  
  m_VCrossLineData = vtkSmartPointer<vtkPolyData>::New();
  m_VCrossLineData->SetPoints(VPoints);
  m_VCrossLineData->SetLines(VLine);
  
  vtkSmartPointer<vtkPolyDataMapper> VMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  VMapper->SetInputData(m_VCrossLineData);
  
  m_VCrossLine = vtkSmartPointer<vtkActor>::New();
  m_VCrossLine->SetMapper(VMapper);
  m_VCrossLine->GetProperty()->SetLineWidth(2);
  m_VCrossLine->SetPickable(false);
}

//-----------------------------------------------------------------------------
void View2D::setupUI()
{
  m_view->installEventFilter(this);

  m_zoomButton->setIcon(QIcon(":/espina/zoom_reset.png"));
  m_zoomButton->setToolTip(tr("Reset Camera"));
  m_zoomButton->setFlat(true);
  m_zoomButton->setIconSize(QSize(20,20));
  m_zoomButton->setMaximumSize(QSize(22,22));
  m_zoomButton->setCheckable(false);
  connect(m_zoomButton, SIGNAL(clicked()), this, SLOT(resetView()));

  m_snapshot->setIcon(QIcon(":/espina/snapshot_scene.svg"));
  m_snapshot->setToolTip(tr("Save Scene as Image"));
  m_snapshot->setFlat(true);
  m_snapshot->setIconSize(QSize(20,20));
  m_snapshot->setMaximumSize(QSize(22,22));
  m_snapshot->setEnabled(true);
  connect(m_snapshot,SIGNAL(clicked(bool)),this,SLOT(onTakeSnapshot()));

  m_scrollBar->setMaximum(0);
  m_scrollBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  m_spinBox->setMaximum(0);
  m_spinBox->setDecimals(0);
  m_spinBox->setMinimumWidth(40);
  m_spinBox->setMaximumHeight(20);
  m_spinBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  m_spinBox->setAlignment(Qt::AlignRight);
  m_spinBox->setSingleStep(1);

  connect(m_spinBox, SIGNAL(valueChanged(double)), this, SLOT(spinValueChanged(double)));
  connect(m_scrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrollValueChanged(int)));

  m_mainLayout->addWidget(m_view);
  m_controlLayout->addWidget(m_zoomButton);
  m_controlLayout->addWidget(m_snapshot);
  m_controlLayout->addWidget(m_scrollBar);
  m_controlLayout->addLayout(m_fromLayout);
  m_controlLayout->addWidget(m_spinBox);
  m_controlLayout->addLayout(m_toLayout);

  m_mainLayout->addLayout(m_controlLayout);

  // Color background
  QPalette pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Window));
  this->setPalette(pal);
  this->setStyleSheet("QSpinBox { background-color: white;}");
}

//-----------------------------------------------------------------------------
void View2D::setCrosshairColors(const QColor& hColor, const QColor& vColor)
{
  double hc[3] = {hColor.redF(), hColor.greenF(), hColor.blueF()};
  double vc[3] = {vColor.redF(), vColor.greenF(), vColor.blueF()};

  m_HCrossLine->GetProperty()->SetColor(hc);
  m_VCrossLine->GetProperty()->SetColor(vc);
}

//-----------------------------------------------------------------------------
void View2D::setCrosshairVisibility(bool visible)
{
  if (visible)
  {
    m_renderer->AddActor(m_HCrossLine);
    m_renderer->AddActor(m_VCrossLine);
  }else
  {
    m_renderer->RemoveActor(m_HCrossLine);
    m_renderer->RemoveActor(m_VCrossLine);
  }

  updateView();
}

//-----------------------------------------------------------------------------
void View2D::setThumbnailVisibility(bool visible)
{
  m_showThumbnail = visible;

  m_thumbnail->SetDraw(visible && m_sceneReady);

  updateView();
}

//-----------------------------------------------------------------------------
Selector::SelectionList View2D::pick(Selector::SelectionFlags    filter,
                                     Selector::DisplayRegionList regions)
{
  bool multiSelection = false;
  Selector::SelectionList selectedItems;

  // Select all products that belongs to all regions
  // NOTE: Should first loop be removed? Only useful to select disconnected regions...
  for(const Selector::DisplayRegion &region : regions)
  {
    for(auto p : region)
    {
      for(auto tag : filter)
      {
        if (Selector::CHANNEL == tag)
        {
          for(auto renderer : m_renderers)
            if (canRender(renderer, RenderableType::CHANNEL))
              for(auto item : renderer->pick(p.x(), p.y(), slicingPosition(), m_renderer, RenderableItems(RenderableType::CHANNEL), multiSelection))
              {
                Selector::WorldRegion wRegion = worldRegion(region, item);
                selectedItems << Selector::SelectedItem(wRegion, item);
              }
        }
        else
        {
          if (Selector::SEGMENTATION == tag)
          {
            for(auto renderer : m_renderers)
              if (canRender(renderer, RenderableType::SEGMENTATION))
                for(auto item : renderer->pick(p.x(), p.y(), slicingPosition(), m_renderer, RenderableItems(RenderableType::SEGMENTATION), multiSelection))
                {
                  Selector::WorldRegion wRegion = worldRegion(region, item);
                  selectedItems << Selector::SelectedItem(wRegion, item);
                }
          }
        }
      }
    }
  }

  return selectedItems;
}


//-----------------------------------------------------------------------------
void View2D::updateView()
{
  if (isVisible())
  {
//    qDebug() << "Updating View";
    updateRuler();
    updateWidgetVisibility();
    updateThumbnail();
    m_view->GetRenderWindow()->Render();
    m_view->update();
  }
}

//-----------------------------------------------------------------------------
void View2D::resetCamera()
{
  NmVector3 origin{ 0, 0, 0 };

  m_state->updateCamera(m_renderer ->GetActiveCamera(), origin);
  m_state->updateCamera(m_thumbnail->GetActiveCamera(), origin);

  m_thumbnail->RemoveActor(m_channelBorder);
  m_thumbnail->RemoveActor(m_viewportBorder);
  updateSceneBounds();
  updateThumbnail();
  m_renderer->ResetCamera();
  m_thumbnail->ResetCamera();
  m_thumbnail->AddActor(m_channelBorder);
  m_thumbnail->AddActor(m_viewportBorder);

  m_sceneReady = !m_channelStates.isEmpty();
}

//-----------------------------------------------------------------------------
void View2D::addWidget(EspinaWidget *eWidget)
{
  Q_ASSERT(!m_widgets.contains(eWidget));

  SliceWidget *sWidget = eWidget->createSliceWidget(this);
  if (!sWidget)
    return;

  sWidget->setSlice(slicingPosition(), m_plane);

  vtkAbstractWidget *widget = *sWidget;
  if (widget)
  {
    widget->SetCurrentRenderer(m_view->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
    widget->SetInteractor(m_view->GetRenderWindow()->GetInteractor());
    if (widget->GetRepresentation())
      widget->GetRepresentation()->SetVisibility(true);
    widget->On();
  }
  m_renderer->ResetCameraClippingRange();
  m_widgets[eWidget] = sWidget;
}

//-----------------------------------------------------------------------------
void View2D::removeWidget(EspinaWidget *eWidget)
{
  if (!m_widgets.contains(eWidget))
    return;

  vtkAbstractWidget *widget = *m_widgets[eWidget];
  widget->SetInteractor(nullptr); // calls widget->Off();
  widget->RemoveAllObservers();
  m_widgets.remove(eWidget);
}

//-----------------------------------------------------------------------------
void View2D::addActor(vtkProp* actor)
{
  vtkProp3D *actor3D = reinterpret_cast<vtkProp3D*>(actor);
  m_state->updateActor(actor3D);

  m_renderer->AddActor(actor);
  m_thumbnail->AddActor(actor);

  m_thumbnail->RemoveActor(m_channelBorder);
  m_thumbnail->RemoveActor(m_viewportBorder);

  updateThumbnail();
  m_thumbnail->ResetCamera();
  updateThumbnail();

  m_thumbnail->AddActor(m_channelBorder);
  m_thumbnail->AddActor(m_viewportBorder);
}

//-----------------------------------------------------------------------------
void View2D::removeActor(vtkProp* actor)
{
  m_renderer->RemoveActor(actor);
  m_thumbnail->RemoveActor(actor);

  updateThumbnail();
}

//-----------------------------------------------------------------------------
Bounds View2D::previewBounds(bool cropToSceneBounds) const
{
  // Display Orientation (up means up according to screen)
  // but in vtk coordinates UR[V] < LL[V]
  double LL[3], UR[3];
  // Display bounds in world coordinates
  vtkSmartPointer<vtkCoordinate> coords = vtkSmartPointer<vtkCoordinate>::New();
  coords->SetViewport(m_renderer);
  coords->SetCoordinateSystemToNormalizedViewport();
  coords->SetValue(0, 0); //LL
  memcpy(LL, coords->GetComputedWorldValue(m_renderer),3*sizeof(double));
  coords->SetValue(1, 1); //UR
  memcpy(UR, coords->GetComputedWorldValue(m_renderer),3*sizeof(double));

  int H = (Plane::YZ == m_plane)?2:0;
  int V = (Plane::XZ  == m_plane)?2:1;

  Bounds bounds;

  bounds[2*H]   = LL[H];
  bounds[2*H+1] = UR[H];
  bounds[2*V]   = UR[V];
  bounds[2*V+1] = LL[V];

  bounds[2*m_normalCoord]   = slicingPosition();
  bounds[2*m_normalCoord+1] = slicingPosition();

  if (cropToSceneBounds)
  {
    bounds[2*H]   = std::max(LL[H], m_sceneBounds[2*H]);
    bounds[2*H+1] = std::min(UR[H], m_sceneBounds[2*H+1]);
    bounds[2*V]   = std::max(UR[V], m_sceneBounds[2*V]);
    bounds[2*V+1] = std::min(LL[V], m_sceneBounds[2*V+1]);
  }

  return bounds;
}

//-----------------------------------------------------------------------------
void View2D::sliceViewCenterChanged(NmVector3 center)
{
  //qDebug() << "Slice View: " << m_plane << " has new center";
  emit centerChanged(center);
}

//-----------------------------------------------------------------------------
void View2D::scrollValueChanged(int value /*slice index */)
{
  // WARNING: Any modification to this method must be taken into account
  // at the end block of setSlicingStep
  m_crosshairPoint[m_normalCoord] = voxelCenter(value, m_plane);

  updateRepresentations();

  m_spinBox->blockSignals(true);
  m_spinBox->setValue(m_fitToSlices ? value + 1: slicingPosition());
  m_spinBox->blockSignals(false);

  updateView();

  emit sliceChanged(m_plane, slicingPosition());
}

//-----------------------------------------------------------------------------
void View2D::spinValueChanged(double value /* nm or slices depending on m_fitToSlices */)
{
  int sliceIndex = m_fitToSlices ? (value - 1) : voxelSlice(value, m_plane);
  
  m_crosshairPoint[m_normalCoord] = voxelCenter(sliceIndex, m_plane);

  updateRepresentations();

  m_scrollBar->blockSignals(true);
  m_scrollBar->setValue(m_fitToSlices? (value - 1) : vtkMath::Round(value/m_slicingStep[m_normalCoord]));
  m_scrollBar->blockSignals(false);

  updateView();

  emit sliceChanged(m_plane, slicingPosition());
}

//-----------------------------------------------------------------------------
void View2D::selectFromSlice()
{
//   m_fromSlice->setToolTip(tr("From Slice %1").arg(m_spinBox->value()));
//   emit sliceSelected(slicingPosition(), m_plane, ViewManager::From);
}

//-----------------------------------------------------------------------------
void View2D::selectToSlice()
{
//   m_toSlice->setToolTip(tr("To Slice %1").arg(m_spinBox->value()));
//   emit sliceSelected(slicingPosition(), m_plane, ViewManager::To);
}

//-----------------------------------------------------------------------------
bool View2D::eventFilter(QObject* caller, QEvent* e)
{
  static bool inFocus = false;

  // prevent other widgets from stealing the focus while the mouse cursor over
  // this widget
  if (true == inFocus && QEvent::FocusOut == e->type())
  {
    this->setFocus(Qt::OtherFocusReason);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    qApp->sendEvent(this, &event);
    return true;
  }

  if (m_selector && m_selector->filterEvent(e, this))
  {
    QWidget::eventFilter(caller, e);

    return true;
  }

  foreach (EspinaWidget *widget, m_widgets.keys())
  {
    if (widget->filterEvent(e, this))
      return true;
  }

  if (QEvent::Wheel == e->type())
  {
    QWheelEvent *we = static_cast<QWheelEvent *>(e);
    if (we->buttons() != Qt::MidButton)
    {
      int numSteps = we->delta() / 8 / 15 * (m_invertWheel ? -1 : 1);  //Refer to QWheelEvent doc.
      m_scrollBar->setValue(m_scrollBar->value() - numSteps);
      e->ignore();
      return true;
    }
  }
  else if (QEvent::Enter == e->type())
  {
    QWidget::enterEvent(e);

    // get the focus this very moment
    inFocus = true;
    this->setFocus(Qt::OtherFocusReason);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    qApp->sendEvent(this, &event);

    if (m_selector)
    {
      m_view->setCursor(m_selector->cursor());
    }
    e->accept();}
   else if (QEvent::Leave == e->type())
   {
     inFocus = false;
  }
  else if (QEvent::MouseMove == e->type())
  {
    int x, y;
    eventPosition(x, y);
    m_inThumbnail = m_thumbnail->GetDraw() && (m_thumbnail->PickProp(x,y) != nullptr);
  }
  else if (QEvent::ContextMenu == e->type())
  {
    QContextMenuEvent *cme = dynamic_cast<QContextMenuEvent*>(e);
    if (cme->modifiers() == Qt::CTRL && m_contextMenu.get() && selectionEnabled())
    {
      m_contextMenu->setSelection(currentSelection());
      m_contextMenu->exec(mapToGlobal(cme->pos()));
    }
  }
  else if (QEvent::ToolTip == e->type())
  {
    int x, y;
    eventPosition(x, y);
    SelectableView::Selection selection = pickSegmentations(x, y, m_renderer);
    QString toopTip;
    foreach(ViewItemAdapterPtr pick, selection)
    {
      toopTip = toopTip.append(pick->data(Qt::ToolTipRole).toString());
    }
    m_view->setToolTip(toopTip);
  }

  if (QEvent::KeyPress == e->type())
  {
    QKeyEvent *ke = dynamic_cast<QKeyEvent *>(e);
    if (ke->modifiers() != Qt::CTRL &&
        ke->modifiers() != Qt::SHIFT &&
        ke->key() != Qt::Key_Backspace)
      return true;
  }

  if ( QEvent::MouseMove == e->type()
    || QEvent::MouseButtonPress == e->type()
    || QEvent::MouseButtonRelease == e->type()
    || QEvent::KeyPress == e->type()
    || QEvent::KeyRelease == e->type())
  {
    if (m_inThumbnail)
    {
      m_view->setCursor(Qt::ArrowCursor);
    }
    else if (m_selector)
    {
      m_view->setCursor(m_selector->cursor());
    }

    updateRuler();
    updateThumbnail();
  }

  if (e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    if (me->button() == Qt::LeftButton)
    {
      if (me->modifiers() == Qt::CTRL)
        centerCrosshairOnMousePosition();
      else
        if (m_inThumbnail)
        {
          centerViewOnMousePosition();
          updateThumbnail();
          return true;
        }
        else if (selectionEnabled())
          selectPickedItems(me->modifiers() == Qt::SHIFT);
    }
  }

  return QWidget::eventFilter(caller, e);
}

//-----------------------------------------------------------------------------
void View2D::centerCrosshairOnMousePosition()
{
  int xPos, yPos;
  eventPosition(xPos, yPos);

  NmVector3 center;  //World coordinates
  bool channelPicked = false;
  if (m_inThumbnail)
  {
    foreach(RendererSPtr renderer, m_renderers)
    {
      if (canRender(renderer, RenderableType::CHANNEL))
      {
        SelectableView::Selection selection = renderer->pick(xPos, yPos, slicingPosition(), m_thumbnail, RenderableItems(RenderableType::CHANNEL));
        if (!selection.isEmpty())
        {
          channelPicked = true;
          center = renderer->pickCoordinates();
          break;
        }
      }
    }
  }
  else
  {
    foreach(RendererSPtr renderer, m_renderers)
    {
      if (canRender(renderer, RenderableType::CHANNEL))
      {
        SelectableView::Selection selection = renderer->pick(xPos, yPos, slicingPosition(), m_renderer, RenderableItems(RenderableType::CHANNEL));
        if (!selection.isEmpty())
        {
          channelPicked = true;
          center = renderer->pickCoordinates();
          break;
        }
      }
    }
  }

  if (channelPicked)
  {
    center[m_normalCoord] = slicingPosition();
    centerViewOn(center);
    emit centerChanged(m_crosshairPoint);
  }
}

//-----------------------------------------------------------------------------
void View2D::centerViewOnMousePosition()
{
  int xPos, yPos;
  eventPosition(xPos, yPos);

  foreach(RendererSPtr renderer, m_renderers)
    if (canRender(renderer, RenderableType::CHANNEL))
    {
      SelectableView::Selection selection = renderer->pick(xPos, yPos, slicingPosition(), m_thumbnail, RenderableItems(RenderableType::CHANNEL), false);
      if (!selection.isEmpty())
      {
        // TODO 2013-10-04: Check if it is needed inside the loop
        centerViewOnPosition(renderer->pickCoordinates());
      }
    }
}

//-----------------------------------------------------------------------------
SelectableView::Selection View2D::pickChannels(double vx, double vy,
                                                  bool repeatable)
{
  SelectableView::Selection selection;

  foreach(RendererSPtr renderer, m_renderers)
    if (canRender(renderer, RenderableType::CHANNEL))
      foreach(ViewItemAdapterPtr item, renderer->pick(vx,vy, slicingPosition(), m_renderer, RenderableItems(RenderableType::CHANNEL), repeatable))
      {
        if (!selection.contains(item))
          selection << item;
      }

  return selection;
}

//-----------------------------------------------------------------------------
SelectableView::Selection View2D::pickSegmentations(double vx, double vy,
                                                       bool repeatable)
{
  SelectableView::Selection selection;

  foreach(RendererSPtr renderer, m_renderers)
    if (canRender(renderer, RenderableType::SEGMENTATION))
      foreach(ViewItemAdapterPtr item, renderer->pick(vx,vy, slicingPosition(), m_renderer, RenderableItems(RenderableType::SEGMENTATION), repeatable))
        if (!selection.contains(item))
          selection << item;

  return selection;
}

//-----------------------------------------------------------------------------
void View2D::selectPickedItems(bool append)
{
  int vx, vy;
  eventPosition(vx, vy);

  SelectableView::Selection selection;
  if (append)
    selection = currentSelection();

  // segmentations have priority over channels
  foreach(ViewItemAdapterPtr item, pickSegmentations(vx, vy, append))
  {
    if (selection.contains(item))
      selection.removeAll(item);
    else
      selection << item;

    if (!append)
      break;
  }

  if (selection.isEmpty() || append)
    foreach(ViewItemAdapterPtr item, pickChannels(vx, vy, append))
    {
      if (selection.contains(item))
        selection.removeAll(item);
      else
        selection << item;

      if (!append)
        break;
    }

    emit selectionChanged(selection);
}

//-----------------------------------------------------------------------------
void View2D::updateWidgetVisibility()
{
  foreach(SliceWidget * widget, m_widgets)
  {
    widget->setSlice(slicingPosition(), m_plane);
  }
}

//-----------------------------------------------------------------------------
void View2D::updateChannelsOpactity()
{
  // TODO: Define opacity behaviour
  double opacity = suggestedChannelOpacity();

  foreach(ChannelAdapterPtr channel, m_channelStates.keys())
  {
    if (Channel::AUTOMATIC_OPACITY == channel->opacity())
    {
      foreach(RepresentationSPtr representation, m_channelStates[channel].representations)
      {
        representation->setOpacity(opacity);
      }
    }
  }
}

//-----------------------------------------------------------------------------
void View2D::onTakeSnapshot()
{
  takeSnapshot(m_renderer);
}

//-----------------------------------------------------------------------------
bool View2D::pick(vtkPropPicker *picker, int x, int y, Nm pickPos[3])
{
  if (m_thumbnail->GetDraw() && picker->Pick(x, y, 0.1, m_thumbnail))
    return false;//ePick Fail

  if (!picker->Pick(x, y, 0.1, m_renderer))
      return false;//ePick Fail

  picker->GetPickPosition(pickPos);
  pickPos[m_normalCoord] = slicingPosition();

  return true;
}

//-----------------------------------------------------------------------------
void View2D::setShowPreprocessing(bool visible)
{
  if (m_channelStates.size() < 2)
    return;

  ChannelAdapterPtr hiddenChannel = m_channelStates.keys()[visible];
  ChannelAdapterPtr visibleChannel = m_channelStates.keys()[1 - visible];
  hiddenChannel->setData(false, Qt::CheckStateRole);
  hiddenChannel->notifyModification();
  visibleChannel->setData(true, Qt::CheckStateRole);
  visibleChannel->notifyModification();

  for (int i = 2; i < m_channelStates.keys().size(); i++)
  {
    ChannelAdapterPtr otherChannel = m_channelStates.keys()[i];
    otherChannel->setData(false, Qt::CheckStateRole);
    otherChannel->notifyModification();
  }

  updateRepresentations(ChannelAdapterList());
}

//-----------------------------------------------------------------------------
void View2D::setRulerVisibility(bool visible)
{
  m_rulerVisibility = visible;
  updateRuler();
  updateView();
}

// //-----------------------------------------------------------------------------
// void SliceView::addSliceSelectors(SliceSelectorWidget* widget,
//                                   ViewManager::SliceSelectors selectors)
// {
//   if (m_sliceSelector.first != widget)
//   {
//     if (m_sliceSelector.second)
//       delete m_sliceSelector.second;
// 
//     m_sliceSelector.first  = widget;
//     m_sliceSelector.second = widget->clone();
//   }
// 
//   SliceSelectorWidget *sliceSelector = m_sliceSelector.second;
// 
//   sliceSelector->setPlane(m_plane);
//   sliceSelector->setView (this);
// 
//   QWidget *fromWidget = sliceSelector->leftWidget();
//   QWidget *toWidget   = sliceSelector->rightWidget();
// 
//   bool showFrom = selectors.testFlag(ViewManager::From);
//   bool showTo   = selectors.testFlag(ViewManager::To  );
// 
//   fromWidget->setVisible(showFrom);
//   toWidget  ->setVisible(showTo  );
// 
//   m_fromLayout->addWidget (fromWidget );
//   m_toLayout->insertWidget(0, toWidget);
// }
// 
// //-----------------------------------------------------------------------------
// void SliceView::removeSliceSelectors(SliceSelectorWidget* widget)
// {
//   if (m_sliceSelector.first == widget)
//   {
//     if (m_sliceSelector.second)
//       delete m_sliceSelector.second;
// 
//     m_sliceSelector.first  = nullptr;
//     m_sliceSelector.second = nullptr;
//   }
// }


//----------------------------------------------------------------------------
NmVector3 View2D::slicingStep() const
{
  return m_slicingStep;
}

//-----------------------------------------------------------------------------
void View2D::setSlicingStep(const NmVector3& steps)
{
  if (steps[0] <= 0 || steps[1] <= 0 || steps[2] <= 0)
  {
    qFatal("SliceView: Invalid Step value. Slicing Step not changed");
    return;
  }

  m_slicingStep = steps;

  int sliceIndex = voxelSlice(slicingPosition(), m_plane);

  setSlicingBounds(m_sceneBounds);

  if(m_fitToSlices)
    m_spinBox->setSingleStep(1);
  else
    m_spinBox->setSingleStep(m_slicingStep[m_normalCoord]);

  m_scrollBar->setValue(sliceIndex);

  {
    // We want to avoid the view update while loading channels
    // on scrollValueChanged(sliceIndex)
    m_crosshairPoint[m_normalCoord] = voxelCenter(sliceIndex, m_plane);

    updateRepresentations();

    m_spinBox->blockSignals(true);
    m_spinBox->setValue(m_fitToSlices ? sliceIndex + 1: slicingPosition());
    m_spinBox->blockSignals(false);

    emit sliceChanged(m_plane, slicingPosition());
    //scrollValueChanged(sliceIndex); // Updates spin box's value
  }
}

//-----------------------------------------------------------------------------
Nm View2D::slicingPosition() const
{
  return m_crosshairPoint[m_normalCoord];
}


//-----------------------------------------------------------------------------
void View2D::setSlicingBounds(const Bounds& bounds)
{
  if (bounds[1] < bounds[0] || bounds[3] < bounds[2] || bounds[5] < bounds[4])
  {
    qFatal("SliceView: Invalid Slicing Ranges. Ranges not changed");
    return;
  }

  int sliceMax = voxelSlice(bounds[2*m_normalCoord+1], m_plane) - 1; // [lowerBound, upperBound) upper bound doesn't belong to the voxel
  int sliceMin = voxelSlice(bounds[2*m_normalCoord]  , m_plane);

  m_spinBox->blockSignals(true);
  if(m_fitToSlices)
  {
    m_spinBox->setSuffix(" slice");
    m_spinBox->setMinimum(sliceMin+1);
    m_spinBox->setMaximum(sliceMax+1);
  }
  else
  {
    m_spinBox->setSuffix(" nm");
    m_spinBox->setMinimum(voxelCenter(sliceMin, m_plane));
    m_spinBox->setMaximum(voxelCenter(sliceMax, m_plane));
  }
  m_spinBox->blockSignals(false);

  m_scrollBar->blockSignals(true);
  m_scrollBar->setMinimum(sliceMin);
  m_scrollBar->setMaximum(sliceMax);
  m_scrollBar->blockSignals(false);

  //bool enabled = m_spinBox->minimum() < m_spinBox->maximum();
  //TODO 2012-11-14 m_fromSlice->setEnabled(enabled);
  //                m_toSlice->setEnabled(enabled);

  // update crosshair
  m_state->setCrossHairs(m_HCrossLineData, m_VCrossLineData,
                         m_crosshairPoint, m_sceneBounds, m_slicingStep);
}

//-----------------------------------------------------------------------------
void View2D::centerViewOn(const NmVector3& point, bool force)
{
  NmVector3 centerVoxel;
  // Adjust crosshairs to fit slicing steps
  for (int i = 0; i < 3; i++)
    centerVoxel[i] = voxelCenter(point[i], toPlane(i));

  if (!isVisible() ||
     (m_crosshairPoint[0] == centerVoxel[0] &&
      m_crosshairPoint[1] == centerVoxel[1] &&
      m_crosshairPoint[2] == centerVoxel[2] &&
      !force))
    return;

  // Adjust crosshairs to fit slicing steps
  m_crosshairPoint = centerVoxel;

  // Disable scrollbar signals to avoid calling setting slice
  m_scrollBar->blockSignals(true);
  m_spinBox->blockSignals(true);

  int slicingPos = voxelSlice(point[m_normalCoord], m_plane);

  m_scrollBar->setValue(slicingPos);

  if (m_fitToSlices)
    slicingPos++; // Correct 0 index
  else
    slicingPos = vtkMath::Round(centerVoxel[m_normalCoord]);

  m_spinBox->setValue(slicingPos);

  m_spinBox->blockSignals(false);
  m_scrollBar->blockSignals(false);

  updateRepresentations(); //m_state->setSlicingPosition(m_slicingMatrix, voxelBottom(m_scrollBar->value(), m_plane));
  m_state->setCrossHairs(m_HCrossLineData, m_VCrossLineData,
                         m_crosshairPoint, m_sceneBounds, m_slicingStep);

  // Only center camera if center is out of the display view
  vtkSmartPointer<vtkCoordinate> coords = vtkSmartPointer<vtkCoordinate>::New();
  coords->SetViewport(m_renderer);
  coords->SetCoordinateSystemToNormalizedViewport();
  double ll[3], ur[3];
  coords->SetValue(0, 0); //LL
  memcpy(ll,coords->GetComputedWorldValue(m_renderer),3*sizeof(double));
  coords->SetValue(1, 1); //UR
  memcpy(ur,coords->GetComputedWorldValue(m_renderer),3*sizeof(double));

  int H = (Plane::YZ == m_plane)?2:0;
  int V = (Plane::XZ == m_plane)?2:1;
  bool centerOutOfCamera = m_crosshairPoint[H] < ll[H] || m_crosshairPoint[H] > ur[H] // Horizontally out
                        || m_crosshairPoint[V] > ll[V] || m_crosshairPoint[V] < ur[V];// Vertically out

  if (centerOutOfCamera || force)
  {
    m_state->updateCamera(m_renderer->GetActiveCamera(), m_crosshairPoint);
    m_renderer->ResetCameraClippingRange();
  }

  updateView();
}

//-----------------------------------------------------------------------------
void View2D::centerViewOnPosition(const NmVector3& center)
{
  if (!isVisible() || m_crosshairPoint == center)
    return;

  m_state->updateCamera(m_renderer->GetActiveCamera(), center);
  m_renderer->ResetCameraClippingRange();
  updateView();
}

//-----------------------------------------------------------------------------
Selector::WorldRegion View2D::worldRegion(const Selector::DisplayRegion& region,
                                              ViewItemAdapterPtr item)
{
  Selector::WorldRegion wRegion = Selector::WorldRegion::New();

  foreach(QPointF point, region)
  {
    Nm pickPos[3];
    if (ItemAdapter::Type::CHANNEL == item->type())
    {
      foreach(RendererSPtr renderer, m_renderers)
        if (canRender(renderer, RenderableType::CHANNEL) &&
            !renderer->pick(point.x(), point.y(), slicingPosition(), m_renderer, RenderableItems(RenderableType::CHANNEL), false).isEmpty())
        {
          auto pc = renderer->pickCoordinates();
          for(int i = 0; i < 3; ++i) pickPos[i] = pc[i];
          pickPos[m_normalCoord] = slicingPosition();
          wRegion->InsertNextPoint(pickPos);
        }
    }
    else
    {
      foreach(RendererSPtr renderer, m_renderers)
        if (canRender(renderer, RenderableType::SEGMENTATION) &&
            !renderer->pick(point.x(), point.y(), slicingPosition(), m_renderer, RenderableItems(RenderableType::SEGMENTATION), false).isEmpty())
        {
          auto pc = renderer->pickCoordinates();
          for(int i = 0; i < 3; ++i) pickPos[i] = pc[i];
          pickPos[m_normalCoord] = slicingPosition();
          wRegion->InsertNextPoint(pickPos);
        }
    }
  }

  return wRegion;
}

//-----------------------------------------------------------------------------
RepresentationSPtr View2D::cloneRepresentation(ViewItemAdapterPtr item, Representation::Type representation)
{
  RepresentationSPtr prototype = item->representation(representation);
  RepresentationSPtr rep;

  if (prototype->canRenderOnView().testFlag(Representation::RENDERABLEVIEW_SLICE))
  {
    rep = prototype->clone(this);
  }

  return rep;
}

//-----------------------------------------------------------------------------
void View2D::addRendererControls(RendererSPtr renderer)
{
  renderer->setView(this);
  renderer->setEnable(true);

  // add representations to renderer
  foreach(SegmentationAdapterPtr segmentation, m_segmentationStates.keys())
  {
    if (renderer->canRender(segmentation))
      foreach(RepresentationSPtr rep, m_segmentationStates[segmentation].representations)
         if (renderer->managesRepresentation(rep)) renderer->addRepresentation(segmentation, rep);
  }

  foreach(ChannelAdapterPtr channel, m_channelStates.keys())
  {
    if (renderer->canRender(channel))
      foreach(RepresentationSPtr rep, m_channelStates[channel].representations)
        if (renderer->managesRepresentation(rep)) renderer->addRepresentation(channel, rep);
  }

  QPushButton *button = new QPushButton();
  m_renderers[button] = renderer;

  if (!renderer->isHidden())
  {
    if (canRender(renderer, RenderableType::SEGMENTATION))
      this->m_numEnabledSegmentationRenders++;

    if (canRender(renderer, RenderableType::CHANNEL))
      this->m_numEnabledChannelRenders++;
  }

  if (0 != m_numEnabledSegmentationRenders)
  {
    QMap<EspinaWidget *, SliceWidget *>::const_iterator it = m_widgets.begin();
    for( ; it != m_widgets.end(); ++it)
    {
      if (it.key()->manipulatesSegmentations())
      {
        it.value()->SetEnabled(true);
        it.value()->SetVisibility(true);
      }
    }
  }
}

//-----------------------------------------------------------------------------
void View2D::removeRendererControls(QString name)
{
  RendererSPtr removedRenderer;
  foreach (RendererSPtr renderer, m_renderers)
  {
    if (renderer->name() == name)
    {
      removedRenderer = renderer;
      break;
    }
  }

  if (removedRenderer)
  {
    if (!removedRenderer->isHidden())
      removedRenderer->hide();

    if (!removedRenderer->isHidden() && (canRender(removedRenderer, RenderableType::SEGMENTATION)))
      this->m_numEnabledSegmentationRenders--;

    if (!removedRenderer->isHidden() && canRender(removedRenderer, RenderableType::CHANNEL))
      this->m_numEnabledChannelRenders--;

    if (0 == m_numEnabledSegmentationRenders)
    {
      QMap<EspinaWidget *, SliceWidget *>::const_iterator it = m_widgets.begin();
      for( ; it != m_widgets.end(); ++it)
      {
        if (it.key()->manipulatesSegmentations())
        {
          it.value()->SetEnabled(false);
          it.value()->SetVisibility(false);
        }
      }
    }

    QMap<QPushButton*, RendererSPtr>::iterator it = m_renderers.begin();
    bool erased = false;
    while(!erased && it != m_renderers.end())
    {
      if (it.value() == removedRenderer)
      {
        m_renderers.erase(it);
        erased = true;
      }
      else
        ++it;
    }
  }
}

//-----------------------------------------------------------------------------
void View2D::updateCrosshairPoint(Plane plane, Nm slicePos)
{
  int normalCoord = normalCoordinateIndex(plane);

  m_crosshairPoint[normalCoord] = voxelCenter(slicePos, plane);
  m_state->setCrossHairs(m_HCrossLineData, m_VCrossLineData,
                         m_crosshairPoint, m_sceneBounds, m_slicingStep);

  // render if present
  if (this->m_renderer->HasViewProp(this->m_HCrossLine))
    updateView();
}
