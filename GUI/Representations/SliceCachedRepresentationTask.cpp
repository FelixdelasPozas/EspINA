/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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
#include "CachedRepresentation.h"
#include "SliceCachedRepresentationTask.h"
#include <Core/Utils/Bounds.h>
#include <Core/Analysis/Data/VolumetricDataUtils.h>
#include <GUI/ColorEngines/TransparencySelectionHighlighter.h>

// VTK
#include <vtkImageMapToColors.h>
#include <vtkLookupTable.h>
#include <vtkImageActor.h>
#include <vtkImageShiftScale.h>
#include <vtkImageMapper3D.h>
#include <vtkAssembly.h>

// Qt
#include <QMutexLocker>

// C++
#include <chrono>

namespace EspINA
{
  //-----------------------------------------------------------------------------
  ChannelSliceCachedRepresentationTask::ChannelSliceCachedRepresentationTask(SchedulerSPtr scheduler)
  : CachedRepresentationTask(scheduler)
  , m_brightness{0.0}
  , m_contrast{0.0}
  , m_color{QColor()}
  , m_data{nullptr}
  , m_position{0}
  , m_plane{Plane::XY}
  {
  }
  
  //-----------------------------------------------------------------------------
  void ChannelSliceCachedRepresentationTask::setInput(DefaultVolumetricDataSPtr data, Nm position, Plane plane, double brightness, double contrast, QColor color, NmVector3 depth, CachedRepresentation::CacheNode* node)
  {
    // depth ignored for channels
    m_data = data;
    m_position = position;
    m_plane = plane;
    m_brightness = brightness;
    m_contrast = contrast;
    m_color = color;
    m_node = node;
  }
  
  //-----------------------------------------------------------------------------
  void ChannelSliceCachedRepresentationTask::run()
  {
    Q_ASSERT(m_data != nullptr);

    auto start = std::chrono::high_resolution_clock::now();

    int index = normalCoordinateIndex(m_plane);
    Bounds bounds = m_data->bounds();

    // check if we can create an actor for that position
    if ((bounds[2*index] > m_position) || (bounds[2*index + 1] <= m_position))
      return;

    emit progress(0);

    bounds[2*index] = bounds[2*index +1] = m_position;
    bounds.setLowerInclusion(true);
    bounds.setUpperInclusion(toAxis(index), true);

    vtkSmartPointer<vtkImageData> slice = vtkSmartPointer<vtkImageData>::New();
    slice = vtkImage(m_data, bounds);

    if (!canExecute())
    {
      slice = nullptr;
      return;
    }
    emit progress(20);

    vtkSmartPointer<vtkImageShiftScale> shiftScaleFilter = vtkSmartPointer<vtkImageShiftScale>::New();
    shiftScaleFilter->SetInputData(slice);
    shiftScaleFilter->SetNumberOfThreads(1);
    shiftScaleFilter->SetShift(static_cast<int>(m_brightness*255));
    shiftScaleFilter->SetScale(m_contrast);
    shiftScaleFilter->SetClampOverflow(true);
    shiftScaleFilter->SetOutputScalarType(slice->GetScalarType());
    shiftScaleFilter->Update();

    if (!canExecute())
    {
      slice = nullptr;
      shiftScaleFilter = nullptr;
      return;
    }
    emit progress(40);

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

    if (!canExecute())
    {
      slice = nullptr;
      shiftScaleFilter = nullptr;
      lut = nullptr;
      return;
    }
    emit progress(60);

    vtkSmartPointer<vtkImageMapToColors> mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
    mapToColors->SetInputConnection(shiftScaleFilter->GetOutputPort());
    mapToColors->SetLookupTable(lut);
    mapToColors->SetNumberOfThreads(1);
    mapToColors->Update();

    if (!canExecute())
    {
      slice = nullptr;
      shiftScaleFilter = nullptr;
      lut = nullptr;
      mapToColors = nullptr;
      return;
    }
    emit progress(80);

    vtkSmartPointer<vtkImageActor> actor = vtkSmartPointer<vtkImageActor>::New();
    actor->SetInterpolate(false);
    actor->GetMapper()->BorderOn();
    actor->GetMapper()->SetNumberOfThreads(1);
    actor->GetMapper()->SetInputConnection(mapToColors->GetOutputPort());
    actor->GetMapper()->Update();
    actor->SetDisplayExtent(slice->GetExtent());
    actor->Update();

    if (!canExecute())
    {
      slice = nullptr;
      shiftScaleFilter = nullptr;
      lut = nullptr;
      mapToColors = nullptr;
      actor = nullptr;
      return;
    }
    emit progress(100);

    m_node->mutex.lock();
    if (m_node->worker.get() != this)
    {
      m_node->mutex.unlock();
      slice = nullptr;
      shiftScaleFilter = nullptr;
      lut = nullptr;
      mapToColors = nullptr;
      actor = nullptr;
      return;
    }

    m_node->actor = actor;
    m_node->worker = nullptr;
    m_node->creationTime = m_data->lastModified();
    m_node->mutex.unlock();

    emit render(m_node);

    auto end = std::chrono::high_resolution_clock::now();
    m_executionTime = std::chrono::duration_cast < std::chrono::milliseconds > (end - start).count();
  }

  //-----------------------------------------------------------------------------
  SegmentationSliceCachedRepresentationTask::SegmentationSliceCachedRepresentationTask(SchedulerSPtr scheduler)
  : CachedRepresentationTask(scheduler)
  , m_color{QColor()}
  , m_data{nullptr}
  , m_position{0}
  , m_plane{Plane::XY}
  {
  }

  //-----------------------------------------------------------------------------
  void SegmentationSliceCachedRepresentationTask::setInput(DefaultVolumetricDataSPtr data, Nm position, Plane plane, double brightness, double contrast, QColor color, NmVector3 depth, CachedRepresentation::CacheNode *node)
  {
    // brightness & contrast ignored for segmentations
    m_data = data;
    m_position = position;
    m_color = color;
    m_depth = depth;
    m_plane = plane;
    m_node = node;
  }

  //-----------------------------------------------------------------------------
  void SegmentationSliceCachedRepresentationTask::run()
  {
    Q_ASSERT(m_data != nullptr);

    auto start = std::chrono::high_resolution_clock::now();

    int index = normalCoordinateIndex(m_plane);
    Bounds bounds = m_data->bounds();

    if ((bounds[2*index] > m_position) || (bounds[(2*index) +1] < m_position))
      return;

    bounds[2*index] = bounds[2*index+1] = m_position;
    bounds.setLowerInclusion(true);
    bounds.setUpperInclusion(toAxis(index), true);

    vtkSmartPointer<vtkImageData> slice = vtkSmartPointer<vtkImageData>::New();
    slice = vtkImage(m_data, bounds);

    if (!canExecute())
    {
      slice = nullptr;
      return;
    }
    emit progress(33);

    auto highlighter = std::shared_ptr<TransparencySelectionHighlighter>(new TransparencySelectionHighlighter());

    vtkSmartPointer<vtkImageMapToColors> mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
    mapToColors->SetInputData(slice);
    mapToColors->SetLookupTable(highlighter->lut(m_color));
    mapToColors->SetNumberOfThreads(1);
    mapToColors->Update();

    if (!canExecute())
    {
      slice = nullptr;
      mapToColors = nullptr;
      return;
    }
    emit progress(66);

    vtkSmartPointer<vtkImageActor> actor = vtkSmartPointer<vtkImageActor>::New();
    actor->SetInterpolate(false);
    actor->GetMapper()->BorderOn();
    actor->GetMapper()->SetNumberOfThreads(1);
    actor->GetMapper()->SetInputConnection(mapToColors->GetOutputPort());
    actor->GetMapper()->Update();
    actor->SetDisplayExtent(slice->GetExtent());
    actor->Update();

    Nm pos[3];
    actor->GetPosition(pos);
    pos[0] += m_depth[0];
    pos[1] += m_depth[1];
    pos[2] += m_depth[2];
    actor->SetPosition(pos);
    actor->Update();

    if (!canExecute())
    {
      slice = nullptr;
      mapToColors = nullptr;
      actor = nullptr;
      return;
    }
    emit progress(100);

    m_node->mutex.lock();
    if (m_node->worker.get() != this)
    {
      m_node->mutex.unlock();
      slice = nullptr;
      mapToColors = nullptr;
      actor = nullptr;
      return;
    }

    m_node->actor = actor;
    m_node->worker = nullptr;
    m_node->creationTime = m_data->lastModified();
    m_node->mutex.unlock();

    emit render(m_node);

    auto end = std::chrono::high_resolution_clock::now();
    m_executionTime = std::chrono::duration_cast < std::chrono::milliseconds > (end - start).count();
  }

} // namespace EspINA
