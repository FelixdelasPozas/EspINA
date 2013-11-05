/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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
#include "RenderView.h"
#include <Core/Analysis/Channel.h>
#include <GUI/ColorEngines/NumberColorEngine.h>
#include <GUI/Extension/Visualization/VisualizationState.h>

// VTK
#include <vtkMath.h>
#include <QVTKWidget.h>
#include <vtkRenderWindow.h>
#include <vtkPNGWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkWindowToImageFilter.h>

// Qt
#include <QApplication>
#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

using namespace EspINA;

//-----------------------------------------------------------------------------
RenderView::RenderView(QWidget* parent)
: QWidget(parent)
, m_colorEngine{new NumberColorEngine()}
, m_view{new QVTKWidget()}
, m_numEnabledChannelRenders{0}
, m_numEnabledSegmentationRenders{0}
, m_sceneCameraInitialized(false)
, m_showSegmentations(true)
, m_sceneResolution{1, 1, 1}
{
}

//-----------------------------------------------------------------------------
RenderView::~RenderView()
{
}


//-----------------------------------------------------------------------------
void RenderView::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);

  updateRepresentations();
}

//-----------------------------------------------------------------------------
void RenderView::takeSnapshot(vtkSmartPointer<vtkRenderer> renderer)
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
      // avoid artifacts when acquiring the image
      int offScreenRender = renderWindow()->GetOffScreenRendering();
      renderWindow()->SetOffScreenRendering(true);

      vtkSmartPointer<vtkWindowToImageFilter> image = vtkSmartPointer<vtkWindowToImageFilter>::New();
      image->SetInput(renderWindow());
      image->SetMagnification(4096.0/renderWindow()->GetSize()[0]+0.5);
      image->Update();

      renderWindow()->SetOffScreenRendering(offScreenRender);

      if (QString("PNG") == extension)
      {
        vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
        writer->SetFileDimensionality(2);
        writer->SetFileName(selectedFile.toUtf8());
        writer->SetInputConnection(image->GetOutputPort());
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        writer->Write();
        QApplication::restoreOverrideCursor();
      }

      if (QString("JPG") == extension)
      {
        vtkSmartPointer<vtkJPEGWriter> writer = vtkSmartPointer<vtkJPEGWriter>::New();
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
double RenderView::suggestedChannelOpacity()
{
  double numVisibleRep = 0;

  foreach(ChannelAdapterPtr  channel, m_channelStates.keys())
    if (channel->isVisible())
      numVisibleRep++;

  if (numVisibleRep == 0)
    return 1.0;

  return 1.0 / numVisibleRep;
}

//-----------------------------------------------------------------------------
void RenderView::resetSceneBounds()
{
  m_sceneResolution[0] = m_sceneResolution[1] = m_sceneResolution[2] = 1;
  m_sceneBounds[0] = m_sceneBounds[2] = m_sceneBounds[4] = 0;
  m_sceneBounds[1] = m_sceneBounds[3] = m_sceneBounds[5] = 0;
}


//-----------------------------------------------------------------------------
void RenderView::setSegmentationsVisibility(bool visible)
{
  m_showSegmentations = visible;

  updateRepresentations(SegmentationAdapterList());
}

//-----------------------------------------------------------------------------
void RenderView::updateSceneBounds()
{
  if (!m_channelStates.isEmpty())
  {
    // TODO 2013-10-04 Get channel volumetric data
//     m_channelStates.keys().first()->volume()->spacing(m_sceneResolution);
//     m_channelStates.keys().first()->volume()->bounds(m_sceneBounds);

    ChannelAdapterList channels = m_channelStates.keys();
    for (int i = 0; i < channels.size(); ++i)
    {
      double channelSpacing[3];
      double channelBounds[6];

    // TODO 2013-10-04 Get channel volumetric data
//       channels[i]->volume()->spacing(channelSpacing);
//       channels[i]->volume()->bounds(channelBounds);

      for (int i = 0; i < 3; i++)
      {
        m_sceneResolution[i] = std::min(m_sceneResolution[i], channelSpacing[i]);

        m_sceneBounds[2*i]     = std::min(m_sceneBounds[2*i]    , channelBounds[2*i]);
        m_sceneBounds[(2*i)+1] = std::max(m_sceneBounds[(2*i)+1], channelBounds[(2*i)+1]);
      }
    }
  }
  else
    resetSceneBounds();
}

//-----------------------------------------------------------------------------
void RenderView::addSegmentation(SegmentationAdapterPtr seg)
{
  Q_ASSERT(!m_segmentationStates.contains(seg));

  seg->output()->update();

  SegmentationState state;

  state.visible   = false;

  m_segmentationStates.insert(seg, state);

  updateRepresentation(seg, false);
}

//-----------------------------------------------------------------------------
void RenderView::removeSegmentation(SegmentationAdapterPtr seg)
{
  Q_ASSERT(m_segmentationStates.contains(seg));

  foreach(RepresentationSPtr rep, m_segmentationStates[seg].representations)
    foreach(RendererSPtr renderer, m_renderers)
      if (renderer->hasRepresentation(rep))
        renderer->removeRepresentation(rep);

  m_segmentationStates.remove(seg);
}

//-----------------------------------------------------------------------------
void RenderView::addChannel(ChannelAdapterPtr channel)
{
  Q_ASSERT(!m_channelStates.contains(channel));

  channel->output()->update();

  ChannelState state;

  state.visible = !channel->isVisible();

  m_channelStates.insert(channel, state);

  // need to manage other channels' opacity too.
  updateSceneBounds();

  updateRepresentation(channel, false);

  // NOTE: this signal is not disconnected when a channel is removed because is
  // used in the redo/undo of UnloadChannelCommand
  //TODO 2013-10-04 
//   connect(channel->volume().get(), SIGNAL(representationChanged()),
//           this, SLOT(updateSceneBounds()));
}


//-----------------------------------------------------------------------------
void RenderView::removeChannel(ChannelAdapterPtr channel)
{
  Q_ASSERT(m_channelStates.contains(channel));

  foreach(RepresentationSPtr representation, m_channelStates[channel].representations)
    foreach(RendererSPtr renderer, m_renderers)
      if (renderer->hasRepresentation(representation))
        renderer->removeRepresentation(representation);

  m_channelStates.remove(channel);

  updateSceneBounds();
  updateChannelsOpactity();
}

//-----------------------------------------------------------------------------
bool RenderView::updateRepresentation(ChannelAdapterPtr channel, bool render)
{
  if (!isVisible())
    return false;

  if (!m_channelStates.contains(channel))
  {
    qWarning() << "Update Graphical Representation on non-registered channel";
    return false;
  }

  Q_ASSERT(m_channelStates.contains(channel));

  ChannelState &state = m_channelStates[channel];

  bool requestedVisibility = channel->isVisible();

  bool visibilityChanged = state.visible != requestedVisibility;
  state.visible = requestedVisibility;

  bool brightnessChanged = false;
  bool contrastChanged   = false;
  bool opacityChanged    = false;
  bool stainChanged      = false;
  bool outputChanged     = false;

  if (visibilityChanged)
    updateChannelsOpactity();

  double hue = -1.0 == channel->hue() ? 0 : channel->hue();
  double sat = -1.0 == channel->hue() ? 0 : channel->saturation();

  if (state.visible)
  {
    double    requestedBrightness = channel->brightness();
    double    requestedContrast   = channel->contrast();
    double    requestedOpacity    = channel->opacity();
    QColor    requestedStain      = QColor::fromHsvF(hue, sat, 1.0);
    TimeStamp requestedTimeStamp  = channel->output()->lastModified();

    Q_ASSERT(channel->output());
    if (!state.output)
    {
      state.timeStamp = channel->output()->lastModified();
    }

    outputChanged     = state.output     != channel->output()   || state.timeStamp != requestedTimeStamp;
    brightnessChanged = state.brightness != requestedBrightness || outputChanged;
    contrastChanged   = state.contrast   != requestedContrast   || outputChanged;
    opacityChanged    = state.opacity    != requestedOpacity    || outputChanged;
    stainChanged      = state.stain      != requestedStain      || outputChanged;

    state.brightness  = requestedBrightness;
    state.contrast    = requestedContrast;
    state.opacity     = requestedOpacity;
    state.stain       = requestedStain;
    state.output      = channel->output();
    state.timeStamp   = requestedTimeStamp;
  }

  if (outputChanged)
  {
    removeRepresentations(state);

    foreach(RepresentationSPtr prototype, channel->output()->representations())
    {
      RepresentationSPtr representationClone = cloneRepresentation(prototype);
      if (representationClone.get() != nullptr)
      {
        foreach(RendererSPtr renderer, m_renderers)
          if (renderer->canRender(channel) && renderer->managesRepresentation(representationClone))
          {
            representationClone->setVisible(visibilityChanged);
            renderer->addRepresentation(channel, representationClone);
          }

        state.representations << std::dynamic_pointer_cast<Representation>(representationClone);
      }
    }
  }

  bool hasChanged = visibilityChanged || brightnessChanged || contrastChanged || opacityChanged || stainChanged;
  if (hasChanged)
  {
    opacityChanged &= Channel::AUTOMATIC_OPACITY != state.opacity;

    foreach (RepresentationSPtr representation, state.representations)
    {
      RepresentationSPtr rep = std::dynamic_pointer_cast<Representation>(representation);
      if (brightnessChanged) rep->setBrightness(state.brightness);
      if (contrastChanged  ) rep->setContrast(state.contrast);
      if (stainChanged     ) rep->setColor(state.stain);
      if (opacityChanged   ) rep->setOpacity(state.opacity);
      if (visibilityChanged) rep->setVisible(state.visible);

      rep->updateRepresentation();
    }
  }

  if (!m_sceneCameraInitialized && state.visible)
  {
    resetCamera();
    m_sceneCameraInitialized = true;
  }

  if (render && isVisible())
  {
    m_view->GetRenderWindow()->Render();
    m_view->update();
  }

  return hasChanged;
}

//-----------------------------------------------------------------------------
void RenderView::updateRepresentations(ChannelAdapterList list)
{
  if (isVisible())
  {
    ChannelAdapterList updateChannels;

    if (list.empty())
      updateChannels = m_channelStates.keys();
    else
      updateChannels = list;

    bool updated = false;
    foreach(ChannelAdapterPtr channel, updateChannels)
      updated |= updateRepresentation(channel, false);

    if (updated)
      updateView();
  }
}

//-----------------------------------------------------------------------------
bool RenderView::updateRepresentation(SegmentationAdapterPtr seg, bool render)
{
  if (!isVisible())
    return false;

  if (!m_segmentationStates.contains(seg))
  {
    qWarning() << "Update Graphical Representation on non-registered segmentation";
    return false;
  }
  Q_ASSERT(m_segmentationStates.contains(seg));

  SegmentationState &state = m_segmentationStates[seg];

  bool requestedVisibility = seg->isVisible() && m_showSegmentations;

  bool visibilityChanged = state.visible != requestedVisibility;
  state.visible = requestedVisibility;

  bool colorChanged     = false;
  bool outputChanged    = false;
  bool highlightChanged = false;

  if (state.visible)
  {
    QColor    requestedColor       = m_colorEngine->color(seg);
    bool      requestedHighlighted = seg->isSelected();
    TimeStamp requestedTimeStamp   = seg->output()->lastModified();

    Q_ASSERT(seg->output());
    if (!state.output)
    {
      state.timeStamp = requestedTimeStamp;
    }

    outputChanged    = state.output    != seg->output()        || state.timeStamp != requestedTimeStamp;
    colorChanged     = state.color     != requestedColor       || outputChanged;
    highlightChanged = state.highlited != requestedHighlighted || outputChanged;

    state.color     = requestedColor;
    state.highlited = requestedHighlighted;
    state.output    = seg->output();
    state.timeStamp = requestedTimeStamp;
  }

  if (outputChanged)
  {
    removeRepresentations(state);

    foreach(RepresentationSPtr prototype, seg->output()->representations())
    {
      RepresentationSPtr representationClone = cloneRepresentation(prototype);
      if (representationClone.get() != nullptr)
      {
        foreach(RendererSPtr renderer, m_renderers)
          if (renderer->canRender(seg) && renderer->managesRepresentation(representationClone))
          {
            representationClone->setVisible(requestedVisibility);
            renderer->addRepresentation(seg, representationClone);
          }

        state.representations << std::dynamic_pointer_cast<Representation>(representationClone);
      }

      if (seg->hasExtension(VisualizationState::TYPE))
      {
        VisualizationStateSPtr stateExtension = std::dynamic_pointer_cast<VisualizationState>(seg->extension(VisualizationState::TYPE));

        prototype->restoreSettings(stateExtension->state(prototype->label()));
      }
    }
  }

  bool hasChanged = visibilityChanged || colorChanged || highlightChanged;
  if (hasChanged)
  {
    foreach(RepresentationSPtr representation, state.representations)
    {
      if (colorChanged)      representation->setColor(m_colorEngine->color(seg));
      if (highlightChanged)  representation->setHighlighted(state.highlited);
      if (visibilityChanged) representation->setVisible(state.visible);

      representation->updateRepresentation();
    }
  }

  if (render && isVisible())
  {
    m_view->GetRenderWindow()->Render();
    m_view->update();
  }

  return hasChanged;
}

//-----------------------------------------------------------------------------
void RenderView::updateRepresentations(SegmentationAdapterList list)
{
  if (isVisible())
  {
    SegmentationAdapterList updateSegmentations;

    if (list.empty())
      updateSegmentations = m_segmentationStates.keys();
    else
      updateSegmentations = list;

    bool updated = false;
    foreach(SegmentationAdapterPtr seg, updateSegmentations)
      updated |= updateRepresentation(seg, false);

    if (updated)
      updateView();
  }
}

//-----------------------------------------------------------------------------
void RenderView::updateRepresentations()
{
  updateRepresentations(ChannelAdapterList());
  updateRepresentations(SegmentationAdapterList());
}

//-----------------------------------------------------------------------------
void RenderView::setCursor(const QCursor& cursor)
{
  m_view->setCursor(cursor);
}

//-----------------------------------------------------------------------------
vtkRenderWindow* RenderView::renderWindow()
{
  return m_view->GetRenderWindow();
}

//-----------------------------------------------------------------------------
vtkRenderer* RenderView::mainRenderer()
{
  return m_renderer;
}

//-----------------------------------------------------------------------------
void RenderView::eventPosition(int& x, int& y)
{
  x = y = -1;

  if (m_renderer)
  {
    vtkRenderWindowInteractor *rwi = renderWindow()->GetInteractor();
    Q_ASSERT(rwi);
    rwi->GetEventPosition(x, y);
  }
}

//-----------------------------------------------------------------------------
void RenderView::updateSelection(SelectableView::Selection selection, bool render)
{
  updateRepresentations();
  if (render)
    updateView();
}

//-----------------------------------------------------------------------------
void RenderView::addActor(vtkProp *actor)
{
  m_renderer->AddActor(actor);
}

//-----------------------------------------------------------------------------
void RenderView::removeActor(vtkProp *actor)
{
  m_renderer->RemoveActor(actor);
}

//-----------------------------------------------------------------------------
void RenderView::resetView()
{
  updateSceneBounds();
  resetCamera();
  updateView();
}

//-----------------------------------------------------------------------------
void RenderView::removeRepresentations(ChannelState &state)
{
  foreach(RepresentationSPtr rep, state.representations)
  {
    foreach(RendererSPtr renderer, m_renderers)
    {
      renderer->removeRepresentation(rep);
    }
  }

  state.representations.clear();
}

//-----------------------------------------------------------------------------
void RenderView::removeRepresentations(SegmentationState &state)
{
  foreach(RepresentationSPtr rep, state.representations)
  {
    foreach(RendererSPtr renderer, m_renderers)
    {
      renderer->removeRepresentation(rep);
    }
  }

  state.representations.clear();
}
