/*

 Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "View2D.h"

#include <Core/Analysis/Channel.h>
#include <Core/Utils/BinaryMask.hxx>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/View/PlanarBehaviour.h>
#include <GUI/View/vtkInteractorStyleEspinaSlice.h>
#include <GUI/Representations/Frame.h>
#include <GUI/Dialogs/DefaultDialogs.h>

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
#include <QStyle>
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

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::Model::Utils;

//-----------------------------------------------------------------------------
// SLICE VIEW
//-----------------------------------------------------------------------------
View2D::View2D(GUI::View::ViewState &state, Plane plane, QWidget *parent)
: RenderView        {state, ViewType::VIEW_2D, parent}
, m_mainLayout      {new QVBoxLayout()}
, m_controlLayout   {new QHBoxLayout()}
, m_fromLayout      {new QHBoxLayout()}
, m_toLayout        {new QHBoxLayout()}
, m_scrollBar       {new QScrollBar(Qt::Horizontal)}
, m_spinBox         {new QDoubleSpinBox()}
, m_cameraReset     {nullptr}
, m_snapshot        {nullptr}
, m_showThumbnail   {true}
, m_inThumbnail     {false}
, m_inThumbnailClick{true}
, m_scaleValue      {1.0}
, m_scaleVisibility {true}
, m_scale           {vtkSmartPointer<vtkAxisActor2D>::New()}
, m_plane           {plane}
, m_normalCoord     {normalCoordinateIndex(plane)}
, m_invertWheel     {false}
, m_invertSliceOrder{false}
{
  setupUI();

  qRegisterMetaType<Plane>("Plane");
  qRegisterMetaType<Nm>("Nm");

  switch (m_plane)
  {
    case Plane::XY:
      m_state2D = std::unique_ptr<PlanarBehaviour>(new AxialBehaviour());
      break;
    case Plane::XZ:
      m_state2D = std::unique_ptr<PlanarBehaviour>(new CoronalBehaviour());
      break;
    case Plane::YZ:
      m_state2D = std::unique_ptr<PlanarBehaviour>(new SagittalBehaviour());
      break;
    default:
      break;
  };

  // Init Render Window
  auto renderWindow = m_view->GetRenderWindow();
  renderWindow->DoubleBufferOn();
  renderWindow->SetNumberOfLayers(2);

  // Init Renderers
  m_renderer = vtkSmartPointer<vtkRenderer>::New();
  m_renderer->GetActiveCamera()->ParallelProjectionOn();
  m_renderer->GetActiveCamera()->SetThickness(2000);
  m_renderer->SetNearClippingPlaneTolerance(0.001);
  m_renderer->LightFollowCameraOn();
  m_renderer->BackingStoreOff();
  m_renderer->SetLayer(0);

  m_thumbnail = vtkSmartPointer<vtkRenderer>::New();
  m_thumbnail->SetViewport(0.75, 0.0, 1.0, 0.25);
  m_thumbnail->BackingStoreOff();
  m_thumbnail->SetLayer(1);
  m_thumbnail->InteractiveOff();
  m_thumbnail->GetActiveCamera()->ParallelProjectionOn();
  m_thumbnail->DrawOff();

  // Init Ruler
  m_scale->SetPosition(0.1, 0.1);
  m_scale->SetPosition2(0.1, 0.1);
  m_scale->SetPickable(false);
  m_scale->SetLabelFactor(0.8);
  m_scale->SetFontFactor(1);
  m_scale->SetTitle("nm");
  m_scale->RulerModeOff();
  m_scale->SetLabelFormat("%.0f");
  m_scale->SetAdjustLabels(false);
  m_scale->SetNumberOfLabels(2);
  m_scale->SizeFontRelativeToAxisOff();
  m_renderer->AddViewProp(m_scale);

  auto interactor = View2DInteractor::New();
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

  this->setAutoFillBackground(true);
  this->setLayout(m_mainLayout);
  this->setFocusPolicy(Qt::WheelFocus);

  setScaleVisibility(false);
  setThumbnailVisibility(false);
}

//-----------------------------------------------------------------------------
View2D::~View2D()
{
  //   qDebug() << "********************************************************";
  //   qDebug() << "              Destroying Slice View" << m_plane;
  //   qDebug() << "********************************************************";
  // Representation destructors may need to access slice view in their destructors
  m_renderer->RemoveAllViewProps();
  m_thumbnail->RemoveAllViewProps();

  m_state2D = nullptr;
}

//-----------------------------------------------------------------------------
void View2D::setInvertSliceOrder(bool value)
{
  m_invertSliceOrder = value;
}

//-----------------------------------------------------------------------------
vtkRenderer *View2D::mainRenderer() const
{
  return m_renderer;
}

//-----------------------------------------------------------------------------
void View2D::showEvent(QShowEvent *event)
{
  onRenderRequest();
}

//-----------------------------------------------------------------------------
Nm scaleResolution(Nm value)
{
  int factor = 100;

  if (value < 10)
  {
    factor = 1;
  }
  else if (value < 25)
  {
    factor = 5;
  }
  else if (value < 100)
  {
    factor = 10;
  }
  else if (value < 250)
  {
    factor = 50;
  }

  int res = int(value/factor)*factor;

  return std::max(res,1);
}

//-----------------------------------------------------------------------------
void View2D::updateScale()
{
  if (!m_renderer || !m_view->GetRenderWindow()) return;

  double *value;
  Nm left, right;

  int index = (m_plane == Plane::YZ) ? 2 : 0;

  auto coords = vtkSmartPointer<vtkCoordinate>::New();
  coords->SetCoordinateSystemToNormalizedViewport();

  coords->SetValue(0, 0); //Viewport Lower Left Corner
  value = coords->GetComputedWorldValue(m_renderer);
  left = value[index];

  coords->SetValue(1, 0); // Viewport Lower Right Corner
  value = coords->GetComputedWorldValue(m_renderer);
  right = value[index];

  Nm rulerLength = 0.07;//viewport coordinates - Configuration file
  Nm viewWidth = fabs(left-right);

  Nm scale = rulerLength * viewWidth;
  scale = scaleResolution(scale);
  rulerLength = scale / viewWidth;

  m_scale->SetRange(0, scale);
  m_scale->SetPosition2(0.1+rulerLength, 0.1);
  m_scale->SetVisibility(sceneBounds().areValid() && m_scaleVisibility && (0.02 < rulerLength) && (rulerLength < 0.8));
}

//-----------------------------------------------------------------------------
void View2D::updateThumbnail()
{
  if (m_showThumbnail)
  {
    double *value;
    // Position of world margins acording to the display
    // Depending on the plane being shown can refer to different
    // bound components
    double viewLeft, viewRight, viewUpper, viewLower;

    auto coords = vtkSmartPointer<vtkCoordinate>::New();
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

    auto bounds = sceneBounds();
    double sceneLeft  = bounds[2*h];
    double sceneRight = bounds[2*h+1];
    double sceneLower = bounds[2*v];
    double sceneUpper = bounds[2*v+1];

    bool isLeftHidden  = isOutsideLimits(sceneLeft, viewLeft, viewRight);
    bool isRightHidden = isOutsideLimits(sceneRight, viewLeft, viewRight);
    bool isUpperHidden = isOutsideLimits(sceneUpper, viewUpper, viewLower);
    bool isLowerHidden = isOutsideLimits(sceneLower, viewUpper, viewLower);

    if (bounds.areValid() && (isLeftHidden || isRightHidden || isUpperHidden || isLowerHidden))
    {
      m_thumbnail->DrawOn();
      updateBorder(m_viewportBorderData, viewLeft, viewRight, viewUpper, viewLower);
      m_thumbnail->ResetCameraClippingRange();
    }
    else
    {
      m_thumbnail->DrawOff();
    }
  }
}

//-----------------------------------------------------------------------------
void View2D::initBorders(vtkPolyData* data, vtkActor* actor)
{
  double unusedPoint[3]{0,0,0};
  auto corners = vtkPoints::New();
  corners->SetNumberOfPoints(4);
  corners->InsertNextPoint(unusedPoint);
  corners->InsertNextPoint(unusedPoint);
  corners->InsertNextPoint(unusedPoint);
  corners->InsertNextPoint(unusedPoint);

  auto borders = vtkCellArray::New();
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

  corners->Delete();
  borders->Delete();

  auto Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  Mapper->SetInputData(data);
  actor->SetMapper(Mapper);
  actor->GetProperty()->SetLineWidth(2);
  actor->SetPickable(false);
}

//-----------------------------------------------------------------------------
void View2D::updateBorder(vtkPolyData* data, Nm left, Nm right, Nm upper, Nm lower)
{
  auto corners = data->GetPoints();

  Nm   zShift;
  auto bounds = sceneBounds();
  switch(m_plane)
  {
    case Plane::XY:
      zShift = bounds[4] + widgetDepth();
      corners->SetPoint(0, left,  upper, zShift); //UL
      corners->SetPoint(1, right, upper, zShift); //UR
      corners->SetPoint(2, right, lower, zShift); //LR
      corners->SetPoint(3, left,  lower, zShift); //LL
      break;
    case Plane::XZ:
      zShift = bounds[3] + widgetDepth();
      corners->SetPoint(0, left,  zShift, upper); //UL
      corners->SetPoint(1, right, zShift, upper); //UR
      corners->SetPoint(2, right, zShift, lower); //LR
      corners->SetPoint(3, left,  zShift, lower); //LL
      break;
    case Plane::YZ:
      zShift = bounds[1] + widgetDepth();
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
void View2D::setupUI()
{
  m_view->installEventFilter(this);

  m_cameraReset = createButton(":/espina/reset_view.svg", tr("Reset View"), this);
  connect(m_cameraReset, SIGNAL(clicked()),
          this,          SLOT(resetCamera()));

  m_snapshot = createButton(":/espina/snapshot_scene.svg", tr("Save Scene as Image"), this);
  connect(m_snapshot, SIGNAL(clicked(bool)),
          this,       SLOT(onTakeSnapshot()));

  m_scrollBar->setMaximum(0);
  m_scrollBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  m_spinBox->setMaximum(0);
  m_spinBox->setDecimals(0);
  m_spinBox->setMinimumWidth(40);
  m_spinBox->setMaximumHeight(20);
  m_spinBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  m_spinBox->setAlignment(Qt::AlignRight);
  m_spinBox->setSingleStep(1);

  connect(m_spinBox,   SIGNAL(valueChanged(double)),
          this,        SLOT(spinValueChanged(double)));

  connect(m_scrollBar, SIGNAL(valueChanged(int)),
          this,        SLOT(scrollValueChanged(int)));

  m_mainLayout   ->addWidget(m_view);
  m_controlLayout->addWidget(m_cameraReset);
  m_controlLayout->addWidget(m_snapshot);
  m_controlLayout->addWidget(m_scrollBar);
  m_controlLayout->addLayout(m_fromLayout);
  m_controlLayout->addWidget(m_spinBox);
  m_controlLayout->addLayout(m_toLayout);

  m_mainLayout->addLayout(m_controlLayout);

  // Color background
  auto pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Window));
  this->setPalette(pal);
  this->setStyleSheet("QSpinBox { background-color: white;}");
}

//-----------------------------------------------------------------------------
double View2D::segmentationDepth() const
{
  auto segmentationShift = 0.05 * this->sceneResolution()[normalCoordinateIndex(m_plane)];
  return Plane::XY == m_plane ? -segmentationShift : segmentationShift;
}

//-----------------------------------------------------------------------------
double View2D::widgetDepth() const
{
  auto widgetShift = 0.15 * this->sceneResolution()[normalCoordinateIndex(m_plane)];
  return Plane::XY == m_plane ? -widgetShift : widgetShift;
}

//-----------------------------------------------------------------------------
Nm View2D::slicingPosition() const
{
  return crosshair()[m_normalCoord];
}

//-----------------------------------------------------------------------------
void View2D::setThumbnailVisibility(bool visible)
{
  if(m_showThumbnail != visible)
  {
    m_showThumbnail = visible;

    m_thumbnail->SetDraw(visible);

    refresh();
  }
}

//-----------------------------------------------------------------------------
void View2D::addActor(vtkProp* actor)
{
  m_renderer->AddViewProp(actor);
  m_thumbnail->AddViewProp(actor);

  m_thumbnail->RemoveViewProp(m_channelBorder);
  m_thumbnail->RemoveViewProp(m_viewportBorder);

  m_thumbnail->AddViewProp(m_channelBorder);
  m_thumbnail->AddViewProp(m_viewportBorder);
}

//-----------------------------------------------------------------------------
void View2D::removeActor(vtkProp* actor)
{
  m_renderer->RemoveActor(actor);
  m_thumbnail->RemoveActor(actor);

  //updateThumbnail();
}

//-----------------------------------------------------------------------------
void View2D::updateViewActions(RepresentationManager::ManagerFlags flags)
{
  auto hasActors = flags.testFlag(RepresentationManager::HAS_ACTORS);

  m_cameraReset->setEnabled(hasActors);
  m_snapshot->setEnabled(hasActors);
}

//-----------------------------------------------------------------------------
void View2D::resetCameraImplementation()
{
  auto origin = state().coordinateSystem()->origin();

  m_state2D->updateCamera(m_renderer ->GetActiveCamera(), origin);
  m_state2D->updateCamera(m_thumbnail->GetActiveCamera(), origin);

  m_thumbnail->RemoveViewProp(m_channelBorder);
  m_thumbnail->RemoveViewProp(m_viewportBorder);

  updateThumbnail();

  m_renderer->ResetCamera();
  m_thumbnail->ResetCamera();
  m_thumbnail->AddViewProp(m_channelBorder);
  m_thumbnail->AddViewProp(m_viewportBorder);
}

//-----------------------------------------------------------------------------
bool View2D::isCrosshairPointVisible() const
{
  // Only center camera if center is out of the display view
  auto coords = vtkSmartPointer<vtkCoordinate>::New();
  coords->SetViewport(m_renderer);
  coords->SetCoordinateSystemToNormalizedViewport();

  double ll[3], ur[3];
  coords->SetValue(0, 0); //LL
  memcpy(ll,coords->GetComputedWorldValue(m_renderer),3*sizeof(double));
  coords->SetValue(1, 1); //UR
  memcpy(ur,coords->GetComputedWorldValue(m_renderer),3*sizeof(double));

  int H = (Plane::YZ == m_plane)?2:0;
  int V = (Plane::XZ == m_plane)?2:1;

  auto current = crosshair();

  return !(isOutsideLimits(current[H], ll[H], ur[H]) || isOutsideLimits(current[V], ll[V], ur[V]));
}

//-----------------------------------------------------------------------------
void View2D::refreshViewImplementation()
{
  updateScale();
  updateThumbnail();
  updateScaleValue();
}

//-----------------------------------------------------------------------------
Bounds View2D::previewBounds(bool cropToSceneBounds) const
{
  // Display Orientation (up means up according to screen)
  // but in vtk coordinates UR[V] < LL[V]
  double LL[3], UR[3];
  // Display bounds in world coordinates
  auto coords = vtkSmartPointer<vtkCoordinate>::New();
  coords->SetViewport(m_renderer);
  coords->SetCoordinateSystemToNormalizedViewport();
  coords->SetValue(0, 0); //LL
  memcpy(LL, coords->GetComputedWorldValue(m_renderer),3*sizeof(double));
  coords->SetValue(1, 1); //UR
  memcpy(UR, coords->GetComputedWorldValue(m_renderer),3*sizeof(double));

  int H = (Plane::YZ == m_plane)?2:0;
  int V = (Plane::XZ == m_plane)?2:1;

  Bounds bounds;
  bounds[2*H]   = std::min(LL[H], UR[H]);
  bounds[2*H+1] = std::max(LL[H], UR[H]);
  bounds[2*V]   = std::min(UR[V], LL[V]);
  bounds[2*V+1] = std::max(UR[V], LL[V]);

  bounds[2*m_normalCoord+1] = bounds[2*m_normalCoord] = slicingPosition();

  if (cropToSceneBounds)
  {
    auto sBounds = sceneBounds();

    bounds[2*H]   = std::max(bounds[2*H]  , sBounds[2*H]);
    bounds[2*H+1] = std::min(bounds[2*H+1], sBounds[2*H+1]);
    bounds[2*V]   = std::max(bounds[2*V]  , sBounds[2*V]);
    bounds[2*V+1] = std::min(bounds[2*V+1], sBounds[2*V+1]);
  }
  bounds.setUpperInclusion(true);

  return bounds;
}

//-----------------------------------------------------------------------------
void View2D::scrollValueChanged(int value)
{
  auto position = (fitToSlices() ? voxelCenter(value, m_plane) : value);

  emit crosshairPlaneChanged(m_plane, position);
}

//-----------------------------------------------------------------------------
void View2D::spinValueChanged(double value /* nm or slices depending on m_fitToSlices */)
{
  int sliceIndex = fitToSlices()?(value - 1) : voxelSlice(value, m_plane);

  auto position = voxelCenter(sliceIndex, m_plane);

  emit crosshairPlaneChanged(m_plane, position);
}

//-----------------------------------------------------------------------------
void View2D::resetImplementation()
{
  m_thumbnail->DrawOff();
}

//-----------------------------------------------------------------------------
bool View2D::eventFilter(QObject* caller, QEvent* e)
{
  int xPos, yPos;

  eventPosition(xPos, yPos);

  m_inThumbnail = m_thumbnail && m_thumbnail->GetDraw() && m_thumbnail->PickProp(xPos, yPos);

  if (!m_inThumbnail && eventHandlerFilterEvent(e))
  {
    return true;
  }

  switch (e->type())
  {
    case QEvent::Resize:
      refresh();
      e->accept();
      break;
    case QEvent::Wheel:
      {
        auto we = static_cast<QWheelEvent *>(e);
        int numSteps = we->delta() / 8 / 15 * (m_invertWheel ? -1 : 1);  //Refer to QWheelEvent doc.
        m_scrollBar->setValue(m_scrollBar->value() - numSteps);
        e->ignore();
        this->setFocus(Qt::OtherFocusReason);
        return true;
      }
      break;
    case QEvent::Enter:
      QWidget::enterEvent(e);

      // get the focus this very moment
      setFocus(Qt::OtherFocusReason);

      if (m_inThumbnail)
      {
        m_view->setCursor(Qt::ArrowCursor);
      }
      else
      {
        onCursorChanged();
      }

      e->accept();
      break;
    case QEvent::Leave:
      m_inThumbnail = false;
      break;
    case QEvent::ContextMenu:
      {
        auto cme = dynamic_cast<QContextMenuEvent*>(e);
        if (cme->modifiers() == Qt::CTRL && m_contextMenu.get())
        {
          m_contextMenu->setSelection(currentSelection());
          m_contextMenu->exec(mapToGlobal(cme->pos()));
        }
      }
      break;
    case QEvent::ToolTip:
      showSegmentationTooltip(xPos, yPos);
      break;
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
      {
        auto me = static_cast<QMouseEvent*>(e);

        if (m_inThumbnail)
        {
          m_view->setCursor(Qt::ArrowCursor);

          if (((e->type() == QEvent::MouseButtonPress) && me->button() == Qt::LeftButton) || (e->type() == QEvent::MouseMove && m_inThumbnailClick))
          {
            m_inThumbnailClick = true;
            centerViewOnMousePosition(xPos, yPos);
          }
          else if ((e->type() == QEvent::MouseButtonRelease) && (me->button() == Qt::LeftButton))
          {
            m_inThumbnailClick = false;
          }
        }
        else
        {
          // in case the cursor gets out of thumbnail during a click+move, usually an if goes here but we
          // assign false directly and avoid conditional code. Getting out of the thumbnail while in a drag breaks
          // the drag movement though.
          m_inThumbnailClick = false;

          if (me->button() == Qt::LeftButton)
          {
            if ((e->type() == QEvent::MouseButtonPress))
            {
              if (me->modifiers() == Qt::CTRL)
              {
                centerCrosshairOnMousePosition(xPos, yPos);
              }
              else if (!eventHandler())
              {
                bool appendSelectedItems = me->modifiers() == Qt::SHIFT;
                selectPickedItems(xPos, yPos, appendSelectedItems);
              }
            }
          }
          // to avoid interfering with ctrl use in the event handler/selector
          onCursorChanged();
        }

        updateScale();
        updateThumbnail();
      }
      break;
    default:
      break;
  }

  return QWidget::eventFilter(caller, e);
}

//-----------------------------------------------------------------------------
void View2D::centerCrosshairOnMousePosition(int x, int y)
{
  auto pickedChannels = pick(Selector::CHANNEL, x, y);

  if (!pickedChannels.isEmpty())
  {
    auto point = toNormalizeWorldPosition(rendererUnderCursor(), x, y);

    emit viewFocusedOn(point);
  }
}

//-----------------------------------------------------------------------------
void View2D::centerViewOnMousePosition(int x, int y)
{
  auto pickedItems = pick(Selector::CHANNEL, x, y);

  if (!pickedItems.isEmpty())
  {
    auto point = toNormalizeWorldPosition(rendererUnderCursor(), x, y);

    moveCamera(point);
  }
}

//-----------------------------------------------------------------------------
void View2D::configureManager(RepresentationManagerSPtr manager)
{
  auto manager2D = dynamic_cast<RepresentationManager2D *>(manager.get());

  if (manager2D)
  {
    manager2D->setPlane(m_plane);
    manager2D->setRepresentationDepth(segmentationDepth());
  }
}

//-----------------------------------------------------------------------------
void View2D::normalizeWorldPosition(NmVector3 &point) const
{
  point[normalCoordinateIndex(m_plane)] = slicingPosition();
}

//-----------------------------------------------------------------------------
NmVector3 View2D::toNormalizeWorldPosition(vtkRenderer* renderer, int x, int y) const
{
  auto point = toWorldCoordinates(renderer, x, y, 0);

  normalizeWorldPosition(point);

  return point;
}

//-----------------------------------------------------------------------------
vtkSmartPointer< vtkRenderer > View2D::rendererUnderCursor() const
{
  return m_inThumbnail?m_thumbnail:m_renderer;
}

//-----------------------------------------------------------------------------
void View2D::onTakeSnapshot()
{
  takeSnapshot();
}

//-----------------------------------------------------------------------------
void View2D::updateManagersDepth(const NmVector3& resolution)
{
  for(auto manager: m_managers)
  {
    auto manager2D = dynamic_cast<RepresentationManager2D *>(manager.get());

    if (manager2D)
    {
      manager2D->setRepresentationDepth(segmentationDepth());
    }
  }
}

//-----------------------------------------------------------------------------
const QString View2D::viewName() const
{
  if (m_plane == Plane::XY) return "XY";
  if (m_plane == Plane::XZ) return "XZ";
  if (m_plane == Plane::YZ) return "YZ";

  return "Unknown View";
}

//-----------------------------------------------------------------------------
void View2D::updateThumbnailBounds(const Bounds &bounds)
{
  Nm sceneLeft = 0, sceneRight = 0, sceneUpper = 0, sceneLower = 0;
  if(bounds.areValid())
  {
    // reset thumbnail channel border
    int h = m_plane == Plane::YZ ? 2 : 0;
    int v = m_plane == Plane::XZ ? 2 : 1;

    sceneLeft  = bounds[2*h  ];
    sceneRight = bounds[2*h+1];
    sceneUpper = bounds[2*v  ];
    sceneLower = bounds[2*v+1];
  }

  m_channelBorder->SetVisibility(bounds.areValid());
  updateBorder(m_channelBorderData, sceneLeft, sceneRight, sceneUpper, sceneLower);
}

//-----------------------------------------------------------------------------
void View2D::updateWidgetLimits(const Bounds &bounds)
{
  int sliceMin = 0, sliceMax = 0;

  if(bounds.areValid())
  {
    sliceMin = voxelSlice(bounds[2*m_normalCoord]  , m_plane);
    sliceMax = voxelSlice(bounds[2*m_normalCoord+1], m_plane) - 1; // [lowerBound, upperBound) upper bound doesn't belong to the voxel
  }

  updateSpinBoxLimits  (sliceMin, sliceMax);
  updateScrollBarLimits(sliceMin, sliceMax);
}

//-----------------------------------------------------------------------------
void View2D::updateScrollBarLimits(int min, int max)
{
  m_scrollBar->blockSignals(true);
  if(fitToSlices())
  {
    m_scrollBar->setMinimum(min);
    m_scrollBar->setMaximum(max);
  }
  else
  {
    m_scrollBar->setMinimum(voxelCenter(min, m_plane));
    m_scrollBar->setMaximum(voxelCenter(max, m_plane));
  }
  m_scrollBar->blockSignals(false);
}

//-----------------------------------------------------------------------------
void View2D::updateSpinBoxLimits(int min, int max)
{
  m_spinBox->blockSignals(true);
  if(fitToSlices())
  {
    m_spinBox->setSuffix(" slice");
    m_spinBox->setMinimum(min+1);
    m_spinBox->setMaximum(max+1);
  }
  else
  {
    m_spinBox->setSuffix(" nm");
    m_spinBox->setMinimum(voxelCenter(min, m_plane));
    m_spinBox->setMaximum(voxelCenter(max, m_plane));
  }
  m_spinBox->blockSignals(false);
}

//-----------------------------------------------------------------------------
bool View2D::fitToSlices() const
{
  return state().fitToSlices();
}

//-----------------------------------------------------------------------------
Nm View2D::voxelCenter(const int slice, const Plane plane) const
{
  return state().coordinateSystem()->voxelCenter(slice, plane);
}

//-----------------------------------------------------------------------------
Nm View2D::voxelCenter(const Nm position, const Plane plane) const
{
  return state().coordinateSystem()->voxelCenter(position, plane);
}

//-----------------------------------------------------------------------------
int View2D::voxelSlice(const Nm position, const Plane plane) const
{
  return state().coordinateSystem()->voxelSlice(position, plane);
}

//-----------------------------------------------------------------------------
void View2D::setScaleVisibility(bool visible)
{
  if(m_scaleVisibility != visible)
  {
    m_scaleVisibility = visible;

    updateScale();

    refresh();
  }
}

//-----------------------------------------------------------------------------
void View2D::updateScaleValue()
{
  double *world,   worldWidth;
  int    *display, displayWidth;

  auto xCoord = (m_normalCoord == 0) ? 2 : 0;

  auto coords = vtkSmartPointer<vtkCoordinate>::New();
  coords->SetCoordinateSystemToNormalizedViewport();

  coords->SetValue(0, 0); //Viewport Lower Left Corner
  world = coords->GetComputedWorldValue(m_renderer);
  worldWidth = world[xCoord];
  display = coords->GetComputedDisplayValue(m_renderer);
  displayWidth = display[0];

  coords->SetValue(1, 0); // Viewport Lower Right Corner
  world = coords->GetComputedWorldValue(m_renderer);
  worldWidth = fabs(worldWidth - world[xCoord]);
  display = coords->GetComputedDisplayValue(m_renderer);
  displayWidth = abs(displayWidth - display[0]);

  m_scaleValue = worldWidth/displayWidth;
}

//-----------------------------------------------------------------------------
void View2D::onCrosshairChanged(const FrameCSPtr frame)
{
  auto point = frame->crosshair;

  // Disable scrollbar signals to avoid calling setting slice
  m_spinBox  ->blockSignals(true);
  m_scrollBar->blockSignals(true);

  int slicingPos = (fitToSlices() ? voxelSlice(point[m_normalCoord], m_plane) : vtkMath::Round(point[m_normalCoord]));

  m_scrollBar->setValue(slicingPos);

  if (fitToSlices())
  {
    slicingPos++; // Correct 0 index
  }
  m_spinBox->setValue(slicingPos);

  m_spinBox  ->blockSignals(false);
  m_scrollBar->blockSignals(false);
}

//-----------------------------------------------------------------------------
void View2D::moveCamera(const NmVector3 &point)
{
  double fp[3];
  m_renderer->GetActiveCamera()->GetFocalPoint(fp);
  NmVector3 focalPoint{fp};

  if(point != focalPoint)
  {
    m_state2D->updateCamera(m_renderer->GetActiveCamera(), point);

    refresh();
  }
}

//-----------------------------------------------------------------------------
void View2D::onSceneResolutionChanged(const NmVector3 &resolution)
{
  int sliceIndex = voxelSlice(slicingPosition(), m_plane);

  updateThumbnailBounds(sceneBounds());

  updateWidgetLimits(sceneBounds());

  updateManagersDepth(resolution);

  m_scrollBar->setValue(sliceIndex);

  resetCamera();
}

//-----------------------------------------------------------------------------
void View2D::onSceneBoundsChanged(const Bounds &bounds)
{
  if(m_scaleVisibility)
  {
    m_scale->SetVisibility(bounds.areValid());
  }

  updateThumbnailBounds(bounds);

  updateWidgetLimits(bounds);

  resetCamera();
}

//-----------------------------------------------------------------------------
void View2D::addSliceSelectors(SliceSelectorSPtr selector, SliceSelectionType type)
{
  auto sliceSelector = selector->clone(this, m_plane);

  auto fromWidget = sliceSelector->lowerWidget();
  auto toWidget   = sliceSelector->upperWidget();

  fromWidget->setVisible(type.testFlag(SliceSelectionTypes::From));
  toWidget  ->setVisible(type.testFlag(SliceSelectionTypes::To));

  m_fromLayout->addWidget (fromWidget );
  m_toLayout->insertWidget(0, toWidget);

  m_sliceSelectors << SliceSelectorPair(selector, sliceSelector);
}

//-----------------------------------------------------------------------------
void View2D::removeSliceSelectors(SliceSelectorSPtr widget)
{
  SliceSelectorPair requestedsliceSelectors;

  for (auto sliceSelectors : m_sliceSelectors)
  {
    if (sliceSelectors.first == widget)
    {
      requestedsliceSelectors = sliceSelectors;
      break;
    }
  }

  m_sliceSelectors.removeOne(requestedsliceSelectors);
}

//-----------------------------------------------------------------------------
void View2D::setCameraState(CameraState state)
{
  if (state.plane == m_plane)
  {
    auto camera = m_renderer->GetActiveCamera();
    camera->SetViewUp(state.upVector[0], state.upVector[1], state.upVector[2]);
    camera->SetPosition(state.cameraPosition[0], state.cameraPosition[1], state.cameraPosition[2]);
    camera->SetFocalPoint(state.focalPoint[0], state.focalPoint[1], state.focalPoint[2]);
    camera->Zoom(viewHeightLength() / state.heightLength);

    // NOTE: next line removed to avoid several changes in the crosshair. Just one change in the crosshair
    // is made from PositionMarks tool. If used outside the positions tool a manual crosshair change must
    // be made. TODO: review when Managers class has been refactorized.
    //
    // m_scrollBar->setValue(state.slice);

    refresh();
  }
}

//-----------------------------------------------------------------------------
RenderView::CameraState View2D::cameraState()
{
  RenderView::CameraState state;

  auto camera = m_renderer->GetActiveCamera();

  double cameraPos[3], focalPoint[3], up[3];

  camera->GetFocalPoint(focalPoint);
  camera->GetPosition(cameraPos);
  camera->GetViewUp(up);

  state.plane          = m_plane;
  state.slice          = m_scrollBar->value();
  state.cameraPosition = NmVector3{cameraPos[0], cameraPos[1], cameraPos[2]};
  state.focalPoint     = NmVector3{focalPoint[0], focalPoint[1], focalPoint[2]};
  state.upVector       = NmVector3{up[0], up[1], up[2]};
  state.heightLength   = viewHeightLength();

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
Selector::Selection View2D::pickImplementation(const Selector::SelectionFlags flags, const int x, const int y, bool multiselection) const
{
  Selector::Selection finalSelection;

  auto picker      = vtkSmartPointer<vtkPropPicker>::New();
  picker->PickFromListOn();

  auto sceneActors = rendererUnderCursor()->GetViewProps();

  NeuroItemAdapterList pickedItems;

  vtkProp *pickedProp;
  QList<vtkProp *> pickedProps;

  bool finished = false;
  bool picked   = false;

  do
  {
    picked     = picker->PickProp(x,y, rendererUnderCursor(), sceneActors);
    pickedProp = picker->GetViewProp();

    if (pickedProp && pickedProp->GetVisibility())
    {
      pickedProps << pickedProp;
      sceneActors->RemoveItem(pickedProp);
    }

    auto worldPoint = toNormalizeWorldPosition(rendererUnderCursor(), x, y);

    for(auto manager: m_managers)
    {
      auto items = manager->pick(worldPoint, pickedProp);

      for(auto item: items)
      {
        if(!pickedItems.contains(item))
        {
          if (Selector::IsValid(item, flags))
          {
            NeuroItemAdapterPtr neuroItem = item;
            if(flags.testFlag(Selector::SAMPLE) && isChannel(item))
            {
              neuroItem = QueryAdapter::sample(channelPtr(item)).get();
            }
            finalSelection << Selector::SelectionItem(pointToMask<unsigned char>(worldPoint, item->output()->spacing()), neuroItem);
            finished = !multiselection;
          }

          pickedItems << item;
          picked = true;
        }
      }
    }
  }
  while(picked && !finished);


  for(auto prop: pickedProps)
  {
    sceneActors->AddItem(prop);
    prop->VisibilityOn();
  }
  sceneActors->Modified();

  return finalSelection;
}

//-----------------------------------------------------------------------------
void View2D::incrementSlice()
{
  m_scrollBar->triggerAction(QScrollBar::SliderSingleStepAdd);
}

//-----------------------------------------------------------------------------
void View2D::decrementSlice()
{
  m_scrollBar->triggerAction(QScrollBar::SliderSingleStepSub);
}

//-----------------------------------------------------------------------------
double View2D::scale()
{
  updateScaleValue();

  return m_scaleValue;
}
