/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "SliceRepresentation.h"
#include <GUI/QtWidget/SliceView.h>
#include <Core/ColorEngines/TransparencySelectionHighlighter.h>

#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkImageShiftScale.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkImageData.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
ChannelSliceRepresentation::ChannelSliceRepresentation(ChannelVolumeSPtr data,
                                                       SliceView        *view)
: ChannelGraphicalRepresentation(view)
, m_data(data)
{
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::setBrightness(double value)
{
  ChannelGraphicalRepresentation::setBrightness(value);

  if (m_actor != NULL)
    m_shiftScaleFilter->SetShift(static_cast<int>(m_brightness*255));
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::setContrast(double value)
{
  ChannelGraphicalRepresentation::setContrast(value);

  if (m_actor != NULL)
    m_shiftScaleFilter->SetScale(m_contrast);
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::setColor(const QColor &color)
{
  GraphicalRepresentation::setColor(color);

  if (m_actor != NULL)
  {
    m_lut->SetHueRange(color.hueF(), color.hueF());
    m_lut->SetSaturationRange(0.0, color.saturationF());
    m_lut->Build();
  }
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::setOpacity(double value)
{
  ChannelGraphicalRepresentation::setOpacity(value);

  if (m_actor != NULL)
    m_actor->SetOpacity(m_opacity);
}

//-----------------------------------------------------------------------------
bool ChannelSliceRepresentation::hasActor(vtkProp *actor) const
{
  if (m_actor == NULL)
    return false;

  return m_actor.GetPointer() == actor;
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::updateRepresentation()
{
  if (m_actor != NULL)
  {
    m_reslice->Update();
    m_mapToColors->Update();
    m_shiftScaleFilter->Update();
    m_actor->Update();
  }
}

//-----------------------------------------------------------------------------
GraphicalRepresentationSPtr ChannelSliceRepresentation::cloneImplementation(SliceView *view)
{
  ChannelSliceRepresentation *representation = new ChannelSliceRepresentation(m_data, view);
  representation->setView(view);

  return GraphicalRepresentationSPtr(representation);
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::updateVisibility(bool visible)
{
  if (m_actor != NULL)
    m_actor->SetVisibility(visible);
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::updatePipelineConnections()
{
  if ((m_actor != NULL) && (m_reslice->GetInputConnection(0,0) != m_data->toVTK()))
  {
    m_reslice->SetInputConnection(m_data->toVTK());
    m_reslice->Update();
  }
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::initializePipeline()
{
  connect(m_data.get(), SIGNAL(representationChanged()),
          this, SLOT(updatePipelineConnections()));

  m_reslice = vtkSmartPointer<vtkImageReslice>::New();
  m_reslice->SetInputConnection(m_data->toVTK());
  m_reslice->SetOutputDimensionality(2);
  m_reslice->SetResliceAxes(slicingMatrix(reinterpret_cast<SliceView *>(m_view)));
  m_reslice->SetNumberOfThreads(1);
  m_reslice->Update();

  m_shiftScaleFilter = vtkSmartPointer<vtkImageShiftScale>::New();
  m_shiftScaleFilter->SetInputConnection(m_reslice->GetOutputPort());
  m_shiftScaleFilter->SetShift(static_cast<int>(m_brightness*255));
  m_shiftScaleFilter->SetScale(m_contrast);
  m_shiftScaleFilter->SetClampOverflow(true);
  m_shiftScaleFilter->SetOutputScalarType(m_reslice->GetOutput()->GetScalarType());
  m_shiftScaleFilter->Update();

  m_lut = vtkSmartPointer<vtkLookupTable>::New();
  m_lut->Allocate();
  m_lut->SetTableRange(0,255);
  m_lut->SetHueRange(m_color.hueF(), m_color.hueF());
  m_lut->SetSaturationRange(0.0, m_color.saturationF());
  m_lut->SetValueRange(0.0, 1.0);
  m_lut->SetAlphaRange(1.0,1.0);
  m_lut->SetNumberOfColors(256);
  m_lut->SetRampToLinear();
  m_lut->Build();

  m_mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
  m_mapToColors->SetInputConnection(m_shiftScaleFilter->GetOutputPort());
  m_mapToColors->SetLookupTable(m_lut);
  m_mapToColors->SetNumberOfThreads(1);
  m_mapToColors->Update();

  m_actor = vtkSmartPointer<vtkImageActor>::New();
  m_actor->SetInterpolate(false);
  m_actor->GetMapper()->BorderOn();
  m_actor->GetMapper()->SetInputConnection(m_mapToColors->GetOutputPort());
  m_actor->Update();
}

//-----------------------------------------------------------------------------
QList<vtkProp*> ChannelSliceRepresentation::getActors()
{
  QList<vtkProp*> list;

  if (m_actor == NULL)
    initializePipeline();

  list << m_actor.GetPointer();

  return list;
}

//-----------------------------------------------------------------------------
bool ChannelSliceRepresentation::isInside(Nm point[3])
{
  itkVolumeType::IndexType voxel = m_data->index(point[0], point[1], point[2]);
  return m_data->volumeRegion().IsInside(voxel);
};

//-----------------------------------------------------------------------------
TransparencySelectionHighlighter *SegmentationSliceRepresentation::s_highlighter = new TransparencySelectionHighlighter();

//-----------------------------------------------------------------------------
SegmentationSliceRepresentation::SegmentationSliceRepresentation(SegmentationVolumeSPtr data,
                                                                 SliceView             *view)
: SegmentationGraphicalRepresentation(view)
, m_data(data)
, m_reslice(NULL)
, m_mapToColors(NULL)
, m_actor(NULL)
{

}

//-----------------------------------------------------------------------------
void SegmentationSliceRepresentation::setColor(const QColor &color)
{
  GraphicalRepresentation::setColor(color);

  if (m_actor != NULL)
    m_mapToColors->SetLookupTable(s_highlighter->lut(m_color, m_highlight));
}

//-----------------------------------------------------------------------------
void SegmentationSliceRepresentation::setHighlighted(bool highlighted)
{
  SegmentationGraphicalRepresentation::setHighlighted(highlighted);

  if (m_actor != NULL)
    m_mapToColors->SetLookupTable(s_highlighter->lut(m_color, m_highlight));
}

//-----------------------------------------------------------------------------
bool SegmentationSliceRepresentation::hasActor(vtkProp *actor) const
{
  if (m_actor == NULL)
    return false;

  return m_actor.GetPointer() == actor;
}

//-----------------------------------------------------------------------------
bool SegmentationSliceRepresentation::isInside(Nm point[3])
{
  Q_ASSERT(m_data.get());
  Q_ASSERT(m_data->toITK().IsNotNull());

  itkVolumeType::IndexType voxel = m_data->index(point[0], point[1], point[2]);

  return m_data->volumeRegion().IsInside(voxel)
      && m_data->toITK()->GetPixel(voxel) == SEG_VOXEL_VALUE;
}

//-----------------------------------------------------------------------------
void SegmentationSliceRepresentation::initializePipeline()
{
  connect(m_data.get(), SIGNAL(representationChanged()),
          this, SLOT(updatePipelineConnections()));

  SliceView *view = reinterpret_cast<SliceView *>(m_view);

  m_reslice = vtkSmartPointer<vtkImageReslice>::New();
  m_reslice->SetInputConnection(m_data->toVTK());
  m_reslice->SetOutputDimensionality(2);
  m_reslice->SetResliceAxes(slicingMatrix(view));
  m_reslice->SetNumberOfThreads(1);
  m_reslice->Update();

  // actor should be allocated first or the next call to setColor() would do nothing
  m_actor = vtkSmartPointer<vtkImageActor>::New();

  m_mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
  m_mapToColors->SetInputConnection(m_reslice->GetOutputPort());
  setColor(m_color);
  m_mapToColors->SetNumberOfThreads(1);
  m_mapToColors->Update();

  m_actor->SetInterpolate(false);
  m_actor->GetMapper()->BorderOn();
  m_actor->GetMapper()->SetInputConnection(m_mapToColors->GetOutputPort());
  m_actor->Update();

  // need to reposition the actor so it will always be over the channels actors'
  double pos[3];
  m_actor->GetPosition(pos);
  pos[view->plane()] = view->segmentationDepth();
  m_actor->SetPosition(pos);
}

//-----------------------------------------------------------------------------
void SegmentationSliceRepresentation::updateRepresentation()
{
  if (m_actor != NULL)
  {
    m_reslice->Update();
    m_mapToColors->Update();
    m_actor->Update();
  }
}

//-----------------------------------------------------------------------------
QList<vtkProp*> SegmentationSliceRepresentation::getActors()
{
  QList<vtkProp *> list;

  if (m_actor == NULL)
    initializePipeline();

  list << m_actor.GetPointer();

  return list;
}

//-----------------------------------------------------------------------------
GraphicalRepresentationSPtr SegmentationSliceRepresentation::cloneImplementation(SliceView *view)
{
  SegmentationSliceRepresentation *representation = new SegmentationSliceRepresentation(m_data, view);
  representation->setView(view);

  return GraphicalRepresentationSPtr(representation);
}

//-----------------------------------------------------------------------------
void SegmentationSliceRepresentation::updateVisibility(bool visible)
{
  if (m_actor != NULL)
    m_actor->SetVisibility(visible);
}

//-----------------------------------------------------------------------------
void SegmentationSliceRepresentation::updatePipelineConnections()
{
  if ((m_actor != NULL) && (m_reslice->GetInputConnection(0,0) != m_data->toVTK()))
  {
    m_reslice->SetInputConnection(m_data->toVTK());
    m_reslice->Update();
  }
}
