/*
 
 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// EspINA
#include "SliceCachedRepresentation.h"
#include "RepresentationEmptySettings.h"
#include <Core/Analysis/Data/VolumetricDataUtils.h>
#include <GUI/ColorEngines/TransparencySelectionHighlighter.h>

// VTK
#include <vtkMath.h>
#include <vtkImageActor.h>
#include <vtkImageMapToColors.h>
#include <vtkImageShiftScale.h>
#include <vtkLookupTable.h>
#include <vtkImageMapper3D.h>
#include <vtkProperty.h>

namespace EspINA
{
  //-----------------------------------------------------------------------------
  CachedRepresentation::CachedRepresentation(DefaultVolumetricDataSPtr data,
                                             View2D *view)
  : Representation(view)
  , m_data{data}
  , m_planeIndex{-1}
  , m_min{-1}
  , m_max{-1}
  {}

  //-----------------------------------------------------------------------------
  bool CachedRepresentation::existsIn(const Nm position) const
  {
    if (m_planeIndex == -1)
      return false;

    return (m_min <= position) && (position < m_max);
  }

  //-----------------------------------------------------------------------------
  void CachedRepresentation::computeLimits()
  {
    if(m_planeIndex == -1)
      return;

    VolumeBounds imageBounds{ m_data->bounds(), m_data->spacing(), m_data->origin() };

    m_min = imageBounds[2*m_planeIndex];
    m_max = imageBounds[2*m_planeIndex + 1];
  }

  //-----------------------------------------------------------------------------
  const Representation::Type ChannelSliceCachedRepresentation::TYPE = "Channel Slice (Cached)";

  //-----------------------------------------------------------------------------
  ChannelSliceCachedRepresentation::ChannelSliceCachedRepresentation(DefaultVolumetricDataSPtr data,
                                                                     View2D *view)
  : CachedRepresentation(data, view)
  {
    setType(TYPE);
  }
  
  //-----------------------------------------------------------------------------
  RepresentationSPtr ChannelSliceCachedRepresentation::cloneImplementation(View2D* view)
  {
    ChannelSliceCachedRepresentation *representation =  new ChannelSliceCachedRepresentation(m_data, view);
    representation->setView(view);

    return RepresentationSPtr(representation);
  }

  //-----------------------------------------------------------------------------
  bool ChannelSliceCachedRepresentation::isInside(const NmVector3 &point) const
  {
    return contains(m_data->bounds(), point);
  }

  //-----------------------------------------------------------------------------
  void ChannelSliceCachedRepresentation::setView(View2D *view)
  {
    m_view = view;
    m_planeIndex = normalCoordinateIndex(view->plane());

    computeLimits();
  }

  //-----------------------------------------------------------------------------
  vtkSmartPointer<vtkImageActor> ChannelSliceCachedRepresentation::getActor(const Nm slicePos) const
  {
    if (m_planeIndex == -1 || m_view == nullptr)
      return nullptr;

    if (!existsIn(slicePos))
      return nullptr;

    Bounds imageBounds = m_data->bounds();
    imageBounds.setLowerInclusion(true);
    imageBounds.setUpperInclusion(toAxis(m_planeIndex), true);
    imageBounds[2*m_planeIndex] = imageBounds[(2*m_planeIndex)+1] = slicePos;

    auto slice = vtkImage(m_data, imageBounds);

    vtkSmartPointer<vtkImageShiftScale> shiftScaleFilter = vtkSmartPointer<vtkImageShiftScale>::New();
    shiftScaleFilter->SetInputData(slice);
    shiftScaleFilter->SetShift(static_cast<int>(m_brightness*255));
    shiftScaleFilter->SetScale(m_contrast);
    shiftScaleFilter->SetClampOverflow(true);
    shiftScaleFilter->SetOutputScalarType(slice->GetScalarType());
    shiftScaleFilter->Update();

    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->Allocate();
    lut->SetTableRange(0,255);
    lut->SetHueRange(m_color.hueF(), m_color.hueF());
    lut->SetSaturationRange(0.0, m_color.saturationF());
    lut->SetValueRange(0.0, 1.0);
    lut->SetAlphaRange(1.0,1.0);
    lut->SetNumberOfColors(256);
    lut->SetRampToLinear();
    lut->Build();

    vtkSmartPointer<vtkImageMapToColors> mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
    mapToColors->SetInputConnection(shiftScaleFilter->GetOutputPort());
    mapToColors->SetLookupTable(lut);
    mapToColors->SetNumberOfThreads(1);
    mapToColors->Update();

    vtkSmartPointer<vtkImageActor> actor = vtkSmartPointer<vtkImageActor>::New();
    actor->SetInterpolate(false);
    actor->GetMapper()->BorderOn();
    actor->GetMapper()->SetInputConnection(mapToColors->GetOutputPort());
    actor->GetMapper()->ReleaseDataFlagOn();
    actor->SetDisplayExtent(slice->GetExtent());
    actor->SetOpacity(m_opacity);
    actor->Update();

    m_lastUpdatedTime = m_data->lastModified();
    return actor;
  }

  //-----------------------------------------------------------------------------
  void ChannelSliceCachedRepresentation::updateRepresentation()
  {
    if(m_data->lastModified() != m_lastUpdatedTime)
    {
      computeLimits();
      emit update();
    }
    else
    {
      emit changeVisibility();
    }
  }

  //-----------------------------------------------------------------------------
  void ChannelSliceCachedRepresentation::updateVisibility(bool unused)
  {
    emit changeVisibility();
  }

  //-----------------------------------------------------------------------------
  TransparencySelectionHighlighter *SegmentationSliceCachedRepresentation::s_highlighter = new TransparencySelectionHighlighter();
  const Representation::Type SegmentationSliceCachedRepresentation::TYPE = "Segmentation Slice (Cached)";

  //-----------------------------------------------------------------------------
  SegmentationSliceCachedRepresentation::SegmentationSliceCachedRepresentation(DefaultVolumetricDataSPtr data, View2D *view)
  : CachedRepresentation(data, view)
  , m_depth{NmVector3()}
  {
    setType(TYPE);
    connect(data.get(), SIGNAL(dataChanged()), this, SLOT(dataChanged()), Qt::QueuedConnection);
  }

  //-----------------------------------------------------------------------------
  QString SegmentationSliceCachedRepresentation::serializeSettings()
  {
    QStringList values;

    values << Representation::serializeSettings();
    values << QString("%1").arg(m_color.alphaF());

    return values.join(";");
  }

  //-----------------------------------------------------------------------------
  void SegmentationSliceCachedRepresentation::restoreSettings(QString settings)
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
  void SegmentationSliceCachedRepresentation::setColor(const QColor& color)
  {
    Representation::setColor(color);

    for (auto clone: m_clones)
      clone->setColor(color);

    emit changeColor();
  }
  
  //-----------------------------------------------------------------------------
  QColor SegmentationSliceCachedRepresentation::color() const
  {
    if (!m_clones.isEmpty())
      return m_clones.first()->color();
    else
      return Representation::color();
  }
  
  //-----------------------------------------------------------------------------
  void SegmentationSliceCachedRepresentation::setHighlighted(bool highlighted)
  {
    Representation::setHighlighted(highlighted);

    for (auto clone: m_clones)
      clone->setHighlighted(highlighted);

    emit changeColor();
  }
  
  //-----------------------------------------------------------------------------
  bool SegmentationSliceCachedRepresentation::isInside(const NmVector3& point) const
  {
    return isSegmentationVoxel(m_data, point);
  }

  //-----------------------------------------------------------------------------
  RepresentationSPtr SegmentationSliceCachedRepresentation::cloneImplementation(View2D* view)
  {
    SegmentationSliceCachedRepresentation *representation =  new SegmentationSliceCachedRepresentation(m_data, view);
    representation->setView(view);

    return RepresentationSPtr(representation);
  }
  
  //-----------------------------------------------------------------------------
  vtkSmartPointer<vtkImageActor> SegmentationSliceCachedRepresentation::getActor(const Nm slicePos) const
  {
    if (m_planeIndex == -1 || m_view == nullptr)
      return nullptr;

    if (!existsIn(slicePos))
      return nullptr;

    Bounds imageBounds = m_data->bounds();
    imageBounds.setLowerInclusion(true);
    imageBounds.setUpperInclusion(toAxis(m_planeIndex), true);
    imageBounds[2*m_planeIndex] = imageBounds[2*m_planeIndex+1] = slicePos;

    auto slice = vtkImage(m_data, imageBounds);

    vtkSmartPointer<vtkImageMapToColors> mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
    mapToColors->SetInputData(slice);
    mapToColors->SetLookupTable(s_highlighter->lut(m_color, m_highlight));
    mapToColors->SetNumberOfThreads(1);
    mapToColors->Update();

    vtkSmartPointer<vtkImageActor> actor = vtkSmartPointer<vtkImageActor>::New();
    actor->SetInterpolate(false);
    actor->GetMapper()->BorderOn();
    actor->GetMapper()->ReleaseDataFlagOn();
    actor->GetMapper()->SetNumberOfThreads(1);
    actor->GetMapper()->SetInputConnection(mapToColors->GetOutputPort());
    actor->SetDisplayExtent(slice->GetExtent());

    // need to reposition the actor so it will always be over the channels actors'
    double pos[3];
    actor->GetPosition(pos);
    pos[m_planeIndex] += m_depth[m_planeIndex];
    actor->SetPosition(pos);

    actor->SetOpacity(m_opacity);
    actor->Update();

    m_lastUpdatedTime = m_data->lastModified();
    return actor;
  }

  //-----------------------------------------------------------------------------
  void SegmentationSliceCachedRepresentation::setView(View2D *view)
  {
    m_view = view;
    m_planeIndex = normalCoordinateIndex(view->plane());
    m_depth[m_planeIndex] = view->segmentationDepth();

    computeLimits();
  }

  //-----------------------------------------------------------------------------
  void SegmentationSliceCachedRepresentation::updateRepresentation()
  {
    if(m_data->lastModified() != m_lastUpdatedTime)
    {
      computeLimits();
      emit update();
    }
    else
    {
      emit changeVisibility();
      emit changeColor();
    }
  }

  //-----------------------------------------------------------------------------
  void SegmentationSliceCachedRepresentation::updateVisibility(bool value)
  {
    emit changeVisibility();
  }

  //-----------------------------------------------------------------------------
  void SegmentationSliceCachedRepresentation::dataChanged()
  {
    if(m_data->lastModified() != m_lastUpdatedTime)
    {
      computeLimits();
      emit update();
    }
  }


} // namespace EspINA
