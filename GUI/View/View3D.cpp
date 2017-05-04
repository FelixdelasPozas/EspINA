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
#include "View3D.h"
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Representations/Frame.h>
#include <GUI/Widgets/Styles.h>

// Qt
#include <QApplication>
#include <QEvent>
#include <QFileDialog>
#include <QPushButton>
#include <QMouseEvent>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollBar>

// VTK
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <QVTKWidget.h>
#include <vtkPOVExporter.h>
#include <vtkIVExporter.h>
#include <vtkOBJExporter.h>
#include <vtkOOGLExporter.h>
#include <vtkVRMLExporter.h>
#include <vtkX3DExporter.h>
#include <vtkAbstractWidget.h>
#include <vtkWidgetRepresentation.h>
#include <vtkMath.h>
#include <vtkCubeAxesActor2D.h>
#include <vtkAxisActor2D.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkTextProperty.h>
#include <vtkPropPicker.h>

// C++
#include <clocale>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::Widgets::Styles;

//-----------------------------------------------------------------------------
View3D::View3D(GUI::View::ViewState &state, bool showCrosshairPlaneSelectors, QWidget *parent)
: RenderView                   {state, ViewType::VIEW_3D, parent}
, m_mainLayout                 {new QVBoxLayout()}
, m_controlLayout              {new QHBoxLayout()}
, m_cameraCommand              {vtkSmartPointer<vtkCameraCommand>::New()}
, m_showCrosshairPlaneSelectors{showCrosshairPlaneSelectors}
{
  setupUI();

  connectCamera();

  onCrosshairChanged(state.createFrame());
}

//-----------------------------------------------------------------------------
View3D::~View3D()
{
  mainRenderer()->RemoveAllViewProps();
}

//-----------------------------------------------------------------------------
void View3D::buildViewActionsButtons()
{
  m_controlLayout = new QHBoxLayout();
  m_controlLayout->addStretch();

  m_cameraReset = createButton(QString(":/espina/reset_view.svg"), tr("Reset View"), this);
  connect(m_cameraReset, SIGNAL(clicked()),
          this,          SLOT(resetCamera()));

  m_snapshot = createButton(QString(":/espina/snapshot_scene.svg"), tr("Save Scene as Image"), this);
  connect(m_snapshot,  SIGNAL(clicked()),
          this,        SLOT(onTakeSnapshot()));

  m_export = createButton(QString(":/espina/export_scene.svg"), tr("Export 3D Scene"), this);
  connect(m_export,    SIGNAL(clicked()),
          this,        SLOT(exportScene()));

  auto horizontalSpacer = new QSpacerItem(4000, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

  const auto tooltip = tr("Distance from the camera to the focal point.");
  auto label = new QLabel("Focal distance:");
  label->setToolTip(tooltip);

  m_zoomFactor = new QDoubleSpinBox();
  m_zoomFactor->setToolTip(tooltip);
  m_zoomFactor->setSuffix(tr(" nm"));
  m_zoomFactor->setDecimals(2);
  m_zoomFactor->setMinimum(1);
  m_zoomFactor->setMaximum(VTK_DOUBLE_MAX);
  m_zoomFactor->setCorrectionMode(QDoubleSpinBox::CorrectToNearestValue);

  connect(m_zoomFactor, SIGNAL(valueChanged(double)), this, SLOT(onFocalDistanceChanged(double)));

  m_controlLayout->addWidget(m_cameraReset);
  m_controlLayout->addWidget(m_snapshot);
  m_controlLayout->addWidget(m_export);
  m_controlLayout->addItem(horizontalSpacer);
  m_controlLayout->addWidget(label);
  m_controlLayout->addWidget(m_zoomFactor);

  m_mainLayout->addLayout(m_controlLayout);
}

//-----------------------------------------------------------------------------
void View3D::onCrosshairChanged(const FrameCSPtr frame)
{
  auto point = frame->crosshair;

  if (m_showCrosshairPlaneSelectors)
  {
    auto resolution = frame->resolution;

    int iCenter[3] = { vtkMath::Round(point[0]/resolution[0]),
                       vtkMath::Round(point[1]/resolution[1]),
                       vtkMath::Round(point[2]/resolution[2]) };

    m_axialScrollBar   ->blockSignals(true);
    m_coronalScrollBar ->blockSignals(true);
    m_sagittalScrollBar->blockSignals(true);

    m_sagittalScrollBar->setValue(iCenter[0]);
    m_coronalScrollBar ->setValue(iCenter[1]);
    m_axialScrollBar   ->setValue(iCenter[2]);

    m_axialScrollBar   ->blockSignals(false);
    m_coronalScrollBar ->blockSignals(false);
    m_sagittalScrollBar->blockSignals(false);
  }
}

//-----------------------------------------------------------------------------
void View3D::moveCamera(const NmVector3 &point)
{
  m_renderer->GetActiveCamera()->SetFocalPoint(point[0],point[1],point[2]);
  m_renderer->ResetCameraClippingRange();
}

//-----------------------------------------------------------------------------
void View3D::onSceneResolutionChanged(const NmVector3 &resolution)
{
  updateScrollBarsLimits();
}

//-----------------------------------------------------------------------------
void View3D::onSceneBoundsChanged(const Bounds &bounds)
{
  updateScrollBarsLimits();
}

//-----------------------------------------------------------------------------
Selector::Selection View3D::pickImplementation(const Selector::SelectionFlags flags, const int x, const int y, bool multiselection) const
{
  Selector::Selection finalSelection;

  auto picker      = vtkSmartPointer<vtkPropPicker>::New();
  auto sceneActors = m_renderer->GetViewProps();

  NeuroItemAdapterList pickedItems;

  vtkProp *pickedProp;
  auto pickedProps = vtkSmartPointer<vtkPropCollection>::New();

  bool finished = false;
  bool picked   = false;

  do
  {
    picked = picker->PickProp(x,y, m_renderer, sceneActors);
    pickedProp = picker->GetViewProp();

    if(pickedProp)
    {
      sceneActors->RemoveItem(pickedProp);
      pickedProps->AddItem(pickedProp);

      NmVector3 worldPoint;
      double point[3];
      picker->GetPickPosition(point);
      worldPoint = NmVector3{point};

      for(auto manager: m_managers)
      {
        auto items = manager->pick(worldPoint, pickedProp);

        for(auto item: items)
        {
          NeuroItemAdapterPtr neuroItem = item;
          if(!pickedItems.contains(item))
          {
            if (Selector::IsValid(item, flags))
            {
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
    else
    {
      finished = true;
    }
  }
  while(picked && !finished);

  pickedProps->InitTraversal();

  while ((pickedProp = pickedProps->GetNextProp()))
  {
    sceneActors->AddItem(pickedProp);
  }
  sceneActors->Modified();

  return finalSelection;
}

//-----------------------------------------------------------------------------
Bounds View3D::previewBounds(bool cropToSceneBounds) const
{
  return sceneBounds();
}

//-----------------------------------------------------------------------------
void View3D::setupUI()
{
  if (m_showCrosshairPlaneSelectors)
  {
    m_additionalGUI  = new QHBoxLayout();
    m_axialScrollBar = new QScrollBar(Qt::Horizontal);

    m_axialScrollBar->setEnabled(false);
    m_axialScrollBar->setFixedHeight(15);
    m_axialScrollBar->setToolTip(tr("Axial scroll bar"));
    connect(m_axialScrollBar, SIGNAL(valueChanged(int)),
            this,             SLOT(scrollBarMoved(int)));

    m_coronalScrollBar = new QScrollBar(Qt::Vertical);
    m_coronalScrollBar->setEnabled(false);
    m_coronalScrollBar->setFixedWidth(15);
    m_coronalScrollBar->setToolTip(tr("Coronal scroll bar"));
    connect(m_coronalScrollBar, SIGNAL(valueChanged(int)),
            this,               SLOT(scrollBarMoved(int)));

    m_sagittalScrollBar = new QScrollBar(Qt::Vertical);
    m_sagittalScrollBar->setEnabled(false);
    m_sagittalScrollBar->setFixedWidth(15);
    m_sagittalScrollBar->setToolTip(tr("Sagittal scroll bar"));
    connect(m_sagittalScrollBar, SIGNAL(valueChanged(int)),
            this,                SLOT(scrollBarMoved(int)));

    updateScrollBarsLimits();

    m_additionalGUI->insertWidget(0, m_coronalScrollBar,0);
    m_additionalGUI->insertWidget(1, m_view,1);
    m_additionalGUI->insertWidget(2, m_sagittalScrollBar,0);

    m_mainLayout->insertLayout(0, m_additionalGUI, 1);
    m_mainLayout->insertWidget(1, m_axialScrollBar, 0);
  }
  else
  {
    m_mainLayout->insertWidget(0,m_view);
  }

  m_view->show();
  m_renderer = vtkSmartPointer<vtkRenderer>::New();
  m_renderer->LightFollowCameraOn();
  m_renderer->BackingStoreOff();
  m_renderer->GetActiveCamera(); // creates default camera.

  auto interactorstyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
  interactorstyle->AutoAdjustCameraClippingRangeOn();
  interactorstyle->KeyPressActivationOff();

  renderWindow()->AddRenderer(m_renderer);
  renderWindow()->GetInteractor()->SetInteractorStyle(interactorstyle);
  m_view->installEventFilter(this);

  buildViewActionsButtons();

  // Color background
  QPalette pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Window));
  this->setPalette(pal);

  setLayout(m_mainLayout);
}

//-----------------------------------------------------------------------------
void View3D::refreshViewImplementation()
{
}

//-----------------------------------------------------------------------------
void View3D::resetCameraImplementation()
{
  auto activeCamera = m_renderer->GetActiveCamera();
  activeCamera->SetViewUp(0,1,0);
  activeCamera->SetPosition(0,0,-1);
  activeCamera->SetFocalPoint(0,0,0);
  activeCamera->SetRoll(180);
  m_renderer->ResetCamera();
}

//-----------------------------------------------------------------------------
bool View3D::isCrosshairPointVisible() const
{
  return true;
}

//-----------------------------------------------------------------------------
void View3D::addActor(vtkProp *actor)
{
  m_renderer->AddActor(actor);
}

//-----------------------------------------------------------------------------
void View3D::removeActor(vtkProp *actor)
{
  m_renderer->RemoveActor(actor);
}

//-----------------------------------------------------------------------------
vtkRenderer *View3D::mainRenderer() const
{
  return m_renderer;
}

//-----------------------------------------------------------------------------
bool View3D::eventFilter(QObject* caller, QEvent* e)
{
  // there is not a "single-click" event so we need to remember the position of the
  // press event and compare it with the position of the release event (for picking).
  static int x = -1;
  static int y = -1;

  int xPos, yPos;
  eventPosition(xPos, yPos);

  if(eventHandlerFilterEvent(e))
  {
    return true;
  }

  switch(e->type())
  {
    case QEvent::Enter:
      QWidget::enterEvent(e);

      // get the focus this very moment
      setFocus(Qt::OtherFocusReason);

      if (eventHandler())
      {
        m_view->setCursor(eventHandler()->cursor());
      }
      else
      {
        m_view->setCursor(Qt::ArrowCursor);
      }

      e->accept();
      break;
    case QEvent::MouseButtonPress:
      {
        auto me = static_cast<QMouseEvent*>(e);
        if (me->button() == Qt::LeftButton)
        {
          if (me->modifiers() == Qt::CTRL)
          {
            Selector::SelectionFlags flags(Selector::SelectionTag::CHANNEL|Selector::SelectionTag::SEGMENTATION);
            auto picked = pick(flags, xPos, yPos, true);

            if(!picked.isEmpty() != 0)
            {
              auto element = picked.first();
              auto maskBounds = element.first->bounds();

              NmVector3 point{(maskBounds[0]+maskBounds[1])/2,
                              (maskBounds[2]+maskBounds[3])/2,
                              (maskBounds[4]+maskBounds[5])/2};

              // point is not guaranteed to belong to the picked item, as the vtkPropPicker can
              // select a point outside the picked point in 3D picking. That's why this method
              // won't work with 3D channel representation as it's manager won't recognize the
              // point as part of the actor.
              Bounds itemBounds = viewItemAdapter(element.second)->bounds();
              if(!contains(itemBounds, point))
              {
                // adjust point to avoid changing the crosshair to a point outside the channel.
                for(int i: {0,1,2})
                {
                  if (point[i] < itemBounds[2*i]) point[i] = itemBounds[2*i];
                  if (point[i] > itemBounds[(2*i)+1]) point[i] = itemBounds[(2*i)+1];
                }
              }
              emit crosshairChanged(point);
            }
          }
          else
          {
            x = xPos;
            y = yPos;
          }
        }
      }
      break;
    case QEvent::MouseButtonRelease:
      {
        auto me = static_cast<QMouseEvent*>(e);

        if ((me->button() == Qt::LeftButton) && !(me->modifiers() == Qt::CTRL))
        {
          if ((xPos == x) && (yPos == y))
          {
            selectPickedItems(xPos, yPos, me->modifiers() == Qt::SHIFT);
          }
        }
      }
      break;
    case QEvent::ToolTip:
      showSegmentationTooltip(xPos, yPos);
      break;
    default:
      break;
  }

  return QObject::eventFilter(caller, e);
}

//-----------------------------------------------------------------------------
void View3D::exportScene()
{
  auto title      = tr("Export 3D scene");
  auto suggestion = tr("scene.wrl");
  auto formats    = SupportedFormats(tr("VRML 2.0 format"),         "wrl")
                          .addFormat(tr("Blender X3D format"),      "x3d")
                          .addFormat(tr("POV-Ray format"),          "pov")
                          .addFormat(tr("OpenInventor 2.0 format"), "iv")
                          .addFormat(tr("Wavefront format"),        "obj")
                          .addFormat(tr("Geomview format"),         "oogl");

  auto fileName = DefaultDialogs::SaveFile(title, formats, QDir::homePath(), ".wrl", suggestion, this);

  if (!fileName.isEmpty())
  {
    QStringList splittedName = fileName.split(".");
    QString extension = splittedName[((splittedName.size())-1)].toUpper().remove(' ');

    QStringList validFileExtensions;
    validFileExtensions << "POV" << "X3D" << "WRL" << "IV" << "OBJ" << "OOGL";

    if (validFileExtensions.contains(extension))
    {
      // NOTE: locale affects the filters when writing numbers to a file and most of the formats
      // require a point instead of a comma for the decimal point separator.
      auto locale = std::localeconv();
      bool changed_locale = false;

      if(locale->decimal_point[0] != '.')
      {
        std::setlocale(LC_NUMERIC, "en_US.UTF-8");
        changed_locale = true;
      }

      if (QString("POV") == extension)
      {
        auto exporter = vtkSmartPointer<vtkPOVExporter>::New();
        exporter->SetFileName(fileName.toUtf8());
        exporter->SetRenderWindow(m_renderer->GetRenderWindow());
        {
          WaitingCursor cursor;
          exporter->Write();
        }
      }

      if (QString("WRL") == extension)
      {
        auto exporter = vtkSmartPointer<vtkVRMLExporter>::New();
        exporter->SetFileName(fileName.toUtf8());
        exporter->SetRenderWindow(m_renderer->GetRenderWindow());
        {
          WaitingCursor cursor;
          exporter->Write();
        }
      }

      if (QString("X3D") == extension)
      {
        auto exporter = vtkSmartPointer<vtkX3DExporter>::New();
        exporter->SetFileName(fileName.toUtf8());
        exporter->SetRenderWindow(m_renderer->GetRenderWindow());
        exporter->SetBinary(false);
        {
          WaitingCursor cursor;
          exporter->Write();
        }
      }

      if (QString("IV") == extension)
      {
        auto exporter = vtkSmartPointer<vtkIVExporter>::New();
        exporter->SetFileName(fileName.toUtf8());
        exporter->SetRenderWindow(m_renderer->GetRenderWindow());
        {
          WaitingCursor cursor;
          exporter->Write();
        }
      }

      if (QString("OBJ") == extension)
      {
        auto file = QFileInfo(fileName);
        auto prefix = file.absoluteDir().absolutePath();

        if(!prefix.endsWith('/')) prefix += QString('/');

        prefix += file.baseName();

        auto exporter = vtkSmartPointer<vtkOBJExporter>::New();
        exporter->SetFilePrefix(prefix.toUtf8());
        exporter->SetRenderWindow(m_renderer->GetRenderWindow());
        {
          WaitingCursor cursor;
          exporter->Write();
        }
      }

      if (QString("OOGL") == extension)
      {
        auto exporter = vtkSmartPointer<vtkOOGLExporter>::New();
        exporter->SetFileName(fileName.toUtf8());
        exporter->SetRenderWindow(m_renderer->GetRenderWindow());
        {
          WaitingCursor cursor;
          exporter->Write();
        }
      }

      if(changed_locale)
      {
        std::setlocale(LC_NUMERIC, "");
      }

      // check if write was successful, vtk classes don't throw exceptions as a general rule.
      QFileInfo fileInfo{fileName};
      if(!fileInfo.exists() || fileInfo.size() == 0)
      {
        auto message = tr("Couldn't export %1. Couldn't save file.").arg(fileName);
        DefaultDialogs::InformationMessage(message, title, "", this);
      }
    }
    else
    {
      auto message = tr("Couldn't export %1. Format not supported.").arg(fileName);
      DefaultDialogs::InformationMessage(message, title, "", this);
    }
  }
}

//-----------------------------------------------------------------------------
void View3D::onTakeSnapshot()
{
  takeSnapshot();
}

//-----------------------------------------------------------------------------
void View3D::updateViewActions(RepresentationManager::ManagerFlags flags)
{
  bool hasActors = flags.testFlag(RepresentationManager::HAS_ACTORS);
  bool exports3D = flags.testFlag(RepresentationManager::EXPORTS_3D);

  m_cameraReset->setEnabled(hasActors);
  m_snapshot   ->setEnabled(hasActors);
  m_export     ->setEnabled(exports3D);

  if(m_showCrosshairPlaneSelectors)
  {
    m_axialScrollBar   ->setEnabled(hasActors);
    m_coronalScrollBar ->setEnabled(hasActors);
    m_sagittalScrollBar->setEnabled(hasActors);
    updateScrollBarsLimits();
  }
}

//-----------------------------------------------------------------------------
void View3D::scrollBarMoved(int value)
{
  NmVector3 point;
  auto bar = qobject_cast<QScrollBar *>(sender());

  Plane plane{Plane::UNDEFINED};

  if(bar == m_axialScrollBar)
  {
    plane = Plane::XY;
  }
  else
  {
    if(bar == m_coronalScrollBar)
    {
      plane = Plane::XZ;
    }
    else
    {
      if(bar == m_sagittalScrollBar)
      {
        plane = Plane::YZ;
      }
    }
  }

  if(plane == Plane::UNDEFINED)
  {
    qWarning() << "unknown signal sender" << __FILE__ << __LINE__;
    return;
  }

  auto resolution = sceneResolution();

  emit crosshairPlaneChanged(plane, value * resolution[normalCoordinateIndex(plane)]);
}

//-----------------------------------------------------------------------------
void View3D::updateScrollBarsLimits()
{
  if(m_showCrosshairPlaneSelectors)
  {
    auto bounds     = sceneBounds();
    auto resolution = sceneResolution();
    int axialMax = 0, axialMin = 0, coronalMax = 0, coronalMin = 0, sagittalMax = 0, sagittalMin = 0;

    if(bounds.areValid())
    {
      sagittalMin = vtkMath::Round((bounds[0]+(resolution[0]/2))/resolution[0]);
      sagittalMax = vtkMath::Round((bounds[1]+(resolution[0]/2))/resolution[0])-1;
      coronalMin  = vtkMath::Round((bounds[2]+(resolution[1]/2))/resolution[1]);
      coronalMax  = vtkMath::Round((bounds[3]+(resolution[1]/2))/resolution[1])-1;
      axialMin    = vtkMath::Round((bounds[4]+(resolution[2]/2))/resolution[2]);
      axialMax    = vtkMath::Round((bounds[5]+(resolution[2]/2))/resolution[2])-1;
    }

    m_sagittalScrollBar->setMinimum(sagittalMin);
    m_sagittalScrollBar->setMaximum(sagittalMax);
    m_coronalScrollBar ->setMinimum(coronalMin);
    m_coronalScrollBar ->setMaximum(coronalMax);
    m_axialScrollBar   ->setMinimum(axialMin);
    m_axialScrollBar   ->setMaximum(axialMax);
  }
}

//-----------------------------------------------------------------------------
void View3D::setCameraState(CameraState state)
{
  if (state.plane == Plane::UNDEFINED)
  {
    auto camera = m_renderer->GetActiveCamera();

    camera->SetViewUp(state.upVector[0], state.upVector[1], state.upVector[2]);
    camera->SetPosition(state.cameraPosition[0], state.cameraPosition[1], state.cameraPosition[2]);
    camera->SetFocalPoint(state.focalPoint[0], state.focalPoint[1], state.focalPoint[2]);

    refresh();
  }
}

//-----------------------------------------------------------------------------
RenderView::CameraState View3D::cameraState()
{
  CameraState state;

  auto camera = m_renderer->GetActiveCamera();
  double cameraPos[3], focalPoint[3], up[3];

  camera->GetFocalPoint(focalPoint);
  camera->GetPosition(cameraPos);
  camera->GetViewUp(up);

  state.plane          = Plane::UNDEFINED;
  state.slice          = -1;
  state.cameraPosition = NmVector3{cameraPos[0], cameraPos[1], cameraPos[2]};
  state.focalPoint     = NmVector3{focalPoint[0], focalPoint[1], focalPoint[2]};
  state.upVector       = NmVector3{up[0], up[1], up[2]};
  state.heightLength   = -1;

  return state;
}

//-----------------------------------------------------------------------------
void View3D::resetImplementation()
{
}

//-----------------------------------------------------------------------------
const QString View3D::viewName() const
{
  return "3D";
}

//-----------------------------------------------------------------------------
void View3D::connectCamera()
{
  m_cameraCommand->SetView(this);
  auto camera = mainRenderer()->GetActiveCamera();

  camera->Zoom(1.0);
  camera->AddObserver(vtkCommand::ModifiedEvent, m_cameraCommand.Get());
}

//-----------------------------------------------------------------------------
void View3D::onFocalDistanceChanged(double distance)
{
  double fp[3], position[3];

  auto camera = mainRenderer()->GetActiveCamera();
  const auto currentDistance = camera->GetDistance();
  camera->GetPosition(position);
  camera->GetFocalPoint(fp);
  const double vec[3]{position[0]-fp[0], position[1]-fp[1], position[2]-fp[2]};
  const auto factor = distance/currentDistance;

  camera->SetPosition(fp[0] + factor*vec[0], fp[1] + factor*vec[1], fp[2] + factor*vec[2]);
  m_renderer->ResetCameraClippingRange();

  refresh();
}
