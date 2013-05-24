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

// EspINA
#include "SliceView.h"
#include "GUI/ISettingsPanel.h"
#include "GUI/QtWidget/SliceViewState.h"
#include "GUI/QtWidget/VolumeView.h"
#include "GUI/QtWidget/vtkInteractorStyleEspinaSlice.h"
#include "GUI/QtWidget/vtkInteractorStyleEspinaSlice.h"
#include "GUI/QtWidget/SliceSelectorWidget.h"
#include "GUI/ViewManager.h"
#include <GUI/Representations/GraphicalRepresentation.h>
#include <GUI/Representations/SliceRepresentation.h>
#include <Core/ColorEngines/IColorEngine.h>
#include <Core/ColorEngines/TransparencySelectionHighlighter.h>
#include <Core/EspinaTypes.h>
#include <Core/Model/Channel.h>
#include <Core/Model/Segmentation.h>
#include <Core/EspinaSettings.h>

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

// ITK
#include <itkImageToVTKImageFilter.h>

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
#include <vtkPolyDataMapper.h>
#include <vtkProp3DCollection.h>
#include <vtkPropCollection.h>
#include <vtkMath.h>
#include <vtkPropPicker.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkWidgetRepresentation.h>
#include <vtkWorldPointPicker.h>
#include <vtkImageShiftScale.h>
#include <vtkSmartPointer.h>
#include <vtkAxisActor2D.h>

using namespace EspINA;

const double SliceView::SEGMENTATION_SHIFT = 0.05;

//-----------------------------------------------------------------------------
// SLICE VIEW
//-----------------------------------------------------------------------------
SliceView::SliceView(EspinaFactoryPtr factory, ViewManager* vm, PlaneType plane, QWidget* parent)
: EspinaRenderView(vm, parent)
, m_titleLayout(new QHBoxLayout())
, m_title(new QLabel("Undefined"))
, m_mainLayout(new QVBoxLayout())
, m_controlLayout(new QHBoxLayout())
, m_fromLayout(new QHBoxLayout())
, m_toLayout(new QHBoxLayout())
, m_scrollBar(new QScrollBar(Qt::Horizontal))
, m_spinBox(new QDoubleSpinBox())
, m_zoomButton(new QPushButton())
, m_ruler(vtkSmartPointer<vtkAxisActor2D>::New())
, m_selectionEnabled(true)
, m_showSegmentations(true)
, m_showThumbnail(true)
, m_sliceSelector(QPair<SliceSelectorWidget*,SliceSelectorWidget*>(NULL, NULL))
, m_inThumbnail(false)
, m_sceneReady(false)
{
  QSettings settings(CESVIMA, ESPINA);
  m_fitToSlices = settings.value("ViewManager::FitToSlices").toBool();

  memset(m_crosshairPoint, 0, 3*sizeof(Nm));
  m_plane = plane;
  m_settings = SettingsPtr(new Settings(factory, this, m_plane));
  m_slicingStep[0] = m_slicingStep[1] = m_slicingStep[2] = 1;

  setupUI();

  // Color background
  QPalette pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Window));
  this->setPalette(pal);
  this->setStyleSheet("QSpinBox { background-color: white;}");

  switch (m_plane)
  {
    case AXIAL:
      m_state = new AxialState();
      break;
    case SAGITTAL:
      m_state = new SagittalState();
      break;
    case CORONAL:
      m_state = new CoronalState();
      break;
    case VOLUME:
    default:
      Q_ASSERT(false);
      break;
  };

  m_viewManager->registerView(this);

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

  m_slicingMatrix = vtkMatrix4x4::New();

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

  m_state->updateSlicingMatrix(m_slicingMatrix);
  vtkSmartPointer<vtkInteractorStyleEspinaSlice> interactor = vtkSmartPointer<vtkInteractorStyleEspinaSlice>::New();
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

  connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection, bool)),
          this, SLOT(updateSelection(ViewManager::Selection, bool)));

  foreach(IRenderer *renderer, m_settings->renderers())
    if (renderer->getRenderType().testFlag(IRenderer::RENDERER_SLICEVIEW))
      this->addRendererControls(renderer->clone());
}

//-----------------------------------------------------------------------------
SliceView::~SliceView()
{
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying Slice View" << m_plane;
//   qDebug() << "********************************************************";

  // Representation destructors may need to access slice view in their destructors
  m_channelStates.clear();
  m_segmentationStates.clear();

  m_viewManager->unregisterView(this);

  m_slicingMatrix->Delete();
  delete m_state;
}

//-----------------------------------------------------------------------------
void SliceView::reset()
{
  foreach(EspinaWidget *widget, m_widgets.keys())
    removeWidget(widget);

  foreach(SegmentationPtr segmentation, m_segmentationStates.keys())
    removeSegmentation(segmentation);

  foreach(ChannelPtr channel, m_channelStates.keys())
    removeChannel(channel);
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
void SliceView::updateRuler()
{
  if (!m_renderer || !m_view->GetRenderWindow())
    return;

  double *value;
  Nm left, right;
  vtkSmartPointer<vtkCoordinate> coords = vtkSmartPointer<vtkCoordinate>::New();
  coords->SetCoordinateSystemToNormalizedViewport();

  int c = m_plane==SAGITTAL?2:0;
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
void SliceView::updateThumbnail()
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

  int h = m_plane==SAGITTAL?2:0;
  int v = m_plane==CORONAL?2:1;
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
void SliceView::updateSceneBounds()
{
  EspinaRenderView::updateSceneBounds();
  setSlicingStep(m_sceneResolution);

  // we need to update the view only if a signal has been sent
  // (the volume of a channel has been updated)
  if (sender() != NULL)
    updateView();
}

//-----------------------------------------------------------------------------
void SliceView::initBorder(vtkPolyData* data, vtkActor* actor)
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
  data->Update();

  vtkSmartPointer<vtkPolyDataMapper> Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  Mapper->SetInput(data);
  actor->SetMapper(Mapper);
  actor->GetProperty()->SetLineWidth(2);
  actor->SetPickable(false);

  m_thumbnail->AddActor(actor);
}

//-----------------------------------------------------------------------------
void SliceView::updateBorder(vtkPolyData* data,
                             double left, double right,
                             double upper, double lower)
{
  vtkPoints *corners = data->GetPoints();

  switch (m_plane)
  {
    case AXIAL:
      corners->SetPoint(0, left,  upper, -0.1); //UL
      corners->SetPoint(1, right, upper, -0.1); //UR
      corners->SetPoint(2, right, lower, -0.1); //LR
      corners->SetPoint(3, left,  lower, -0.1); //LL
      break;
    case SAGITTAL:
      corners->SetPoint(0, 0.1, upper,  left); //UL
      corners->SetPoint(1, 0.1, lower,  left); //UR
      corners->SetPoint(2, 0.1, lower, right); //LR
      corners->SetPoint(3, 0.1, upper, right); //LL
      break;
    case CORONAL:
      corners->SetPoint(0, left,  0.1, upper); //UL
      corners->SetPoint(1, right, 0.1, upper); //UR
      corners->SetPoint(2, right, 0.1, lower); //LR
      corners->SetPoint(3, left,  0.1, lower); //LL
      break;
    default:
      Q_ASSERT(false);
      break;
  }
  data->Modified();
}

//-----------------------------------------------------------------------------
Nm SliceView::voxelBottom(int sliceIndex, PlaneType plane) const
{
  return (sliceIndex - 0.5) * m_slicingStep[plane];
}

//-----------------------------------------------------------------------------
Nm SliceView::voxelBottom(Nm position, PlaneType plane) const
{
  return voxelBottom(voxelSlice(position, plane), plane);
}

//-----------------------------------------------------------------------------
Nm SliceView::voxelCenter(int sliceIndex, PlaneType plane) const
{
  return sliceIndex * m_slicingStep[plane];
}

//-----------------------------------------------------------------------------
Nm SliceView::voxelCenter(Nm position, PlaneType plane) const
{
  return voxelCenter(voxelSlice(position, plane), plane);
}


//-----------------------------------------------------------------------------
Nm SliceView::voxelTop(int sliceIndex, PlaneType plane) const
{
  return (sliceIndex + 0.5) * m_slicingStep[plane];
}

//-----------------------------------------------------------------------------
Nm SliceView::voxelTop(Nm position, PlaneType plane) const
{
  return voxelTop(voxelSlice(position, plane), plane);
}


//-----------------------------------------------------------------------------
int SliceView::voxelSlice(Nm position, PlaneType plane) const
{
  return vtkMath::Round(position/m_slicingStep[plane]);
}

//-----------------------------------------------------------------------------
void SliceView::buildCrosshairs()
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
  m_HCrossLineData->Update();

  vtkSmartPointer<vtkPolyDataMapper> HMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  HMapper->SetInput(m_HCrossLineData);

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
  m_VCrossLineData->Update();

  vtkSmartPointer<vtkPolyDataMapper> VMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  VMapper->SetInput(m_VCrossLineData);

  m_VCrossLine = vtkSmartPointer<vtkActor>::New();
  m_VCrossLine->SetMapper(VMapper);
  m_VCrossLine->GetProperty()->SetLineWidth(2);
  m_VCrossLine->SetPickable(false);

}

//-----------------------------------------------------------------------------
void SliceView::buildTitle()
{
  QPushButton *close = new QPushButton("x");
  close->setMaximumHeight(20);
  close->setMaximumWidth(20);

  QPushButton *max = new QPushButton("-");
  max->setMaximumHeight(20);
  max->setMaximumWidth(20);

  QPushButton *undock = new QPushButton("^");
  undock->setMaximumHeight(20);
  undock->setMaximumWidth(20);

  connect(close, SIGNAL(clicked(bool)),
          this, SLOT(close()));
  connect(max, SIGNAL(clicked(bool)),
          this, SLOT(maximize()));
  connect(undock, SIGNAL(clicked(bool)),
          this, SLOT(undock()));

  m_titleLayout->addWidget(m_title);
  m_titleLayout->addWidget(max);
  m_titleLayout->addWidget(undock);
  m_titleLayout->addWidget(close);

  m_mainLayout->addLayout(m_titleLayout);
}

//-----------------------------------------------------------------------------
void SliceView::setupUI()
{
  m_view->installEventFilter(this);

  m_zoomButton->setIcon(QIcon(":/espina/zoom_reset.png"));
  m_zoomButton->setToolTip(tr("Reset view's camera"));
  m_zoomButton->setFlat(true);
  m_zoomButton->setIconSize(QSize(20,20));
  m_zoomButton->setMaximumSize(QSize(22,22));
  m_zoomButton->setCheckable(false);
  connect(m_zoomButton, SIGNAL(clicked()), this, SLOT(resetView()));

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
  m_controlLayout->addWidget(m_scrollBar);
  m_controlLayout->addLayout(m_fromLayout);
  m_controlLayout->addWidget(m_spinBox);
  m_controlLayout->addLayout(m_toLayout);

  m_mainLayout->addLayout(m_controlLayout);
}

//-----------------------------------------------------------------------------
QString SliceView::title() const
{
  return m_title->text();
}

//-----------------------------------------------------------------------------
void SliceView::setTitle(const QString& title)
{
  m_title->setText(title);
}

//-----------------------------------------------------------------------------
void SliceView::setCrosshairColors(double hcolor[3], double vcolor[3])
{
  m_HCrossLine->GetProperty()->SetColor(hcolor);
  m_VCrossLine->GetProperty()->SetColor(vcolor);
}

//-----------------------------------------------------------------------------
void SliceView::showCrosshairs(bool visible)
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
void SliceView::setThumbnailVisibility(bool visible)
{
  m_showThumbnail = visible;
  m_thumbnail->SetDraw(visible && m_sceneReady);
  updateView();
}

//-----------------------------------------------------------------------------
ISelector::PickList SliceView::pick(ISelector::PickableItems     filter,
                                    ISelector::DisplayRegionList regions)
{
  bool multiSelection = false;
  ISelector::PickList pickedItems;

  vtkRenderer *renderer = m_renderer;
  Q_ASSERT(renderer);

  // Select all products that belongs to all regions
  // NOTE: Should first loop be removed? Only useful to select disconnected regions...
  foreach(const ISelector::DisplayRegion &region, regions)
  {
    foreach(QPointF p, region)
    {
      foreach(ISelector::Tag tag, filter)
      {
        if (ISelector::CHANNEL == tag)
        {
          foreach(IRendererSPtr renderer, m_itemRenderers)
          {
            if (renderer->getRenderableItemsType().testFlag(IRenderer::RENDERER_CHANNEL))
            {
              foreach(PickableItemPtr item, renderer->pick(p.x(), p.y(), NULL, multiSelection))
              {
                ISelector::WorldRegion wRegion = worldRegion(region, item);
                pickedItems << ISelector::PickedItem(wRegion, item);
              }
            }
          }
        }
        else
        {
          if (ISelector::SEGMENTATION == tag)
          {
            foreach(IRendererSPtr renderer, m_itemRenderers)
            {
              if (renderer->getRenderableItemsType().testFlag(IRenderer::RENDERER_CHANNEL))
              {
                foreach(PickableItemPtr item, renderer->pick(p.x(), p.y(), NULL, multiSelection))
                {
                  ISelector::WorldRegion wRegion = worldRegion(region, item);
                  pickedItems << ISelector::PickedItem(wRegion, item);
                }
              }
            }
          }
        }
      }
    }
  }

  return pickedItems;
}

//-----------------------------------------------------------------------------
void SliceView::worldCoordinates(const QPoint& displayPos,
                                 double worldPos[3])
{
  double LL[3], UR[3];
  int viewSize[2];
  memcpy(viewSize, m_view->GetRenderWindow()->GetSize(), 2*sizeof(int));

  // Display bounds in world coordinates
  vtkSmartPointer<vtkCoordinate> coords = vtkSmartPointer<vtkCoordinate>::New();
  coords->SetViewport(m_renderer);
  coords->SetCoordinateSystemToNormalizedViewport();
  coords->SetValue(0, 0); //LL
  memcpy(LL,coords->GetComputedWorldValue(m_renderer),3*sizeof(double));
  coords->SetValue(1, 1); //UR
  memcpy(UR,coords->GetComputedWorldValue(m_renderer),3*sizeof(double));

  int H = (SAGITTAL == m_plane)?2:0;
  int V = (CORONAL  == m_plane)?2:1;

  double worldSize[2];

  worldSize[0] = fabs(UR[H] - LL[H]);
  worldSize[1] = fabs(UR[V] - LL[V]);

  worldPos[m_plane] = slicingPosition();
  worldPos[H]       = LL[H] + displayPos.x()*worldSize[0]/viewSize[0];
  worldPos[V]       = UR[V] + displayPos.y()*worldSize[1]/viewSize[1];
}

//-----------------------------------------------------------------------------
void SliceView::setSelectionEnabled(bool enable)
{
  m_selectionEnabled = enable;
}

//-----------------------------------------------------------------------------
void SliceView::updateView()
{
  if (isVisible())
  {
    //qDebug() << "Updating View";
    updateRuler();
    updateWidgetVisibility();
    updateThumbnail();
    m_view->GetRenderWindow()->Render();
    m_view->update();
  }
}

//-----------------------------------------------------------------------------
void SliceView::resetCamera()
{
  double origin[3] = { 0, 0, 0 };

  m_state->updateCamera(m_renderer ->GetActiveCamera(), origin);
  m_state->updateCamera(m_thumbnail->GetActiveCamera(), origin);

  m_renderer->ResetCamera();

  m_thumbnail->RemoveActor(m_channelBorder);
  m_thumbnail->RemoveActor(m_viewportBorder);
  updateThumbnail();
  m_thumbnail->ResetCamera();
  m_thumbnail->AddActor(m_channelBorder);
  m_thumbnail->AddActor(m_viewportBorder);

  m_sceneReady = !m_channelStates.isEmpty();
}

//-----------------------------------------------------------------------------
void SliceView::addWidget(EspinaWidget *eWidget)
{
  Q_ASSERT(!m_widgets.contains(eWidget));

  SliceWidget *sWidget = eWidget->createSliceWidget(this);
  if (!sWidget)
    return;

  sWidget->setSlice(slicingPosition(), m_plane);

  vtkAbstractWidget *widget = *sWidget;
  if (widget)
  {
    widget->SetInteractor(m_view->GetRenderWindow()->GetInteractor());
    if (widget->GetRepresentation())
      widget->GetRepresentation()->SetVisibility(true);
    widget->On();
  }
  m_renderer->ResetCameraClippingRange();
  m_widgets[eWidget] = sWidget;
}

//-----------------------------------------------------------------------------
void SliceView::removeWidget(EspinaWidget *eWidget)
{
  if (!m_widgets.contains(eWidget))
    return;

  vtkAbstractWidget *widget = *m_widgets[eWidget];
  widget->Off();
  widget->SetInteractor(NULL);
  widget->RemoveAllObservers();
  m_widgets.remove(eWidget);
}

//-----------------------------------------------------------------------------
void SliceView::addActor(vtkProp3D* actor)
{
  m_state->updateActor(actor);

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
void SliceView::removeActor(vtkProp3D* actor)
{
  m_renderer->RemoveActor(actor);
  m_thumbnail->RemoveActor(actor);

  updateThumbnail();
}

//-----------------------------------------------------------------------------
void SliceView::previewBounds(Nm bounds[6], bool cropToSceneBounds)
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

  int H = (SAGITTAL == m_plane)?2:0;
  int V = (CORONAL  == m_plane)?2:1;

  bounds[2*H]     = LL[H];
  bounds[(2*H)+1] = UR[H];
  bounds[2*V]     = UR[V];
  bounds[(2*V)+1] = LL[V];
  bounds[2*m_plane]   = slicingPosition();
  bounds[2*m_plane+1] = slicingPosition();

  if (cropToSceneBounds)
  {
    bounds[2*H]     = std::max(LL[H], m_sceneBounds[2*H]);
    bounds[(2*H)+1] = std::min(UR[H], m_sceneBounds[2*H+1]);
    bounds[2*V]     = std::max(UR[V], m_sceneBounds[2*V]);
    bounds[(2*V)+1] = std::min(LL[V], m_sceneBounds[2*V+1]);
  }
}

//-----------------------------------------------------------------------------
void SliceView::sliceViewCenterChanged(Nm x, Nm y, Nm z)
{
  //qDebug() << "Slice View: " << m_plane << " has new center";
  emit centerChanged(x, y, z);
}

//-----------------------------------------------------------------------------
void SliceView::scrollValueChanged(int value /*slice index */)
{
  // WARNING: Any modification to this method must be taken into account
  // at the end block of setSlicingStep
  m_crosshairPoint[m_plane] = voxelCenter(value, m_plane);

  m_state->setSlicingPosition(m_slicingMatrix, voxelBottom(value, m_plane));

  m_spinBox->blockSignals(true);
  m_spinBox->setValue(m_fitToSlices ? value + 1: slicingPosition());
  m_spinBox->blockSignals(false);

  updateView();

  emit sliceChanged(m_plane, slicingPosition());
}

//-----------------------------------------------------------------------------
void SliceView::spinValueChanged(double value /* nm or slices depending on m_fitToSlices */)
{
  int sliceIndex = m_fitToSlices ? (value - 1) : voxelSlice(value, m_plane);
  m_crosshairPoint[m_plane] = voxelCenter(sliceIndex, m_plane);

  m_state->setSlicingPosition(m_slicingMatrix, voxelBottom(sliceIndex, m_plane));

  m_scrollBar->blockSignals(true);
  m_scrollBar->setValue(m_fitToSlices? (value - 1) : vtkMath::Round(value/m_slicingStep[m_plane]));
  m_scrollBar->blockSignals(false);

  updateView();
  emit sliceChanged(m_plane, slicingPosition());
}

//-----------------------------------------------------------------------------
void SliceView::selectFromSlice()
{
//   m_fromSlice->setToolTip(tr("From Slice %1").arg(m_spinBox->value()));
//   emit sliceSelected(slicingPosition(), m_plane, ViewManager::From);
}

//-----------------------------------------------------------------------------
void SliceView::selectToSlice()
{
//   m_toSlice->setToolTip(tr("To Slice %1").arg(m_spinBox->value()));
//   emit sliceSelected(slicingPosition(), m_plane, ViewManager::To);
}

//-----------------------------------------------------------------------------
bool SliceView::eventFilter(QObject* caller, QEvent* e)
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

  if (m_viewManager->filterEvent(e, this))
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
      int numSteps = we->delta() / 8 / 15 * (m_settings->invertWheel() ? -1 : 1);  //Refer to QWheelEvent doc.
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

    m_view->setCursor(m_viewManager->cursor());
    e->accept();}
   else if (QEvent::Leave == e->type())
   {
     inFocus = false;
  }
  else if (QEvent::MouseMove == e->type())
  {
    int x, y;
    eventPosition(x, y);
    m_inThumbnail = m_thumbnail->GetDraw() && (m_thumbnail->PickProp(x,y) != NULL);
  }
  else if (QEvent::ContextMenu == e->type())
  {
    QContextMenuEvent *cme = dynamic_cast<QContextMenuEvent*>(e);
    if (cme->modifiers() == Qt::CTRL && m_contextMenu.get() && m_selectionEnabled)
    {
      m_contextMenu->setSelection(m_viewManager->selectedSegmentations());
      m_contextMenu->exec(mapToGlobal(cme->pos()));
    }
  }
  else if (QEvent::ToolTip == e->type())
  {
    int x, y;
    eventPosition(x, y);
    ViewManager::Selection selection = pickSegmentations(x, y, m_renderer);
    QString toopTip;
    foreach(PickableItemPtr pick, selection)
    {
      toopTip = toopTip.append("<b>%1</b><br>").arg(pick->data().toString());
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
      m_view->setCursor(Qt::ArrowCursor);
    else
      m_view->setCursor(m_viewManager->cursor());

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
        else if (m_selectionEnabled)
          selectPickedItems(me->modifiers() == Qt::SHIFT);
    }
  }

  return QWidget::eventFilter(caller, e);
}

//-----------------------------------------------------------------------------
void SliceView::centerCrosshairOnMousePosition()
{
  int xPos, yPos;
  eventPosition(xPos, yPos);

  Nm center[3];  //World coordinates
  bool channelPicked = false;
  if (m_inThumbnail)
  {
    foreach(IRendererSPtr renderer, m_itemRenderers)
    {
      if (renderer->getRenderableItemsType().testFlag(IRenderer::RENDERER_CHANNEL))
      {
        ViewManager::Selection selection = renderer->pick(xPos, yPos, m_thumbnail);
        if (!selection.isEmpty())
        {
          channelPicked = true;
          renderer->getPickCoordinates(center);
          break;
        }
      }
    }
  }
  else
  {
    foreach(IRendererSPtr renderer, m_itemRenderers)
    {
      if (renderer->getRenderableItemsType().testFlag(IRenderer::RENDERER_CHANNEL))
      {
        ViewManager::Selection selection = renderer->pick(xPos, yPos, NULL);
        if (!selection.isEmpty())
        {
          channelPicked = true;
          renderer->getPickCoordinates(center);
          break;
        }
      }
    }
  }

  if (channelPicked)
  {
    center[this->m_plane] = slicingPosition();
    centerViewOn(center);
    emit centerChanged(m_crosshairPoint[0], m_crosshairPoint[1], m_crosshairPoint[2]);
  }
}

//-----------------------------------------------------------------------------
void SliceView::centerViewOnMousePosition()
{
  int xPos, yPos;
  eventPosition(xPos, yPos);

  foreach(IRendererSPtr renderer, m_itemRenderers)
    if (renderer->getRenderableItemsType().testFlag(IRenderer::RENDERER_CHANNEL))
    {
      ViewManager::Selection selection = renderer->pick(xPos, yPos, m_thumbnail, false);
      if (!selection.isEmpty())
      {
        Nm center[3];
        renderer->getPickCoordinates(center);
        centerViewOnPosition(center);
      }
    }
}

//-----------------------------------------------------------------------------
ViewManager::Selection SliceView::pickChannels(double vx, double vy,
                                               vtkRenderer* renderer,
                                               bool repeatable)
{
  ViewManager::Selection selection;

  foreach(IRendererSPtr renderer, m_itemRenderers)
    if (renderer->getRenderableItemsType().testFlag(IRenderer::RENDERER_CHANNEL))
      foreach(PickableItemPtr item, renderer->pick(vx,vy, NULL, repeatable))
      {
        if (!selection.contains(item))
          selection << item;
      }

  return selection;
}

//-----------------------------------------------------------------------------
ViewManager::Selection SliceView::pickSegmentations(double vx, double vy,
                                                    vtkRenderer* renderer,
                                                    bool repeatable)
{
  ViewManager::Selection selection;

  foreach(IRendererSPtr renderer, m_itemRenderers)
    if (renderer->getRenderableItemsType().testFlag(IRenderer::RENDERER_SEGMENTATION))
      foreach(PickableItemPtr item, renderer->pick(vx,vy, NULL, repeatable))
      {
        if (!selection.contains(item))
          selection << item;
      }

  return selection;
}

//-----------------------------------------------------------------------------
void SliceView::selectPickedItems(bool append)
{
  int vx, vy;
  eventPosition(vx, vy);

  ViewManager::Selection selection;
  if (append)
    selection = m_viewManager->selection();

  // segmentations have priority over channels
  foreach(PickableItemPtr item, pickSegmentations(vx, vy, m_renderer, append))
  {
    if (selection.contains(item))
      selection.removeAll(item);
    else
      selection << item;

    if (!append)
      break;
  }

  foreach(PickableItemPtr item, pickChannels(vx, vy, m_renderer, append))
  {
    if (selection.contains(item))
      selection.removeAll(item);
    else
      selection << item;

    if (!append)
      break;
  }

  m_viewManager->setSelection(selection);
}

//-----------------------------------------------------------------------------
void SliceView::updateWidgetVisibility()
{
  foreach(SliceWidget * widget, m_widgets)
  {
    widget->setSlice(slicingPosition(), m_plane);
  }
}

//-----------------------------------------------------------------------------
void SliceView::updateChannelsOpactity()
{
  // TODO: Define opacity behaviour
  double opacity = suggestedChannelOpacity();

  foreach(ChannelPtr channel, m_channelStates.keys())
  {
    if (Channel::AUTOMATIC_OPACITY == channel->opacity())
    {
      foreach(ChannelGraphicalRepresentationSPtr representation, m_channelStates[channel].representations)
      {
        representation->setOpacity(opacity);
      }
    }
  }
}

//-----------------------------------------------------------------------------
bool SliceView::pick(vtkPropPicker *picker, int x, int y, Nm pickPos[3])
{
  if (m_thumbnail->GetDraw() && picker->Pick(x, y, 0.1, m_thumbnail))
    return false;//ePick Fail

  if (!picker->Pick(x, y, 0.1, m_renderer))
      return false;//ePick Fail

  picker->GetPickPosition(pickPos);
  pickPos[m_plane] = slicingPosition();

  return true;
}

//-----------------------------------------------------------------------------
void SliceView::setSegmentationVisibility(bool visible)
{
  m_showSegmentations = visible;

  updateSegmentationRepresentations();

  updateView();
}

//-----------------------------------------------------------------------------
void SliceView::setShowPreprocessing(bool visible)
{
  if (m_channelStates.size() < 2)
    return;

  ChannelPtr hiddenChannel = m_channelStates.keys()[visible];
  ChannelPtr visibleChannel = m_channelStates.keys()[1 - visible];
  hiddenChannel->setData(false, Qt::CheckStateRole);
  hiddenChannel->notifyModification();
  visibleChannel->setData(true, Qt::CheckStateRole);
  visibleChannel->notifyModification();
  for (int i = 2; i < m_channelStates.keys().size(); i++)
  {
    ChannelPtr otherChannel = m_channelStates.keys()[i];
    otherChannel->setData(false, Qt::CheckStateRole);
    otherChannel->notifyModification();
  }
  updateChannelRepresentations();
}

//-----------------------------------------------------------------------------
void SliceView::setRulerVisibility(bool visible)
{
  m_rulerVisibility = visible;
  updateRuler();
  updateView();
}

//-----------------------------------------------------------------------------
void SliceView::addSliceSelectors(SliceSelectorWidget* widget,
                                  ViewManager::SliceSelectors selectors)
{
  if (m_sliceSelector.first != widget)
  {
    if (m_sliceSelector.second)
      delete m_sliceSelector.second;

    m_sliceSelector.first  = widget;
    m_sliceSelector.second = widget->clone();
  }

  SliceSelectorWidget *sliceSelector = m_sliceSelector.second;

  sliceSelector->setPlane(m_plane);
  sliceSelector->setView (this);

  QWidget *fromWidget = sliceSelector->leftWidget();
  QWidget *toWidget   = sliceSelector->rightWidget();

  bool showFrom = selectors.testFlag(ViewManager::From);
  bool showTo   = selectors.testFlag(ViewManager::To  );

  fromWidget->setVisible(showFrom);
  toWidget  ->setVisible(showTo  );

  m_fromLayout->addWidget (fromWidget );
  m_toLayout->insertWidget(0, toWidget);
}

//-----------------------------------------------------------------------------
void SliceView::removeSliceSelectors(SliceSelectorWidget* widget)
{
  if (m_sliceSelector.first == widget)
  {
    if (m_sliceSelector.second)
      delete m_sliceSelector.second;

    m_sliceSelector.first  = NULL;
    m_sliceSelector.second = NULL;
  }
}


//----------------------------------------------------------------------------
void SliceView::slicingStep(Nm steps[3])
{
  memcpy(steps, m_slicingStep, 3 * sizeof(Nm));
}

//-----------------------------------------------------------------------------
void SliceView::setSlicingStep(const Nm steps[3])
{
  if (steps[0] <= 0 || steps[1] <= 0 || steps[2] <= 0)
  {
    qFatal("SliceView: Invalid Step value. Slicing Step not changed");
    return;
  }

  int sliceIndex = voxelSlice(slicingPosition(), m_plane);

  memcpy(m_slicingStep, steps, 3*sizeof(Nm));

  QSettings settings(CESVIMA, ESPINA);
  m_fitToSlices = m_plane == AXIAL && settings.value("ViewManager::FitToSlices").toBool();

  setSlicingBounds(m_sceneBounds);

  if(m_fitToSlices)
    m_spinBox->setSingleStep(1);
  else
    m_spinBox->setSingleStep(m_slicingStep[m_plane]);

  m_scrollBar->setValue(sliceIndex);

  {
    // We want to avoid the view update while loading channels
    // on scrollValueChanged(sliceIndex)
    m_crosshairPoint[m_plane] = voxelCenter(sliceIndex, m_plane);

    m_state->setSlicingPosition(m_slicingMatrix, voxelBottom(sliceIndex, m_plane));

    m_spinBox->blockSignals(true);
    m_spinBox->setValue(m_fitToSlices ? sliceIndex + 1: slicingPosition());
    m_spinBox->blockSignals(false);

    emit sliceChanged(m_plane, slicingPosition());
    //scrollValueChanged(sliceIndex); // Updates spin box's value
  }
}

//-----------------------------------------------------------------------------
Nm SliceView::slicingPosition() const
{
  return m_crosshairPoint[m_plane];
}


//-----------------------------------------------------------------------------
void SliceView::setSlicingBounds(Nm bounds[6])
{
  if (bounds[1] < bounds[0] || bounds[3] < bounds[2] || bounds[5] < bounds[4])
  {
    qFatal("SliceView: Invalid Slicing Ranges. Ranges not changed");
    return;
  }

  int sliceMax = voxelSlice(bounds[2*m_plane+1], m_plane);
  int sliceMin = voxelSlice(bounds[2*m_plane]  , m_plane);

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
void SliceView::centerViewOn(Nm center[3], bool force)
{
  Nm centerVoxel[3];
  // Adjust crosshairs to fit slicing steps
  for (int i = 0; i < 3; i++)
    centerVoxel[i] = voxelCenter(center[i], PlaneType(i));

  if (!isVisible() ||
     (m_crosshairPoint[0] == centerVoxel[0] &&
      m_crosshairPoint[1] == centerVoxel[1] &&
      m_crosshairPoint[2] == centerVoxel[2] &&
      !force))
    return;

  // Adjust crosshairs to fit slicing steps
  memcpy(m_crosshairPoint, centerVoxel, 3*sizeof(Nm));

  // Disable scrollbar signals to avoid calling setting slice
  m_scrollBar->blockSignals(true);
  m_spinBox->blockSignals(true);

  Nm slicingPos = voxelSlice(center[m_plane], m_plane);

  m_scrollBar->setValue(slicingPos);

  if (m_fitToSlices)
    slicingPos++; // Correct 0 index
  else
    slicingPos = voxelCenter(slicingPos, m_plane);

  m_spinBox->setValue(slicingPos);

  m_spinBox->blockSignals(false);
  m_scrollBar->blockSignals(false);

  m_state->setSlicingPosition(m_slicingMatrix, voxelBottom(m_scrollBar->value(), m_plane));
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

  int H = (SAGITTAL == m_plane)?2:0;
  int V = (CORONAL  == m_plane)?2:1;
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
void SliceView::centerViewOnPosition(Nm center[3])
{
  if (!isVisible() || (m_crosshairPoint[0] == center[0] &&
     m_crosshairPoint[1] == center[1] &&
     m_crosshairPoint[2] == center[2]))
    return;

  m_state->updateCamera(m_renderer->GetActiveCamera(), center);
  m_renderer->ResetCameraClippingRange();
  updateView();
}

//-----------------------------------------------------------------------------
ISelector::WorldRegion SliceView::worldRegion(const ISelector::DisplayRegion& region,
                                              PickableItemPtr item)
{
  ISelector::WorldRegion wRegion = ISelector::WorldRegion::New();

  foreach(QPointF point, region)
  {
    Nm pickPos[3];
    if (EspINA::CHANNEL == item->type())
    {
      foreach(IRendererSPtr renderer, m_itemRenderers)
        if (renderer->getRenderableItemsType().testFlag(IRenderer::RENDERER_CHANNEL) && !renderer->pick(point.x(), point.y(), NULL, false).isEmpty())
        {
          renderer->getPickCoordinates(pickPos);
          wRegion->InsertNextPoint(pickPos);
        }
    }
    else
    {
      foreach(IRendererSPtr renderer, m_itemRenderers)
        if (renderer->getRenderableItemsType().testFlag(IRenderer::RENDERER_SEGMENTATION) && !renderer->pick(point.x(), point.y(), NULL, false).isEmpty())
        {
          renderer->getPickCoordinates(pickPos);
          wRegion->InsertNextPoint(pickPos);
        }
    }
  }

  return wRegion;
}

//-----------------------------------------------------------------------------
GraphicalRepresentationSPtr SliceView::cloneRepresentation(GraphicalRepresentationSPtr prototype)
{
  GraphicalRepresentationSPtr rep = GraphicalRepresentationSPtr();

  if (prototype->canRenderOnView().testFlag(GraphicalRepresentation::RENDERABLEVIEW_SLICE))
    rep = prototype->clone(this);

  return rep;
}

//-----------------------------------------------------------------------------
void SliceView::addRendererControls(IRendererSPtr renderer)
{
  renderer->setViewData(this, this->m_renderer);
  renderer->setEnable(true);

  // add representations to renderer
  foreach(SegmentationPtr segmentation, m_segmentationStates.keys())
  {
    if (renderer->itemCanBeRendered(segmentation))
      foreach(GraphicalRepresentationSPtr rep, m_segmentationStates[segmentation].representations)
         if (renderer->managesRepresentation(rep)) renderer->addRepresentation(segmentation, rep);
  }

  foreach(ChannelPtr channel, m_channelStates.keys())
  {
    if (renderer->itemCanBeRendered(channel))
      foreach(GraphicalRepresentationSPtr rep, m_channelStates[channel].representations)
        if (renderer->managesRepresentation(rep)) renderer->addRepresentation(channel, rep);
  }

  QPushButton *button = new QPushButton();
  m_itemRenderers << renderer;
  m_renderers[button] = renderer;

  if (!renderer->isHidden())
  {
    if (renderer->getRenderableItemsType().testFlag(IRenderer::RENDERER_SEGMENTATION))
      this->m_numEnabledSegmentationRenders++;

    if (renderer->getRenderableItemsType().testFlag(IRenderer::RENDERER_CHANNEL))
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
void SliceView::removeRendererControls(QString name)
{
  IRendererSPtr removedRenderer;
  foreach (IRendererSPtr renderer, m_itemRenderers)
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

    if (!removedRenderer->isHidden() && (removedRenderer->getRenderableItemsType().testFlag(IRenderer::RENDERER_SEGMENTATION)))
      this->m_numEnabledSegmentationRenders--;

    if (!removedRenderer->isHidden() && (removedRenderer->getRenderableItemsType().testFlag(IRenderer::RENDERER_CHANNEL)))
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

    QMap<QPushButton*, IRendererSPtr>::iterator it = m_renderers.begin();
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

    m_itemRenderers.removeAll(removedRenderer);
  }
}

//-----------------------------------------------------------------------------
// SLICEVIEW::SETTINGS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
SliceView::Settings::Settings(const EspinaFactoryPtr factory, SliceView *parent, PlaneType plane, const QString prefix)
: INVERT_SLICE_ORDER(prefix + view(plane) + "::invertSliceOrder")
, INVERT_WHEEL(prefix + view(plane) + "::invertWheel")
, SHOW_AXIS(prefix + view(plane) + "::showAxis")
, RENDERERS(prefix + "SliceView::renderers")
, m_InvertWheel(false)
, m_InvertSliceOrder(false)
, m_ShowAxis(false)
, m_parent(parent)
, m_plane(plane)
{
  QSettings settings(CESVIMA, ESPINA);

  m_InvertSliceOrder = settings.value(INVERT_SLICE_ORDER, false).toBool();
  m_InvertWheel      = settings.value(INVERT_WHEEL, false).toBool();
  m_ShowAxis         = settings.value(SHOW_AXIS, false).toBool();

  m_parent = parent;

  if (!settings.contains(RENDERERS))
    settings.setValue(RENDERERS, QStringList() << "Slice");

  QMap<QString, IRenderer *> renderers = factory->renderers();
  foreach(QString name, settings.value(RENDERERS).toStringList())
  {
    IRenderer *renderer = renderers.value(name, NULL);
    if (renderer)
      m_renderers << renderer;
  }

}

//-----------------------------------------------------------------------------
const QString SliceView::Settings::view(PlaneType plane)
{
  switch (plane)
  {
    case AXIAL:
      return QString("AxialSliceView");
      break;
    case SAGITTAL:
      return QString("SagittalSliceView");
      break;
    case CORONAL:
      return QString("CoronalSliceView");
      break;
    case VOLUME:
      return QString("VolumeView");
      break;
    default:
      break;
  };

  return QString("Unknown");
}

//-----------------------------------------------------------------------------
void SliceView::Settings::setInvertSliceOrder(bool value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(INVERT_SLICE_ORDER, value);
  m_InvertSliceOrder = value;
}

//-----------------------------------------------------------------------------
void SliceView::Settings::setInvertWheel(bool value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(INVERT_WHEEL, value);
  m_InvertWheel = value;
}

//-----------------------------------------------------------------------------
void SliceView::Settings::setShowAxis(bool value)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.setValue(SHOW_AXIS, value);
  m_ShowAxis = value;
}

//-----------------------------------------------------------------------------
void SliceView::updateCrosshairPoint(PlaneType plane, Nm slicePos)
{
  m_crosshairPoint[plane] = voxelCenter(slicePos, plane);
  m_state->setCrossHairs(m_HCrossLineData, m_VCrossLineData,
                         m_crosshairPoint, m_sceneBounds, m_slicingStep);

  // render if present
  if (this->m_renderer->HasViewProp(this->m_HCrossLine))
    updateView();
}

//-----------------------------------------------------------------------------
void SliceView::Settings::setRenderers(QList<IRenderer *> values)
{
  QSettings settings(CESVIMA, ESPINA);
  QStringList activeRenderersNames;
  QList<IRenderer *> activeRenderers;

  // remove controls for unused renderers
  foreach(IRenderer *oldRenderer, m_renderers)
  {
    bool selected = false;
    int i = 0;
    while (!selected && i < values.size())
      selected = values[i++]->name() == oldRenderer->name();

    if (!selected)
      m_parent->removeRendererControls(oldRenderer->name());
    else
      activeRenderers << oldRenderer;
  }

  // add controls for added renderers
  foreach(IRenderer *renderer, values)
  {
    activeRenderersNames << renderer->name();
    if (!activeRenderers.contains(renderer))
    {
      activeRenderers << renderer;
      m_parent->addRendererControls(renderer->clone());
    }
  }

  settings.setValue(RENDERERS, activeRenderersNames);
  settings.sync();
  m_renderers = activeRenderers;
}
