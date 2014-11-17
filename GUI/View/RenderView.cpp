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
#include "RenderView.h"
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Utils/VolumeBounds.h>
#include <GUI/ColorEngines/NumberColorEngine.h>
#include <GUI/Extension/Visualization/VisualizationState.h>
#include <GUI/Model/Utils/QueryAdapter.h>

// VTK
#include <vtkMath.h>
#include <QVTKWidget.h>
#include <vtkCoordinate.h>
#include <vtkRenderWindow.h>
#include <vtkPNGWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkWindowToImageFilter.h>

// Qt
#include <QApplication>
#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QPushButton>
#include <QDebug>

using namespace ESPINA;

//-----------------------------------------------------------------------------
RenderView::RenderView(QWidget* parent)
: QWidget                 {parent}
, m_colorEngine           {new NumberColorEngine()}
, m_view                  {new QVTKWidget()}
, m_sceneResolution       {1, 1, 1}
, m_sceneCameraInitialized{false}
, m_showSegmentations     {true}
{
}

//-----------------------------------------------------------------------------
RenderView::~RenderView()
{
  // subclasses of this one should take care of removing elements
  // (channels, segmentations, widgets and renderers).
  delete m_view;
}

//-----------------------------------------------------------------------------
void RenderView::onSelectionSet(SelectionSPtr selection)
{
  connect(selection.get(), SIGNAL(selectionStateChanged(SegmentationAdapterList)),
          this, SLOT(updateSelection(SegmentationAdapterList)));
}

//-----------------------------------------------------------------------------
void RenderView::addWidget(EspinaWidgetSPtr widget)
{
  if(m_widgets.contains(widget))
    return;

  widget->registerView(this);

  m_widgets << widget;
}

//-----------------------------------------------------------------------------
void RenderView::removeWidget(EspinaWidgetSPtr widget)
{
  if (!m_widgets.contains(widget))
    return;

  widget->unregisterView(this);

  m_widgets.removeOne(widget);
}


//-----------------------------------------------------------------------------
void RenderView::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);

  updateRepresentations();
}

//-----------------------------------------------------------------------------
void RenderView::takeSnapshot()
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

  for(auto channel: m_channelStates.keys())
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
DefaultVolumetricDataSPtr volumetricData(ViewItemAdapterPtr item)
{
  return std::dynamic_pointer_cast<VolumetricData<itkVolumeType>>(item->outputData(VolumetricData<itkVolumeType>::TYPE));
}

//-----------------------------------------------------------------------------
void RenderView::updateSceneBounds()
{
  NmVector3 resolution = m_sceneResolution;

  if (!m_channelStates.isEmpty())
  {
    auto channels = m_channelStates.keys();
    auto volume   = volumetricData(channels.first());

    m_sceneBounds     = volume->bounds();
    m_sceneResolution = volume->spacing();

    for (int i = 1; i < channels.size(); ++i)
    {
      NmVector3 channelSpacing;
      Bounds    channelBounds;

      DefaultVolumetricDataSPtr volume = volumetricData(channels[i]);
      channelSpacing = volume->spacing();
      channelBounds  = volume->bounds();

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

  if (resolution != m_sceneResolution)
    emit sceneResolutionChanged();
}

//-----------------------------------------------------------------------------
void RenderView::add(ChannelAdapterPtr channel)
{
  Q_ASSERT(!m_channelStates.contains(channel));

  channel->output()->update();

  ChannelState state;

  state.visible = !channel->isVisible();

  m_channelStates.insert(channel, state);

  connect(channel, SIGNAL(outputChanged(ViewItemAdapterPtr)),
          this,    SLOT(changedOutput(ViewItemAdapterPtr)));

  // need to manage other channels' opacity too.
  updateSceneBounds();

  updateRepresentation(channel, false);
}


//-----------------------------------------------------------------------------
void RenderView::add(SegmentationAdapterPtr seg)
{
  Q_ASSERT(!m_segmentationStates.contains(seg));

  seg->output()->update();

  SegmentationState state;

  state.visible   = false;

  connect(seg,  SIGNAL(outputChanged(ViewItemAdapterPtr)),
          this, SLOT(changedOutput(ViewItemAdapterPtr)));

  m_segmentationStates.insert(seg, state);

  updateRepresentation(seg, false);
}

//-----------------------------------------------------------------------------
void RenderView::remove(SegmentationAdapterPtr seg)
{
  Q_ASSERT(m_segmentationStates.contains(seg));

  for(auto rep: m_segmentationStates[seg].representations)
  {
    for(auto renderer: m_renderers)
    {
      if (renderer->type() == Renderer::Type::Representation)
      {
        auto repRenderer = representationRenderer(renderer);
        if (repRenderer->hasRepresentation(rep))
        {
          repRenderer->removeRepresentation(rep);
        }
      }
    }
  }

  disconnect(seg,  SIGNAL(outputChanged(ViewItemAdapterPtr)),
             this, SLOT(changedOutput(ViewItemAdapterPtr)));

  m_segmentationStates.remove(seg);
}

//-----------------------------------------------------------------------------
void RenderView::remove(ChannelAdapterPtr channel)
{
  Q_ASSERT(m_channelStates.contains(channel));

  for(auto representation: m_channelStates[channel].representations)
  {
    for(auto renderer: m_renderers)
    {
      if (renderer->type() == Renderer::Type::Representation)
      {
        auto repRenderer = representationRenderer(renderer);
        if (repRenderer->hasRepresentation(representation))
        {
          repRenderer->removeRepresentation(representation);
        }
      }
    }
  }

  m_channelStates.remove(channel);

  disconnect(channel, SIGNAL(outputChanged(ViewItemAdapterPtr)),
             this,    SLOT(changedOutput(ViewItemAdapterPtr)));

  updateSceneBounds();
  updateChannelsOpacity();
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
    updateChannelsOpacity();

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

  bool hasChanged = visibilityChanged || brightnessChanged || contrastChanged || opacityChanged || stainChanged;

  if (outputChanged)
  {
    removeRepresentations(state);
    createRepresentations(channel);
  }

  for(auto representation : state.representations)
  {
    bool crosshairChanged = representation->crosshairDependent() && representation->crosshairPoint() != crosshairPoint();
    if (hasChanged || crosshairChanged || outputChanged)
    {
      opacityChanged &= Channel::AUTOMATIC_OPACITY != state.opacity;

      if (brightnessChanged) representation->setBrightness(state.brightness);
      if (contrastChanged  ) representation->setContrast(state.contrast);
      if (stainChanged     ) representation->setColor(state.stain);
      if (opacityChanged   ) representation->setOpacity(state.opacity);
      if (visibilityChanged) representation->setVisible(state.visible);

      representation->updateRepresentation();
    }
  }

  if (!m_sceneCameraInitialized && state.visible)
  {
    m_sceneCameraInitialized = true;
    resetCamera();
  }

  m_renderer->ResetCameraClippingRange();

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
    {
      updateChannels = m_channelStates.keys();
    }
    else
    {
      updateChannels = list;
    }

    bool updated = false;
    for(ChannelAdapterPtr channel : updateChannels)
    {
      updated |= updateRepresentation(channel, false);
    }

    if (updated)
      updateView();
  }
}

//-----------------------------------------------------------------------------
bool RenderView::updateRepresentation(SegmentationAdapterPtr seg, bool render)
{
  if (!isVisible()) return false;

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
    createRepresentations(seg);
  }

  bool hasChanged = visibilityChanged || colorChanged || highlightChanged;
  for(auto representation : state.representations)
  {
    bool crosshairChanged = representation->crosshairDependent() && representation->crosshairPoint() != crosshairPoint();
    if (hasChanged || crosshairChanged || representation->needUpdate())
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
    {
      updateSegmentations = m_segmentationStates.keys();
    }
    else
    {
      updateSegmentations = list;
    }

    bool updated = false;
    for(auto seg : updateSegmentations)
    {
      updated |= updateRepresentation(seg, false);
    }

    if (updated)
    {
      updateView();
    }
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
vtkRenderWindow* RenderView::renderWindow() const
{
  return m_view->GetRenderWindow();
}

//-----------------------------------------------------------------------------
vtkRenderer* RenderView::mainRenderer() const
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
void RenderView::updateSelection(SegmentationAdapterList selection)
{
  updateRepresentations(selection);
}

//-----------------------------------------------------------------------------
QPushButton* RenderView::createButton(const QString& icon, const QString& tooltip)
{
  QPushButton *button = new QPushButton();

  button->setIcon(QIcon(icon));
  button->setToolTip(tooltip);
  button->setFlat(true);
  button->setIconSize(QSize(20,20));
  button->setMinimumSize(QSize(22,22));
  button->setMaximumSize(QSize(22,22));
  button->setEnabled(false);
  button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  return button;
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
void RenderView::createRepresentations(ChannelAdapterPtr channel)
{
  for(auto representationName : channel->representationTypes())
  {
    for(auto renderer : m_renderers)
      if(renderer->type() == Renderer::Type::Representation)
      {
        auto repRenderer = representationRenderer(renderer);
        if (repRenderer->canRender(channel) && repRenderer->managesRepresentation(representationName))
        {
          RepresentationSPtr representation = cloneRepresentation(channel, representationName);
          if (representation.get() != nullptr)
          {
            representation->setVisible(channel->isVisible());
            repRenderer->addRepresentation(channel, representation);
            m_channelStates[channel].representations << representation;
          }
        }
      }
  }
}

//-----------------------------------------------------------------------------
void RenderView::createRepresentations(SegmentationAdapterPtr segmentation)
{
  for(auto representationName : segmentation->representationTypes())
  {
    for(auto renderer : m_renderers)
      if(renderer->type() == Renderer::Type::Representation)
      {
        auto repRenderer = representationRenderer(renderer);
        if (repRenderer->canRender(segmentation) && repRenderer->managesRepresentation(representationName))
        {
          RepresentationSPtr representation = cloneRepresentation(segmentation, representationName);
          if (representation.get() != nullptr)
          {
            representation->setVisible(segmentation->isVisible() && m_showSegmentations);
            repRenderer->addRepresentation(segmentation, representation);
            m_segmentationStates[segmentation].representations << representation;

            if (segmentation->hasExtension(VisualizationState::TYPE))
            {
              VisualizationStateSPtr stateExtension = std::dynamic_pointer_cast<VisualizationState>(segmentation->extension(VisualizationState::TYPE));

              representation->restoreSettings(stateExtension->state(representation->type()));
            }
          }
        }
      }
  }
}

//-----------------------------------------------------------------------------
void RenderView::removeRepresentations(ChannelState &state)
{
  for(auto rep: state.representations)
  {
    for(auto renderer: m_renderers)
    {
      if(renderer->type() == Renderer::Type::Representation)
      {
        auto repRenderer = representationRenderer(renderer);
        repRenderer->removeRepresentation(rep);
      }
    }
  }

  state.representations.clear();
}

//-----------------------------------------------------------------------------
void RenderView::removeRepresentations(SegmentationState &state)
{
  for(auto rep: state.representations)
  {
    for(auto renderer: m_renderers)
    {
      if(renderer->type() == Renderer::Type::Representation)
      {
        auto repRenderer = representationRenderer(renderer);
        repRenderer->removeRepresentation(rep);
      }
    }
  }

  state.representations.clear();
}

//-----------------------------------------------------------------------------
unsigned int RenderView::numEnabledRenderersForViewItem(RenderableType type)
{
  unsigned int count = 0;
  for(auto renderer: m_renderers)
    if(renderer->type() == Renderer::Type::Representation)
    {
      auto repRenderer = representationRenderer(renderer);
      if (canRender(repRenderer, type) && !renderer->isHidden())
      ++count;
    }

  return count;
}

//-----------------------------------------------------------------------------
Selector::Selection RenderView::select(const Selector::SelectionFlags flags, const Selector::SelectionMask &mask, bool multiselection) const
{
  Selector::Selection selectedItems;

  if(flags.contains(Selector::CHANNEL) || flags.contains(Selector::SAMPLE))
  {
    for(auto channelAdapter: m_channelStates.keys())
    {
      if (intersect(channelAdapter->bounds(), mask->bounds().bounds()))
      {
        auto intersectionBounds = intersection(channelAdapter->bounds(), mask->bounds().bounds());
        auto selectionMask = BinaryMaskSPtr<unsigned char>(new BinaryMask<unsigned char>(intersectionBounds, channelAdapter->output()->spacing(), channelAdapter->position()));

        BinaryMask<unsigned char>::const_region_iterator crit(mask.get(), intersectionBounds);
        crit.goToBegin();

        if(channelAdapter->output()->spacing() == mask->spacing())
        {
          BinaryMask<unsigned char>::iterator it(selectionMask.get());
          it.goToBegin();

          while(!crit.isAtEnd())
          {
            if(crit.isSet())
              it.Set();

            ++crit;
          }
        }
        else
        {
          // mask interpolation needed, more costly
          auto spacing = mask->spacing();
          while(!crit.isAtEnd())
          {
            if(crit.isSet())
            {
              auto center = crit.getCenter();
              auto voxelBounds = Bounds{center[0]-spacing[0]/2, center[0]+spacing[0]/2,
                                        center[1]-spacing[1]/2, center[1]+spacing[1]/2,
                                        center[2]-spacing[2]/2, center[2]+spacing[2]/2};

              BinaryMask<unsigned char>::region_iterator rit(selectionMask.get(), voxelBounds);
              rit.goToBegin();

              while(!rit.isAtEnd())
              {
                rit.Set();
                ++rit;
              }
            }

            ++crit;
          }
        }

        if(flags.contains(Selector::CHANNEL))
          selectedItems << QPair<Selector::SelectionMask, NeuroItemAdapterPtr>(selectionMask, channelAdapter);

        if(flags.contains(Selector::SAMPLE))
        {
          auto sampleAdapter = QueryAdapter::sample(channelAdapter);
          selectedItems << QPair<Selector::SelectionMask, NeuroItemAdapterPtr>(selectionMask, sampleAdapter.get());
        }
      }

      if(!multiselection && selectedItems.size() == 1)
        break;
    }
  }

  if(flags.contains(Selector::SEGMENTATION))
  {
    for(auto segAdapter: m_segmentationStates.keys())
    {
      if(intersect(segAdapter->bounds(), mask->bounds().bounds()))
      {
        auto intersectionBounds = intersection(segAdapter->bounds(), mask->bounds().bounds());
        BinaryMask<unsigned char>::const_region_iterator crit(mask.get(), intersectionBounds);
        crit.goToBegin();

        auto volume = volumetricData(segAdapter->output());
        auto itkVolume = volume->itkImage(intersectionBounds);
        auto value = itkVolume->GetBufferPointer();
        auto selectionMask = BinaryMaskSPtr<unsigned char>(new BinaryMask<unsigned char>(intersectionBounds, volume->spacing(), volume->origin()));

        if(segAdapter->output()->spacing() == mask->spacing())
        {
          BinaryMask<unsigned char>::iterator it(selectionMask.get());
          it.goToBegin();

          while(!crit.isAtEnd())
          {
            if (SEG_VOXEL_VALUE == *value && crit.isSet())
              it.Set();

            ++value;
            ++crit;
            ++it;
          }
        }
        else
        {
          // mask interpolation needed, more costly
          auto spacing = mask->spacing();
          while(!crit.isAtEnd())
          {
            if(crit.isSet() && SEG_VOXEL_VALUE == *value)
            {
              auto center = crit.getCenter();
              auto voxelBounds = Bounds{center[0]-spacing[0]/2, center[0]+spacing[0]/2,
                                        center[1]-spacing[1]/2, center[1]+spacing[1]/2,
                                        center[2]-spacing[2]/2, center[2]+spacing[2]/2};

              BinaryMask<unsigned char>::region_iterator rit(selectionMask.get(), voxelBounds);
              rit.goToBegin();

              while(!rit.isAtEnd())
              {
                rit.Set();
                ++rit;
              }
            }

            ++value;
            ++crit;
          }
        }

        selectedItems << QPair<Selector::SelectionMask, NeuroItemAdapterPtr>(selectionMask, segAdapter);
      }

      if(!multiselection && selectedItems.size() == 1)
        break;
    }
  }

  return selectedItems;
}

//-----------------------------------------------------------------------------
Selector::Selection RenderView::select(const Selector::SelectionFlags flags, const NmVector3 &point, bool multiselection) const
{
  vtkSmartPointer<vtkCoordinate> coords = vtkSmartPointer<vtkCoordinate>::New();
  coords->SetCoordinateSystemToWorld();
  coords->SetValue(point[0], point[1], point[2]);
  int *displayCoords = coords->GetComputedDisplayValue(m_renderer);

  return select(flags, displayCoords[0], displayCoords[1], multiselection);
}

//-----------------------------------------------------------------------------
RendererSList RenderView::renderers() const
{
  return m_renderers;
}

//-----------------------------------------------------------------------------
void RenderView::setRenderersState(QMap<QString, bool> state)
{
  for(auto renderer: m_renderers)
  {
    if(state.keys().contains(renderer->name()))
    {
      if(state[renderer->name()])
        activateRender(renderer->name());
       else
         deactivateRender(renderer->name());
    }
  }
}

//-----------------------------------------------------------------------------
void RenderView::changedOutput(ViewItemAdapterPtr item)
{
  if(isSegmentation(item))
  {
    updateRepresentation(dynamic_cast<SegmentationAdapterPtr>(item));
  }
  else
  {
    updateRepresentation(dynamic_cast<ChannelAdapterPtr>(item));
  }
}
