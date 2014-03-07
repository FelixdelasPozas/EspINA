/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#include "View3D.h"

// EspINA
#include "GUI/View/Widgets/EspinaWidget.h"
#include "ViewRendererMenu.h"

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


using namespace EspINA;

//-----------------------------------------------------------------------------
View3D::View3D(bool showCrosshairPlaneSelectors, QWidget* parent)
: RenderView{parent}
, m_mainLayout{new QVBoxLayout()}
, m_controlLayout{new QHBoxLayout()}
, m_showCrosshairPlaneSelectors{showCrosshairPlaneSelectors}
, m_numEnabledRenderers{0}
{
  setupUI();
}

//-----------------------------------------------------------------------------
View3D::~View3D()
{
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying Volume View";
//   qDebug() << "********************************************************";

  // Representation destructors may need to access slice view in their destructors
  m_channelStates.clear();
  m_segmentationStates.clear();
}

//-----------------------------------------------------------------------------
void View3D::setRenderers(RendererSList renderers)
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
    if (!canRender(renderer, RendererType::RENDERER_VIEW3D))
      continue;

    if (!oldRenderersNames.contains(renderer->name()))
      addRendererControls(renderer->clone());
  }
}

//-----------------------------------------------------------------------------
void View3D::reset()
{
  for(auto widget: m_widgets.keys())
    removeWidget(widget);

  for(auto segmentation: m_segmentationStates.keys())
    remove(segmentation);

  // After removing segmentations there should be only channels
  for(auto channel: m_channelStates.keys())
    remove(channel);

  Q_ASSERT(m_channelStates.isEmpty());
  Q_ASSERT(m_segmentationStates.isEmpty());
  Q_ASSERT(m_widgets.isEmpty());

  updateRenderersControls();
}


//-----------------------------------------------------------------------------
void View3D::addRendererControls(RendererSPtr renderer)
{
  if (m_renderers.contains(renderer) || (renderer->renderType() != RendererTypes(RENDERER_VIEW3D)))
    return;

  m_renderers << renderer;

  renderer->setView(this);
  renderer->setEnable(false);

  // add segmentation representations to renderer
  for(auto segmentation : m_segmentationStates.keys())
    if(renderer->type() == Renderer::Type::Representation)
    {
      auto repRenderer = representationRenderer(renderer);
      if (repRenderer->canRender(segmentation))
        for(auto rep : m_segmentationStates[segmentation].representations)
           if (repRenderer->managesRepresentation(rep))
             repRenderer->addRepresentation(segmentation, rep);
    }

  // add channel representations to renderer
  for(auto channel : m_channelStates.keys())
    if(renderer->type() == Renderer::Type::Representation)
    {
      auto repRenderer = representationRenderer(renderer);
      if (repRenderer->canRender(channel))
        for(auto rep : m_channelStates[channel].representations)
          if (repRenderer->managesRepresentation(rep))
            repRenderer->addRepresentation(channel, rep);
    }

  ViewRendererMenu *configMenu = qobject_cast<ViewRendererMenu*>(m_renderConfig.menu());
  if (configMenu == nullptr)
  {
    configMenu = new ViewRendererMenu(&m_renderConfig);
    m_renderConfig.setMenu(configMenu);
    m_renderConfig.setEnabled(true);
  }
  configMenu->add(renderer);

  connect(renderer.get(), SIGNAL(renderRequested()), this, SLOT(updateRenderersControls()), Qt::QueuedConnection);
  connect(renderer.get(), SIGNAL(renderRequested()), this, SLOT(updateView()), Qt::QueuedConnection);

  updateRenderersControls();
}

//-----------------------------------------------------------------------------
void View3D::removeRendererControls(const QString name)
{
  RendererSPtr removedRenderer;
  for(auto renderer: m_renderers)
    if (renderer->name() == name)
    {
      removedRenderer = renderer;
      break;
    }

  if (!removedRenderer)
    return;

  m_renderers.removeOne(removedRenderer);

  if (!removedRenderer->isHidden())
    removedRenderer->setEnable(false);

  ViewRendererMenu *configMenu = qobject_cast<ViewRendererMenu*>(m_renderConfig.menu());
  if (configMenu != nullptr)
  {
    configMenu->remove(removedRenderer);
    if (configMenu->actions().isEmpty())
    {
      m_renderConfig.setMenu(nullptr);
      delete configMenu;
      m_renderConfig.setEnabled(false);
    }
  }
  disconnect(removedRenderer.get(), SIGNAL(renderRequested()), this, SLOT(updateView()));

  updateRenderersControls();
}

//-----------------------------------------------------------------------------
void View3D::buildControls()
{
  m_controlLayout = new QHBoxLayout();
  m_controlLayout->addStretch();

  m_zoom.setIcon(QIcon(":/espina/zoom_reset.png"));
  m_zoom.setToolTip(tr("Reset Camera"));
  m_zoom.setFlat(true);
  m_zoom.setIconSize(QSize(22,22));
  m_zoom.setMaximumSize(QSize(32,32));
  m_zoom.setCheckable(false);
  connect(&m_zoom, SIGNAL(clicked()), this, SLOT(resetView()));

  m_snapshot.setIcon(QIcon(":/espina/snapshot_scene.svg"));
  m_snapshot.setToolTip(tr("Save Scene as Image"));
  m_snapshot.setFlat(true);
  m_snapshot.setIconSize(QSize(22,22));
  m_snapshot.setMaximumSize(QSize(32,32));
  m_snapshot.setEnabled(false);
  connect(&m_snapshot,SIGNAL(clicked(bool)),this,SLOT(onTakeSnapshot()));

  m_export.setIcon(QIcon(":/espina/export_scene.svg"));
  m_export.setToolTip(tr("Export 3D Scene"));
  m_export.setFlat(true);
  m_export.setIconSize(QSize(22,22));
  m_export.setMaximumSize(QSize(32,32));
  m_export.setEnabled(false);
  connect(&m_export,SIGNAL(clicked(bool)),this,SLOT(exportScene()));

  m_renderConfig.setIcon(QIcon(":/espina/settings.png"));
  m_renderConfig.setToolTip(tr("Configure this view's renderers"));
  m_renderConfig.setFlat(true);
  m_renderConfig.setIconSize(QSize(22,22));
  m_renderConfig.setMaximumSize(QSize(32,32));
  m_renderConfig.setEnabled(false);

  QSpacerItem * horizontalSpacer = new QSpacerItem(4000, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

  m_controlLayout->addWidget(&m_zoom);
  m_controlLayout->addWidget(&m_snapshot);
  m_controlLayout->addWidget(&m_export);
  m_controlLayout->addItem(horizontalSpacer);
  m_controlLayout->addWidget(&m_renderConfig);

  for(auto renderer : m_renderers)
    if (canRender(renderer, RendererType::RENDERER_VIEW3D))
      this->addRendererControls(renderer->clone());

  m_mainLayout->addLayout(m_controlLayout);
}

//-----------------------------------------------------------------------------
void View3D::centerViewOn(const NmVector3& point, bool force)
{
  if (!isVisible() ||
      (m_center[0] == point[0] &&
       m_center[1] == point[1] &&
       m_center[2] == point[2]))
    return;

  m_center = point;

  bool updated = false;

  for(auto renderer: m_renderers)
  {
    auto channelRenderer = dynamic_cast<ChannelRenderer *>(renderer.get());
    if (channelRenderer)
      channelRenderer->setCrosshair(m_center);

    updated |= !renderer->isHidden() && (renderer->numberOfRenderedItems() != 0);
  }

  if (m_showCrosshairPlaneSelectors && !m_channelStates.empty())
  {
    NmVector3 minSpacing = m_channelStates.begin().key()->output()->spacing();

    foreach(ChannelAdapterPtr channel, m_channelStates.keys())
    {
      NmVector3 spacing;
      spacing = channel->output()->spacing();
      if (spacing[0] < minSpacing[0])
        minSpacing[0] = spacing[0];

      if (spacing[1] < minSpacing[1])
        minSpacing[1] = spacing[1];

      if (spacing[2] < minSpacing[2])
        minSpacing[2] = spacing[2];
    }

    int iCenter[3] = { vtkMath::Round(point[0]/minSpacing[0]), vtkMath::Round(point[1]/minSpacing[1]), vtkMath::Round(point[2]/minSpacing[2]) };
    m_axialScrollBar->blockSignals(true);
    m_coronalScrollBar->blockSignals(true);
    m_sagittalScrollBar->blockSignals(true);
    m_axialScrollBar->setValue(iCenter[0]);
    m_coronalScrollBar->setValue(iCenter[1]);
    m_sagittalScrollBar->setValue(iCenter[2]);
    m_axialScrollBar->blockSignals(false);
    m_coronalScrollBar->blockSignals(false);
    m_sagittalScrollBar->blockSignals(false);
  }

  if (updated)
  {
    setCameraFocus(point);
    updateView();
  }
}

//-----------------------------------------------------------------------------
void View3D::setCameraFocus(const NmVector3& center)
{
  m_renderer->GetActiveCamera()->SetFocalPoint(center[0],center[1],center[2]);
  m_renderer->ResetCameraClippingRange();
}

//-----------------------------------------------------------------------------
void View3D::resetCamera()
{
  this->m_renderer->GetActiveCamera()->SetViewUp(0,1,0);
  this->m_renderer->GetActiveCamera()->SetPosition(0,0,-1);
  this->m_renderer->GetActiveCamera()->SetFocalPoint(0,0,0);
  this->m_renderer->GetActiveCamera()->SetRoll(180);
  this->m_renderer->ResetCamera();
}

//-----------------------------------------------------------------------------
void View3D::add(ChannelAdapterPtr channel)
{
  RenderView::add(channel);

  updateRenderersControls();
  updateScrollBarsLimits();
}

//-----------------------------------------------------------------------------
void View3D::remove(ChannelAdapterPtr channel)
{
  RenderView::remove(channel);

  updateRenderersControls();
  updateScrollBarsLimits();
}

//-----------------------------------------------------------------------------
bool View3D::updateRepresentation(ChannelAdapterPtr channel, bool render)
{
  bool returnVal = RenderView::updateRepresentation(channel, render);

  updateRenderersControls();
  return returnVal;
}

//-----------------------------------------------------------------------------
bool View3D::updateRepresentation(SegmentationAdapterPtr seg, bool render)
{
  bool returnVal = RenderView::updateRepresentation(seg, render);

  updateRenderersControls();
  return returnVal;
}


//-----------------------------------------------------------------------------
void View3D::addWidget(EspinaWidget* eWidget)
{
  if(m_widgets.contains(eWidget))
    return;

  vtkAbstractWidget *widget = eWidget->create3DWidget(this);
  if (!widget)
    return;

  widget->SetCurrentRenderer(this->m_renderer);
  widget->SetInteractor(m_view->GetInteractor());

  bool activate = (numEnabledRenderersForViewItem(RenderableType::SEGMENTATION) != 0);
  if (eWidget->manipulatesSegmentations())
  {
    widget->SetEnabled(activate);
    if (widget->GetRepresentation())
      widget->GetRepresentation()->SetVisibility(activate);
  }
  else
  {
    widget->SetEnabled(true);
    if (widget->GetRepresentation())
      widget->GetRepresentation()->SetVisibility(true);
  }

  m_renderer->ResetCameraClippingRange();
  m_widgets[eWidget] = widget;

  updateRenderersControls();
}

//-----------------------------------------------------------------------------
void View3D::removeWidget(EspinaWidget* eWidget)
{
  if (!m_widgets.contains(eWidget))
    return;

  vtkAbstractWidget *widget = m_widgets[eWidget];
  widget->Off();
  widget->SetInteractor(nullptr);
  widget->RemoveAllObservers();
  m_widgets.remove(eWidget);

  updateRenderersControls();
}

//-----------------------------------------------------------------------------
Bounds View3D::previewBounds(bool cropToSceneBounds) const
{
  Bounds resultBounds;
  for (auto channel: m_channelStates.keys())
  {
    if (!resultBounds.areValid())
      resultBounds = channel->bounds();
    else
      resultBounds = boundingBox(resultBounds, channel->bounds());
  }

  return resultBounds;
}

//-----------------------------------------------------------------------------
Selector::SelectionList View3D::pick(Selector::SelectionFlags    filter,
                                     Selector::DisplayRegionList regions)
{
  return Selector::SelectionList();
}

//-----------------------------------------------------------------------------
void View3D::setupUI()
{
  if (m_showCrosshairPlaneSelectors)
  {
    m_additionalGUI = new QHBoxLayout();
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
  vtkSmartPointer<vtkInteractorStyleTrackballCamera> interactorstyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
  interactorstyle->AutoAdjustCameraClippingRangeOn();
  interactorstyle->KeyPressActivationOff();
  m_view->GetRenderWindow()->AddRenderer(m_renderer);
  m_view->GetRenderWindow()->GetInteractor()->SetInteractorStyle(interactorstyle);
  m_view->GetRenderWindow()->Render();
  m_view->installEventFilter(this);

  buildControls();

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
    this->m_view->GetRenderWindow()->Render();
    this->m_view->update();
  }
}

//-----------------------------------------------------------------------------
void View3D::selectPickedItems(int vx, int vy, bool append)
{
  ViewItemAdapterList selection, pickedItems;
  if (append)
    selection = currentSelection()->items();

  // If no append, segmentations have priority over channels
  for(auto renderer : m_renderers)
  {
    if(renderer->type() == Renderer::Type::Representation)
    {
      auto repRenderer = representationRenderer(renderer);
      if (!repRenderer->isHidden() && canRender(repRenderer, RenderableType::SEGMENTATION))
      {
        pickedItems = repRenderer->pick(vx, vy, 0, m_renderer, RenderableItems(RenderableType::SEGMENTATION), append);
        if (!pickedItems.empty())
        {
          for(ViewItemAdapterPtr item : pickedItems)
            if (!selection.contains(item))
              selection << item;
            else
              selection.removeAll(item);
        }
      }
    }
  }

  pickedItems.clear();

  for(auto renderer : m_renderers)
  {
    if(renderer->type() == Renderer::Type::Representation)
    {
      auto repRenderer = representationRenderer(renderer);
      if (!repRenderer->isHidden() && canRender(repRenderer, RenderableType::CHANNEL))
      {
        pickedItems = repRenderer->pick(vx, vy, 0, m_renderer, RenderableItems(RenderableType::CHANNEL), append);
        if (!pickedItems.empty())
        {
          for(ViewItemAdapterPtr item : pickedItems)
            if (!selection.contains(item))
              selection << item;
            else
              selection.removeAll(item);
        }
      }
    }
  }

  if (!append && !selection.empty())
  {
    ViewItemAdapterPtr returnItem = selection.first();
    selection.clear();
    selection << returnItem;
  }

  currentSelection()->set(selection);
}

//-----------------------------------------------------------------------------
bool View3D::eventFilter(QObject* caller, QEvent* e)
{
  // there is not a "singleclick" event so we need to remember the position of the
  // press event and compare it with the position of the release event (for picking).
  static int x = -1;
  static int y = -1;

  int newX, newY;
  eventPosition(newX, newY);

  if (e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent *me = static_cast<QMouseEvent*>(e);
    if (me->button() == Qt::LeftButton)
    {
      if (me->modifiers() == Qt::CTRL)
      {
        for(auto renderer: m_renderers)
        {
          if(renderer->type() == Renderer::Type::Representation)
          {
            auto repRenderer = representationRenderer(renderer);
            if (!repRenderer->isHidden())
            {
              auto selection = repRenderer->pick(newX, newY, 0, m_renderer, RenderableItems(RenderableType::SEGMENTATION|RenderableType::CHANNEL), false);
              if (!selection.empty())
              {
                NmVector3 point = repRenderer->pickCoordinates();

                emit centerChanged(point);
                break;
              }
            }
          }
        }
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
    QMouseEvent *me = static_cast<QMouseEvent*>(e);
    if ((me->button() == Qt::LeftButton) && !(me->modifiers() == Qt::CTRL))
      if ((newX == x) && (newY == y))
        selectPickedItems(newX, newY, me->modifiers() == Qt::SHIFT);
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
  takeSnapshot(m_renderer);
}

//-----------------------------------------------------------------------------
void View3D::changePlanePosition(Plane plane, Nm dist)
{
  bool needUpdate = false;

  for(auto renderer: m_renderers)
  {
    auto channelRenderer = dynamic_cast<ChannelRenderer *>(renderer.get());
    if (channelRenderer)
    {
      channelRenderer->setPlanePosition(plane, dist);
      needUpdate = !renderer->isHidden() && (renderer->numberOfRenderedItems() != 0);
    }
  }

  if (needUpdate)
    updateView();
}

//-----------------------------------------------------------------------------
void View3D::updateRenderersControls()
{
  bool canTakeSnapshot = false;
  bool canBeExported = false;

  for(auto renderer: m_renderers)
  {
    bool canRenderItems = renderer->numberOfRenderedItems() != 0;

    canTakeSnapshot |= (!renderer->isHidden()) && canRenderItems;
    canBeExported |= canTakeSnapshot && (renderer->numberOfvtkActors() != 0);
  }

  m_snapshot.setEnabled(canTakeSnapshot);
  m_export.setEnabled(canBeExported);

  if (0 != numEnabledRenderersForViewItem(RenderableType::SEGMENTATION))
  {
    for(auto it = m_widgets.begin(); it != m_widgets.end(); ++it)
    {
      if (it.key()->manipulatesSegmentations())
      {
        it.value()->SetEnabled(true);
        it.value()->GetRepresentation()->SetVisibility(true);
      }
    }
  }

  if(m_numEnabledRenderers == 0)
    resetCamera();

  m_numEnabledRenderers = 0;
  for(RendererSPtr render: m_renderers)
    if(!render->isHidden())
      ++m_numEnabledRenderers;
}

//-----------------------------------------------------------------------------
void View3D::scrollBarMoved(int value)
{
  NmVector3 point;

  Q_ASSERT(!m_channelStates.isEmpty());
  NmVector3 minSpacing = m_channelStates.keys().first()->output()->spacing();

  for(auto channel: m_channelStates.keys())
  {
    NmVector3 spacing = channel->output()->spacing();
    minSpacing[0] = std::min(minSpacing[0], spacing[0]);
    minSpacing[1] = std::min(minSpacing[1], spacing[1]);
    minSpacing[2] = std::min(minSpacing[2], spacing[2]);
  }

  point[0] = m_axialScrollBar   ->value() * minSpacing[0];
  point[1] = m_coronalScrollBar ->value() * minSpacing[1];
  point[2] = m_sagittalScrollBar->value() * minSpacing[2];

  bool needUpdate = false;
  for(auto renderer: m_renderers)
  {
    auto channelRenderer = dynamic_cast<ChannelRenderer *>(renderer.get());
    if (channelRenderer)
    {
      channelRenderer->setCrosshair(point);
      needUpdate = !renderer->isHidden() && (renderer->numberOfRenderedItems() != 0);
    }
  }

  if (needUpdate)
    m_view->update();
}

//-----------------------------------------------------------------------------
void View3D::updateScrollBarsLimits()
{
  if (!m_showCrosshairPlaneSelectors)
    return;

  if(m_channelStates.isEmpty())
  {
    m_axialScrollBar   ->setMinimum(0);
    m_axialScrollBar   ->setMaximum(0);
    m_coronalScrollBar ->setMinimum(0);
    m_coronalScrollBar ->setMaximum(0);
    m_sagittalScrollBar->setMinimum(0);
    m_sagittalScrollBar->setMaximum(0);
    return;
  }

  // scrollbars deals with ints for their limits, so we need to use 'extent'
  Bounds maxBounds = previewBounds();
  auto minSpacing = m_channelStates.keys().first()->output()->spacing();

  for(auto channel: m_channelStates.keys())
  {
    auto spacing = channel->output()->spacing();
    for(auto i: { 0,1,2 })
      minSpacing[i] = std::min(minSpacing[i], spacing[i]);
  }

  m_axialScrollBar   ->setMinimum(vtkMath::Round(maxBounds[0]/minSpacing[0]));
  m_axialScrollBar   ->setMaximum(vtkMath::Round(maxBounds[1]/minSpacing[0]));
  m_coronalScrollBar ->setMinimum(vtkMath::Round(maxBounds[2]/minSpacing[1]));
  m_coronalScrollBar ->setMaximum(vtkMath::Round(maxBounds[3]/minSpacing[1]));
  m_sagittalScrollBar->setMinimum(vtkMath::Round(maxBounds[4]/minSpacing[2]));
  m_sagittalScrollBar->setMaximum(vtkMath::Round(maxBounds[5]/minSpacing[2]));
}

//-----------------------------------------------------------------------------
RepresentationSPtr View3D::cloneRepresentation(ViewItemAdapterPtr item, Representation::Type representation)
{
  RepresentationSPtr prototype = item->representation(representation);
  RepresentationSPtr rep;

  if (prototype && prototype->canRenderOnView().testFlag(Representation::RENDERABLEVIEW_VOLUME))
    rep = prototype->clone(this);

  return rep;
}

//-----------------------------------------------------------------------------
RendererSList View3D::renderers() const
{
  return m_renderers;
}

//-----------------------------------------------------------------------------
void View3D::activateRender(const QString &rendererName)
{
  for(auto action: m_renderConfig.menu()->actions())
    if (action->text() == rendererName)
      action->setChecked(true);

  for(auto renderer: m_renderers)
    if (renderer->name() == rendererName)
      renderer->setEnable(true);
}

//-----------------------------------------------------------------------------
void View3D::deactivateRender(const QString &rendererName)
{
  for(auto action: m_renderConfig.menu()->actions())
    if (action->text() == rendererName)
      action->setChecked(false);

  for(auto renderer: m_renderers)
    if (renderer->name() == rendererName)
      renderer->setEnable(false);
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

