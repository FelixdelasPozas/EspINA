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
#include "EspinaRenderView.h"
#include <Core/Model/ModelItem.h>
#include <Core/Model/Channel.h>
#include <Core/Model/Segmentation.h>
#include "GUI/Representations/GraphicalRepresentation.h"

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

// boost
#include <boost/shared_ptr.hpp>

using namespace EspINA;

//-----------------------------------------------------------------------------
EspinaRenderView::EspinaRenderView(ViewManager *vm, QWidget* parent)
: QWidget(parent)
, m_viewManager(vm)
, m_view(new QVTKWidget())
, m_numEnabledSegmentationRenders(0)
, m_numEnabledChannelRenders(0)
, m_sceneCameraInitialized(false)
, m_showSegmentations(true)
{
  m_sceneResolution[0] = m_sceneResolution[1] = m_sceneResolution[2] = 1;

  m_sceneBounds[0] = m_sceneBounds[2] = m_sceneBounds[4] = 0;
  m_sceneBounds[1] = m_sceneBounds[3] = m_sceneBounds[5] = 0;

  m_plane = VOLUME;
}

//-----------------------------------------------------------------------------
EspinaRenderView::~EspinaRenderView()
{
}

//-----------------------------------------------------------------------------
void EspinaRenderView::previewBounds(Nm bounds[6], bool cropToSceneBounds)
{
  vtkMath::UninitializeBounds(bounds);
  //qDebug() << bounds[0] << bounds[1] << bounds[2] << bounds[3] << bounds[4] << bounds[5];
  bounds[0] = bounds[2] = bounds[4] =  0;
  bounds[1] = bounds[3] = bounds[5] = -1;
}

//-----------------------------------------------------------------------------
void EspinaRenderView::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);

  updateChannelRepresentations();
  updateSegmentationRepresentations();
}

//-----------------------------------------------------------------------------
void EspinaRenderView::takeSnapshot(vtkSmartPointer<vtkRenderer> renderer)
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
double EspinaRenderView::suggestedChannelOpacity()
{
  double numVisibleRep = 0;

  foreach(ChannelPtr  channel, m_channelStates.keys())
    if (channel->isVisible())
      numVisibleRep++;

  if (numVisibleRep == 0)
    return 1.0;

  return 1.0 / numVisibleRep;
}

//-----------------------------------------------------------------------------
void EspinaRenderView::resetSceneBounds()
{
  m_sceneResolution[0] = m_sceneResolution[1] = m_sceneResolution[2] = 1;
  m_sceneBounds[0] = m_sceneBounds[2] = m_sceneBounds[4] = 0;
  m_sceneBounds[1] = m_sceneBounds[3] = m_sceneBounds[5] = 0;
}


//-----------------------------------------------------------------------------
void EspinaRenderView::setSegmentationsVisibility(bool visible)
{
  m_showSegmentations = visible;

  updateSegmentationRepresentations();
}

//-----------------------------------------------------------------------------
void EspinaRenderView::updateSceneBounds()
{
  if (!m_channelStates.isEmpty())
  {
    m_channelStates.keys().first()->volume()->spacing(m_sceneResolution);
    m_channelStates.keys().first()->volume()->bounds(m_sceneBounds);

    ChannelList channels = m_channelStates.keys();
    for (int i = 0; i < channels.size(); ++i)
    {
      double channelSpacing[3];
      double channelBounds[6];

      channels[i]->volume()->spacing(channelSpacing);
      channels[i]->volume()->bounds(channelBounds);

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
void EspinaRenderView::setViewType(PlaneType plane)
{
  m_plane = plane;
}

//-----------------------------------------------------------------------------
PlaneType EspinaRenderView::getViewType()
{
  return m_plane;
}

//-----------------------------------------------------------------------------
void EspinaRenderView::addSegmentation(SegmentationPtr seg)
{
  Q_ASSERT(!m_segmentationStates.contains(seg));

  seg->output()->update();

  SegmentationState state;

  state.visible   = false;

  m_segmentationStates.insert(seg, state);

  updateSegmentationRepresentation(seg, false);
}

//-----------------------------------------------------------------------------
void EspinaRenderView::removeSegmentation(SegmentationPtr seg)
{
  Q_ASSERT(m_segmentationStates.contains(seg));

  foreach(GraphicalRepresentationSPtr rep, m_segmentationStates[seg].representations)
    foreach(IRendererSPtr renderer, m_renderers)
      if (renderer->hasRepresentation(rep))
        renderer->removeRepresentation(rep);

  m_segmentationStates.remove(seg);
}

//-----------------------------------------------------------------------------
void EspinaRenderView::addChannel(ChannelPtr channel)
{
  Q_ASSERT(!m_channelStates.contains(channel));

  channel->output()->update();

  ChannelState state;

  state.visible = !channel->isVisible();

  m_channelStates.insert(channel, state);

  // need to manage other channels' opacity too.
  updateSceneBounds();

  updateChannelRepresentation(channel, false);

  // NOTE: this signal is not disconnected when a channel is removed because is
  // used in the redo/undo of UnloadChannelCommand
  connect(channel->volume().get(), SIGNAL(representationChanged()),
          this, SLOT(updateSceneBounds()));
}


//-----------------------------------------------------------------------------
void EspinaRenderView::removeChannel(ChannelPtr channel)
{
  Q_ASSERT(m_channelStates.contains(channel));

  foreach(GraphicalRepresentationSPtr representation, m_channelStates[channel].representations)
    foreach(IRendererSPtr renderer, m_renderers)
      if (renderer->hasRepresentation(representation))
        renderer->removeRepresentation(representation);

  m_channelStates.remove(channel);

  updateSceneBounds();
  updateChannelsOpactity();
}

//-----------------------------------------------------------------------------
bool EspinaRenderView::updateChannelRepresentation(ChannelPtr channel, bool render)
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
    double requestedBrightness = channel->brightness();
    double requestedContrast   = channel->contrast();
    double requestedOpacity    = channel->opacity();
    QColor requestedStain      = QColor::fromHsvF(hue, sat, 1.0);

    outputChanged     = state.output     != channel->output();
    brightnessChanged = state.brightness != requestedBrightness || outputChanged;
    contrastChanged   = state.contrast   != requestedContrast   || outputChanged;
    opacityChanged    = state.opacity    != requestedOpacity    || outputChanged;
    stainChanged      = state.stain      != requestedStain      || outputChanged;

    state.brightness  = requestedBrightness;
    state.contrast    = requestedContrast;
    state.opacity     = requestedOpacity;
    state.stain       = requestedStain;
    state.output      = channel->output();
  }

  if (outputChanged)
  {
    removeGraphicalRepresentations(state);

    foreach(GraphicalRepresentationSPtr prototype, channel->representations())
    {
      GraphicalRepresentationSPtr representationClone = cloneRepresentation(prototype);
      if (representationClone.get() != NULL)
      {
        foreach(IRendererSPtr renderer, m_renderers)
          if (renderer->itemCanBeRendered(channel) && renderer->managesRepresentation(representationClone))
          {
            representationClone->setVisible(visibilityChanged);
            renderer->addRepresentation(channel, representationClone);
          }

        state.representations << boost::dynamic_pointer_cast<ChannelGraphicalRepresentation>(representationClone);
      }
    }
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
void EspinaRenderView::updateChannelRepresentations(ChannelList list)
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
      updated |= updateChannelRepresentation(channel, false);

    if (updated)
      updateView();
  }
}

//-----------------------------------------------------------------------------
bool EspinaRenderView::updateSegmentationRepresentation(SegmentationPtr seg, bool render)
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

  bool requestedVisibility = seg->visible() && m_showSegmentations;

  bool visibilityChanged = state.visible != requestedVisibility;
  state.visible = requestedVisibility;

  bool colorChanged     = false;
  bool outputChanged    = false;
  bool highlightChanged = false;

  if (state.visible)
  {
    QColor requestedColor       = m_viewManager->color(seg);
    bool   requestedHighlighted = seg->isSelected();

    outputChanged    = state.output    != seg->output();
    colorChanged     = state.color     != requestedColor       || outputChanged;
    highlightChanged = state.highlited != requestedHighlighted || outputChanged;

    state.color     = requestedColor;
    state.highlited = requestedHighlighted;
    state.output    = seg->output();
  }

  if (outputChanged)
  {
    removeGraphicalRepresentations(state);

    foreach(GraphicalRepresentationSPtr prototype, seg->representations())
    {
      GraphicalRepresentationSPtr representationClone = cloneRepresentation(prototype);
      if (representationClone.get() != NULL)
      {
        foreach(IRendererSPtr renderer, m_renderers)
          if (renderer->itemCanBeRendered(seg) && renderer->managesRepresentation(representationClone))
          {
            representationClone->setVisible(requestedVisibility);
            renderer->addRepresentation(seg, representationClone);
          }

        state.representations << boost::dynamic_pointer_cast<SegmentationGraphicalRepresentation>(representationClone);
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
void EspinaRenderView::updateSegmentationRepresentations(SegmentationList list)
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
      updated |= updateSegmentationRepresentation(seg, false);

    if (updated)
      updateView();
  }
}

//-----------------------------------------------------------------------------
void EspinaRenderView::setCursor(const QCursor& cursor)
{
  m_view->setCursor(cursor);
}

//-----------------------------------------------------------------------------
vtkRenderWindow* EspinaRenderView::renderWindow()
{
  return m_view->GetRenderWindow();
}

//-----------------------------------------------------------------------------
vtkRenderer* EspinaRenderView::mainRenderer()
{
  return m_renderer;
}

//-----------------------------------------------------------------------------
void EspinaRenderView::eventPosition(int& x, int& y)
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
void EspinaRenderView::updateSelection(ViewManager::Selection selection, bool render)
{
  updateSegmentationRepresentations();
  if (render)
    updateView();
}

//-----------------------------------------------------------------------------
void EspinaRenderView::addActor(vtkProp *actor)
{
  m_renderer->AddActor(actor);
}

//-----------------------------------------------------------------------------
void EspinaRenderView::removeActor(vtkProp *actor)
{
  m_renderer->RemoveActor(actor);
}

//-----------------------------------------------------------------------------
void EspinaRenderView::resetView()
{
  resetCamera();
  updateView();
}

//-----------------------------------------------------------------------------
void EspinaRenderView::removeGraphicalRepresentations(ChannelState &state)
{
  foreach(ChannelGraphicalRepresentationSPtr gr, state.representations)
  {
    foreach(IRendererSPtr renderer, m_renderers)
    {
      renderer->removeRepresentation(gr);
    }
  }

  state.representations.clear();
}

//-----------------------------------------------------------------------------
void EspinaRenderView::removeGraphicalRepresentations(SegmentationState &state)
{
  foreach(SegmentationGraphicalRepresentationSPtr gr, state.representations)
  {
    foreach(IRendererSPtr renderer, m_renderers)
    {
      renderer->removeRepresentation(gr);
    }
  }

  state.representations.clear();
}
