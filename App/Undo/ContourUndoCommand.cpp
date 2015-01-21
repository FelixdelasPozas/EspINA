/*
 
 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Factory/FilterFactory.h>
#include <Core/Analysis/Filter.h>
#include <Core/Factory/CoreFactory.h>
#include <Core/Utils/vtkPolyDataUtils.h>
#include <Support/ViewManager.h>
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Model/SampleAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Model/Utils/ModelAdapterUtils.h>
#include <Filters/SourceFilter.h>
#include <GUI/Representations/BasicRepresentationFactory.h>
#include <Undo/ContourUndoCommand.h>

// VTK
#include <vtkMath.h>
#include <vtkPolyData.h>

#include "Core/Analysis/Sample.h"
namespace ESPINA
{
  const Filter::Type SOURCE_FILTER    = "FreeFormSource";

  //-----------------------------------------------------------------------------
  ContourUndoCommand::ContourUndoCommand(SegmentationAdapterPtr seg,
                                         ViewManagerSPtr        vm,
                                         ManualEditionToolSPtr  tool)
  : m_segmentation {seg}
  , m_viewManager  {vm}
  , m_tool         {tool}
  , m_rasterized   {false}
  , m_createData   {!hasVolumetricData(seg->output())}
  , m_value        {SEG_VOXEL_VALUE}
  {
  }

  //-----------------------------------------------------------------------------
  ContourUndoCommand::~ContourUndoCommand()
  {
    if (m_contour.polyData)
    {
      m_contour.polyData->Delete();
    }
  }

  //-----------------------------------------------------------------------------
  void ContourUndoCommand::redo()
  {
    if (m_rasterized)
    {
      if(m_createData)
      {
        auto volume = std::make_shared<SparseVolume<itkVolumeType>>(m_volume->bounds().bounds(), m_volume->spacing(), m_volume->origin());
        volume->draw(m_volume);

        m_segmentation->output()->setData(volume);
      }
      else
      {
        auto volume = volumetricData(m_segmentation->output());
        auto bbox = boundingBox(m_volume->bounds().bounds(), volume->bounds());
        volume->resize(bbox);
        volume->draw(m_volume, m_value);

        if (m_value == SEG_BG_VALUE)
        {
          fitToContents(volume, SEG_BG_VALUE);
        }
      }

      m_viewManager->updateSegmentationRepresentations(m_segmentation);
    }
    else
    {
      if (m_contour.polyData)
      {
        m_tool->setContour(m_contour);
      }
    }
  }

  //-----------------------------------------------------------------------------
  void ContourUndoCommand::undo()
  {
    if (m_rasterized)
    {
      if(!m_createData)
      {
        auto volume = volumetricData(m_segmentation->output());
        volume->undo();
      }
      else
      {
        m_segmentation->output()->removeData(SparseVolume<itkVolumeType>::TYPE);
      }

      m_viewManager->updateSegmentationRepresentations(m_segmentation);
    }
    else
    {
      ContourWidget::ContourData contour = m_tool->getContour();
      Q_ASSERT(contour.polyData && contour.polyData->GetNumberOfPoints() != 0);
      m_contour.polyData = vtkPolyData::New();
      m_contour.polyData->DeepCopy(contour.polyData);
      m_contour.plane = contour.plane;
      m_contour.mode = contour.mode;

      contour.polyData = nullptr;
      m_tool->setContour(contour);
    }
  }

  //----------------------------------------------------------------------------
  void ContourUndoCommand::rasterizeContour(ContourWidget::ContourData contour) const
  {
    if (m_contour.polyData)
    {
      m_contour.polyData->Delete();
      m_contour.polyData = nullptr;
    }

    m_volume = PolyDataUtils::rasterizeContourToMask(contour.polyData, contour.plane, contour.contourPosition, m_segmentation->output()->spacing());
    m_value = (contour.mode == BrushSelector::BrushMode::BRUSH ? SEG_VOXEL_VALUE : SEG_BG_VALUE);
    m_rasterized = true;

    if(m_createData)
    {
      auto volume = std::make_shared<SparseVolume<itkVolumeType>>(m_volume->bounds().bounds(), m_volume->spacing(), m_volume->origin());
      volume->draw(m_volume);

      m_segmentation->output()->setData(volume);
    }
    else
    {
      auto volume = volumetricData(m_segmentation->output());
      auto bbox = boundingBox(m_volume->bounds().bounds(), volume->bounds());
      volume->resize(bbox);
      volume->draw(m_volume, m_value);

      if (m_value == SEG_BG_VALUE)
      {
        fitToContents(volume, SEG_BG_VALUE);
      }
    }

    m_viewManager->updateSegmentationRepresentations(m_segmentation);
  }

  //----------------------------------------------------------------------------
  ContourAddSegmentation::ContourAddSegmentation(ChannelAdapterPtr     channel,
                                                 CategoryAdapterSPtr   category,
                                                 ModelAdapterSPtr      model,
                                                 ModelFactorySPtr      factory,
                                                 ViewManagerSPtr       vm,
                                                 ManualEditionToolSPtr tool)
  : m_model         {model}
  , m_factory       {factory}
  , m_channel       {channel}
  , m_category      {category}
  , m_viewManager   {vm}
  , m_tool          {tool}
  , m_rasterized    {false}
  {
    m_sample = QueryAdapter::sample(m_channel);
    Q_ASSERT(m_sample);

    m_contour.polyData = nullptr;
  }

  //----------------------------------------------------------------------------
  ContourAddSegmentation::~ContourAddSegmentation()
  {
    if (m_contour.polyData)
    {
      m_contour.polyData->Delete();
    }
  }

  //----------------------------------------------------------------------------
  void ContourAddSegmentation::redo()
  {
    if (m_rasterized)
    {
      m_model->add(m_segmentation);
      m_model->addRelation(m_sample, m_segmentation, Sample::CONTAINS);
      m_model->emitSegmentationsAdded(m_segmentation);
    }
    else
    {
      if (m_contour.polyData)
      {
        m_tool->setContour(m_contour);
      }
    }
  }

  //----------------------------------------------------------------------------
  void ContourAddSegmentation::undo()
  {
    if (m_rasterized)
    {
      m_model->remove(m_segmentation);
    }
    else
    {
      auto contour = m_tool->getContour();
      Q_ASSERT(contour.polyData && contour.polyData->GetNumberOfPoints() != 0);
      m_contour.polyData = vtkPolyData::New();
      m_contour.polyData->DeepCopy(contour.polyData);
      m_contour.plane = contour.plane;
      m_contour.mode = contour.mode;

      contour.polyData = nullptr;
      m_tool->setContour(contour);
    }
  }

  //----------------------------------------------------------------------------
  void ContourAddSegmentation::rasterizeContour(ContourWidget::ContourData contour) const
  {
    if (m_contour.polyData)
    {
      m_contour.polyData->Delete();
      m_contour.polyData = nullptr;
    }

    auto output  = m_channel->output();
    auto filter = m_factory->createFilter<SourceFilter>(InputSList(), SOURCE_FILTER);

    double bounds[6];
    contour.polyData->GetBounds(bounds);
    auto contourBounds = Bounds{bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]};
    auto spacing = output->spacing();
    auto origin  = m_channel->position();

    auto volume = std::make_shared<SparseVolume<itkVolumeType>>(contourBounds, spacing, origin);
    auto mask = PolyDataUtils::rasterizeContourToMask(contour.polyData, contour.plane, contour.contourPosition, spacing);
    volume->draw(mask);

    auto mesh = std::make_shared<MarchingCubesMesh<itkVolumeType>>(volume);

    filter->addOutputData(0, volume);
    filter->addOutputData(0, mesh);

    m_segmentation = m_factory->createSegmentation(filter, 0);
    m_segmentation->setCategory(m_category);

    unsigned int number = ModelAdapterUtils::firstUnusedSegmentationNumber(m_model);
    m_segmentation->setNumber(number);

    m_model->add(m_segmentation);
    m_model->addRelation(m_sample, m_segmentation, Sample::CONTAINS);
    m_model->emitSegmentationsAdded(m_segmentation);

    ViewItemAdapterList selection;
    selection << m_segmentation.get();
    m_viewManager->setSelection(selection);

    m_rasterized = true;
  }

  //----------------------------------------------------------------------------
  SegmentationAdapterSPtr ContourAddSegmentation::getCreatedSegmentation() const
  {
    return m_segmentation;
  }

} /* namespace ESPINA */

