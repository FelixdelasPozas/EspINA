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
#include "GUI/View/Widgets/EspinaWidget.h"
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Model/Utils/SegmentationUtils.h>

// Qt
#include <QApplication>
#include <QEvent>
#include <QFileDialog>
#include <QPushButton>
#include <QMouseEvent>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QDebug>

// VTK
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <QVTKWidget.h>
#include <vtkCamera.h>
#include <vtkPOVExporter.h>
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

using namespace ESPINA;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::Representations;

//-----------------------------------------------------------------------------
View3D::View3D(GUI::View::ViewState &state, bool showCrosshairPlaneSelectors)
: RenderView                   {state, ViewType::VIEW_3D}
, m_mainLayout                 {new QVBoxLayout()}
, m_controlLayout              {new QHBoxLayout()}
, m_showCrosshairPlaneSelectors{showCrosshairPlaneSelectors}
{
  setupUI();
}

//-----------------------------------------------------------------------------
View3D::~View3D()
{
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying Volume View";
//   qDebug() << "********************************************************";

}

//-----------------------------------------------------------------------------
void View3D::buildViewActionsButtons()
{
  m_controlLayout = new QHBoxLayout();
  m_controlLayout->addStretch();

  m_cameraReset = createButton(QString(":/espina/zoom_reset.png"), tr("Reset Camera"));
  connect(m_cameraReset, SIGNAL(clicked()),
          this,          SLOT(onCameraResetPressed()));

  m_snapshot = createButton(QString(":/espina/snapshot_scene.svg"), tr("Save Scene as Image"));
  connect(m_snapshot,  SIGNAL(clicked()),
          this,        SLOT(onTakeSnapshot()));

  m_export = createButton(QString(":/espina/export_scene.svg"), tr("Export 3D Scene"));
  connect(m_export,    SIGNAL(clicked()),
          this,        SLOT(exportScene()));

  auto horizontalSpacer = new QSpacerItem(4000, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

  m_controlLayout->addWidget(m_cameraReset);
  m_controlLayout->addWidget(m_snapshot);
  m_controlLayout->addWidget(m_export);
  m_controlLayout->addItem(horizontalSpacer);

  m_mainLayout->addLayout(m_controlLayout);
}

//-----------------------------------------------------------------------------
void View3D::onCrosshairChanged(const NmVector3 &point)
{
  if (m_showCrosshairPlaneSelectors)
  {
    int iCenter[3] = {
      vtkMath::Round(point[0]/sceneResolution()[0]),
      vtkMath::Round(point[1]/sceneResolution()[1]),
      vtkMath::Round(point[2]/sceneResolution()[2])
    };

    m_axialScrollBar   ->blockSignals(true);
    m_coronalScrollBar ->blockSignals(true);
    m_sagittalScrollBar->blockSignals(true);

    m_axialScrollBar   ->setValue(iCenter[0]);
    m_coronalScrollBar ->setValue(iCenter[1]);
    m_sagittalScrollBar->setValue(iCenter[2]);

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
void View3D::onSceneResolutionChanged(const NmVector3 &reslotuion)
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
        auto pickedItem = manager->pick(worldPoint, pickedProp);

        NeuroItemAdapterPtr neuroItem = pickedItem;
        if(pickedItem && !pickedItems.contains(pickedItem))
        {
          if (Selector::IsValid(pickedItem, flags))
          {
            if(flags.testFlag(Selector::SAMPLE) && isChannel(pickedItem))
            {
              neuroItem = QueryAdapter::sample(channelPtr(pickedItem)).get();
            }
            finalSelection << Selector::SelectionItem(pointToMask<unsigned char>(worldPoint, pickedItem->output()->spacing()), neuroItem);
            finished = !multiselection;
          }

          pickedItems << pickedItem;
          picked |= pickedItem != nullptr;
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

  switch(e->type())
  {
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
  QFileDialog fileDialog(this, tr("Save Scene"), QString(), tr("All supported formats (*.x3d *.pov *.vrml);; POV-Ray files (*.pov);; VRML files (*.vrml);; X3D format (*.x3d)"));
  fileDialog.setObjectName("SaveSceneFileDialog");
  fileDialog.setWindowTitle("Save View as a 3D Scene");
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  fileDialog.setDefaultSuffix(QString(tr("vrml")));
  fileDialog.setFileMode(QFileDialog::AnyFile);
  fileDialog.selectFile("");

  if (fileDialog.exec() == QDialog::Accepted)
  {
    const QString selectedFile = fileDialog.selectedFiles().first();

    QStringList splittedName = selectedFile.split(".");
    QString extension = splittedName[((splittedName.size())-1)].toUpper();

    QStringList validFileExtensions;
    validFileExtensions << "X3D" << "POV" << "VRML";

    if (validFileExtensions.contains(extension))
    {
      if (QString("POV") == extension)
      {
        vtkPOVExporter *exporter = vtkPOVExporter::New();
        exporter->DebugOn();
        exporter->SetGlobalWarningDisplay(true);
        exporter->SetFileName(selectedFile.toUtf8());
        exporter->SetRenderWindow(m_renderer->GetRenderWindow());
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        exporter->Write();
        QApplication::restoreOverrideCursor();
        exporter->Delete();
      }

      if (QString("VRML") == extension)
      {
        vtkVRMLExporter *exporter = vtkVRMLExporter::New();
        exporter->DebugOn();
        exporter->SetFileName(selectedFile.toUtf8());
        exporter->SetRenderWindow(m_renderer->GetRenderWindow());
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        exporter->Write();
        QApplication::restoreOverrideCursor();
        exporter->Delete();
      }

      if (QString("X3D") == extension)
      {
        vtkX3DExporter *exporter = vtkX3DExporter::New();
        exporter->DebugOn();
        exporter->SetFileName(selectedFile.toUtf8());
        exporter->SetRenderWindow(m_renderer->GetRenderWindow());
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        exporter->Write();
        QApplication::restoreOverrideCursor();
        exporter->Delete();
      }
    }
    else
    {
      QMessageBox msgBox;
      QString message(tr("Scene not exported. Unrecognized extension "));
      message.append(extension).append(".");
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.setText(message);
      msgBox.exec();
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

  auto resolution = sceneResolution();

  point[0] = m_axialScrollBar   ->value() * resolution[0];
  point[1] = m_coronalScrollBar ->value() * resolution[1];
  point[2] = m_sagittalScrollBar->value() * resolution[2];

  emit crosshairChanged(point);
}

//-----------------------------------------------------------------------------
void View3D::updateScrollBarsLimits()
{
  if(m_showCrosshairPlaneSelectors)
  {
    auto bounds     = sceneBounds();
    auto resolution = sceneResolution();

    m_axialScrollBar   ->setMinimum(vtkMath::Round(bounds[0]/resolution[0]));
    m_axialScrollBar   ->setMaximum(vtkMath::Round(bounds[1]/resolution[0])-1);
    m_coronalScrollBar ->setMinimum(vtkMath::Round(bounds[2]/resolution[1]));
    m_coronalScrollBar ->setMaximum(vtkMath::Round(bounds[3]/resolution[1])-1);
    m_sagittalScrollBar->setMinimum(vtkMath::Round(bounds[4]/resolution[2]));
    m_sagittalScrollBar->setMaximum(vtkMath::Round(bounds[5]/resolution[2])-1);
  }
}

//-----------------------------------------------------------------------------
void View3D::setCameraState(CameraState state)
{
  if (state.plane == Plane::UNDEFINED)
  {
    auto camera = m_renderer->GetActiveCamera();

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
  double cameraPos[3], focalPoint[3];

  camera->GetFocalPoint(focalPoint);
  camera->GetPosition(cameraPos);

  state.plane = Plane::UNDEFINED;
  state.slice = -1;
  state.cameraPosition = NmVector3{cameraPos[0], cameraPos[1], cameraPos[2]};
  state.focalPoint = NmVector3{focalPoint[0], focalPoint[1], focalPoint[2]};
  state.heightLength = -1;

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
