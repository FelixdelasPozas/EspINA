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
#include "ViewRendererMenu.h"
#include "GUI/View/Widgets/EspinaWidget.h"
#include <GUI/Model/Utils/QueryAdapter.h>

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

using namespace ESPINA;

//-----------------------------------------------------------------------------
View3D::View3D(bool showCrosshairPlaneSelectors, QWidget* parent)
: RenderView                   {ViewType::VIEW_3D, parent}
, m_mainLayout                 {new QVBoxLayout()}
, m_controlLayout              {new QHBoxLayout()}
, m_showCrosshairPlaneSelectors{showCrosshairPlaneSelectors}
, m_numEnabledRenderers        {0}
{
  setupUI();
}

//-----------------------------------------------------------------------------
View3D::~View3D()
{
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying Volume View";
//   qDebug() << "********************************************************";

  for(auto widget: m_widgets)
  {
    RenderView::removeWidget(widget); // faster than calling this->removeWidget(widget);
  }

  m_widgets.clear();
}

//-----------------------------------------------------------------------------
void View3D::reset()
{
  for(auto widget: m_widgets)
  {
    widget->unregisterView(this);
  }

  m_widgets.clear();

  Q_ASSERT(m_widgets.isEmpty());

  updateViewActions();
}

// //-----------------------------------------------------------------------------
// void View3D::addRendererControls(RendererSPtr renderer)
// {
//   if (m_renderers.contains(renderer) || !renderer->renderType().testFlag(RendererType::RENDERER_VIEW3D))
//     return;
//
//   m_renderers << renderer;
//
//   renderer->setView(this);
//   renderer->setEnable(false);
//
//   // add segmentation representations to renderer
//   for(auto segmentation : m_segmentationStates.keys())
//     if(renderer->type() == Renderer::Type::Representation)
//     {
//       auto repRenderer = representationRenderer(renderer);
//       if (repRenderer->canRender(segmentation))
//         for(auto rep : m_segmentationStates[segmentation].representations)
//            if (repRenderer->managesRepresentation(rep->type()))
//              repRenderer->addRepresentation(segmentation, rep);
//     }
//
//   // add channel representations to renderer
//   for(auto channel : m_channelStates.keys())
//     if(renderer->type() == Renderer::Type::Representation)
//     {
//       auto repRenderer = representationRenderer(renderer);
//       if (repRenderer->canRender(channel))
//         for(auto rep : m_channelStates[channel].representations)
//           if (repRenderer->managesRepresentation(rep->type()))
//             repRenderer->addRepresentation(channel, rep);
//     }
//
//   auto configMenu = qobject_cast<ViewRendererMenu*>(m_renderConfig->menu());
//   if (configMenu == nullptr)
//   {
//     configMenu = new ViewRendererMenu(m_renderConfig);
//     m_renderConfig->setMenu(configMenu);
//     m_renderConfig->setEnabled(true);
//   }
//   configMenu->add(renderer);
//
//   connect(renderer.get(), SIGNAL(renderRequested()), this, SLOT(updateViewActions()), Qt::QueuedConnection);
//   connect(renderer.get(), SIGNAL(renderRequested()), this, SLOT(updateView()), Qt::QueuedConnection);
//
//   updateViewActions();
// }
//
// //-----------------------------------------------------------------------------
// void View3D::removeRendererControls(const QString name)
// {
//   RendererSPtr removedRenderer;
//   for(auto renderer: m_renderers)
//     if (renderer->name() == name)
//     {
//       removedRenderer = renderer;
//       break;
//     }
//
//   if (!removedRenderer)
//     return;
//
//   m_renderers.removeOne(removedRenderer);
//
//   if (!removedRenderer->isHidden())
//     removedRenderer->setEnable(false);
//
//   auto configMenu = qobject_cast<ViewRendererMenu*>(m_renderConfig->menu());
//   if (configMenu != nullptr)
//   {
//     configMenu->remove(removedRenderer);
//     if (configMenu->actions().isEmpty())
//     {
//       m_renderConfig->setMenu(nullptr);
//       delete configMenu;
//       m_renderConfig->setEnabled(false);
//     }
//   }
//   disconnect(removedRenderer.get(), SIGNAL(renderRequested()), this, SLOT(updateView()));
//
//   updateViewActions();
// }

//-----------------------------------------------------------------------------
void View3D::buildViewActionsButtons()
{
  m_controlLayout = new QHBoxLayout();
  m_controlLayout->addStretch();

  m_zoom = createButton(QString(":/espina/zoom_reset.png"), tr("Reset Camera"));
  connect(m_zoom, SIGNAL(clicked()), this, SLOT(resetView()));

  m_snapshot = createButton(QString(":/espina/snapshot_scene.svg"), tr("Save Scene as Image"));
  connect(m_snapshot,SIGNAL(clicked(bool)),this,SLOT(onTakeSnapshot()));

  m_export = createButton(QString(":/espina/export_scene.svg"), tr("Export 3D Scene"));
  connect(m_export,SIGNAL(clicked(bool)),this,SLOT(exportScene()));

  m_renderConfig = createButton(QString(":/espina/settings.png"), tr("Configure this view's renderers"));
  m_renderConfig->setStyleSheet("QPushButton::menu-indicator {image: "";}");

  QSpacerItem * horizontalSpacer = new QSpacerItem(4000, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

  m_controlLayout->addWidget(m_zoom);
  m_controlLayout->addWidget(m_snapshot);
  m_controlLayout->addWidget(m_export);
  m_controlLayout->addItem(horizontalSpacer);
  m_controlLayout->addWidget(m_renderConfig);

//   for(auto renderer : m_renderers)
//     if (canRender(renderer, RendererType::RENDERER_VIEW3D))
//       this->addRendererControls(renderer->clone());

  m_mainLayout->addLayout(m_controlLayout);
}

//-----------------------------------------------------------------------------
bool View3D::isCrosshairVisible() const
{
  auto coords = vtkSmartPointer<vtkCoordinate>::New();
  coords->SetViewport(m_renderer);
  coords->SetCoordinateSystemToNormalizedViewport();

  double ll[3], ur[3], ch[3];
  coords->SetValue(0, 0); //LL
  memcpy(ll,coords->GetComputedDisplayValue(m_renderer),3*sizeof(double));
  coords->SetValue(1, 1); //UR
  memcpy(ur,coords->GetComputedDisplayValue(m_renderer),3*sizeof(double));

  auto current = crosshair();

  coords->SetCoordinateSystemToWorld();
  coords->SetValue(current[0], current[1], current[2]);
  memcpy(ch,coords->GetComputedDisplayValue(m_renderer),3*sizeof(double));

  return  current[0] < ll[0] || current[0] > ur[0] // Horizontally out
       || current[1] > ll[1] || current[1] < ur[1];// Vertically out
}

//-----------------------------------------------------------------------------
void View3D::onCrosshairChanged(const NmVector3 &point)
{
  if (m_showCrosshairPlaneSelectors && !m_channelSources->isEmpty())
  {
    int iCenter[3] = {
      vtkMath::Round(point[0]/m_sceneResolution[0]),
      vtkMath::Round(point[1]/m_sceneResolution[1]),
      vtkMath::Round(point[2]/m_sceneResolution[2])
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

//   if (!isCrosshairVisible())
//   {
//     moveCamera(point);
//   }
}

//-----------------------------------------------------------------------------
void View3D::moveCamera(const NmVector3 &point)
{
  m_renderer->GetActiveCamera()->SetFocalPoint(point[0],point[1],point[2]);
  m_renderer->ResetCameraClippingRange();
}

//-----------------------------------------------------------------------------
Selector::Selection View3D::pickImplementation(const Selector::SelectionFlags flags, const int x, const int y, bool multiselection) const
{
  QMap<NeuroItemAdapterPtr, BinaryMaskSPtr<unsigned char>> selectedItems;
  Selector::Selection finalSelection;

//   for(auto renderer: m_renderers)
//   {
//     if(renderer->type() != Renderer::Type::Representation)
//       continue;
//
//     auto repRenderer = representationRenderer(renderer);
//
//     if(flags.contains(Selector::SEGMENTATION) && canRender(repRenderer, RenderableType::SEGMENTATION))
//     {
//       for (auto item : repRenderer->pick(x, y, 0, m_renderer, RenderableItems(RenderableType::SEGMENTATION), multiselection))
//       {
//         BinaryMaskSPtr<unsigned char> bm { new BinaryMask<unsigned char> { Bounds(repRenderer->pickCoordinates()), item->output()->spacing() } };
//         BinaryMask<unsigned char>::iterator bmit(bm.get());
//         bmit.goToBegin();
//         bmit.Set();
//
//         selectedItems[item] = bm;
//       }
//     }
//
//     if((flags.contains(Selector::CHANNEL) || flags.contains(Selector::SAMPLE)) && canRender(repRenderer, RenderableType::CHANNEL))
//     {
//       for (auto item : repRenderer->pick(x, y, 0, m_renderer, RenderableItems(RenderableType::CHANNEL), multiselection))
//       {
//         if(flags.contains(Selector::CHANNEL))
//         {
//           BinaryMaskSPtr<unsigned char> bm { new BinaryMask<unsigned char> { Bounds(repRenderer->pickCoordinates()), item->output()->spacing() } };
//           BinaryMask<unsigned char>::iterator bmit(bm.get());
//           bmit.goToBegin();
//           bmit.Set();
//
//           selectedItems[item] = bm;
//         }
//
//
//         if(flags.contains(Selector::SAMPLE))
//         {
//           BinaryMaskSPtr<unsigned char> bm { new BinaryMask<unsigned char> { Bounds(repRenderer->pickCoordinates()), item->output()->spacing() } };
//           BinaryMask<unsigned char>::iterator bmit(bm.get());
//           bmit.goToBegin();
//           bmit.Set();
//
//           auto sample = QueryAdapter::sample(dynamic_cast<ChannelAdapterPtr>(item));
//           selectedItems[item] = bm;
//         }
//       }
//     }
//   }
//
//   for(auto item: selectedItems.keys())
//     finalSelection << QPair<Selector::SelectionMask, NeuroItemAdapterPtr>(selectedItems[item], item);

  return finalSelection;
}

//-----------------------------------------------------------------------------
void View3D::resetCamera()
{
  m_renderer->GetActiveCamera()->SetViewUp(0,1,0);
  m_renderer->GetActiveCamera()->SetPosition(0,0,-1);
  m_renderer->GetActiveCamera()->SetFocalPoint(0,0,0);
  m_renderer->GetActiveCamera()->SetRoll(180);
  m_renderer->ResetCamera();
}

//-----------------------------------------------------------------------------
void View3D::addWidget(EspinaWidgetSPtr widget)
{
  RenderView::addWidget(widget);

  updateViewActions();
}

//-----------------------------------------------------------------------------
void View3D::removeWidget(EspinaWidgetSPtr widget)
{
  RenderView::removeWidget(widget);

  updateViewActions();
}

//-----------------------------------------------------------------------------
Bounds View3D::previewBounds(bool cropToSceneBounds) const
{
  Bounds resultBounds;
//   for (auto channel: m_channelStates.keys())
//   {
//     if (!resultBounds.areValid())
//       resultBounds = channel->bounds();
//     else
//       resultBounds = boundingBox(resultBounds, channel->bounds());
//   }

  return resultBounds;
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
    m_axialScrollBar->setToolTip("Axial scroll bar");
    connect(m_axialScrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrollBarMoved(int)));

    m_coronalScrollBar = new QScrollBar(Qt::Vertical);
    m_coronalScrollBar->setEnabled(false);
    m_coronalScrollBar->setFixedWidth(15);
    m_coronalScrollBar->setToolTip("Coronal scroll bar");
    connect(m_coronalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrollBarMoved(int)));

    m_sagittalScrollBar = new QScrollBar(Qt::Vertical);
    m_sagittalScrollBar->setEnabled(false);
    m_sagittalScrollBar->setFixedWidth(15);
    m_sagittalScrollBar->setToolTip("Sagittal scroll bar");
    connect(m_sagittalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrollBarMoved(int)));

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
  m_view->GetRenderWindow()->AddRenderer(m_renderer);
  m_view->GetRenderWindow()->GetInteractor()->SetInteractorStyle(interactorstyle);
  m_view->GetRenderWindow()->Render();
  m_view->installEventFilter(this);

  buildViewActionsButtons();

  // Color background
  QPalette pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Window));
  this->setPalette(pal);

  setLayout(m_mainLayout);
}

//-----------------------------------------------------------------------------
void View3D::updateView()
{
  if(isVisible())
  {
    qDebug() << "Render 3D";
    m_view->GetRenderWindow()->Render();
    m_view->update();
  }
}

//-----------------------------------------------------------------------------
void View3D::selectPickedItems(int vx, int vy, bool append)
{
  ViewItemAdapterList selection, pickedItems;
//   if (append)
//     selection = currentSelection()->items();
//
//   // If no append, segmentations have priority over channels
//   for(auto renderer : m_renderers)
//   {
//     if(renderer->type() == Renderer::Type::Representation)
//     {
//       auto repRenderer = representationRenderer(renderer);
//       if (!repRenderer->isHidden() && canRender(repRenderer, RenderableType::SEGMENTATION))
//       {
//         pickedItems = repRenderer->pick(vx, vy, 0, m_renderer, RenderableItems(RenderableType::SEGMENTATION), append);
//         if (!pickedItems.empty())
//         {
//           for(ViewItemAdapterPtr item : pickedItems)
//             if (!selection.contains(item))
//               selection << item;
//             else
//               selection.removeAll(item);
//         }
//       }
//     }
//   }
//
//   pickedItems.clear();
//
//   for(auto renderer : m_renderers)
//   {
//     if(renderer->type() == Renderer::Type::Representation)
//     {
//       auto repRenderer = representationRenderer(renderer);
//       if (!repRenderer->isHidden() && canRender(repRenderer, RenderableType::CHANNEL))
//       {
//         pickedItems = repRenderer->pick(vx, vy, 0, m_renderer, RenderableItems(RenderableType::CHANNEL), append);
//         if (!pickedItems.empty())
//         {
//           for(ViewItemAdapterPtr item : pickedItems)
//             if (!selection.contains(item))
//               selection << item;
//             else
//               selection.removeAll(item);
//         }
//       }
//     }
//   }
//
//   if (!append && !selection.empty())
//   {
//     ViewItemAdapterPtr returnItem = selection.first();
//     selection.clear();
//     selection << returnItem;
//   }
//
//   currentSelection()->set(selection);
}

//-----------------------------------------------------------------------------
bool View3D::eventFilter(QObject* caller, QEvent* e)
{
  // there is not a "single-click" event so we need to remember the position of the
  // press event and compare it with the position of the release event (for picking).
  static int x = -1;
  static int y = -1;

  int newX, newY;
  eventPosition(newX, newY);

  if (e->type() == QEvent::MouseButtonPress)
  {
    auto me = static_cast<QMouseEvent*>(e);
    if (me->button() == Qt::LeftButton)
    {
      if (me->modifiers() == Qt::CTRL)
      {
//         for(auto renderer: m_renderers)
//         {
//           if(renderer->type() == Renderer::Type::Representation)
//           {
//             auto repRenderer = representationRenderer(renderer);
//             if (!repRenderer->isHidden())
//             {
//               auto selection = repRenderer->pick(newX, newY, 0, m_renderer, RenderableItems(RenderableType::SEGMENTATION|RenderableType::CHANNEL), false);
//               if (!selection.empty())
//               {
//                 NmVector3 point = repRenderer->pickCoordinates();
//
//                 emit centerChanged(point);
//                 break;
//               }
//             }
//           }
//         }
      }
      else
      {
        x = newX;
        y = newY;
      }
    }
  }

  if (e->type() == QEvent::MouseButtonRelease)
  {
    auto me = static_cast<QMouseEvent*>(e);

    if ((me->button() == Qt::LeftButton) && !(me->modifiers() == Qt::CTRL))
    {
      if ((newX == x) && (newY == y))
      {
        selectPickedItems(newX, newY, me->modifiers() == Qt::SHIFT);
      }
    }
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
void View3D::updateViewActions()
{
  bool canTakeSnapshot = false;
  bool canBeExported = false;

//   for(auto renderer: m_renderers)
//   {
//     if(renderer->isHidden())
//       continue;
//
//     canTakeSnapshot |= (renderer->numberOfRenderedItems() != 0);
//     canBeExported |= canTakeSnapshot && (renderer->numberOfvtkActors() != 0);
//   }
//
//   m_zoom->setEnabled(canTakeSnapshot);
//   m_snapshot->setEnabled(canTakeSnapshot);
//   m_export->setEnabled(canBeExported);
//
//   if (0 != numEnabledRenderersForViewItem(RenderableType::SEGMENTATION))
//   {
//     for(auto widget: m_widgets)
//       if (widget->manipulatesSegmentations())
//         widget->setEnabled(true);
//   }
//
//   if(m_numEnabledRenderers == 0)
//     resetCamera();
//
//   m_numEnabledRenderers = 0;
//   bool channelRendererIsEnabled = false;
//
//   for(RendererSPtr render: m_renderers)
//   {
//     if(!render->isHidden())
//       ++m_numEnabledRenderers;
//
//     if(m_showCrosshairPlaneSelectors)
//     {
//       auto channelRenderer = std::dynamic_pointer_cast<ChannelRenderer>(render);
//       if(channelRenderer != nullptr)
//       {
//         channelRendererIsEnabled |= !channelRenderer->isHidden();
//         m_axialScrollBar->setEnabled(channelRendererIsEnabled);
//         m_coronalScrollBar->setEnabled(channelRendererIsEnabled);
//         m_sagittalScrollBar->setEnabled(channelRendererIsEnabled);
//         updateScrollBarsLimits();
//       }
//     }
//   }
}

//-----------------------------------------------------------------------------
void View3D::scrollBarMoved(int value)
{
  NmVector3 point;

  point[0] = m_axialScrollBar   ->value() * m_sceneResolution[0];
  point[1] = m_coronalScrollBar ->value() * m_sceneResolution[1];
  point[2] = m_sagittalScrollBar->value() * m_sceneResolution[2];

  emit crosshairChanged(point);
}

//-----------------------------------------------------------------------------
void View3D::updateScrollBarsLimits()
{
  if(!m_showCrosshairPlaneSelectors) return;

  if (m_channelSources->isEmpty())
  {
    m_axialScrollBar   ->setMinimum(0);
    m_axialScrollBar   ->setMaximum(0);
    m_coronalScrollBar ->setMinimum(0);
    m_coronalScrollBar ->setMaximum(0);
    m_sagittalScrollBar->setMinimum(0);
    m_sagittalScrollBar->setMaximum(0);

    return;
  }

  m_axialScrollBar   ->setMinimum(vtkMath::Round(m_sceneBounds[0]/m_sceneResolution[0]));
  m_axialScrollBar   ->setMaximum(vtkMath::Round(m_sceneBounds[1]/m_sceneResolution[0])-1);
  m_coronalScrollBar ->setMinimum(vtkMath::Round(m_sceneBounds[2]/m_sceneResolution[1]));
  m_coronalScrollBar ->setMaximum(vtkMath::Round(m_sceneBounds[3]/m_sceneResolution[1])-1);
  m_sagittalScrollBar->setMinimum(vtkMath::Round(m_sceneBounds[4]/m_sceneResolution[2]));
  m_sagittalScrollBar->setMaximum(vtkMath::Round(m_sceneBounds[5]/m_sceneResolution[2])-1);
}

//-----------------------------------------------------------------------------
void View3D::setVisualState(struct RenderView::VisualState state)
{
  if (state.plane != Plane::UNDEFINED)
    return;

  auto camera = m_renderer->GetActiveCamera();
  camera->SetPosition(state.cameraPosition[0], state.cameraPosition[1], state.cameraPosition[2]);
  camera->SetFocalPoint(state.focalPoint[0], state.focalPoint[1], state.focalPoint[2]);
  m_renderer->ResetCameraClippingRange();

  updateView();
}

//-----------------------------------------------------------------------------
struct RenderView::VisualState View3D::visualState()
{
  struct RenderView::VisualState state;
  double cameraPos[3], focalPoint[3];
  auto camera = m_renderer->GetActiveCamera();
  camera->GetFocalPoint(focalPoint);
  camera->GetPosition(cameraPos);

  state.plane = Plane::UNDEFINED;
  state.slice = -1;
  state.cameraPosition = NmVector3{cameraPos[0], cameraPos[1], cameraPos[2]};
  state.focalPoint = NmVector3{focalPoint[0], focalPoint[1], focalPoint[2]};
  state.heightLength = -1;

  return state;
}