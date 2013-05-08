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

using namespace EspINA;

//-----------------------------------------------------------------------------
ChannelSliceRepresentation::ChannelSliceRepresentation(ChannelVolumeSPtr data,
                                                       SliceView        *view)
: ChannelGraphicalRepresentation(view)
, m_data(data)
{
}

//-----------------------------------------------------------------------------
ChannelSliceRepresentation::~ChannelSliceRepresentation()
{
  SliceView *sliceView = static_cast<SliceView *>(m_view);
  if (sliceView)
  {
    sliceView->removeChannelActor(slice);
  }
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::setBrightness(double value)
{
  ChannelGraphicalRepresentation::setBrightness(value);

  shiftScaleFilter->SetShift(static_cast<int>(m_brightness*255));
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::setContrast(double value)
{
  ChannelGraphicalRepresentation::setContrast(value);

  shiftScaleFilter->SetScale(m_contrast);
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::setColor(const QColor &color)
{
  GraphicalRepresentation::setColor(color);

  lut->SetHueRange(color.hueF(), color.hueF());
  lut->SetSaturationRange(0.0, color.saturationF());
  lut->Build();
}

void ChannelSliceRepresentation::setHighlighted(bool highlighted)
{
  GraphicalRepresentation::setHighlighted(highlighted);
}


//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::setOpacity(double value)
{
  ChannelGraphicalRepresentation::setOpacity(value);

  slice->SetOpacity(m_opacity);
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::setVisible(bool visible)
{
  ChannelGraphicalRepresentation::setVisible(visible);

  slice->SetVisibility(m_visible);
}

//-----------------------------------------------------------------------------
bool ChannelSliceRepresentation::isInside(Nm point[3])
{
  return false;
}

//-----------------------------------------------------------------------------
GraphicalRepresentationSPtr ChannelSliceRepresentation::clone(SliceView *view)
{
  ChannelSliceRepresentation *rep = new ChannelSliceRepresentation(m_data, view);

  rep->initializePipeline(view);

  GraphicalRepresentationSPtr representation(rep);

  m_clones << representation;

  return representation;
}

//-----------------------------------------------------------------------------
bool ChannelSliceRepresentation::hasActor(vtkProp *actor) const
{
  return slice.GetPointer() == actor;
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::updateRepresentation()
{
  reslice->Update();
  mapToColors->Update();
  shiftScaleFilter->Update();
  slice->Update();
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::updatePipelineConnections()
{
  if (reslice->GetInputConnection(0,0) != m_data->toVTK())
  {
    reslice->SetInputConnection(m_data->toVTK());
    reslice->Update();
  }
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::initializePipeline(SliceView *view)
{
  connect(m_data.get(), SIGNAL(representationChanged()),
          this, SLOT(updatePipelineConnections()));

  reslice = vtkSmartPointer<vtkImageReslice>::New();
  reslice->SetInputConnection(m_data->toVTK());
  reslice->SetOutputDimensionality(2);
  reslice->SetResliceAxes(slicingMatrix(view));
  reslice->SetNumberOfThreads(1);
  reslice->Update();

  shiftScaleFilter = vtkSmartPointer<vtkImageShiftScale>::New();
  shiftScaleFilter->SetInputConnection(reslice->GetOutputPort());
  shiftScaleFilter->SetShift(static_cast<int>(m_brightness*255));
  shiftScaleFilter->SetScale(m_contrast);
  shiftScaleFilter->SetClampOverflow(true);
  shiftScaleFilter->SetOutputScalarType(reslice->GetOutput()->GetScalarType());
  shiftScaleFilter->Update();

  lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->Allocate();
  lut->SetTableRange(0,255);
  lut->SetHueRange(0.0, 0.0);
  lut->SetSaturationRange(0.0, 0.0);
  lut->SetValueRange(0.0, 1.0);
  lut->SetAlphaRange(1.0,1.0);
  lut->SetNumberOfColors(256);
  lut->SetRampToLinear();
  lut->Build();

  mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
  mapToColors->SetInputConnection(shiftScaleFilter->GetOutputPort());
  mapToColors->SetLookupTable(lut);
  mapToColors->SetNumberOfThreads(1);
  mapToColors->Update();

  slice = vtkSmartPointer<vtkImageActor>::New();
  slice->SetInterpolate(false);
  slice->GetMapper()->BorderOn();
  slice->GetMapper()->SetInputConnection(mapToColors->GetOutputPort());
  slice->Update();

  view->addChannelActor(slice);

  m_view = view;
}


//-----------------------------------------------------------------------------
TransparencySelectionHighlighter *SegmentationSliceRepresentation::s_highlighter = new TransparencySelectionHighlighter();

//-----------------------------------------------------------------------------
SegmentationSliceRepresentation::SegmentationSliceRepresentation(SegmentationVolumeSPtr data,
                                                                 SliceView             *view)
: SegmentationGraphicalRepresentation(view)
, m_data(data)
, m_view(view)
{

}

//-----------------------------------------------------------------------------
SegmentationSliceRepresentation::~SegmentationSliceRepresentation()
{
  if (m_view)
    m_view->removeSegmentationActor(slice);
}

//-----------------------------------------------------------------------------
void SegmentationSliceRepresentation::setColor(const QColor &color)
{
  GraphicalRepresentation::setColor(color);

  mapToColors->SetLookupTable(s_highlighter->lut(m_color, m_highlight));
}

//-----------------------------------------------------------------------------
void SegmentationSliceRepresentation::setHighlighted(bool highlighted)
{
  SegmentationGraphicalRepresentation::setHighlighted(highlighted);

  mapToColors->SetLookupTable(s_highlighter->lut(m_color, m_highlight));
}

//-----------------------------------------------------------------------------
void SegmentationSliceRepresentation::setVisible(bool visible)
{
  SegmentationGraphicalRepresentation::setVisible(visible);

  slice->SetVisibility(m_visible);
}

//-----------------------------------------------------------------------------
bool SegmentationSliceRepresentation::hasActor(vtkProp *actor) const
{
  return slice.GetPointer() == actor;
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
GraphicalRepresentationSPtr SegmentationSliceRepresentation::clone(SliceView *view)
{
  SegmentationSliceRepresentation *representation = new SegmentationSliceRepresentation(m_data, view);

  representation->initializePipeline(view);

  return GraphicalRepresentationSPtr(representation);
}

//-----------------------------------------------------------------------------
void SegmentationSliceRepresentation::initializePipeline(SliceView *view)
{
  connect(m_data.get(), SIGNAL(representationChanged()),
          this, SLOT(updatePipelineConnections()));

  reslice = vtkSmartPointer<vtkImageReslice>::New();
  reslice->SetInputConnection(m_data->toVTK());
  reslice->SetOutputDimensionality(2);
  reslice->SetResliceAxes(slicingMatrix(view));
  reslice->SetNumberOfThreads(1);
  reslice->Update();

  mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
  mapToColors->SetInputConnection(reslice->GetOutputPort());
  setColor(m_color);
  mapToColors->SetNumberOfThreads(1);
  mapToColors->Update();

  slice = vtkSmartPointer<vtkImageActor>::New();
  slice->SetInterpolate(false);
  slice->GetMapper()->BorderOn();
  slice->GetMapper()->SetInputConnection(mapToColors->GetOutputPort());
  slice->Update();

  // need to reposition the actor so it will always be over the channels actors'
  double pos[3];
  slice->GetPosition(pos);
  pos[m_view->plane()] = m_view->segmentationDepth();
  slice->SetPosition(pos);

  view->addSegmentationActor(slice);

  m_view = view;
}

//-----------------------------------------------------------------------------
void SegmentationSliceRepresentation::updateRepresentation()
{
  reslice->Update();
  mapToColors->Update();
  slice->Update();
}

//-----------------------------------------------------------------------------
void SegmentationSliceRepresentation::updatePipelineConnections()
{
  if (reslice->GetInputConnection(0,0) != m_data->toVTK())
  {
    reslice->SetInputConnection(m_data->toVTK());
    reslice->Update();
  }
}
