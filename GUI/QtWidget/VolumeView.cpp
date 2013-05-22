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

#include "VolumeView.h"

// EspINA
#include "GUI/Renderers/CrosshairRenderer.h"
#include <Core/EspinaSettings.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/Channel.h>
#include <Core/Model/EspinaFactory.h>

// QT
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
#include <vtkPropPicker.h>
#include <vtkPolyDataMapper.h>
#include <QVTKWidget.h>
#include <vtkCamera.h>
#include <vtkPOVExporter.h>
#include <vtkVRMLExporter.h>
#include <vtkX3DExporter.h>
#include <vtkPNGWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkRenderLargeImage.h>
#include <QApplication>
#include <vtkAbstractWidget.h>
#include <vtkWidgetRepresentation.h>
#include <vtkMath.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
VolumeView::VolumeView(const EspinaFactory *factory,
                       ViewManager *viewManager,
                       bool additionalScrollBars,
                       QWidget* parent)
: EspinaRenderView      (parent)
, m_viewManager         (viewManager)
, m_mainLayout          (new QVBoxLayout())
, m_controlLayout       (new QHBoxLayout())
, m_additionalScrollBars(additionalScrollBars)
, m_settings            (new Settings(factory, QString(), this))
, m_numEnabledRenders   (0)
, m_numEnabledSegmentationRenders(0)
, m_numEnabledChannelRenders(0)
, m_meshPicker          (vtkSmartPointer<vtkPropPicker>::New())
{
  setupUI();
  buildControls();

  setLayout(m_mainLayout);

  // Color background
  QPalette pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Window));
  this->setPalette(pal);

  memset(m_center,0,3*sizeof(double));
  connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection, bool)),
          this, SLOT(updateSelection(ViewManager::Selection,bool)));

  viewManager->registerView(this);
}

//-----------------------------------------------------------------------------
VolumeView::~VolumeView()
{
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying Volume View";
//   qDebug() << "********************************************************";

  // Representation destructors may need to access slice view in their destructors
  m_channelStates.clear();
  m_segmentationStates.clear();

  m_viewManager->unregisterView(this);
}


//-----------------------------------------------------------------------------
void VolumeView::reset()
{
  foreach(EspinaWidget *widget, m_widgets.keys())
  {
    removeWidget(widget);
  }

  foreach(SegmentationPtr segmentation, m_segmentationStates.keys())
  {
    removeSegmentation(segmentation);
  }

  // After removing segmentations there should be only channels
  foreach(ChannelPtr channel, m_channelStates.keys())
  {
    removeChannel(channel);
  }

  Q_ASSERT(m_channelStates.isEmpty());
  Q_ASSERT(m_segmentationStates.isEmpty());
  Q_ASSERT(m_widgets.isEmpty());

  QMap<QPushButton *, IRendererSPtr>::iterator it = m_renderers.begin();
  while (it != m_renderers.end())
  {
    it.key()->setChecked(false);
    ++it;
  }

  m_numEnabledChannelRenders = 0;
  m_numEnabledSegmentationRenders = 0;
  m_numEnabledRenders = 0;
}

//-----------------------------------------------------------------------------
void VolumeView::addRendererControls(IRendererSPtr renderer)
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
  connect(button, SIGNAL(clicked(bool)), renderer.get(), SLOT(setEnable(bool)));
  connect(button, SIGNAL(clicked(bool)), this, SLOT(countEnabledRenderers(bool)));
  connect(button, SIGNAL(destroyed(QObject*)), renderer.get(), SLOT(deleteLater()));
  connect(renderer.get(), SIGNAL(renderRequested()), this, SLOT(updateView()));
  m_controlLayout->addWidget(button);
  renderer->setVTKRenderer(this->m_renderer);

  // add representations to renderer
  foreach(SegmentationPtr segmentation, m_segmentationStates.keys())
  {
    if (renderer->itemCanBeRendered(segmentation))
      foreach(GraphicalRepresentationSPtr rep, m_segmentationStates[segmentation].representations)
         if (renderer->managesRepresentation(rep)) renderer->addRepresentation(rep);
  }

  foreach(ChannelPtr channel, m_channelStates.keys())
    if (renderer->itemCanBeRendered(channel))
      reinterpret_cast<CrosshairRenderer *>(renderer.get())->addItem(channel);

  m_itemRenderers << renderer;
  m_renderers[button] = renderer;

  if (!renderer->isHidden())
  {
    switch(renderer->getRendererType())
    {
      case IRenderer::SEGMENTATION:
        this->m_numEnabledSegmentationRenders++;
        break;
      case IRenderer::CHANNEL:
        this->m_numEnabledChannelRenders++;
        break;
      default:
        break;
    }
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

  updateRenderersButtons();
}

//-----------------------------------------------------------------------------
void VolumeView::removeRendererControls(const QString name)
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
    if (!removedRenderer->isHidden() && (removedRenderer->getRendererType() & IRenderer::SEGMENTATION))
      this->m_numEnabledSegmentationRenders--;

    if (!removedRenderer->isHidden() && (removedRenderer->getRendererType() & IRenderer::CHANNEL))
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

  updateRenderersButtons();
}

//-----------------------------------------------------------------------------
void VolumeView::buildControls()
{
  m_controlLayout = new QHBoxLayout();
  m_controlLayout->addStretch();

  m_zoom.setIcon(QIcon(":/espina/zoom_reset.png"));
  m_zoom.setToolTip(tr("Reset view's camera"));
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
  connect(&m_snapshot,SIGNAL(clicked(bool)),this,SLOT(takeSnapshot()));

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

  foreach(IRenderer *renderer, m_settings->renderers())
    this->addRendererControls(renderer->clone());

  m_mainLayout->addLayout(m_controlLayout);
}


//-----------------------------------------------------------------------------
void VolumeView::centerViewOn(Nm *center, bool notUsed)
{
  if (!isVisible() ||
      (m_center[0] == center[0] &&
       m_center[1] == center[1] &&
       m_center[2] == center[2]))
    return;

  memcpy(m_center, center, 3*sizeof(double));

  foreach(IRendererSPtr ren, m_itemRenderers)
  {
    if (QString("Crosshairs") == ren->name())
    {
      CrosshairRenderer *crossren = reinterpret_cast<CrosshairRenderer *>(ren.get());
      crossren->setCrosshair(center);
    }
  }

  if (m_additionalScrollBars && !m_channelStates.empty())
  {
    Nm minSpacing[3];
    m_channelStates.begin().key()->volume()->spacing(minSpacing);

    foreach(ChannelPtr channel, m_channelStates.keys())
    {
      double spacing[3];
      channel->volume()->spacing(spacing);
      if (spacing[0] < minSpacing[0])
        minSpacing[0] = spacing[0];

      if (spacing[1] < minSpacing[1])
        minSpacing[1] = spacing[1];

      if (spacing[2] < minSpacing[2])
        minSpacing[2] = spacing[2];
    }

    int iCenter[3] = { vtkMath::Round(center[0]/minSpacing[0]), vtkMath::Round(center[1]/minSpacing[1]), vtkMath::Round(center[2]/minSpacing[2]) };
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

  setCameraFocus(center);
  updateView();
}

//-----------------------------------------------------------------------------
void VolumeView::setCameraFocus(const Nm center[3])
{
  m_renderer->GetActiveCamera()->SetFocalPoint(center[0],center[1],center[2]);
  m_renderer->ResetCameraClippingRange();
}

//-----------------------------------------------------------------------------
void VolumeView::resetCamera()
{
  this->m_renderer->GetActiveCamera()->SetViewUp(0,1,0);
  this->m_renderer->GetActiveCamera()->SetPosition(0,0,-1);
  this->m_renderer->GetActiveCamera()->SetFocalPoint(0,0,0);
  this->m_renderer->GetActiveCamera()->SetRoll(180);
  this->m_renderer->ResetCamera();
}

//-----------------------------------------------------------------------------
void VolumeView::addChannel(ChannelPtr channel)
{
  Q_ASSERT(!m_channelStates.contains(channel));

  ChannelState state;

  channel->output()->update();

  double hue = -1.0 == channel->hue() ? 0 : channel->hue();
  double sat = -1.0 == channel->hue() ? 0 : channel->saturation();
  QColor stain = QColor::fromHsvF(hue, sat, 1.0);

  state.brightness = channel->brightness();
  state.contrast   = channel->contrast();
  state.opacity    = channel->opacity();
  state.stain      = stain;
  state.visible    = channel->isVisible();

  foreach (GraphicalRepresentationSPtr prototype, channel->representations())
  {
    if (prototype->canRenderOnView().testFlag(GraphicalRepresentation::VOLUME_VIEW))
    {
      ChannelGraphicalRepresentationSPtr representation = boost::dynamic_pointer_cast<ChannelGraphicalRepresentation>(prototype->clone(this));

      representation->setBrightness(state.brightness);
      representation->setContrast(state.contrast);
      representation->setColor(state.stain);
      if (Channel::AUTOMATIC_OPACITY != state.opacity)
        representation->setOpacity(state.opacity);
      representation->setVisible(state.visible);

      state.representations << representation;

      foreach(IRendererSPtr renderer, m_itemRenderers)
        if (renderer->itemCanBeRendered(channel) && renderer->managesRepresentation(representation))
          renderer->addRepresentation(representation);
    }
  }

  m_channelStates.insert(channel, state);

  // need to manage other channels' opacity too.
  updateSceneBounds();
  updateChannelsOpactity();

  // Prevent displaying channel's corner until app request to reset the camera
  if (m_channelStates.size() == 1 && channel->isVisible())
    resetCamera();

  // NOTE: this signal is not disconnected when a channel is removed because is
  // used in the redo/undo of UnloadChannelCommand
  connect(channel->volume().get(), SIGNAL(representationChanged()),
          this, SLOT(updateSceneBounds()));

  updateRenderersButtons();
  updateScrollBarsLimits();
}

//-----------------------------------------------------------------------------
bool VolumeView::updateChannelRepresentation(ChannelPtr channel, bool render)
{
  Q_ASSERT(m_channelStates.contains(channel));

  ChannelState &state = m_channelStates[channel];

  bool visibilityChanged = state.visible != channel->isVisible();
  state.visible = channel->isVisible();

  bool brightnessChanged = false;
  bool contrastChanged   = false;
  bool opacityChanged    = false;
  bool stainChanged      = false;

  if (visibilityChanged)
    updateChannelsOpactity();

  double hue = -1.0 == channel->hue() ? 0 : channel->hue();
  double sat = -1.0 == channel->hue() ? 0 : channel->saturation();

  if (state.visible)
  {
    QColor stain = QColor::fromHsvF(hue, sat, 1.0);

    brightnessChanged = state.brightness != channel->brightness();
    contrastChanged   = state.contrast   != channel->contrast();
    opacityChanged    = state.opacity    != channel->opacity();
    stainChanged      = state.stain      != stain;

    state.brightness  = channel->brightness();
    state.contrast    = channel->contrast();
    state.opacity     = channel->opacity();
    state.stain       = stain;
  }

  bool hasChanged = visibilityChanged || brightnessChanged || contrastChanged || opacityChanged || stainChanged;
  if (hasChanged)
  {
    opacityChanged &= Channel::AUTOMATIC_OPACITY != state.opacity;

    foreach (GraphicalRepresentationSPtr representation, state.representations)
    {
      ChannelGraphicalRepresentationSPtr rep = boost::dynamic_pointer_cast<ChannelGraphicalRepresentation>(representation);
      if (brightnessChanged) rep->setBrightness(state.brightness);
      if (contrastChanged  ) rep->setContrast(state.contrast);
      if (stainChanged     ) rep->setColor(state.stain);
      if (opacityChanged   ) rep->setOpacity(state.opacity);
      if (visibilityChanged) rep->setVisible(state.visible);

      rep->updateRepresentation();
    }
  }

  if (render)
    updateView();

  return hasChanged;
}

//-----------------------------------------------------------------------------
void VolumeView::updateChannelRepresentations(ChannelList list)
{
  if (isVisible())
  {
    ChannelList updateChannels;

    if (list.empty())
      updateChannels = m_channelStates.keys();
    else
      updateChannels = list;

    bool updated = false;
    foreach(ChannelPtr channel, updateChannels)
      updated |= updateChannelRepresentation(channel);

    if (updated)
      updateView();
  }
}

//-----------------------------------------------------------------------------
void VolumeView::removeChannel(ChannelPtr channel)
{
  Q_ASSERT(m_channelStates.contains(channel));

  foreach(GraphicalRepresentationSPtr representation, m_channelStates[channel].representations)
    foreach(IRendererSPtr renderer, m_itemRenderers)
      if (renderer->hasRepresentation(representation))
        renderer->removeRepresentation(representation);

  m_channelStates.remove(channel);

  updateRenderersButtons();
  updateScrollBarsLimits();
}


//-----------------------------------------------------------------------------
void VolumeView::addSegmentation(SegmentationPtr seg)
{
  Q_ASSERT(!m_segmentationStates.contains(seg));

  seg->output()->update();

  SegmentationState state;

  state.visible = false;

  m_segmentationStates.insert(seg, state);
}

//-----------------------------------------------------------------------------
bool VolumeView::updateSegmentationRepresentation(SegmentationPtr seg, bool render)
{
  if (!m_segmentationStates.contains(seg))
    return false;

  SegmentationState &state = m_segmentationStates[seg];

  bool requestedVisibility = seg->visible();

  bool visibilityChanged = state.visible != requestedVisibility;
  state.visible = requestedVisibility;

  bool colorChanged = false;
  bool outputChanged = false;
  bool highlightChanged = false;

  if (state.visible)
  {
    QColor requestedColor       = m_viewManager->color(seg);
    bool   requestedHighlighted = seg->isSelected();

    outputChanged    = state.output != seg->output();
    colorChanged     = state.color != requestedColor           || outputChanged;
    highlightChanged = state.highlited != requestedHighlighted || outputChanged;

    state.color     = requestedColor;
    state.highlited = requestedHighlighted;
    state.output    = seg->output();
  }

  // TODO: rep visibility not handled well
  if (outputChanged)
  {
    state.representations.clear();

    foreach(GraphicalRepresentationSPtr prototype, seg->representations())
    {
      if (prototype->canRenderOnView().testFlag(GraphicalRepresentation::VOLUME_VIEW))
      {
        GraphicalRepresentationSPtr representation = prototype->clone(this);

        foreach(IRendererSPtr renderer, m_itemRenderers)
          if (renderer->itemCanBeRendered(seg) && renderer->managesRepresentation(representation))
          {
            representation->setVisible( (renderer->isHidden() ? false : requestedVisibility) );
            renderer->addRepresentation(representation);
          }

        state.representations << boost::dynamic_pointer_cast<SegmentationGraphicalRepresentation>(representation);
      }
    }
  }

  bool hasChanged = visibilityChanged || colorChanged || highlightChanged;
  if (hasChanged)
  {
    foreach(GraphicalRepresentationSPtr representation, state.representations)
    {
      if (colorChanged)      representation->setColor(m_viewManager->color(seg));
      if (highlightChanged)  representation->setHighlighted(state.highlited);
      if (visibilityChanged && representation->isVisible()) representation->setVisible(state.visible);

      representation->updateRepresentation();
    }
  }

  if (render)
  {
    m_view->GetRenderWindow()->Render();
    m_view->update();
  }

  updateRenderersButtons();
  return hasChanged;
}

//-----------------------------------------------------------------------------
void VolumeView::updateSegmentationRepresentations(SegmentationList list)
{
  if (isVisible())
  {
    SegmentationList updateSegmentations;

    if (list.empty())
      updateSegmentations = m_segmentationStates.keys();
    else
      updateSegmentations = list;

    bool updated = false;
    foreach(SegmentationPtr seg, updateSegmentations)
      updated |= updateSegmentationRepresentation(seg);

    if (updated)
      updateView();
  }
}

//-----------------------------------------------------------------------------
void VolumeView::removeSegmentation(SegmentationPtr seg)
{
  Q_ASSERT(m_segmentationStates.contains(seg));

  foreach(GraphicalRepresentationSPtr rep, m_segmentationStates[seg].representations)
    foreach(IRendererSPtr renderer, m_itemRenderers)
      if (renderer->hasRepresentation(rep))
        renderer->removeRepresentation(rep);
  m_segmentationStates.remove(seg);

  updateRenderersButtons();
}

//-----------------------------------------------------------------------------
void VolumeView::addWidget(EspinaWidget* eWidget)
{
  if(m_widgets.contains(eWidget))
    return;

  vtkAbstractWidget *widget = eWidget->create3DWidget(this);
  if (!widget)
    return;

  widget->SetCurrentRenderer(this->m_renderer);
  widget->SetInteractor(m_view->GetInteractor());

  if (m_numEnabledSegmentationRenders != 0)
  {
    if (eWidget->manipulatesSegmentations())
    {
      widget->SetEnabled(true);
      widget->GetRepresentation()->SetVisibility(true);
    }
  }
  else
  {
    if (eWidget->manipulatesSegmentations())
    {
      widget->SetEnabled(false);
      widget->GetRepresentation()->SetVisibility(false);
    }
  }

  m_renderer->ResetCameraClippingRange();
  m_widgets[eWidget] = widget;

  updateRenderersButtons();
}

//-----------------------------------------------------------------------------
void VolumeView::removeWidget(EspinaWidget* eWidget)
{
  if (!m_widgets.contains(eWidget))
    return;

  m_widgets.remove(eWidget);

  updateRenderersButtons();
}

//-----------------------------------------------------------------------------
void VolumeView::setCursor(const QCursor& cursor)
{
  m_view->setCursor(cursor);
}

//-----------------------------------------------------------------------------
void VolumeView::eventPosition(int& x, int& y)
{
  x = y = -1;

  if (m_renderer)
  {
   vtkRenderWindowInteractor *rwi = m_renderer->GetRenderWindow()->GetInteractor();
   Q_ASSERT(rwi);
   rwi->GetEventPosition(x, y);
  }
}

//-----------------------------------------------------------------------------
ISelector::PickList VolumeView::pick(ISelector::PickableItems filter, ISelector::DisplayRegionList regions)
{
  return ISelector::PickList();
}

//-----------------------------------------------------------------------------
vtkRenderWindow* VolumeView::renderWindow()
{
  return m_view->GetRenderWindow();
}

//-----------------------------------------------------------------------------
vtkRenderer *VolumeView::mainRenderer()
{
  return m_renderer;
}


//-----------------------------------------------------------------------------
void VolumeView::setupUI()
{
  m_view = new QVTKWidget();

  if (m_additionalScrollBars)
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
}

//-----------------------------------------------------------------------------
void VolumeView::updateView()
{
  if(isVisible())
  {
    this->m_view->GetRenderWindow()->Render();
    this->m_view->update();
  }
}

//-----------------------------------------------------------------------------
void VolumeView::selectPickedItems(int vx, int vy, bool append)
{
  ViewManager::Selection selection;
  if (append)
    selection = m_viewManager->selection();

  // If no append, segmentations have priority over channels
  foreach(IRendererSPtr renderer, m_renderers.values())
  {
    if (!renderer->isHidden() && (renderer->getRendererType() & IRenderer::SEGMENTATION))
    {
      GraphicalRepresentationSList rendererSelection = renderer->pick(vx, vy, append);
      qDebug() << "selected" << rendererSelection.size();
      if (!rendererSelection.empty())
      {
        foreach(GraphicalRepresentationSPtr rep, rendererSelection)
          foreach (SegmentationPtr seg, m_segmentationStates.keys())
            if (m_segmentationStates[seg].representations.contains(rep))
            {
              if (!selection.contains(seg))
                selection << seg;
              else
                selection.removeAll(seg);
            }
      }
    }
  }

  foreach(IRendererSPtr renderer, m_renderers.values())
  {
    if (!renderer->isHidden() && (renderer->getRendererType() & IRenderer::CHANNEL))
    {
      GraphicalRepresentationSList rendererSelection = renderer->pick(vx, vy, append);
      if (!rendererSelection.empty())
      {
        foreach(GraphicalRepresentationSPtr rep, rendererSelection)
          foreach (SegmentationPtr seg, m_segmentationStates.keys())
            if (m_segmentationStates[seg].representations.contains(rep))
            {
              if (!selection.contains(seg))
                selection << seg;
              else
                selection.removeAll(seg);

            }
      }
    }
  }

  if (!append && !selection.empty())
  {
    PickableItemPtr returnItem = selection.first();
    selection.clear();
    selection << returnItem;
  }
  m_viewManager->setSelection(selection);
}

//-----------------------------------------------------------------------------
bool VolumeView::eventFilter(QObject* caller, QEvent* e)
{
  // there is not a "singleclick" event so we need to remember the position of the
  // press event and compare it with the position of the release event.
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
        foreach(IRendererSPtr renderer, m_renderers.values())
        {
          if (!renderer->isHidden())
          {
            GraphicalRepresentationSList rendererSelection = renderer->pick(newX, newY, false);
            if (!rendererSelection.empty())
            {
              double point[3];
              renderer->getPickCoordinates(point);

              emit centerChanged(point[0], point[1], point[2]);
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
      {
        selectPickedItems(newX, newY, me->modifiers() == Qt::SHIFT);
      }
  }

  return QObject::eventFilter(caller, e);
}

//-----------------------------------------------------------------------------
void VolumeView::exportScene()
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
void VolumeView::takeSnapshot()
{
  QFileDialog fileDialog(this, tr("Save Scene As Image"), QString(), tr("All supported formats (*.jpg *.png);; JPEG images (*.jpg);; PNG images (*.png)"));
  fileDialog.setObjectName("SaveSnapshotFileDialog");
  fileDialog.setWindowTitle("Save Scene As Image");
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  fileDialog.setDefaultSuffix(QString(tr("png")));
  fileDialog.setFileMode(QFileDialog::AnyFile);
  fileDialog.selectFile("");

  if (fileDialog.exec() == QDialog::Accepted)
  {
    const QString selectedFile = fileDialog.selectedFiles().first();

    QStringList splittedName = selectedFile.split(".");
    QString extension = splittedName[((splittedName.size()) - 1)].toUpper();

    QStringList validFileExtensions;
    validFileExtensions << "JPG" << "PNG";

    if (validFileExtensions.contains(extension))
    {
      int witdh = m_renderer->GetRenderWindow()->GetSize()[0];
      vtkRenderLargeImage *image = vtkRenderLargeImage::New();
      image->SetInput(m_renderer);
      image->SetMagnification(4096.0/witdh+0.5);
      image->Update();

      if (QString("PNG") == extension)
      {
        vtkPNGWriter *writer = vtkPNGWriter::New();
        writer->SetFileDimensionality(2);
        writer->SetFileName(selectedFile.toUtf8());
        writer->SetInputConnection(image->GetOutputPort());
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        writer->Write();
        QApplication::restoreOverrideCursor();
      }

      if (QString("JPG") == extension)
      {
        vtkJPEGWriter *writer = vtkJPEGWriter::New();
        writer->SetQuality(100);
        writer->ProgressiveOff();
        writer->WriteToMemoryOff();
        writer->SetFileDimensionality(2);
        writer->SetFileName(selectedFile.toUtf8());
        writer->SetInputConnection(image->GetOutputPort());
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        writer->Write();
        QApplication::restoreOverrideCursor();
      }
    }
    else
    {
      QMessageBox msgBox;
      QString message(tr("Snapshot not exported. Unrecognized extension "));
      message.append(extension).append(".");
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.setText(message);
      msgBox.exec();
    }
  }
}

//-----------------------------------------------------------------------------
VolumeView::Settings::Settings(const EspinaFactory *factory,
                               const QString        prefix,
                               VolumeView          *parent)
: RENDERERS(prefix + "VolumeView::renderers")
{
  this->parent = parent;
  QSettings settings(CESVIMA, ESPINA);

  if (!settings.contains(RENDERERS))
    settings.setValue(RENDERERS, QStringList() << "Crosshairs" << "Volumetric" << "Mesh");

  QMap<QString, IRenderer *> renderers = factory->renderers();
  foreach(QString name, settings.value(RENDERERS).toStringList())
  {
    IRenderer *renderer = renderers.value(name, NULL);
    if (renderer)
      m_renderers << renderer;
  }

}

//-----------------------------------------------------------------------------
void VolumeView::Settings::setRenderers(IRendererList values)
{
  QSettings settings(CESVIMA, ESPINA);
  QStringList activeRenderersNames;
  IRendererList activeRenderers;

  // remove controls for unused renderers
  foreach(IRenderer *oldRenderer, m_renderers)
  {
    bool selected = false;
    int i = 0;
    while (!selected && i < values.size())
      selected = values[i++]->name() == oldRenderer->name();

    if (!selected)
    {
      parent->removeRendererControls(oldRenderer->name());
      if (!oldRenderer->isHidden())
      {
        oldRenderer->hide();
        parent->countEnabledRenderers(false);
      }
      oldRenderer->setVTKRenderer(NULL);
    }
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
      parent->addRendererControls(renderer->clone());
    }
  }

  settings.setValue(RENDERERS, activeRenderersNames);
  settings.sync();
  m_renderers = activeRenderers;
}

//-----------------------------------------------------------------------------
IRendererList VolumeView::Settings::renderers() const
{
  return m_renderers;
}

//-----------------------------------------------------------------------------
void VolumeView::changePlanePosition(PlaneType plane, Nm dist)
{
  bool needUpdate = false;
  foreach(IRendererSPtr ren, m_itemRenderers)
  {
    if (QString("Crosshairs") == ren->name())
    {
      CrosshairRenderer *crossren = reinterpret_cast<CrosshairRenderer *>(ren.get());
      crossren->setPlanePosition(plane, dist);
      needUpdate = true;
    }
  }
  if (needUpdate)
    updateView();
}

//-----------------------------------------------------------------------------
void VolumeView::countEnabledRenderers(bool value)
{
  if ((true == value) && (0 == this->m_numEnabledRenders))
  {
    m_renderer->ResetCamera();
    updateView();
  }

  IRendererSPtr renderer;

  QPushButton *button = dynamic_cast<QPushButton*>(sender());
  if (button)
    renderer = m_renderers[button];

  if (value)
  {
    m_numEnabledRenders++;
    if (renderer && (renderer->getRendererType() & IRenderer::SEGMENTATION) && m_additionalScrollBars)
    {
      m_numEnabledChannelRenders++;
      m_axialScrollBar->setEnabled(true);
      m_coronalScrollBar->setEnabled(true);
      m_sagittalScrollBar->setEnabled(true);
    }

    if (renderer && (renderer->getRendererType() & IRenderer::SEGMENTATION))
    {
      m_numEnabledSegmentationRenders++;
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
    }
  }
  else
  {
    m_numEnabledRenders--;
    if (renderer && (renderer->getRendererType() & IRenderer::CHANNEL) && m_additionalScrollBars)
    {
      m_numEnabledChannelRenders--;
      if (0 == m_numEnabledChannelRenders)
      {
        m_axialScrollBar->setEnabled(false);
        m_coronalScrollBar->setEnabled(false);
        m_sagittalScrollBar->setEnabled(false);
      }
    }

    if (renderer && (renderer->getRendererType() & IRenderer::SEGMENTATION))
    {
      m_numEnabledSegmentationRenders--;
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
    }
  }
  updateRenderersButtons();
}

//-----------------------------------------------------------------------------
void VolumeView::updateSelection(ViewManager::Selection selection, bool render)
{
  updateSegmentationRepresentations();
}

//-----------------------------------------------------------------------------
void VolumeView::resetView()
{
  resetCamera();
  updateView();
}

//-----------------------------------------------------------------------------
void VolumeView::forceRender(SegmentationList updatedSegs)
{
  foreach(SegmentationPtr seg, updatedSegs)
    updateSegmentationRepresentation(seg);

  m_view->GetRenderWindow()->Render();
  m_view->update();
}

//-----------------------------------------------------------------------------
void VolumeView::updateRenderersButtons()
{
  bool canTakeSnapshot = false;
  bool canBeExported = false;
  QMap<QPushButton *, IRendererSPtr>::iterator it;
  for(it = m_renderers.begin(); it != m_renderers.end(); ++it)
  {
    bool canRenderItems = it.value()->itemsBeenRendered() != 0;

    canTakeSnapshot |= (!it.value()->isHidden()) && canRenderItems;
    canBeExported |= canTakeSnapshot && (it.value()->getNumberOfvtkActors() != 0);

    it.key()->setEnabled(canRenderItems);
    if (!canRenderItems)
      it.key()->setChecked(false);
  }

  m_snapshot.setEnabled(canTakeSnapshot);
  m_export.setEnabled(canBeExported);
}

//-----------------------------------------------------------------------------
void VolumeView::scrollBarMoved(int value)
{
  Nm point[3];
  Nm minSpacing[3];

  Q_ASSERT(!m_channelStates.isEmpty());
  m_channelStates.keys().first()->volume()->spacing(minSpacing);

  foreach(ChannelPtr channel, m_channelStates.keys())
  {
    double spacing[3];
    channel->volume()->spacing(spacing);
    if (spacing[0] < minSpacing[0])
      minSpacing[0] = spacing[0];

    if (spacing[1] < minSpacing[1])
      minSpacing[1] = spacing[1];

    if (spacing[2] < minSpacing[2])
      minSpacing[2] = spacing[2];
  }

  point[0] = m_axialScrollBar->value() * minSpacing[0];
  point[1] = m_coronalScrollBar->value() * minSpacing[1];
  point[2] = m_sagittalScrollBar->value() * minSpacing[2];

  foreach(IRendererSPtr renderer, m_renderers.values())
    if (renderer->getRendererType() & IRenderer::CHANNEL)
    {
      CrosshairRenderer *crossRender = dynamic_cast<CrosshairRenderer *>(renderer.get());
      if (crossRender != NULL)
        crossRender->setCrosshair(point);
    }

  m_view->update();
}

//-----------------------------------------------------------------------------
void VolumeView::updateScrollBarsLimits()
{
  if (!m_additionalScrollBars)
    return;

  if(m_channelStates.isEmpty())
  {
    m_axialScrollBar->setMinimum(0);
    m_axialScrollBar->setMaximum(0);
    m_coronalScrollBar->setMinimum(0);
    m_coronalScrollBar->setMaximum(0);
    m_sagittalScrollBar->setMinimum(0);
    m_sagittalScrollBar->setMaximum(0);
    return;
  }

  int maxExtent[6];
  m_channelStates.keys().first()->volume()->extent(maxExtent);

  foreach (ChannelPtr channel, m_channelStates.keys())
  {
    int extent[6];
    channel->volume()->extent(extent);

    if (maxExtent[0] > extent[0])
      maxExtent[0] = extent[0];

    if (maxExtent[1] < extent[1])
      maxExtent[1] = extent[1];

    if (maxExtent[2] > extent[2])
      maxExtent[2] = extent[2];

    if (maxExtent[3] < extent[3])
      maxExtent[3] = extent[3];

    if (maxExtent[4] > extent[4])
      maxExtent[4] = extent[4];

    if (maxExtent[5] < extent[5])
      maxExtent[5] = extent[5];
  }

  m_axialScrollBar->setMinimum(maxExtent[0]);
  m_axialScrollBar->setMaximum(maxExtent[1]);
  m_coronalScrollBar->setMinimum(maxExtent[2]);
  m_coronalScrollBar->setMaximum(maxExtent[3]);
  m_sagittalScrollBar->setMinimum(maxExtent[4]);
  m_sagittalScrollBar->setMaximum(maxExtent[5]);
}

//-----------------------------------------------------------------------------
void VolumeView::addActor(vtkProp3D *actor)
{
  m_renderer->AddActor(actor);
//  m_meshPicker->GetPickList()->AddItem(actor);
}

//-----------------------------------------------------------------------------
void VolumeView::removeActor(vtkProp3D *actor)
{
  m_renderer->RemoveActor(actor);
//  m_meshPicker->GetPickList()->RemoveItem(actor);
}
