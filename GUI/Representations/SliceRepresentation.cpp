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
#include "RepresentationSettings.h"
#include "SliceRepresentationSettings.h"
#include "RepresentationEmptySettings.h"
#include <GUI/View/View2D.h>
#include <GUI/ColorEngines/TransparencySelectionHighlighter.h>
#include <Core/Utils/Bounds.h>

#include <itkImage.h>

#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkImageShiftScale.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkImageData.h>
#include <vtkImageImport.h>

#include <QList>

using namespace EspINA;

const Representation::Type ChannelSliceRepresentation::TYPE = "Slice";

//-----------------------------------------------------------------------------
ChannelSliceRepresentation::ChannelSliceRepresentation(DefaultVolumetricDataSPtr data,
                                                       View2D *view)
: Representation(view)
, m_data(data)
, m_planeIndex(-1)
, m_exporter(nullptr)
, m_mapToColors(nullptr)
, m_shiftScaleFilter(nullptr)
, m_actor(nullptr)
, m_lut(nullptr)
{
  setType(TYPE);
  //qDebug() << "Creating Solid Representation";
}

//-----------------------------------------------------------------------------
ChannelSliceRepresentation::~ChannelSliceRepresentation()
{
  //qDebug() << "Destroying Solid Representation";
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::setBrightness(double value)
{
  Representation::setBrightness(value);

  if (m_actor != nullptr)
    m_shiftScaleFilter->SetShift(static_cast<int>(m_brightness*255));
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::setContrast(double value)
{
  Representation::setContrast(value);

  if (m_actor != nullptr)
    m_shiftScaleFilter->SetScale(m_contrast);
}

//-----------------------------------------------------------------------------
RepresentationSettings *ChannelSliceRepresentation::settingsWidget()
{
  return new RepresentationEmptySettings();
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::setColor(const QColor &color)
{
  Representation::setColor(color);

  if (m_actor != nullptr)
  {
    m_lut->SetHueRange(color.hueF(), color.hueF());
    m_lut->SetSaturationRange(0.0, color.saturationF());
    m_lut->Build();
  }
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::setOpacity(double value)
{
  Representation::setOpacity(value);

  if (m_actor != nullptr)
    m_actor->SetOpacity(m_opacity);
}

//-----------------------------------------------------------------------------
bool ChannelSliceRepresentation::hasActor(vtkProp *actor) const
{
  if (m_actor == nullptr)
    return false;

  return m_actor.GetPointer() == actor;
}

//-----------------------------------------------------------------------------
RepresentationSPtr ChannelSliceRepresentation::cloneImplementation(View2D *view)
{
  ChannelSliceRepresentation *representation =  new ChannelSliceRepresentation(m_data, view);
  representation->setView(view);
  representation->m_planeIndex = normalCoordinateIndex(view->plane());

  return RepresentationSPtr(representation);
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::updateVisibility(bool visible)
{
  if (m_actor != nullptr)
    m_actor->SetVisibility(visible);
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::initializePipeline()
{
  if (m_planeIndex == -1)
    return;

  m_reslicePoint = m_crosshair[m_planeIndex];

  Bounds imageBounds = m_data->bounds();
  imageBounds.setLowerInclusion(true);
  imageBounds.setUpperInclusion(toAxis(m_planeIndex), true);
  imageBounds[2*m_planeIndex] = m_reslicePoint;
  imageBounds[(2*m_planeIndex)+1] = m_reslicePoint;

  itkVolumeType::Pointer slice = m_data->itkImage(imageBounds);
  m_exporter = ExporterType::New();
  m_exporter->ReleaseDataFlagOn();
  m_exporter->SetNumberOfThreads(1);
  m_exporter->SetInput(slice);
  m_exporter->Update();

  m_shiftScaleFilter = vtkSmartPointer<vtkImageShiftScale>::New();
  m_shiftScaleFilter->SetInputData(m_exporter->GetOutput());
  m_shiftScaleFilter->SetShift(static_cast<int>(m_brightness*255));
  m_shiftScaleFilter->SetScale(m_contrast);
  m_shiftScaleFilter->SetClampOverflow(true);
  m_shiftScaleFilter->SetOutputScalarType(m_exporter->GetOutput()->GetScalarType());
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
  m_actor->SetDisplayExtent(m_exporter->GetOutput()->GetExtent());
  m_actor->SetVisibility(isVisible());
  m_actor->Update();

  m_lastUpdatedTime = m_data->lastModified();
}

//-----------------------------------------------------------------------------
void ChannelSliceRepresentation::updateRepresentation()
{
  setCrosshairPoint(m_view->crosshairPoint());

  Bounds imageBounds = m_data->bounds();

  bool valid = imageBounds[2*m_planeIndex] <= m_crosshair[m_planeIndex] && m_crosshair[m_planeIndex] <= imageBounds[2*m_planeIndex+1];

  if (m_actor != nullptr && ((m_crosshair[m_planeIndex] != m_reslicePoint) || needUpdate()) && valid)
  {
    m_reslicePoint = m_crosshair[m_planeIndex];

    imageBounds.setLowerInclusion(true);
    imageBounds.setUpperInclusion(toAxis(m_planeIndex), true);

    imageBounds[2*m_planeIndex]   = m_reslicePoint;
    imageBounds[2*m_planeIndex+1] = m_reslicePoint;

    itkVolumeType::Pointer slice = m_data->itkImage(imageBounds);

    m_exporter = ExporterType::New();
    m_exporter->ReleaseDataFlagOn();
    m_exporter->SetNumberOfThreads(1);
    m_exporter->SetInput(slice);
    m_exporter->Update();
    m_shiftScaleFilter->SetInputData(m_exporter->GetOutput());
    m_shiftScaleFilter->Update();
    m_mapToColors->SetInputConnection(m_shiftScaleFilter->GetOutputPort());
    m_mapToColors->Update();
    m_actor->GetMapper()->SetInputConnection(m_mapToColors->GetOutputPort());
    m_actor->SetDisplayExtent(m_exporter->GetOutput()->GetExtent());
    m_actor->Update();

    m_lastUpdatedTime = m_data->lastModified();
  }

  m_actor->SetVisibility(valid && isVisible());
}

//-----------------------------------------------------------------------------
QList<vtkProp*> ChannelSliceRepresentation::getActors()
{
  QList<vtkProp*> list;

  if (m_actor == nullptr)
    initializePipeline();

  list << m_actor.GetPointer();

  return list;
}

//-----------------------------------------------------------------------------
bool ChannelSliceRepresentation::isInside(const NmVector3 &point) const
{
  return contains(m_data->bounds(), point);
};

//-----------------------------------------------------------------------------
TransparencySelectionHighlighter *SegmentationSliceRepresentation::s_highlighter = new TransparencySelectionHighlighter();

const Representation::Type SegmentationSliceRepresentation::TYPE = "Slice";

//-----------------------------------------------------------------------------
SegmentationSliceRepresentation::SegmentationSliceRepresentation(DefaultVolumetricDataSPtr data,
                                                                 View2D *view)
: Representation(view)
, m_data(data)
, m_planeIndex(-1)
, m_exporter(nullptr)
, m_mapToColors(nullptr)
, m_actor(nullptr)
{
  setType(TYPE);
  //qDebug() << "Creating Solid Representation";
}

//-----------------------------------------------------------------------------
SegmentationSliceRepresentation::~SegmentationSliceRepresentation()
{
  //qDebug() << "Destroying Solid Representation";
}
//-----------------------------------------------------------------------------
RepresentationSettings *SegmentationSliceRepresentation::settingsWidget()
{
  return new SliceRepresentationSettings();
}

//-----------------------------------------------------------------------------
QString SegmentationSliceRepresentation::serializeSettings()
{
  QStringList values;

  values << Representation::serializeSettings();
  values << QString("%1").arg(m_color.alphaF());

  return values.join(";");
}

//-----------------------------------------------------------------------------
void SegmentationSliceRepresentation::restoreSettings(QString settings)
{
  if (!settings.isEmpty())
  {
    QStringList values = settings.split(";");

    double alphaF = values[1].toDouble();

    QColor currentColor = color();
    currentColor.setAlphaF(alphaF);

    Representation::restoreSettings(values[0]);
  }
}


//-----------------------------------------------------------------------------
void SegmentationSliceRepresentation::setColor(const QColor &color)
{
  Representation::setColor(color);

  if (m_actor != nullptr)
  {
    m_mapToColors->SetLookupTable(s_highlighter->lut(m_color, m_highlight));
    m_mapToColors->Update();
  }

  for (auto clone: m_clones)
    clone->setColor(color);
}

//-----------------------------------------------------------------------------
QColor SegmentationSliceRepresentation::color() const
{
  if (!m_clones.isEmpty())
    return m_clones.first()->color();
  else
    return EspINA::Representation::color();
}


//-----------------------------------------------------------------------------
void SegmentationSliceRepresentation::setHighlighted(bool highlighted)
{
  Representation::setHighlighted(highlighted);

  if (m_actor != nullptr)
    m_mapToColors->SetLookupTable(s_highlighter->lut(m_color, m_highlight));
}

//-----------------------------------------------------------------------------
bool SegmentationSliceRepresentation::hasActor(vtkProp *actor) const
{
  if (m_actor == nullptr)
    return false;

  return m_actor.GetPointer() == actor;
}

//-----------------------------------------------------------------------------
bool SegmentationSliceRepresentation::isInside(const NmVector3 &point) const
{
  Bounds bounds{ '[', point[0], point[0], point[1], point[1], point[2], point[2], ']'};

  itkVolumeType::Pointer voxel = m_data->itkImage(bounds);

  return (SEG_VOXEL_VALUE == *(static_cast<unsigned char*>(voxel->GetBufferPointer())));
}

//-----------------------------------------------------------------------------
void SegmentationSliceRepresentation::initializePipeline()
{
  if (m_planeIndex == -1)
    return;

  auto reslicePoint = m_crosshair[m_planeIndex];
  m_reslicePoint = reslicePoint + 1;

  View2D* view = reinterpret_cast<View2D *>(m_view);
/*
  Bounds imageBounds = m_data->bounds();

  imageBounds.setLowerInclusion(true);
  imageBounds.setUpperInclusion(toAxis(m_planeIndex), true);

  imageBounds[2*m_planeIndex]   = reslicePoint;
  imageBounds[2*m_planeIndex+1] = reslicePoint; */

  itkVolumeType::Pointer slice = itkVolumeType::New();
  itkVolumeType::RegionType region;
  slice->SetRegions(region);
  slice->Allocate();

 // itkVolumeType::Pointer slice = ;//m_data->itkImage(imageBounds);

  m_exporter = ExporterType::New();
  m_exporter->ReleaseDataFlagOn();
  m_exporter->SetNumberOfThreads(1);
  m_exporter->SetInput(slice);
  m_exporter->Update();

  // actor should be allocated first or the next call to setColor() would do nothing
  m_actor = vtkSmartPointer<vtkImageActor>::New();

  m_mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
  m_mapToColors->SetInputData(m_exporter->GetOutput());
  m_mapToColors->SetLookupTable(s_highlighter->lut(m_color));
  m_mapToColors->SetNumberOfThreads(1);
  m_mapToColors->Update();

  m_actor->SetInterpolate(false);
  m_actor->GetMapper()->BorderOn();
  m_actor->GetMapper()->SetInputConnection(m_mapToColors->GetOutputPort());
  m_actor->SetDisplayExtent(m_exporter->GetOutput()->GetExtent());
  m_actor->Update();

  // need to reposition the actor so it will always be over the channels actors'
  double pos[3];
  m_actor->GetPosition(pos);
  pos[m_planeIndex] = view->segmentationDepth();
  m_actor->SetPosition(pos);

  m_lastUpdatedTime = m_data->lastModified();
}

//-----------------------------------------------------------------------------
void SegmentationSliceRepresentation::updateRepresentation()
{
  setCrosshairPoint(m_view->crosshairPoint());

  Bounds imageBounds = m_data->bounds();

  bool valid = imageBounds[2*m_planeIndex] <= m_crosshair[m_planeIndex] && m_crosshair[m_planeIndex] <= imageBounds[2*m_planeIndex+1];

  if (m_actor != nullptr && ((m_crosshair[m_planeIndex] != m_reslicePoint) || needUpdate()) && valid)
  {
    m_reslicePoint = m_crosshair[m_planeIndex];

    imageBounds.setLowerInclusion(true);
    imageBounds.setUpperInclusion(toAxis(m_planeIndex), true);

    imageBounds[2*m_planeIndex]   = m_reslicePoint;
    imageBounds[2*m_planeIndex+1] = m_reslicePoint;

    itkVolumeType::Pointer slice = m_data->itkImage(imageBounds);

    m_exporter = ExporterType::New();
    m_exporter->ReleaseDataFlagOn();
    m_exporter->SetNumberOfThreads(1);
    m_exporter->SetInput(slice);
    m_exporter->Update();
    m_mapToColors->SetInputData(m_exporter->GetOutput());
    //m_mapToColors->SetLookupTable(s_highlighter->lut(m_color));
    m_mapToColors->Update();
    m_actor->SetDisplayExtent(m_exporter->GetOutput()->GetExtent());
    m_actor->Update();

    m_lastUpdatedTime = m_data->lastModified();
  }

  m_actor->SetVisibility(valid && isVisible());
}

//-----------------------------------------------------------------------------
QList<vtkProp*> SegmentationSliceRepresentation::getActors()
{
  QList<vtkProp *> list;

  if (m_actor == nullptr)
    initializePipeline();

  list << m_actor.GetPointer();

  return list;
}

//-----------------------------------------------------------------------------
RepresentationSPtr SegmentationSliceRepresentation::cloneImplementation(View2D *view)
{
  SegmentationSliceRepresentation *representation = new SegmentationSliceRepresentation(m_data, view);
  representation->setView(view);
  representation->m_planeIndex = normalCoordinateIndex(view->plane());

  return RepresentationSPtr(representation);
}

//-----------------------------------------------------------------------------
void SegmentationSliceRepresentation::updateVisibility(bool visible)
{
  if (m_actor != nullptr)
    m_actor->SetVisibility(visible);
}

