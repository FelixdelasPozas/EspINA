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

#include "SliceView.h"

// // EspINA
#include "GUI/ISettingsPanel.h"
#include "GUI/QtWidget/SliceViewState.h"
#include "GUI/QtWidget/VolumeView.h"
#include "GUI/QtWidget/vtkInteractorStyleEspinaSlice.h"
#include "GUI/QtWidget/vtkInteractorStyleEspinaSlice.h"
#include "GUI/QtWidget/SliceSelectorWidget.h"
#include "GUI/ViewManager.h"
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
#include <QSpinBox>
#include <QVBoxLayout>
#include <QVector3D>
#include <QWheelEvent>
#include <QMenu>
#include <QToolButton>

#include <boost/concept_check.hpp>

#include <QVTKWidget.h>
#include <itkImageToVTKImageFilter.h>
#include <vtkAbstractWidget.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellPicker.h>
#include <vtkCommand.h>
#include <vtkCoordinate.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkImageProperty.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkInteractorStyleImage.h>
#include <vtkLookupTable.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp3DCollection.h>
#include <vtkPropCollection.h>
#include <vtkPropPicker.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkWidgetRepresentation.h>
#include <vtkWorldPointPicker.h>
#include <vtkImageShiftScale.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
// SLICE VIEW
//-----------------------------------------------------------------------------
SliceView::SliceView(ViewManager* vm, PlaneType plane, QWidget* parent)
: EspinaRenderView(parent)
, m_viewManager(vm)
, m_titleLayout(new QHBoxLayout())
, m_title(new QLabel("Sagital"))
, m_mainLayout(new QVBoxLayout())
, m_controlLayout(new QHBoxLayout())
, m_fromLayout(new QHBoxLayout())
, m_toLayout(new QHBoxLayout())
, m_view(new QVTKWidget())
, m_scrollBar(new QScrollBar(Qt::Horizontal))
, m_spinBox(new QSpinBox())
, m_zoomButton(new QPushButton())
, m_ruler(vtkSmartPointer<vtkAxisActor2D>::New())
, m_selectionEnabled(true)
, m_showSegmentations(true)
, m_showThumbnail(true)
, m_sliceSelector(QPair<SliceSelectorWidget*,SliceSelectorWidget*>(NULL, NULL))
, m_inThumbnail(false)
, m_sceneReady(false)
, m_highlighter(new TransparencySelectionHighlighter())
{
  memset(m_crosshairPoint, 0, 3*sizeof(Nm));
  m_plane = plane;
  m_settings = SettingsPtr(new Settings(m_plane));
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
  m_renderWindow = m_view->GetRenderWindow();
//   m_renderWindow->AlphaBitPlanesOn();
  m_renderWindow->DoubleBufferOn();
  m_renderWindow->SetNumberOfLayers(2);

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

  // Init Pickers
  m_channelPicker = vtkSmartPointer<vtkCellPicker>::New();
  m_channelPicker->PickFromListOn();
  m_segmentationPicker = vtkSmartPointer<vtkCellPicker>::New();
  m_segmentationPicker->PickFromListOn();

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
  m_renderWindow->AddRenderer(m_renderer);
  m_renderWindow->AddRenderer(m_thumbnail);
  m_view->GetInteractor()->SetInteractorStyle(interactor);

  m_channelBorderData = vtkPolyData::New(); // leak
  m_channelBorder     = vtkActor::New(); // leak
  initBorder(m_channelBorderData, m_channelBorder);

  m_viewportBorderData = vtkPolyData::New(); // leak
  m_viewportBorder     = vtkActor::New(); // leak
  initBorder(m_viewportBorderData, m_viewportBorder);

  buildCrosshairs();

  this->setAutoFillBackground(true);
  setLayout(m_mainLayout);

  connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection, bool)),
          this, SLOT(updateSelection(ViewManager::Selection, bool)));
}

//-----------------------------------------------------------------------------
SliceView::~SliceView()
{
  qDebug() << "********************************************************";
  qDebug() << "              Destroying Slice View" << m_plane;
  qDebug() << "********************************************************";
  m_viewManager->unregisterView(this);

  m_slicingMatrix->Delete();
  delete m_highlighter;
  delete m_state;
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
  if (!m_renderer || !m_renderWindow)
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
  m_ruler->SetVisibility(m_rulerVisibility && 0.02 < rulerLength && rulerLength < 0.8);
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
  m_spinBox->setMinimumWidth(40);
  m_spinBox->setMaximumHeight(20);
  m_spinBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  m_spinBox->setAlignment(Qt::AlignRight);

  connect(m_scrollBar, SIGNAL(valueChanged(int)), m_spinBox,   SLOT(setValue(int)));
  connect(m_scrollBar, SIGNAL(valueChanged(int)), this,        SLOT(scrollValueChanged(int)));
  connect(m_spinBox,   SIGNAL(valueChanged(int)), m_scrollBar, SLOT(setValue(int)));

  //   connect(SelectionManager::instance(),SIGNAL(VOIChanged(IVOI*)),this,SLOT(setVOI(IVOI*)));
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
void SliceView::setCursor(const QCursor &cursor)
{
  m_view->setCursor(cursor);
}

//-----------------------------------------------------------------------------
IPicker::PickList SliceView::pick(IPicker::PickableItems filter,
                                  IPicker::DisplayRegionList regions)
{
  bool multiSelection = false;
  IPicker::PickList pickedItems;

  vtkRenderer *renderer = m_renderer;
  Q_ASSERT(renderer);

  // Select all products that belongs to all regions
  // NOTE: Should first loop be removed? Only useful to select disconnected regions...
  foreach(const IPicker::DisplayRegion &region, regions)
  {
    QList<vtkProp *> pickedChannels;
    QList<vtkProp *> pickedSegmentations;
    foreach(QPointF p, region)
    {
      foreach(IPicker::Tag tag, filter)
      {
        if (IPicker::CHANNEL == tag)
        {
          foreach(ChannelPtr channel, pickChannels(p.x(), p.y(), renderer, multiSelection))
          {
            IPicker::WorldRegion wRegion = worldRegion(region, channel);
            pickedItems << IPicker::PickedItem(wRegion, channel);
            // remove it from picking list to prevent other points of the region
            // to select it again
            vtkProp *channelProp = m_channelReps[channel].slice;
            m_channelPicker->DeletePickList(channelProp);
            pickedChannels << channelProp;
          }
        } else if (IPicker::SEGMENTATION == tag)
        {
            foreach(SegmentationPtr seg, pickSegmentations(p.x(), p.y(), renderer, multiSelection))
            {
              IPicker::WorldRegion wRegion = worldRegion(region, seg);
              pickedItems << IPicker::PickedItem(wRegion, seg);
            // remove it from picking list to prevent other points of the region
            // to select it again
            vtkProp *segProp = m_segmentationReps[seg].slice;
            m_segmentationPicker->DeletePickList(segProp);
            pickedSegmentations << segProp;
            }
        } else
          Q_ASSERT(false);
      }
    }
    // Restore picked items to picker's pick lists
    foreach(vtkProp *channel, pickedChannels)
      m_channelPicker->AddPickList(channel);
    foreach(vtkProp *seg, pickedSegmentations)
      m_segmentationPicker->AddPickList(seg);
  }

  return pickedItems;
}

//-----------------------------------------------------------------------------
void SliceView::worldCoordinates(const QPoint& displayPos,
                                 double worldPos[3])
{
  double LL[3], UR[3];
  int viewSize[2];
  memcpy(viewSize, m_renderWindow->GetSize(), 2*sizeof(int));

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
  m_thumbnail->RemoveActor(this->m_viewportBorder);
  this->updateThumbnail();
  m_thumbnail->ResetCamera();
  m_thumbnail->AddActor(m_channelBorder);
  m_thumbnail->AddActor(this->m_viewportBorder);

  m_sceneReady = !m_channelReps.isEmpty();
}

//-----------------------------------------------------------------------------
void SliceView::addChannel(ChannelPtr channel)
{
  Q_ASSERT(!m_channelReps.contains(channel));

  SliceRep channelRep;
  double hue, sat, opacity;

  // if hue is -1 then use 0 saturation to make a grayscale image
  if (-1.0 == channel->hue())
  {
    sat = hue = 0;
  }
  else
  {
    hue = channel->hue();
    sat = channel->saturation();
  }

  channelRep.selected = false;
  channelRep.visible = !channel->isVisible();  // Force initialization
  channelRep.color.setHsvF(hue, sat, 1.0);
  channelRep.brightness = channel->brightness();
  channelRep.contrast = channel->contrast();
  channel->position(channelRep.pos);
  channelRep.lut = vtkLookupTable::New();
  channelRep.lut->Allocate();
  channelRep.lut->SetTableRange(0,255);
  channelRep.lut->SetHueRange(hue, hue);
  channelRep.lut->SetSaturationRange(0.0, sat);
  channelRep.lut->SetValueRange(0.0, 1.0);
  channelRep.lut->SetAlphaRange(1.0,1.0);
  channelRep.lut->SetNumberOfColors(256);
  channelRep.lut->SetRampToLinear();
  channelRep.lut->Build();

  channelRep.reslice = vtkImageReslice::New();
  channelRep.reslice->SetResliceAxes(m_slicingMatrix);
  channelRep.reslice->SetInputConnection(channel->volume()->toVTK());
  channelRep.reslice->SetOutputDimensionality(2);
  channelRep.reslice->Update();

  channelRep.shiftScaleFilter = vtkSmartPointer<vtkImageShiftScale>::New();
  channelRep.shiftScaleFilter->SetInputConnection(channelRep.reslice->GetOutputPort());
  channelRep.shiftScaleFilter->SetShift(channelRep.brightness);
  channelRep.shiftScaleFilter->SetScale(channelRep.contrast);
  channelRep.shiftScaleFilter->SetClampOverflow(true);
  channelRep.shiftScaleFilter->SetOutputScalarType(channelRep.reslice->GetOutput()->GetScalarType());
  channelRep.shiftScaleFilter->Update();

  channelRep.mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
  channelRep.mapToColors->SetInputConnection(channelRep.shiftScaleFilter->GetOutputPort());
  channelRep.mapToColors->SetLookupTable(channelRep.lut);
  channelRep.mapToColors->Update();

  channelRep.slice = vtkImageActor::New();
  channelRep.slice->SetInterpolate(false);
  channelRep.slice->GetMapper()->BorderOn();
  channelRep.slice->GetMapper()->SetInputConnection(channelRep.mapToColors->GetOutputPort());
  m_state->updateActor(channelRep.slice);
  channelRep.slice->Update();

  m_channelReps.insert(channel, channelRep);
  addActor(channelRep.slice);

  opacity = (-1 == channel->opacity()) ? this->suggestedChannelOpacity() : channel->opacity();
  channelRep.slice->GetProperty()->SetOpacity(opacity);

  // Prevent displaying channel's corner until app request to reset the camera
  if (m_channelReps.size() == 1)
    resetCamera();

  m_channelPicker->AddPickList(channelRep.slice);
  connect(channel, SIGNAL(modified(ModelItem*)),
          this, SLOT(updateSceneBounds()));

  addChannelBounds(channel);
}

//-----------------------------------------------------------------------------
void SliceView::removeChannel(ChannelPtr channel)
{
  Q_ASSERT(m_channelReps.contains(channel));

  SliceRep rep = m_channelReps[channel];
  removeActor(rep.slice);
  m_channelPicker->DeletePickList(rep.slice);

  m_channelReps.remove(channel);
  rep.reslice->Delete();
  rep.slice->Delete();
  removeChannelBounds(channel);

  // if there is more than one we must update the visibility
  double opacity = this->suggestedChannelOpacity();
  QMap<Channel *, SliceRep>::iterator it = m_channelReps.begin();
  while (it != m_channelReps.end())
  {
    if (it.value().visible)
    {
      it.value().slice->GetProperty()->SetOpacity(opacity);
      it.key()->notifyModification();
    }

    ++it;
  }
}

//-----------------------------------------------------------------------------
bool SliceView::updateChannel(ChannelPtr channel)
{
  Q_ASSERT(m_channelReps.contains(channel));
  SliceRep &rep = m_channelReps[channel];

  double pos[3];
  channel->position(pos);

  bool updated = false;

  // opacity
  if ((channel->opacity() == -1.0) && (rep.slice->GetProperty()->GetOpacity() != suggestedChannelOpacity()))
  {
    double opacity = this->suggestedChannelOpacity();
    rep.slice->GetProperty()->SetOpacity(opacity);

    // we must update all channels
    QMap<Channel *, SliceRep>::iterator it = m_channelReps.begin();
    while (it != m_channelReps.end())
    {
      if (it.key() != channel && it.key()->opacity() != -1 && it.value().visible)
      {
        it.value().slice->GetProperty()->SetOpacity(opacity);
        it.key()->notifyModification();
      }

      ++it;
    }

    updated = true;
  }
  else
    if ((channel->opacity() != -1.0) && (channel->opacity() != rep.slice->GetProperty()->GetOpacity()))
    {
      rep.slice->GetProperty()->SetOpacity(channel->opacity());
      updated = true;
    }

  // visibility
  if (channel->isVisible() != rep.visible)
  {
    rep.visible = channel->isVisible();
    rep.slice->SetVisibility(rep.visible);
    updated = true;
  }

  // position
  if (memcmp(pos, rep.pos, 3 * sizeof(double)))
  {
    memcpy(rep.pos, pos, 3 * sizeof(double));
    rep.slice->SetPosition(rep.pos);
    updated = true;
  }

  // hue/saturation
  if (((channel->hue() != -1) && ((rep.color.hueF() != channel->hue()) || (rep.color.saturation() != channel->saturation()))) ||
     ((channel->hue() == -1) && ((rep.color.hue() != 0.0) || (rep.color.saturation() != 0.0))))
  {
    double hue, sat;
    if (-1.0 == channel->hue())
    {
      sat = hue = 0;
    }
    else
    {
      hue = channel->hue();
      sat = channel->saturation();
    }

    rep.color.setHsvF(hue, sat, 1.0);

    rep.lut->Allocate();
    rep.lut->SetTableRange(0, 255);
    rep.lut->SetHueRange(hue, hue);
    rep.lut->SetSaturationRange(0.0, sat);
    rep.lut->SetValueRange(0.0, 1.0);
    rep.lut->SetAlphaRange(1.0, 1.0);
    rep.lut->SetNumberOfColors(256);
    rep.lut->Build();
    rep.lut->Modified();
    updated = true;
  }

  // brightness/contrast (together to avoid calling modified() twice to the filter)
  if ((channel->contrast() != rep.contrast) || (channel->brightness() != rep.brightness))
  {
    if (channel->contrast() != rep.contrast)
    {
      rep.contrast = channel->contrast();
      rep.shiftScaleFilter->SetScale(channel->contrast());
    }

    if (channel->brightness() != rep.brightness)
    {
      rep.brightness = channel->brightness();
      rep.shiftScaleFilter->SetShift(channel->brightness());
    }
    rep.shiftScaleFilter->Modified();
    updated = true;
  }

  return updated;
}

//-----------------------------------------------------------------------------
void SliceView::addSegmentation(SegmentationPtr seg)
{
  Q_ASSERT(!m_segmentationReps.contains(seg));

  SliceRep segRep;

  seg->filter()->update();

  segRep.reslice = vtkImageReslice::New();
  segRep.reslice->SetResliceAxes(m_slicingMatrix);
  segRep.reslice->SetInputConnection(seg->volume()->toVTK());
  segRep.reslice->SetOutputDimensionality(2);
  segRep.reslice->Update();

  segRep.shiftScaleFilter = NULL;

  segRep.mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
  segRep.mapToColors->SetInputConnection(segRep.reslice->GetOutputPort());
  segRep.mapToColors->SetLookupTable(m_viewManager->lut(seg));
  segRep.mapToColors->Update();

  segRep.slice = vtkImageActor::New();
  segRep.slice->SetInterpolate(false);
  segRep.slice->GetMapper()->BorderOn();
  segRep.slice->GetMapper()->SetInputConnection(segRep.mapToColors->GetOutputPort());
  segRep.slice->Update();
  m_state->updateActor(segRep.slice);

  segRep.selected = seg->isSelected();
  segRep.visible = seg->visible() && m_showSegmentations;
  segRep.color = m_viewManager->color(seg);

  m_segmentationReps.insert(seg, segRep);
  addActor(segRep.slice);
  m_segmentationPicker->AddPickList(segRep.slice);

  // need to reposition the actor so it will always be over the channels actors'
  double pos[3];
  segRep.slice->GetPosition(pos);
  pos[m_plane] = (m_plane == AXIAL) ? -0.05 : 0.05;
  segRep.slice->SetPosition(pos);
  segRep.overridden = seg->OverridesRendering();
  segRep.renderingType = seg->getHierarchyRenderingType();

  if (segRep.overridden)
  {
    switch(segRep.renderingType)
    {
      case HierarchyItem::Opaque:
        segRep.slice->GetProperty()->SetOpacity(1.0);
        if (!segRep.visible)
        {
          segRep.visible = true;
          segRep.slice->SetVisibility(true);
        }
        break;
      case HierarchyItem::Translucent:
        segRep.slice->GetProperty()->SetOpacity(0.3);
        if (!segRep.visible)
        {
          segRep.visible = true;
          segRep.slice->SetVisibility(true);
        }
        break;
      case HierarchyItem::Hidden:
        if (segRep.visible)
        {
          segRep.visible = false;
          segRep.slice->SetVisibility(false);
        }
        break;
      case HierarchyItem::Undefined:
        break;
      default:
        Q_ASSERT(false);
        break;
    }
  }
}

//-----------------------------------------------------------------------------
void SliceView::removeSegmentation(SegmentationPtr seg)
{
  Q_ASSERT(m_segmentationReps.contains(seg));

  SliceRep rep = m_segmentationReps[seg];

  removeActor(rep.slice);
  m_segmentationPicker->DeletePickList(rep.slice);

  // itkvtk filter is handled by a smartpointer, these two are not
  rep.reslice->Delete();
  rep.slice->Delete();

  m_segmentationReps.remove(seg);
}

//-----------------------------------------------------------------------------
bool SliceView::updateSegmentation(SegmentationPtr seg)
{
  if (!m_segmentationReps.contains(seg))
    return false;

  SliceRep &rep = m_segmentationReps[seg];

  bool updated = false;

  if (rep.reslice->GetInputConnection(0,0) != seg->volume()->toVTK())
  {
    rep.reslice->SetInputConnection(seg->volume()->toVTK());
    rep.reslice->Update();
    updated = true;
  }

  if (rep.visible != (seg->visible() && m_showSegmentations))
  {
    rep.visible = seg->visible() && m_showSegmentations;
    rep.slice->SetVisibility(rep.visible && m_showSegmentations);
    updated = true;
  }

  if (rep.visible)
  {
    QColor segColor =  m_viewManager->color(seg);
    bool highlight = seg->isSelected();
    QColor highlightedColor = m_highlighter->color(segColor, highlight);

    if ((seg->isSelected() != rep.selected)
      || (highlightedColor != rep.color)
      || seg->updateForced())
    {
      rep.selected = seg->isSelected();
      rep.color = highlightedColor;

      rep.mapToColors->SetLookupTable(m_highlighter->lut(segColor, highlight));
      rep.mapToColors->Update();
      updated = true;
    }
  }

  if (seg->OverridesRendering())
  {
    switch(rep.renderingType)
    {
      case HierarchyItem::Opaque:
        rep.slice->GetProperty()->SetOpacity(1.0);
        if (!rep.visible)
        {
          rep.visible = true;
          rep.slice->SetVisibility(true);
        }
        break;
      case HierarchyItem::Translucent:
        rep.slice->GetProperty()->SetOpacity(0.3);
        if (!rep.visible)
        {
          rep.visible = true;
          rep.slice->SetVisibility(true);
        }
        break;
      case HierarchyItem::Hidden:
        if (rep.visible)
        {
          rep.visible = false;
          rep.slice->SetVisibility(false);
        }
        break;
      case HierarchyItem::Undefined:
        break;
      default:
        Q_ASSERT(false);
        break;
    }
  }
  else
  {
    if (rep.overridden)
      rep.slice->GetProperty()->SetOpacity(1.0);
  }

  updated |= ((seg->OverridesRendering() != rep.overridden) ||
              (seg->getHierarchyRenderingType() != rep.renderingType));

  rep.overridden = seg->OverridesRendering();
  rep.renderingType = seg->getHierarchyRenderingType();

  return updated;
}

//-----------------------------------------------------------------------------
void SliceView::addWidget(EspinaWidget *eWidget)
{
  Q_ASSERT(!m_widgets.contains(eWidget));

  SliceWidget *sWidget = eWidget->createSliceWidget(m_plane);
  if (!sWidget)
    return;

  sWidget->setSlice(slicingPosition(), m_plane);
  vtkAbstractWidget *widget = *sWidget;
  widget->SetInteractor(m_renderWindow->GetInteractor());
  widget->GetRepresentation()->SetVisibility(true);
  widget->On();
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
  m_widgets.remove(eWidget);
}

//-----------------------------------------------------------------------------
void SliceView::addPreview(vtkProp3D *preview)
{
  m_renderer->AddActor(preview);
  m_state->updateActor(preview);
}

//-----------------------------------------------------------------------------
void SliceView::removePreview(vtkProp3D *preview)
{
  m_renderer->RemoveActor(preview);
}

//-----------------------------------------------------------------------------
void SliceView::previewBounds(Nm bounds[6])
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

  bounds[2*H]         = std::max(LL[H], m_sceneBounds[2*H]);
  bounds[2*H+1]       = std::min(UR[H], m_sceneBounds[2*H+1]);
  bounds[2*V]         = std::max(UR[V], m_sceneBounds[2*V]);
  bounds[2*V+1]       = std::min(LL[V], m_sceneBounds[2*V+1]);
  bounds[2*m_plane]   = slicingPosition();
  bounds[2*m_plane+1] = slicingPosition();
}

//-----------------------------------------------------------------------------
void SliceView::addActor(vtkProp* actor)
{
  m_renderer->AddActor(actor);
  m_thumbnail->AddActor(actor);

  m_thumbnail->RemoveActor(m_channelBorder);
  m_thumbnail->RemoveActor(this->m_viewportBorder);
  this->updateThumbnail();
  m_thumbnail->ResetCamera();
  this->updateThumbnail();
  m_thumbnail->AddActor(m_channelBorder);
  m_thumbnail->AddActor(this->m_viewportBorder);
}

//-----------------------------------------------------------------------------
void SliceView::removeActor(vtkProp* actor)
{
  m_renderer->RemoveActor(actor);
  m_thumbnail->RemoveActor(actor);
  this->updateThumbnail();
}

//-----------------------------------------------------------------------------
void SliceView::updateSelection(ViewManager::Selection selection, bool render)
{
  updateSegmentationRepresentations();
  if (render)
    updateView();
}

//-----------------------------------------------------------------------------
void SliceView::updateSegmentationRepresentations(SegmentationList list)
{
  if (isVisible())
  {
    SegmentationList updateSegmentations;

    if (list.empty())
      updateSegmentations = m_segmentationReps.keys();
    else
      updateSegmentations = list;

    foreach(SegmentationPtr seg, updateSegmentations)
      updateSegmentation(seg);
  }
}

//-----------------------------------------------------------------------------
void SliceView::updateChannelRepresentations(ChannelList list)
{
  if (isVisible())
  {
    ChannelList updateChannels;

    if (list.empty())
      updateChannels = m_channelReps.keys();
    else
      updateChannels = list;

    foreach(ChannelPtr channel, updateChannels)
      this->updateChannel(channel);
  }
}

//-----------------------------------------------------------------------------
vtkRenderWindow *SliceView::renderWindow()
{
  return m_renderWindow;
}

//-----------------------------------------------------------------------------
vtkRenderer* SliceView::mainRenderer()
{
  return m_renderer;
}


//-----------------------------------------------------------------------------
void SliceView::sliceViewCenterChanged(Nm x, Nm y, Nm z)
{
  //qDebug() << "Slice View: " << m_plane << " has new center";
  emit centerChanged(x, y, z);
}

//-----------------------------------------------------------------------------
void SliceView::scrollValueChanged(int value/*nm*/)
{
  m_state->setSlicingPosition(m_slicingMatrix, slicingPosition());
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
      m_spinBox->setValue(m_spinBox->value() - numSteps);
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
    m_inThumbnail = m_thumbnail->GetDraw() && m_channelPicker->Pick(x, y, 0.1, m_thumbnail);

  }
  else if (QEvent::ContextMenu == e->type())
  {
    QContextMenuEvent *cme = dynamic_cast<QContextMenuEvent*>(e);
    if (cme->modifiers() == Qt::CTRL && !m_contextMenu.isNull())
    {
      //m_contextMenu->exec(mapToGlobal(cme->pos()), m_viewManager->selectedSegmentations());
    }
  }
  else if (QEvent::ToolTip == e->type())
  {
    int x, y;
    eventPosition(x, y);
    SegmentationList segs = pickSegmentations(x, y, m_renderer);
    QString toopTip;
    foreach(SegmentationPtr seg, segs)
    {
      toopTip = toopTip.append("<b>%1</b><br>").arg(seg->data().toString());
      toopTip = toopTip.append(seg->data(Qt::ToolTipRole).toString());
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
    channelPicked = m_channelPicker->Pick(xPos, yPos, 0.1, m_thumbnail);
    if (channelPicked)
      m_channelPicker->GetPickPosition(center);
  }
  else
    channelPicked = pick(m_channelPicker, xPos, yPos, center);

  if (channelPicked)
  {
    centerViewOn(center);
    emit centerChanged(m_crosshairPoint[0], m_crosshairPoint[1], m_crosshairPoint[2]);
  }
}

//-----------------------------------------------------------------------------
void SliceView::centerViewOnMousePosition()
{
  int xPos, yPos;
  eventPosition(xPos, yPos);

  if (m_channelPicker->Pick(xPos, yPos, 0.1, m_thumbnail))
  {
    double center[3];  //World coordinates
    m_channelPicker->GetPickPosition(center);
    centerViewOnPosition(center);
  }
}

//-----------------------------------------------------------------------------
void SliceView::eventPosition(int& x, int& y)
{
  vtkRenderWindowInteractor *rwi = m_renderWindow->GetInteractor();
  Q_ASSERT(rwi);
  rwi->GetEventPosition(x, y);
}

//-----------------------------------------------------------------------------
ChannelList SliceView::pickChannels(double vx,
                                         double vy,
                                         vtkRenderer* renderer,
                                         bool repeatable)
{
  ChannelList channels;

  if (m_channelPicker->Pick(vx, vy, 0.1, renderer))
  {
    vtkProp3D *pickedProp;
    m_channelPicker->GetProp3Ds()->InitTraversal();
    while ((pickedProp = m_channelPicker->GetProp3Ds()->GetNextProp3D()))
    {
      ChannelPtr pickedChannel = property3DChannel(pickedProp);
      Q_ASSERT(pickedChannel);
      //       qDebug() << "Picked" << pickedChannel->data().toString();
      channels << pickedChannel;

      if (!repeatable)
        return channels;
    }
  }

  return channels;
}

//-----------------------------------------------------------------------------
SegmentationList SliceView::pickSegmentations(double vx,
                                                   double vy,
                                                   vtkRenderer* renderer,
                                                   bool repeatable)
{
  SegmentationList segmentations;
  if (m_segmentationPicker->Pick(vx, vy, 0.1, renderer))
  {
    QPolygonF selectedRegion;
    selectedRegion << QPointF(vx, vy);

    // Verify BUG: kills app when picking a node in TubularWidget in ZY view (not only that one?)
    m_segmentationPicker->GetProp3Ds()->InitTraversal();

    vtkProp3DCollection* props = m_segmentationPicker->GetProp3Ds();

    QList<vtkProp3D *> pickedProps;
    for(vtkIdType i = 0; i < props->GetNumberOfItems(); i++)
      pickedProps << props->GetNextProp3D();

    // We need to do it in two separate loops to avoid reseting picker on worldRegion call
    foreach(vtkProp3D *pickedProp, pickedProps)
    {
      SegmentationPtr pickedSeg = property3DSegmentation(pickedProp);
      Q_ASSERT(pickedSeg);
      Q_ASSERT(pickedSeg->volume().get());
      Q_ASSERT(pickedSeg->volume()->toITK().IsNotNull());

      //TODO 2012-10-23 Check all the region, not just the first point!
      double pixel[3];
      worldRegion(selectedRegion, pickedSeg)->GetPoint(0, pixel);
      itkVolumeType::IndexType pickedPixel = pickedSeg->volume()->index(pixel[0], pixel[1], pixel[2]);
      if (!pickedSeg->volume()->volumeRegion().IsInside(pickedPixel) ||
         ( pickedSeg->volume()->toITK()->GetPixel(pickedPixel) == 0))
        continue;

      segmentations << pickedSeg;

      if (!repeatable)
        return segmentations;
    }
  }

  return segmentations;
}

//-----------------------------------------------------------------------------
void SliceView::selectPickedItems(bool append)
{
  int vx, vy;
  eventPosition(vx, vy);

  ViewManager::Selection selection;
  if (append)
    selection = m_viewManager->selection();

  // If no append, segmentations have priority over channels
  foreach(SegmentationPtr seg, pickSegmentations(vx, vy, m_renderer, append))
  {
    if (selection.contains(seg))
      selection.removeAll(seg);
    else
      selection << seg;

    if (!append)
      break;
  }

  foreach(ChannelPtr channel, pickChannels(vx, vy, m_renderer, append))
  {
    selection << channel;
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
ChannelPtr SliceView::property3DChannel(vtkProp3D* prop)
{
  foreach(ChannelPtr channel, m_channelReps.keys())
  {
    if (m_channelReps[channel].slice == prop)
      return channel;
  }
  return ChannelPtr();
}

//-----------------------------------------------------------------------------
SegmentationPtr SliceView::property3DSegmentation(vtkProp3D* prop)
{
  foreach(SegmentationPtr seg, m_segmentationReps.keys())
  {
    if (m_segmentationReps[seg].slice == prop)
      return seg;
  }
  return SegmentationPtr();
}


//-----------------------------------------------------------------------------
bool SliceView::pick(vtkPicker *picker, int x, int y, Nm pickPos[3])
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
  foreach(SliceRep rep, m_segmentationReps)
  {
    rep.slice->SetVisibility(visible && rep.visible);
  }
  updateView();
}

//-----------------------------------------------------------------------------
void SliceView::setShowPreprocessing(bool visible)
{
  if (m_channelReps.size() < 2)
    return;

  ChannelPtr hiddenChannel = m_channelReps.keys()[visible];
  ChannelPtr visibleChannel = m_channelReps.keys()[1 - visible];
  hiddenChannel->setData(false, Qt::CheckStateRole);
  hiddenChannel->notifyModification();
  visibleChannel->setData(true, Qt::CheckStateRole);
  visibleChannel->notifyModification();
  for (int i = 2; i < m_channelReps.keys().size(); i++)
  {
    ChannelPtr otherChannel = m_channelReps.keys()[i];
    otherChannel->setData(false, Qt::CheckStateRole);
    otherChannel->notifyModification();
  }
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
void SliceView::setSlicingStep(Nm steps[3])
{
  if (steps[0] <= 0 || steps[1] <= 0 || steps[2] <= 0)
  {
    qFatal("SliceView: Invalid Step value. Slicing Step not changed");
    return;
  }

  Nm slicingPos = slicingPosition();

  if (AXIAL == m_plane)
  {
    memcpy(m_slicingStep, steps, 3*sizeof(Nm));
  }
  setSlicingBounds(m_sceneBounds);

  if (m_slicingStep[0] == 1 && m_slicingStep[1] == 1 && m_slicingStep[2] == 1)
    m_spinBox->setSuffix(" nm");
  else
    m_spinBox->setSuffix("");

  m_scrollBar->setValue(slicingPos/m_slicingStep[m_plane]);
}

//-----------------------------------------------------------------------------
Nm SliceView::slicingPosition() const
{
  return m_slicingStep[m_plane]*m_spinBox->value();
}


//-----------------------------------------------------------------------------
void SliceView::setSlicingBounds(Nm bounds[6])
{
  if (bounds[1] < bounds[0] || bounds[3] < bounds[2] || bounds[5] < bounds[4])
  {
    qFatal("SliceView: Invalid Slicing Ranges. Ranges not changed");
    return;
  }

  Nm min = bounds[2*m_plane] / m_slicingStep[m_plane];
  Nm max = bounds[2*m_plane + 1] / m_slicingStep[m_plane];

  m_scrollBar->setMinimum(static_cast<int>(min));
  m_scrollBar->setMaximum(static_cast<int>(max));
  m_spinBox->setMinimum(static_cast<int>(min));
  m_spinBox->setMaximum(static_cast<int>(max));

  //bool enabled = m_spinBox->minimum() < m_spinBox->maximum();
  //TODO 2012-11-14 m_fromSlice->setEnabled(enabled);
  //                m_toSlice->setEnabled(enabled);

  // update crosshair
  m_state->setCrossHairs(m_HCrossLineData, m_VCrossLineData,
                         m_crosshairPoint, m_sceneBounds);
}

//-----------------------------------------------------------------------------
void SliceView::centerViewOn(Nm center[3], bool force)
{
  if (!isVisible() ||
     (m_crosshairPoint[0] == center[0] &&
      m_crosshairPoint[1] == center[1] &&
      m_crosshairPoint[2] == center[2] &&
      !force))
    return;

  // Adjust crosshairs to fit slicing steps
  int sliceNumbers[3];
  for (int i = 0; i < 3; i++)
  {
    sliceNumbers[i] = center[i] / m_slicingStep[i];
    m_crosshairPoint[i] = floor((center[i]/m_slicingStep[i] + 0.5))*m_slicingStep[i];
  }

  // Disable scrollbox signals to avoid calling seting slice
  m_scrollBar->blockSignals(true);
  m_spinBox->setValue(sliceNumbers[m_plane]);
  m_scrollBar->setValue(sliceNumbers[m_plane]);
  m_scrollBar->blockSignals(false);

  m_state->setSlicingPosition(m_slicingMatrix, slicingPosition());
  m_state->setCrossHairs(m_HCrossLineData, m_VCrossLineData,
                         m_crosshairPoint, m_sceneBounds);

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
IPicker::WorldRegion SliceView::worldRegion(const IPicker::DisplayRegion& region,
                                            PickableItemPtr item)
{
  //Use Render Window Interactor's Picker to find the world coordinates of the stack
  //vtkSMRenderViewProxy* renModule = view->GetRenderWindow()->GetInteractor()->GetRenderView();
  IPicker::WorldRegion wRegion = IPicker::WorldRegion::New();
  vtkPicker *picker;

  if (EspINA::CHANNEL == item->type())
    picker = m_channelPicker;
  else
    picker = m_segmentationPicker;

  foreach(QPointF point, region)
  {
    double pickPos[3];  //World coordinates
    if (pick(picker, point.x(), point.y(), pickPos))
      wRegion->InsertNextPoint(pickPos);
  }

  return wRegion;
}

//-----------------------------------------------------------------------------
SliceView::Settings::Settings(PlaneType plane, const QString prefix)
: INVERT_SLICE_ORDER(prefix + view(plane) + "::invertSliceOrder")
, INVERT_WHEEL(prefix + view(plane) + "::invertWheel")
, SHOW_AXIS(prefix + view(plane) + "::showAxis")
, m_InvertWheel(false)
, m_InvertSliceOrder(false)
, m_ShowAxis(false)
, m_plane(plane)
{
  QSettings settings(CESVIMA, ESPINA);

  m_InvertSliceOrder = settings.value(INVERT_SLICE_ORDER, false).toBool();
  m_InvertWheel      = settings.value(INVERT_WHEEL, false).toBool();
  m_ShowAxis         = settings.value(SHOW_AXIS, false).toBool();
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
void SliceView::updateCrosshairPoint(PlaneType plane, Nm slicepos)
{

  this->m_crosshairPoint[plane] = slicepos;
  m_state->setCrossHairs(m_HCrossLineData, m_VCrossLineData,
                         m_crosshairPoint, m_sceneBounds);

  // render if present
  if (this->m_renderer->HasViewProp(this->m_HCrossLine))
    updateView();
}

//-----------------------------------------------------------------------------
void SliceView::resetView()
{
  resetCamera();
  updateView();
}
