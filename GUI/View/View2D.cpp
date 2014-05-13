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
#include "ViewRendererMenu.h"
#include "Widgets/EspinaWidget.h"
#include <GUI/View/vtkInteractorStyleEspinaSlice.h>
#include <Core/Analysis/Channel.h>
#include <Core/Utils/BinaryMask.h>
#include <GUI/Model/Utils/QueryAdapter.h>

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
#include <vtkPoints.h>
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
const double View2D::WIDGET_SHIFT = 0.15;

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
, m_renderConfig(new QPushButton())
, m_ruler(vtkSmartPointer<vtkAxisActor2D>::New())
, m_slicingStep{1, 1, 1}
, m_showThumbnail(true)
// , m_sliceSelector(QPair<SliceSelectorWidget*,SliceSelectorWidget*>(nullptr, nullptr))
, m_inThumbnail(false)
, m_sceneReady(false)
, m_plane{plane}
, m_normalCoord{normalCoordinateIndex(plane)}
, m_fitToSlices{true}
, m_invertSliceOrder{false}
, m_invertWheel{false}
, m_rulerVisibility{true}
, m_inThumbnailClick{true}
{
  setupUI();
  qRegisterMetaType<Plane>("Plane");
  qRegisterMetaType<Nm>("Nm");

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
    default:
      break;
  };

  // Init Render Window
  vtkRenderWindow* renderWindow = m_view->GetRenderWindow();
  renderWindow->DoubleBufferOn();
  renderWindow->SetNumberOfLayers(2);

  // Init Renderers
  m_renderer = vtkSmartPointer<vtkRenderer>::New();
  m_renderer->GetActiveCamera()->ParallelProjectionOn();
  m_renderer->GetActiveCamera()->SetThickness(2000);
  m_renderer->SetNearClippingPlaneTolerance(0.001);
  m_renderer->LightFollowCameraOn();
  m_renderer->SetLayer(0);
  m_thumbnail = vtkSmartPointer<vtkRenderer>::New();
  m_thumbnail->SetViewport(0.75, 0.0, 1.0, 0.25);
  m_thumbnail->SetLayer(1);
  m_thumbnail->InteractiveOff();
  m_thumbnail->GetActiveCamera()->ParallelProjectionOn();
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
  m_renderer->AddViewProp(m_ruler);

  View2DInteractor interactor = View2DInteractor::New();
  interactor->AutoAdjustCameraClippingRangeOff();
  interactor->KeyPressActivationOff();
  renderWindow->AddRenderer(m_renderer);
  renderWindow->AddRenderer(m_thumbnail);
  m_view->GetInteractor()->SetInteractorStyle(interactor);

  m_channelBorderData = vtkSmartPointer<vtkPolyData>::New();
  m_channelBorder     = vtkSmartPointer<vtkActor>::New();
  initBorders(m_channelBorderData, m_channelBorder);

  m_viewportBorderData = vtkSmartPointer<vtkPolyData>::New();
  m_viewportBorder     = vtkSmartPointer<vtkActor>::New();
  initBorders(m_viewportBorderData, m_viewportBorder);

  buildCrosshairs();

  this->setAutoFillBackground(true);
  this->setLayout(m_mainLayout);

  this->setFocusPolicy(Qt::WheelFocus);
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
  m_state.reset();
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
  QStringList oldRenderersNames, newRenderersNames;

  for (auto renderer: m_renderers)
    oldRenderersNames << renderer->name();

  for (auto renderer: renderers)
    newRenderersNames << renderer->name();

  // remove controls of unused renderers
  for (auto renderer : m_renderers)
    if (!newRenderersNames.contains(renderer->name()))
      removeRendererControls(renderer->name());

  // add controls for new renderers
  for (auto renderer: renderers)
  {
    if (!canRender(renderer, RendererType::RENDERER_VIEW2D))
      continue;

    if (!oldRenderersNames.contains(renderer->name()))
      addRendererControls(renderer->clone());
  }
}

//-----------------------------------------------------------------------------
void View2D::reset()
{
  for(auto widget: m_widgets)
    widget->unregisterView(this);

  for(auto segmentation: m_segmentationStates.keys())
    remove(segmentation);

  for(auto channel: m_channelStates.keys())
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

  coords->SetValue(0, 0); //Viewport Lower Left Corner
  value = coords->GetComputedWorldValue(m_renderer);
  left = value[0];

  coords->SetValue(1, 0); // Viewport Lower Right Corner
  value = coords->GetComputedWorldValue(m_renderer);
  right = value[0];

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

  int h = m_plane == Plane::YZ ? 2 : 0;
  int v = m_plane == Plane::XZ ? 2 : 1;
  coords->SetValue(0, 0); // Viewport Lower Left Corner
  value = coords->GetComputedWorldValue(m_renderer);
  viewLeft  = value[h]; // Left Margin in World Coordinates
  viewLower = value[v]; // Lower Margin in World Coordinates

  coords->SetValue(1, 1);
  value = coords->GetComputedWorldValue(m_renderer);
  viewRight = value[h]; // Right Margin in World Coordinates
  viewUpper = value[v]; // Upper Margin in World Coordinates

  double sceneLeft  = m_sceneBounds[2*h];
  double sceneRight = m_sceneBounds[2*h+1];
  double sceneLower = m_sceneBounds[2*v];
  double sceneUpper = m_sceneBounds[2*v+1];

  // viewLower and viewUpper are inverted because the roll we made
  // in the renderer camera
  bool leftHidden  = sceneLeft < viewLeft;
  bool rightHidden = sceneRight > viewRight;
  bool upperHidden = sceneUpper > viewLower;
  bool lowerHidden = sceneLower < viewUpper;

  if (leftHidden || rightHidden || upperHidden || lowerHidden)
  {
    m_thumbnail->DrawOn();
    updateBorder(m_viewportBorderData, viewLeft, viewRight, viewUpper, viewLower);
    m_thumbnail->ResetCameraClippingRange();
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

  // reset thumbnail channel border
  int h = m_plane == Plane::YZ ? 2 : 0;
  int v = m_plane == Plane::XZ ? 2 : 1;

  double sceneLeft  = m_sceneBounds[2*h];
  double sceneRight = m_sceneBounds[2*h+1];
  double sceneUpper = m_sceneBounds[2*v];
  double sceneLower = m_sceneBounds[2*v+1];
  updateBorder(m_channelBorderData, sceneLeft, sceneRight, sceneUpper, sceneLower);

  // we need to update the view only if a signal has been sent
  // (the volume of a channel has been updated)
  if (sender() != nullptr)
    updateView();
}

//-----------------------------------------------------------------------------
void View2D::initBorders(vtkPolyData* data, vtkActor* actor)
{
  double unusedPoint[3]{0,0,0};
  vtkSmartPointer<vtkPoints> corners = vtkSmartPointer<vtkPoints>::New();
  corners->SetNumberOfPoints(4);
  corners->InsertNextPoint(unusedPoint);
  corners->InsertNextPoint(unusedPoint);
  corners->InsertNextPoint(unusedPoint);
  corners->InsertNextPoint(unusedPoint);

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
  data->Modified();

  vtkSmartPointer<vtkPolyDataMapper> Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  Mapper->SetInputData(data);
  actor->SetMapper(Mapper);
  actor->GetProperty()->SetLineWidth(2);
  actor->SetPickable(false);
}

//-----------------------------------------------------------------------------
void View2D::updateBorder(vtkPolyData* data, Nm left, Nm right, Nm upper, Nm lower)
{
  vtkSmartPointer<vtkPoints> corners = data->GetPoints();
  Nm zShift;
  switch(m_plane)
  {
    case Plane::XY:
      zShift = m_sceneBounds[4]-0.1;
      corners->SetPoint(0, left,  upper, zShift); //UL
      corners->SetPoint(1, right, upper, zShift); //UR
      corners->SetPoint(2, right, lower, zShift); //LR
      corners->SetPoint(3, left,  lower, zShift); //LL
      break;
    case Plane::XZ:
      zShift = m_sceneBounds[3]+0.1;
      corners->SetPoint(0, left,  zShift, upper); //UL
      corners->SetPoint(1, right, zShift, upper); //UR
      corners->SetPoint(2, right, zShift, lower); //LR
      corners->SetPoint(3, left,  zShift, lower); //LL
      break;
    case Plane::YZ:
      zShift = m_sceneBounds[1]+0.1;
      corners->SetPoint(0, zShift, upper,  left); //UL
      corners->SetPoint(1, zShift, lower,  left); //UR
      corners->SetPoint(2, zShift, lower, right); //LR
      corners->SetPoint(3, zShift, upper, right); //LL
      break;
    default:
      Q_ASSERT(false);
      break;
  }
  data->Modified();
}

//-----------------------------------------------------------------------------
Nm View2D::voxelBottom(const int sliceIndex, const Plane plane) const
{
  int index = normalCoordinateIndex(plane);
  return m_sceneBounds[2*index] + sliceIndex * m_slicingStep[index];
}

//-----------------------------------------------------------------------------
Nm View2D::voxelBottom(const Nm position, const Plane plane) const
{
  return voxelBottom(voxelSlice(position, plane), plane);
}

//-----------------------------------------------------------------------------
Nm View2D::voxelCenter(const int sliceIndex, const Plane plane) const
{
  int index = normalCoordinateIndex(plane);
  return m_sceneBounds[2*index] + ((static_cast<double>(sliceIndex) + 0.5)* m_slicingStep[index]);
}

//-----------------------------------------------------------------------------
Nm View2D::voxelCenter(const Nm position, const Plane plane) const
{
  return voxelCenter(voxelSlice(position, plane), plane);
}

//-----------------------------------------------------------------------------
Nm View2D::voxelTop(const int sliceIndex, const Plane plane) const
{
  int index = normalCoordinateIndex(plane);
  return m_sceneBounds[2*index] + (sliceIndex + 1.0) * m_slicingStep[index];
}

//-----------------------------------------------------------------------------
Nm View2D::voxelTop(const Nm position, const Plane plane) const
{
  return voxelTop(voxelSlice(position, plane), plane);
}

//-----------------------------------------------------------------------------
int View2D::voxelSlice(const Nm position, const Plane plane) const
{
  int index = normalCoordinateIndex(plane);
  return vtkMath::Floor((position-m_sceneBounds[2*index])/m_slicingStep[index]);
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

  m_renderConfig->setIcon(QIcon(":/espina/settings.png"));
  m_renderConfig->setToolTip(tr("Configure this view's renderers"));
  m_renderConfig->setFlat(true);
  m_renderConfig->setIconSize(QSize(20,20));
  m_renderConfig->setMaximumSize(QSize(22,22));
  m_renderConfig->setEnabled(false);

  // TODO
  //connect(m_renderConfig,SIGNAL(clicked(bool)),this,SLOT(renderContextualMenu()));

  connect(m_spinBox, SIGNAL(valueChanged(double)), this, SLOT(spinValueChanged(double)));
  connect(m_scrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrollValueChanged(int)));

  m_mainLayout->addWidget(m_view);
  m_controlLayout->addWidget(m_zoomButton);
  m_controlLayout->addWidget(m_snapshot);
  m_controlLayout->addWidget(m_scrollBar);
  m_controlLayout->addWidget(m_renderConfig);
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
void View2D::setCrosshairColors(const QColor& vColor, const QColor& hColor)
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
    m_renderer->AddViewProp(m_HCrossLine);
    m_renderer->AddViewProp(m_VCrossLine);
  }
  else
  {
    m_renderer->RemoveViewProp(m_HCrossLine);
    m_renderer->RemoveViewProp(m_VCrossLine);
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
void View2D::updateView()
{
  if (isVisible())
  {
    updateRuler();
    updateThumbnail();
    m_renderer->ResetCameraClippingRange();
    m_view->GetRenderWindow()->Render();
  }
}

//-----------------------------------------------------------------------------
void View2D::resetCamera()
{
  NmVector3 origin{ 0, 0, 0 };

  m_state->updateCamera(m_renderer ->GetActiveCamera(), origin);
  m_state->updateCamera(m_thumbnail->GetActiveCamera(), origin);

  m_thumbnail->RemoveViewProp(m_channelBorder);
  m_thumbnail->RemoveViewProp(m_viewportBorder);
  updateSceneBounds();
  updateThumbnail();
  m_renderer->ResetCamera();
  m_thumbnail->ResetCamera();
  m_thumbnail->AddViewProp(m_channelBorder);
  m_thumbnail->AddViewProp(m_viewportBorder);

  m_sceneReady = !m_channelStates.isEmpty();
}

//-----------------------------------------------------------------------------
void View2D::addActor(vtkProp* actor)
{
  m_renderer->AddViewProp(actor);
  m_thumbnail->AddViewProp(actor);

  m_thumbnail->RemoveViewProp(m_channelBorder);
  m_thumbnail->RemoveViewProp(m_viewportBorder);

  updateThumbnail();
//  m_thumbnail->ResetCamera();
//  updateThumbnail();

  m_thumbnail->AddViewProp(m_channelBorder);
  m_thumbnail->AddViewProp(m_viewportBorder);
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

  bounds[2*m_normalCoord+1] = bounds[2*m_normalCoord] = slicingPosition();

  if (cropToSceneBounds)
  {
    bounds[2*H]   = std::max(LL[H], m_sceneBounds[2*H]);
    bounds[2*H+1] = std::min(UR[H], m_sceneBounds[2*H+1]);
    bounds[2*V]   = std::max(UR[V], m_sceneBounds[2*V]);
    bounds[2*V+1] = std::min(LL[V], m_sceneBounds[2*V+1]);
  }
  bounds.setUpperInclusion(true);

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
  int xPos, yPos;
  eventPosition(xPos, yPos);
  if (m_thumbnail != nullptr)
    m_inThumbnail = m_thumbnail->GetDraw() && (m_thumbnail->PickProp(xPos, yPos) != nullptr);
  else
    m_inThumbnail = false;

  if (!m_inThumbnail && m_eventHandler && m_eventHandler->filterEvent(e, this))
    return true;

  switch (e->type())
  {
    case QEvent::Resize:
      {
        updateView();
        e->accept();
      }
      break;
    case QEvent::Wheel:
      {
        QWheelEvent *we = static_cast<QWheelEvent *>(e);
        int numSteps = we->delta() / 8 / 15 * (m_invertWheel ? -1 : 1);  //Refer to QWheelEvent doc.
        m_scrollBar->setValue(m_scrollBar->value() - numSteps);
        e->ignore();
        this->setFocus(Qt::OtherFocusReason);
        return true;
      }
      break;
    case QEvent::Enter:
      {
        QWidget::enterEvent(e);

        // get the focus this very moment
        this->setFocus(Qt::OtherFocusReason);

        if (m_eventHandler && !m_inThumbnail)
          m_view->setCursor(m_eventHandler->cursor());
        else
          m_view->setCursor(Qt::ArrowCursor);

        e->accept();
      }
      break;
    case QEvent::Leave:
      {
        m_inThumbnail = false;
      }
      break;
    case QEvent::ContextMenu:
      {
        QContextMenuEvent *cme = dynamic_cast<QContextMenuEvent*>(e);
        if (cme->modifiers() == Qt::CTRL && m_contextMenu.get() && selectionEnabled())
        {
          m_contextMenu->setSelection(currentSelection());
          m_contextMenu->exec(mapToGlobal(cme->pos()));
        }
      }
      break;
    case QEvent::ToolTip:
      {
        auto selection = pickSegmentations(xPos, yPos, m_renderer);
        QString toopTip;

        for (auto pick : selection)
          toopTip = toopTip.append(pick->data(Qt::ToolTipRole).toString());

        m_view->setToolTip(toopTip);
      }
      break;
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
      {
        QMouseEvent* me = static_cast<QMouseEvent*>(e);

        if (m_inThumbnail)
        {
          m_view->setCursor(Qt::ArrowCursor);

          if (((e->type() == QEvent::MouseButtonPress) && me->button() == Qt::LeftButton) || (e->type() == QEvent::MouseMove && m_inThumbnailClick))
          {
            m_inThumbnailClick = true;
            centerViewOnMousePosition();
          }
          else
            if ((e->type() == QEvent::MouseButtonRelease) && (me->button() == Qt::LeftButton))
              m_inThumbnailClick = false;
        }
        else
        {
          // in case the cursor gets out of thumbnail during a click+move, usually an if goes here but we
          // assign false directly and avoid conditional code. Getting out of the thumbnail while in a drag breaks
          // the drag movement though.
          m_inThumbnailClick = false;

          // to avoid interfering with ctrl use in the event handler/selector
          if (!m_eventHandler)
          {
            if ((e->type() == QEvent::MouseButtonPress) && (me->button() == Qt::LeftButton))
            {
              if (me->modifiers() == Qt::CTRL)
                centerCrosshairOnMousePosition();
              else
                if (selectionEnabled() && !m_eventHandler)
                  selectPickedItems(me->modifiers() == Qt::SHIFT);
            }

            m_view->setCursor(Qt::ArrowCursor);
          }
          else
            m_view->setCursor(m_eventHandler->cursor());
        }

        updateRuler();
        updateThumbnail();
      }
      break;
    default:
      break;
  }

  for (auto widget : m_widgets)
  {
    auto eventHandler = dynamic_cast<EventHandler *>(widget.get());
    if(eventHandler && eventHandler->filterEvent(e, this))
      return true;
  }

  return QWidget::eventFilter(caller, e);
}

//-----------------------------------------------------------------------------
void View2D::keyPressEvent(QKeyEvent *e)
{
  if (m_eventHandler && m_eventHandler->filterEvent(e, this))
      updateView();
};

//-----------------------------------------------------------------------------
void View2D::keyReleaseEvent(QKeyEvent *e)
{
  keyPressEvent(e);
};

//-----------------------------------------------------------------------------
void View2D::centerCrosshairOnMousePosition()
{
  int xPos, yPos;
  eventPosition(xPos, yPos);

  NmVector3 center;  //World coordinates
  bool channelPicked = false;
  if (m_inThumbnail)
  {
    for(auto renderer: m_renderers)
    {
      if(renderer->type() == Renderer::Type::Representation)
      {
        auto repRenderer = representationRenderer(renderer);
        if (canRender(repRenderer, RenderableType::CHANNEL))
        {
          auto selection = repRenderer->pick(xPos, yPos, slicingPosition(), m_thumbnail, RenderableItems(RenderableType::CHANNEL));
          if (!selection.isEmpty())
          {
            channelPicked = true;
            center = repRenderer->pickCoordinates();
            break;
          }
        }
      }
    }
  }
  else
  {
    for(auto renderer: m_renderers)
      if(renderer->type() == Renderer::Type::Representation)
        {
          auto repRenderer = representationRenderer(renderer);
          if (canRender(repRenderer, RenderableType::CHANNEL))
          {
          auto selection = repRenderer->pick(xPos, yPos, slicingPosition(), m_renderer, RenderableItems(RenderableType::CHANNEL));
          if (!selection.isEmpty())
          {
            channelPicked = true;
            center = repRenderer->pickCoordinates();
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

  for(auto renderer: m_renderers)
    if(renderer->type() == Renderer::Type::Representation)
    {
      auto repRenderer = representationRenderer(renderer);
      if (canRender(repRenderer, RenderableType::CHANNEL))
      {
        auto selection = repRenderer->pick(xPos, yPos, slicingPosition(), m_thumbnail, RenderableItems(RenderableType::CHANNEL), false);
        if (!selection.isEmpty())
        {
          // TODO 2013-10-04: Check if it is needed inside the loop
          centerViewOnPosition(repRenderer->pickCoordinates());
          return;
        }
      }
    }
}

//-----------------------------------------------------------------------------
ViewItemAdapterList View2D::pickChannels(double vx, double vy,
                                         bool repeatable)
{
  ViewItemAdapterList selection;

  for(auto renderer: m_renderers)
    if(renderer->type() == Renderer::Type::Representation)
    {
      auto repRenderer = representationRenderer(renderer);
      if (canRender(repRenderer, RenderableType::CHANNEL))
        for(auto item: repRenderer->pick(vx,vy, slicingPosition(), m_renderer, RenderableItems(RenderableType::CHANNEL), repeatable))
        {
          if (!selection.contains(item))
            selection << item;
        }
    }

  return selection;
}

//-----------------------------------------------------------------------------
ViewItemAdapterList View2D::pickSegmentations(double vx, double vy,
                                                       bool repeatable)
{
  ViewItemAdapterList selection;

  for(auto renderer: m_renderers)
    if(renderer->type() == Renderer::Type::Representation)
    {
      auto repRenderer = representationRenderer(renderer);
      if (canRender(repRenderer, RenderableType::SEGMENTATION))
        for(auto item: repRenderer->pick(vx,vy, slicingPosition(), m_renderer, RenderableItems(RenderableType::SEGMENTATION), repeatable))
          if (!selection.contains(item))
            selection << item;
    }

  return selection;
}

//-----------------------------------------------------------------------------
void View2D::selectPickedItems(bool append)
{
  int vx, vy;
  eventPosition(vx, vy);

  ViewItemAdapterList selection;
  if (append)
    selection = currentSelection()->items();

  // segmentations have priority over channels
  for(auto item: pickSegmentations(vx, vy, append))
  {
    if (selection.contains(item))
      selection.removeAll(item);
    else
      selection << item;

    if (!append)
      break;
  }

  if (selection.isEmpty() || append)
    for(auto item: pickChannels(vx, vy, append))
    {
      if (selection.contains(item))
        selection.removeAll(item);
      else
        selection << item;

      if (!append)
        break;
    }

  currentSelection()->set(selection);
}

//-----------------------------------------------------------------------------
void View2D::updateChannelsOpactity()
{
  // TODO: Define opacity behaviour
  double opacity = suggestedChannelOpacity();

  for(auto channel: m_channelStates.keys())
    if (Channel::AUTOMATIC_OPACITY == channel->opacity())
      for(auto representation: m_channelStates[channel].representations)
        representation->setOpacity(opacity);
}

//-----------------------------------------------------------------------------
void View2D::onTakeSnapshot()
{
  takeSnapshot(m_renderer);
}

////-----------------------------------------------------------------------------
//bool View2D::pick(vtkPropPicker *picker, int x, int y, Nm pickPos[3])
//{
//  if (m_thumbnail->GetDraw() && picker->Pick(x, y, 0.1, m_thumbnail))
//    return false;
//
//  if (!picker->Pick(x, y, 0.1, m_renderer))
//      return false;
//
//  picker->GetPickPosition(pickPos);
//  pickPos[m_normalCoord] = slicingPosition();
//
//  return true;
//}

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
    qFatal("View2D: Invalid Step value. Slicing Step not changed");
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
    qFatal("View2D: Invalid Slicing Ranges. Ranges not changed");
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

  updateRepresentations();
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
  }

  // needed to make some renderers update (cached ones for example)
  emit sliceChanged(m_plane, slicingPosition());

  updateView();
}

//-----------------------------------------------------------------------------
void View2D::centerViewOnPosition(const NmVector3& center)
{
  if (!isVisible() || m_crosshairPoint == center)
    return;

  m_state->updateCamera(m_renderer->GetActiveCamera(), center);

  updateView();
}

////-----------------------------------------------------------------------------
//Selector::WorldRegion View2D::worldRegion(const Selector::DisplayRegion& region,
//                                              ViewItemAdapterPtr item)
//{
//  Selector::WorldRegion wRegion = Selector::WorldRegion::New();
//
//  for(auto point: region)
//  {
//    Nm pickPos[3];
//    if (ItemAdapter::Type::CHANNEL == item->type())
//    {
//      for(auto renderer: m_renderers)
//        if(renderer->type() == Renderer::Type::Representation)
//        {
//          auto repRenderer = representationRenderer(renderer);
//          if (canRender(repRenderer, RenderableType::CHANNEL) &&
//              !repRenderer->pick(point.x(), point.y(), slicingPosition(), m_renderer, RenderableItems(RenderableType::CHANNEL), false).isEmpty())
//          {
//            auto pc = repRenderer->pickCoordinates();
//            for(int i = 0; i < 3; ++i) pickPos[i] = pc[i];
//            pickPos[m_normalCoord] = slicingPosition();
//            wRegion->InsertNextPoint(pickPos);
//          }
//        }
//    }
//    else
//    {
//      for(auto renderer: m_renderers)
//        if(renderer->type() == Renderer::Type::Representation)
//        {
//          auto repRenderer = representationRenderer(renderer);
//          if (canRender(repRenderer, RenderableType::SEGMENTATION) &&
//              !repRenderer->pick(point.x(), point.y(), slicingPosition(), m_renderer, RenderableItems(RenderableType::SEGMENTATION), false).isEmpty())
//          {
//            auto pc = repRenderer->pickCoordinates();
//            for(int i = 0; i < 3; ++i) pickPos[i] = pc[i];
//            pickPos[m_normalCoord] = slicingPosition();
//            wRegion->InsertNextPoint(pickPos);
//          }
//        }
//    }
//  }
//
//  return wRegion;
//}

//-----------------------------------------------------------------------------
RepresentationSPtr View2D::cloneRepresentation(ViewItemAdapterPtr item, Representation::Type representation)
{
  RepresentationSPtr prototype = item->representation(representation);
  RepresentationSPtr rep;

  if (prototype && prototype->canRenderOnView().testFlag(Representation::RENDERABLEVIEW_SLICE))
  {
    rep = prototype->clone(this);
  }

  return rep;
}

//-----------------------------------------------------------------------------
void View2D::addRendererControls(RendererSPtr renderer)
{
  if(m_renderers.contains(renderer) || (renderer->renderType() != RendererTypes(RENDERER_VIEW2D)))
    return;

  m_renderers << renderer;

  renderer->setView(this);
  renderer->setEnable(true);

  // add representations to renderer
  if(renderer->type() == Renderer::Type::Representation)
  {
    auto repRenderer = representationRenderer(renderer);
    for (auto seg : m_segmentationStates.keys())
    {
      if (repRenderer->canRender(seg))
      {
        for (auto repName : seg->representationTypes())
        {
          if (!repRenderer->managesRepresentation(repName))
            continue;

          bool found = false;
          for (auto rep : m_segmentationStates[seg].representations)
          {
            if (rep->type() == repName)
            {
              repRenderer->addRepresentation(seg, rep);
              found = true;
            }
          }

          if (!found)
          {
            auto rep = cloneRepresentation(seg, repName);
            if (rep.get() != nullptr)
            {
              repRenderer->addRepresentation(seg, rep);
              m_segmentationStates[seg].representations << rep;

              rep->setColor(m_colorEngine->color(seg));
              rep->setHighlighted(m_segmentationStates[seg].highlited);
              rep->setVisible(m_segmentationStates[seg].visible);

              rep->updateRepresentation();
            }
          }
        }
      }
    }

    for (auto channel : m_channelStates.keys())
    {
      if (repRenderer->canRender(channel))
      {
        for (auto repName : channel->representationTypes())
        {
          if (!repRenderer->managesRepresentation(repName))
            continue;

          bool found = false;
          for (auto rep : m_channelStates[channel].representations)
          {
            if (rep->type() == repName)
            {
              repRenderer->addRepresentation(channel, rep);
              found = true;
            }
          }

          if (!found)
          {
            auto rep = cloneRepresentation(channel, repName);
            if (rep.get() != nullptr)
            {
              repRenderer->addRepresentation(channel, rep);
              m_channelStates[channel].representations << rep;

              rep->setBrightness(m_channelStates[channel].brightness);
              rep->setContrast(m_channelStates[channel].contrast);
              rep->setColor(m_channelStates[channel].stain);
              rep->setOpacity(m_channelStates[channel].opacity);
              rep->setVisible(m_channelStates[channel].visible);

              rep->updateRepresentation();
              updateChannelsOpactity();
            }
          }
        }
      }
    }
  }

  if (0 != numEnabledRenderersForViewItem(RenderableType::SEGMENTATION))
  {
    for(auto widget: m_widgets)
    {
      if (widget->manipulatesSegmentations())
      {
        widget->setEnabled(true);
      }
    }
  }

  ViewRendererMenu *configMenu = qobject_cast<ViewRendererMenu*>(m_renderConfig->menu());
  if (configMenu == nullptr)
  {
    configMenu = new ViewRendererMenu(m_renderConfig);
    m_renderConfig->setMenu(configMenu);
    m_renderConfig->setEnabled(true);
  }
  configMenu->add(renderer);

  connect(renderer.get(), SIGNAL(renderRequested()), this, SLOT(updateView()), Qt::QueuedConnection);
}

//-----------------------------------------------------------------------------
void View2D::removeRendererControls(QString name)
{
  RendererSPtr removedRenderer = nullptr;
  for(auto renderer: m_renderers)
    if (renderer->name() == name)
    {
      removedRenderer = renderer;
      break;
    }

  if (removedRenderer == nullptr)
    return;

  // delete representations for this renderer if its a representationRenderer
  if(removedRenderer->type() == Renderer::Type::Representation)
  {
    auto repRenderer = representationRenderer(removedRenderer);

    for(auto seg : m_segmentationStates.keys())
      if(repRenderer->canRender(seg))
      {
        for(auto rep: m_segmentationStates[seg].representations)
          if(repRenderer->managesRepresentation(rep->type()))
            m_segmentationStates[seg].representations.removeOne(rep);
      }

    for(auto channel : m_channelStates.keys())
      if(repRenderer->canRender(channel))
      {
        for(auto rep: m_channelStates[channel].representations)
          if(repRenderer->managesRepresentation(rep->type()))
            m_channelStates[channel].representations.removeOne(rep);
      }
  }

  m_renderers.removeOne(removedRenderer);

    if (!removedRenderer->isHidden())
      removedRenderer->setEnable(false);

  if (0 == numEnabledRenderersForViewItem(RenderableType::SEGMENTATION))
  {
    for (auto widget: m_widgets)
      if (widget->manipulatesSegmentations())
        widget->setEnabled(false);
  }

  ViewRendererMenu *configMenu = qobject_cast<ViewRendererMenu*>(m_renderConfig->menu());
  if (configMenu != nullptr)
  {
    configMenu->remove(removedRenderer);
    if (configMenu->actions().isEmpty())
    {
      m_renderConfig->setMenu(nullptr);
      delete configMenu;
      m_renderConfig->setEnabled(false);
    }
  }
  disconnect(removedRenderer.get(), SIGNAL(renderRequested()), this, SLOT(updateView()));
}

//-----------------------------------------------------------------------------
void View2D::updateCrosshairPoint(const Plane plane, const Nm slicePos)
{
  m_crosshairPoint[normalCoordinateIndex(plane)] = voxelCenter(slicePos, plane);
  m_state->setCrossHairs(m_HCrossLineData, m_VCrossLineData,
                         m_crosshairPoint, m_sceneBounds, m_slicingStep);

  // render if present
  if (this->m_renderer->HasViewProp(this->m_HCrossLine))
    updateView();
}

//-----------------------------------------------------------------------------
RendererSList View2D::renderers() const
{
  RendererSList genericRenderers;
  for (auto renderer: m_renderers)
    genericRenderers << renderer;

  return genericRenderers;
}

//-----------------------------------------------------------------------------
void View2D::activateRender(const QString &rendererName)
{
  for(auto action: m_renderConfig->menu()->actions())
    if (action->text() == rendererName)
      action->setChecked(true);

  for(auto renderer: m_renderers)
    if (renderer->name() == rendererName)
      renderer->setEnable(true);
}

//-----------------------------------------------------------------------------
void View2D::deactivateRender(const QString &rendererName)
{
  for(auto action: m_renderConfig->menu()->actions())
    if (action->text() == rendererName)
      action->setChecked(false);

  for(auto renderer: m_renderers)
    if (renderer->name() == rendererName)
      renderer->setEnable(false);
}

//-----------------------------------------------------------------------------
void View2D::setVisualState(struct RenderView::VisualState state)
{
  if (state.plane != m_plane)
    return;

  auto camera = m_renderer->GetActiveCamera();
  camera->SetPosition(state.cameraPosition[0], state.cameraPosition[1], state.cameraPosition[2]);
  camera->SetFocalPoint(state.focalPoint[0], state.focalPoint[1], state.focalPoint[2]);
  camera->Zoom(viewHeightLength() / state.heightLength);

  m_scrollBar->setValue(state.slice);
  updateView();
}

//-----------------------------------------------------------------------------
struct RenderView::VisualState View2D::visualState()
{
  struct RenderView::VisualState state;
  auto camera = m_renderer->GetActiveCamera();
  double cameraPos[3], focalPoint[3];
  camera->GetFocalPoint(focalPoint);
  camera->GetPosition(cameraPos);

  state.plane = m_plane;
  state.slice = m_scrollBar->value();
  state.cameraPosition = NmVector3{cameraPos[0], cameraPos[1], cameraPos[2]};
  state.focalPoint = NmVector3{focalPoint[0], focalPoint[1], focalPoint[2]};
  state.heightLength = viewHeightLength();
  return state;
}

//-----------------------------------------------------------------------------
double View2D::viewHeightLength()
{
  double ll[3], ur[3];
  vtkSmartPointer<vtkCoordinate> coords = vtkSmartPointer<vtkCoordinate>::New();
  coords->SetCoordinateSystemToNormalizedViewport();
  coords->SetValue(0, 0);
  memcpy(ll,coords->GetComputedWorldValue(m_renderer),3*sizeof(double));
  coords->SetValue(1, 1);
  memcpy(ur,coords->GetComputedWorldValue(m_renderer),3*sizeof(double));
  double heightDist = 0;

  switch (m_plane)
  {
    case Plane::XY:
    case Plane::XZ:
      heightDist = ll[0]-ur[0];
      break;
    case Plane::YZ:
      heightDist = ll[1]-ur[1];
      break;
    default:
      break;
  }

  return heightDist;
}

//-----------------------------------------------------------------------------
Selector::Selection View2D::select(const Selector::SelectionFlags flags, const int x, const int y) const
{
  QMap<NeuroItemAdapterPtr, BinaryMaskSPtr<unsigned char>> selectedItems;
  Selector::Selection finalSelection;

  for(auto renderer: m_renderers)
  {
    if(renderer->type() != Renderer::Type::Representation)
      continue;

    auto repRenderer = representationRenderer(renderer);

    if(flags.contains(Selector::SEGMENTATION) && canRender(repRenderer, RenderableType::SEGMENTATION))
    {
      for (auto item : repRenderer->pick(x, y, 0, m_renderer, RenderableItems(RenderableType::SEGMENTATION), true))
      {
        BinaryMaskSPtr<unsigned char> bm { new BinaryMask<unsigned char> { Bounds(repRenderer->pickCoordinates()), item->output()->spacing() } };
        BinaryMask<unsigned char>::iterator bmit(bm.get());
        bmit.goToBegin();
        bmit.Set();

        selectedItems[item] = bm;
        auto coords = repRenderer->pickCoordinates();
        qDebug() << "picked (in sample)" << item->data().toString() << "coords" << coords[0] << coords[1] << coords[2];

      }

    }

    if((flags.contains(Selector::CHANNEL) || flags.contains(Selector::SAMPLE)) && canRender(repRenderer, RenderableType::CHANNEL))
    {
      for (auto item : repRenderer->pick(x, y, 0, m_renderer, RenderableItems(RenderableType::CHANNEL), true))
      {
        if(flags.contains(Selector::CHANNEL))
        {
          BinaryMaskSPtr<unsigned char> bm { new BinaryMask<unsigned char> { Bounds(repRenderer->pickCoordinates()), item->output()->spacing() } };
          BinaryMask<unsigned char>::iterator bmit(bm.get());
          bmit.goToBegin();
          bmit.Set();

          selectedItems[item] = bm;
          auto coords = repRenderer->pickCoordinates();
          qDebug() << "picked (in sample)" << item->data().toString() << "coords" << coords[0] << coords[1] << coords[2];

        }


        if(flags.contains(Selector::SAMPLE))
        {
          BinaryMaskSPtr<unsigned char> bm { new BinaryMask<unsigned char> { Bounds(repRenderer->pickCoordinates()), item->output()->spacing() } };
          BinaryMask<unsigned char>::iterator bmit(bm.get());
          bmit.goToBegin();
          bmit.Set();

          auto sample = QueryAdapter::sample(dynamic_cast<ChannelAdapterPtr>(item));
          selectedItems[item] = bm;

          auto coords = repRenderer->pickCoordinates();
          qDebug() << "picked (in sample)" << item->data().toString() << "coords" << coords[0] << coords[1] << coords[2];
        }
      }
    }
  }

  for(auto item: selectedItems.keys())
    finalSelection << QPair<Selector::SelectionMask, NeuroItemAdapterPtr>(selectedItems[item], item);

  qDebug() << "view2d select" << x << y;

  return finalSelection;
}
