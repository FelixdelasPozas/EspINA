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

// Qt
#include <QApplication>
#include <QEvent>
#include <QSettings>
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
, m_showCrosshairPlaneSelectors(showCrosshairPlaneSelectors)
{
  setupUI();

  setLayout(m_mainLayout);

//TODO 2013-10-05   connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection, bool)),
//           this, SLOT(updateSelection(ViewManager::Selection,bool)));
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

void View3D::setRenderers(RendererSList renderers)
{
  QStringList   activeRenderersNames;
  RendererSList activeRenderers = m_renderers.values();

  // remove controls for unused renderers // TODO 2013-10-05
//   foreach(RendererSPtr oldRenderer, m_renderers)
//   {
//     bool selected = false;
//     int i = 0;
//     while (!selected && i < values.size())
//       selected = values[i++]->name() == oldRenderer->name();
// 
//     if (!selected)
//       parent->removeRendererControls(oldRenderer->name());
//     else
//       activeRenderers << oldRenderer;
//   }

  // add controls for added renderers
  foreach(RendererSPtr renderer, renderers)
  {
    if (!canRender(renderer, RendererType::RENDERER_VOLUMEVIEW))
      continue;

    activeRenderersNames << renderer->name();
    if (!activeRenderers.contains(renderer))
    {
      activeRenderers << renderer;
      addRendererControls(renderer->clone());
    }
  }
}

//-----------------------------------------------------------------------------
void View3D::reset()
{
  foreach(EspinaWidget *widget, m_widgets.keys())
  {
    removeWidget(widget);
  }

  foreach(SegmentationAdapterPtr segmentation, m_segmentationStates.keys())
  {
    removeSegmentation(segmentation);
  }

  // After removing segmentations there should be only channels
  foreach(ChannelAdapterPtr channel, m_channelStates.keys())
  {
    removeChannel(channel);
  }

  Q_ASSERT(m_channelStates.isEmpty());
  Q_ASSERT(m_segmentationStates.isEmpty());
  Q_ASSERT(m_widgets.isEmpty());

  QMap<QPushButton *, RendererSPtr>::iterator it = m_renderers.begin();
  while (it != m_renderers.end())
  {
    it.key()->setChecked(false);
    ++it;
  }

  m_numEnabledChannelRenders = 0;
  m_numEnabledSegmentationRenders = 0;
}

//-----------------------------------------------------------------------------
void View3D::addRendererControls(RendererSPtr renderer)
{
  QPushButton *button;

  button = new QPushButton(renderer->icon(), "");
  button->setFlat(true);
  button->setCheckable(true);
  button->setChecked(false);
  button->setIconSize(QSize(22, 22));
  button->setMaximumSize(QSize(32, 32));
  button->setToolTip(renderer.get()->tooltip());
  button->setObjectName(renderer.get()->name());
  connect(button, SIGNAL(toggled(bool)), renderer.get(), SLOT(setEnable(bool)));
  connect(button, SIGNAL(toggled(bool)), this, SLOT(updateEnabledRenderersCount(bool)));
  connect(button, SIGNAL(destroyed(QObject*)), renderer.get(), SLOT(deleteLater()));
  connect(renderer.get(), SIGNAL(renderRequested()), this, SLOT(updateView()));
  m_controlLayout->addWidget(button);
  renderer->setView(this);

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
    QMap<EspinaWidget *, vtkAbstractWidget *>::const_iterator it = m_widgets.begin();
    for( ; it != m_widgets.end(); ++it)
    {
      if (it.key()->manipulatesSegmentations())
      {
        it.value()->SetEnabled(true);
        it.value()->GetRepresentation()->SetVisibility(true);
      }
    }
  }

  updateRenderersControls();
}

//-----------------------------------------------------------------------------
void View3D::removeRendererControls(const QString name)
{
  for (int i = 0; i < m_controlLayout->count(); i++)
  {
    if (m_controlLayout->itemAt(i)->isEmpty())
      continue;

    if (m_controlLayout->itemAt(i)->widget()->objectName() == name)
    {
      QWidget *button = m_controlLayout->itemAt(i)->widget();
      m_controlLayout->removeWidget(button);
      delete button;
    }
  }

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

    if (!removedRenderer->isHidden() && canRender(removedRenderer, RenderableType::SEGMENTATION))
      this->m_numEnabledSegmentationRenders--;

    if (!removedRenderer->isHidden() && canRender(removedRenderer, RenderableType::CHANNEL))
      this->m_numEnabledChannelRenders--;

    if (0 == m_numEnabledSegmentationRenders)
    {
      QMap<EspinaWidget *, vtkAbstractWidget *>::const_iterator it = m_widgets.begin();
      for( ; it != m_widgets.end(); ++it)
      {
        if (it.key()->manipulatesSegmentations())
        {
          it.value()->SetEnabled(false);
          it.value()->GetRepresentation()->SetVisibility(false);
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

  QSpacerItem * horizontalSpacer = new QSpacerItem(4000, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

  m_controlLayout->addWidget(&m_zoom);
  m_controlLayout->addWidget(&m_snapshot);
  m_controlLayout->addWidget(&m_export);
  m_controlLayout->addItem(horizontalSpacer);

  foreach(RendererSPtr renderer, m_renderers)
  {
    if (canRender(renderer, RendererType::RENDERER_VOLUMEVIEW))
    {
      this->addRendererControls(renderer->clone());
    }
  }

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
  //TODO: 2013-10-05 Find another solution
//   foreach(RendererSPtr ren, m_renderers)
//   {
//     if (QString("Crosshairs") == ren->name())
//     {
//       CrosshairRenderer *crossren = reinterpret_cast<CrosshairRenderer *>(ren.get());
//       crossren->setCrosshair(center);
//     }
// 
//     updated |= !ren->isHidden() && (ren->itemsBeenRendered() != 0);
//   }

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
void View3D::addChannel(ChannelAdapterPtr channel)
{
  RenderView::addChannel(channel);

  updateRenderersControls();
  updateScrollBarsLimits();
}

//-----------------------------------------------------------------------------
void View3D::removeChannel(ChannelAdapterPtr channel)
{
  RenderView::removeChannel(channel);

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

  bool activate = (m_numEnabledSegmentationRenders != 0);
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

  SelectableView::Selection selection, pickedItems;
  if (append)
    selection = currentSelection();

  // If no append, segmentations have priority over channels
  foreach(RendererSPtr renderer, m_renderers)
  {
    if (!renderer->isHidden() && canRender(renderer, RenderableType::SEGMENTATION))
    {
      pickedItems = renderer->pick(vx, vy, 0, m_renderer, RenderableItems(RenderableType::SEGMENTATION), append);
      if (!pickedItems.empty())
      {
        foreach(ViewItemAdapterPtr item, pickedItems)
          if (!selection.contains(item))
            selection << item;
          else
            selection.removeAll(item);
      }
    }
  }

  pickedItems.clear();

  foreach(RendererSPtr renderer, m_renderers)
  {
    if (!renderer->isHidden() && canRender(renderer, RenderableType::CHANNEL))
    {
      pickedItems = renderer->pick(vx, vy, 0, m_renderer, RenderableItems(RenderableType::CHANNEL), append);
      if (!pickedItems.empty())
      {
        foreach(ViewItemAdapterPtr item, pickedItems)
          if (!selection.contains(item))
            selection << item;
          else
            selection.removeAll(item);
      }
    }
  }

  if (!append && !selection.empty())
  {
    ViewItemAdapterPtr returnItem = selection.first();
    selection.clear();
    selection << returnItem;
  }

  emit selectionChanged(selection);
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
        foreach(RendererSPtr renderer, m_renderers)
        {
          if (!renderer->isHidden())
          {
            SelectableView::Selection selection = renderer->pick(newX, newY, 0, m_renderer, RenderableItems(RenderableType::SEGMENTATION|RenderableType::CHANNEL), false);
            if (!selection.empty())
            {
              NmVector3 point = renderer->pickCoordinates();

              emit centerChanged(point);
              break;
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
  //TODO: 2013-10-05 More Find another method
//   foreach(RendererSPtr ren, m_renderers)
//   {
//     if (QString("Crosshairs") == ren->name())
//     {
//       CrosshairRenderer *crossren = reinterpret_cast<CrosshairRenderer *>(ren.get());
//       crossren->setPlanePosition(plane, dist);
//       needUpdate = !crossren->isHidden() && (crossren->itemsBeenRendered() != 0);
//     }
//   }

  if (needUpdate)
    updateView();
}

//-----------------------------------------------------------------------------
void View3D::updateEnabledRenderersCount(bool value)
{
  int updateValue = (value ? +1 : -1);

  // reset camera if is the first renderer the user activates
  if ((true == value) && (0 == (m_numEnabledChannelRenders + m_numEnabledSegmentationRenders)))
  {
    m_renderer->ResetCamera();
    updateView();
  }

  // get enabled/disabled renderer that triggered the signal
  RendererSPtr renderer;
  QPushButton *button = dynamic_cast<QPushButton*>(sender());
  if (button)
    renderer = m_renderers[button];

  if (renderer && canRender(renderer, RenderableType::CHANNEL))
  {
    m_numEnabledChannelRenders += updateValue;
    if (m_showCrosshairPlaneSelectors)
    {
      m_axialScrollBar->setEnabled(value);
      m_coronalScrollBar->setEnabled(value);
      m_sagittalScrollBar->setEnabled(value);
    }
  }

  if (renderer && canRender(renderer, RenderableType::SEGMENTATION))
  {
    m_numEnabledSegmentationRenders += updateValue;
    if (0 != m_numEnabledSegmentationRenders)
    {
      QMap<EspinaWidget *, vtkAbstractWidget *>::const_iterator it = m_widgets.begin();
      for( ; it != m_widgets.end(); ++it)
      {
        if (it.key()->manipulatesSegmentations())
        {
          it.value()->SetEnabled(value);
          it.value()->GetRepresentation()->SetVisibility(value);
        }
      }
    }
  }

  updateRenderersControls();
}

//-----------------------------------------------------------------------------
void View3D::updateRenderersControls()
{
  bool canTakeSnapshot = false;
  bool canBeExported = false;
  QMap<QPushButton *, RendererSPtr>::iterator it;
  for(it = m_renderers.begin(); it != m_renderers.end(); ++it)
  {
    bool canRenderItems = it.value()->numberOfRenderedItems() != 0;

    canTakeSnapshot |= (!it.value()->isHidden()) && canRenderItems;
    canBeExported |= canTakeSnapshot && (it.value()->numberOfvtkActors() != 0);

    it.key()->setEnabled(canRenderItems);
    if (!canRenderItems)
      it.key()->setChecked(false);
  }

  m_snapshot.setEnabled(canTakeSnapshot);
  m_export.setEnabled(canBeExported);
}

//-----------------------------------------------------------------------------
void View3D::scrollBarMoved(int value)
{
  NmVector3 point;

  Q_ASSERT(!m_channelStates.isEmpty());
  NmVector3 minSpacing = m_channelStates.keys().first()->output()->spacing();

  foreach(ChannelAdapterPtr channel, m_channelStates.keys())
  {
    NmVector3 spacing = channel->output()->spacing();
    if (spacing[0] < minSpacing[0])
      minSpacing[0] = spacing[0];

    if (spacing[1] < minSpacing[1])
      minSpacing[1] = spacing[1];

    if (spacing[2] < minSpacing[2])
      minSpacing[2] = spacing[2];
  }

  point[0] = m_axialScrollBar   ->value() * minSpacing[0];
  point[1] = m_coronalScrollBar ->value() * minSpacing[1];
  point[2] = m_sagittalScrollBar->value() * minSpacing[2];

  // TODO 2013-10-05 Find another method
//   foreach(RendererSPtr renderer, m_renderers)
//   {
//     if (canRender(renderer, RenderableType::CHANNEL))
//     {
//       CrosshairRenderer *crossRender = dynamic_cast<CrosshairRenderer *>(renderer.get());
//       if (crossRender != nullptr)
//         crossRender->setCrosshair(point);
//     }
//   }

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

  // TODO: Review this, before it was extent
  Bounds maxExtent = m_channelStates.keys().first()->bounds();

  foreach (ChannelAdapterPtr channel, m_channelStates.keys())
  {
    Bounds extent = channel->bounds();
    for (int i = 0, j = 1; i < 6; i += 2, j += 2)
    {
      maxExtent[i] = std::min(extent[i], maxExtent[i]);
      maxExtent[j] = std::max(extent[j], maxExtent[j]);
    }
  }

  m_axialScrollBar   ->setMinimum(maxExtent[0]);
  m_axialScrollBar   ->setMaximum(maxExtent[1]);
  m_coronalScrollBar ->setMinimum(maxExtent[2]);
  m_coronalScrollBar ->setMaximum(maxExtent[3]);
  m_sagittalScrollBar->setMinimum(maxExtent[4]);
  m_sagittalScrollBar->setMaximum(maxExtent[5]);
}

//-----------------------------------------------------------------------------
RepresentationSPtr View3D::cloneRepresentation(RepresentationSPtr prototype)
{
  RepresentationSPtr rep = RepresentationSPtr();

  if (prototype->canRenderOnView().testFlag(Representation::RENDERABLEVIEW_VOLUME))
    rep = prototype->clone(this);

  return rep;
}