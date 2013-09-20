/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Felix de las Pozas Alvarez <felixdelaspozas@gmail.com>

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
#include "ContourUndoCommand.h"
#include <App/Tools/Contour/FilledContour.h>
#include <GUI/ViewManager.h>
#include <Core/Model/Channel.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Sample.h>
#include <Core/Model/Segmentation.h>
#include <Core/Relations.h>
#include <Core/Filters/FreeFormSource.h>
#include <Core/EspinaRegion.h>
#include <Core/Model/EspinaFactory.h>
#include <GUI/Representations/BasicGraphicalRepresentationFactory.h>

// VTK
#include <vtkMath.h>

namespace EspINA
{
  //-----------------------------------------------------------------------------
  ContourUndoCommand::ContourUndoCommand(SegmentationSPtr seg,
                                         ViewManager *vm,
                                         FilledContour *tool)
  : m_segmentation(seg)
  , m_viewManager(vm)
  , m_needReduction(false)
  , m_tool(tool)
  , m_rasterized(false)
  {
  }

  //-----------------------------------------------------------------------------
  ContourUndoCommand::~ContourUndoCommand()
  {
    if (m_contour.PolyData != NULL)
      m_contour.PolyData->Delete();
  }

  //-----------------------------------------------------------------------------
  void ContourUndoCommand::redo()
  {
    if (m_rasterized)
    {
      SegmentationVolumeSPtr volume = segmentationVolume(m_segmentation->output());
      volume->draw(m_newVolume);

      if (m_needReduction)
        volume->fitToContent();

      m_viewManager->updateSegmentationRepresentations(m_segmentation.get());
    }
    else
    {
      if (m_contour.PolyData != NULL)
        m_tool->setContour(m_contour);
    }
  }

  //-----------------------------------------------------------------------------
  void ContourUndoCommand::undo()
  {
    if (m_rasterized)
    {
      SegmentationVolumeSPtr volume = segmentationVolume(m_segmentation->output());
      EspinaRegion region(m_bounds);
      volume->fill(region, SEG_BG_VALUE, false);

      if (m_prevVolume.GetPointer() != NULL)
        volume->draw(m_prevVolume, false);

      volume->setEditedRegions(m_prevRegions);

      if (m_needReduction)
        volume->fitToContent();

      m_viewManager->updateSegmentationRepresentations(m_segmentation.get());
    }
    else
    {
      ContourWidget::ContourData contour = m_tool->getContour();
      Q_ASSERT(contour.PolyData != NULL && contour.PolyData->GetNumberOfPoints() != 0);
      m_contour.PolyData = vtkPolyData::New();
      m_contour.PolyData->DeepCopy(contour.PolyData);
      m_contour.Plane = contour.Plane;
      m_contour.Mode = contour.Mode;

      contour.PolyData = NULL;
      m_tool->setContour(contour);
    }
  }

  //----------------------------------------------------------------------------
  bool ContourUndoCommand::intersect(const EspinaRegion &region1, const EspinaRegion &region2) const
  {
    bool xOverlap = region1.xMin() < region2.xMax() && region1.xMax() > region2.xMin();
    bool yOverlap = region1.yMin() < region2.yMax() && region1.yMax() > region2.yMin();
    bool zOverlap = region1.zMin() < region2.zMax() && region1.zMax() > region2.zMin();

    return xOverlap && yOverlap && zOverlap;
  }

  //----------------------------------------------------------------------------
  void ContourUndoCommand::rasterizeContour(ContourWidget::ContourData contour) const
  {
    SegmentationVolumeSPtr volume = segmentationVolume(m_segmentation->output());
    if (m_contour.PolyData != NULL)
    {
      m_contour.PolyData->Delete();
      m_contour.PolyData = NULL;
    }
    else
      m_prevRegions = volume->editedRegions();

    Nm spacingNm[3];
    PlaneType plane = contour.Plane;
    contour.PolyData->ComputeBounds();
    contour.PolyData->GetBounds(m_bounds);

    // bounds need to be corrected in the contour plane and adapted to
    // the soon-to-be-created segmentation volume.
    // FIXME: This bounds are wrong to correct a bug in cloneVolume().
    // "two wrongs make a right" this time.
    volume->spacing(spacingNm);
    m_bounds[(2*plane)+1] = m_bounds[2*plane] + spacingNm[plane];

    for (int i = 0; i < 3; i++)
    {
      if (i == plane)
        continue;

      int voxelIndex = vtkMath::Floor((m_bounds[2*i]+spacingNm[i]/2)/spacingNm[i]);
      m_bounds[2*i] = voxelIndex * spacingNm[i];

      voxelIndex = vtkMath::Floor((m_bounds[(2*i)+1]+spacingNm[i]/2)/spacingNm[i]);
      m_bounds[(2*i)+1] = (voxelIndex+1) * spacingNm[i] - spacingNm[i]/2;
    }

    EspinaRegion contourRegion(m_bounds);

    if (!contourRegion.isInside(volume->espinaRegion()))
    {
      m_needReduction = true;
      EspinaRegion volumeRegion = volume->espinaRegion();
      if (intersect(contourRegion, volumeRegion))
        m_prevVolume = volume->cloneVolume(contourRegion.intersection(volume->espinaRegion()));
    }
    else
      m_prevVolume = volume->cloneVolume(contourRegion);

    Nm pos = m_bounds[2*plane];
    itkVolumeType::PixelType value = ((contour.Mode == Brush::BRUSH) ? SEG_VOXEL_VALUE : SEG_BG_VALUE);
    volume->draw(contour.PolyData, pos, plane, value);
    m_newVolume = volume->cloneVolume(contourRegion);

    m_viewManager->updateSegmentationRepresentations(m_segmentation.get());

    m_rasterized = true;
  }

  //----------------------------------------------------------------------------
  ContourAddSegmentation::ContourAddSegmentation(ChannelSPtr channel,
                                                 TaxonomyElementSPtr taxonomy,
                                                 EspinaModel *model,
                                                 ViewManager *vm,
                                                 FilledContour *tool)
  : m_model         (model)
  , m_channel       (channel)
  , m_taxonomy      (taxonomy)
  , m_viewManager   (vm)
  , m_tool          (tool)
  , m_rasterized    (false)
  {
    m_sample = m_channel->sample();
    Q_ASSERT(m_sample);

    m_contour.PolyData = NULL;
  }

  //----------------------------------------------------------------------------
  ContourAddSegmentation::~ContourAddSegmentation()
  {
    if (m_contour.PolyData != NULL)
      m_contour.PolyData->Delete();
  }

  //----------------------------------------------------------------------------
  void ContourAddSegmentation::redo()
  {
    if (m_rasterized)
    {
      m_model->addFilter(m_filter);
      m_model->addRelation(m_channel, m_filter, Channel::LINK);
      m_segmentation->setTaxonomy(m_taxonomy);
      m_model->addSegmentation(m_segmentation);
      m_model->addRelation(m_filter , m_segmentation, Filter::CREATELINK);
      m_model->addRelation(m_sample , m_segmentation, Relations::LOCATION);
      m_model->addRelation(m_channel, m_segmentation, Channel::LINK);
      m_segmentation->initializeExtensions();
    }
    else
    {
      if (m_contour.PolyData != NULL)
        m_tool->setContour(m_contour);
    }
  }

  //----------------------------------------------------------------------------
  void ContourAddSegmentation::undo()
  {
    if (m_rasterized)
    {
      m_model->removeRelation(m_channel, m_segmentation, Channel::LINK);
      m_model->removeRelation(m_sample , m_segmentation, Relations::LOCATION);
      m_model->removeRelation(m_filter , m_segmentation, Filter::CREATELINK);
      m_model->removeSegmentation(m_segmentation);
      m_model->removeRelation(m_channel, m_filter, Channel::LINK);
      m_model->removeFilter(m_filter);
    }
    else
    {
      ContourWidget::ContourData contour = m_tool->getContour();
      Q_ASSERT(contour.PolyData != NULL && contour.PolyData->GetNumberOfPoints() != 0);
      m_contour.PolyData = vtkPolyData::New();
      m_contour.PolyData->DeepCopy(contour.PolyData);
      m_contour.Plane = contour.Plane;
      m_contour.Mode = contour.Mode;

      contour.PolyData = NULL;
      m_tool->setContour(contour);
    }
  }

  //----------------------------------------------------------------------------
  void ContourAddSegmentation::rasterizeContour(ContourWidget::ContourData contour) const
  {
    if (m_contour.PolyData != NULL)
    {
      m_contour.PolyData->Delete();
      m_contour.PolyData = NULL;
    }

    Nm polyBounds[6], spacingNm[3];
    PlaneType plane = contour.Plane;
    contour.PolyData->GetBounds(polyBounds);

    // bounds need to be corrected in the contour plane and adapted to
    // the soon-to-be-created segmentation volume
    // FIXME: This bounds are wrong on purpose, as we want to correct a bug
    // in RawVolume::volumeRegionAux(). "two wrongs make a right" again.
    m_channel->volume()->spacing(spacingNm);

    polyBounds[(2*plane)+1] = polyBounds[(2*plane)] + spacingNm[plane]/2;

    for (int i = 0; i < 3; i++)
    {
      if (i == plane)
        continue;

      int voxelIndex = vtkMath::Floor((polyBounds[2*i]+spacingNm[i]/2)/spacingNm[i]);
      polyBounds[2*i] = voxelIndex * spacingNm[i];

      voxelIndex = vtkMath::Floor((polyBounds[(2*i)+1]+spacingNm[i]/2)/spacingNm[i]);
      polyBounds[(2*i)+1] = ((voxelIndex+1) * spacingNm[i]) - spacingNm[i]/2;
    }

    m_filter = FilterSPtr(new FreeFormSource(EspinaRegion(polyBounds),
                          m_channel->volume()->spacing(),
                          FilledContour::FILTER_TYPE));
    SetBasicGraphicalRepresentationFactory (m_filter);

    // correct bounds now after the filter has been created
    Nm pos = polyBounds[2*plane];
    for (int i = 0; i < 3; i++)
      polyBounds[2*i] -= spacingNm[i]/2;

    itkVolumeType::PixelType value = ((contour.Mode == Brush::BRUSH) ? SEG_VOXEL_VALUE : SEG_BG_VALUE);

    SegmentationVolumeSPtr currentVolume = segmentationVolume(m_filter->output(0));
    currentVolume->draw(contour.PolyData, pos, plane, value, true);
    currentVolume->fitToContent();
    m_segmentation = m_model->factory()->createSegmentation(m_filter, 0);

    m_model->addFilter(m_filter);
    m_model->addRelation(m_channel, m_filter, Channel::LINK);
    m_segmentation->setTaxonomy(m_taxonomy);
    m_model->addSegmentation(m_segmentation);
    m_model->addRelation(m_filter , m_segmentation, Filter::CREATELINK);
    m_model->addRelation(m_sample , m_segmentation, Relations::LOCATION);
    m_model->addRelation(m_channel, m_segmentation, Channel::LINK);
    m_segmentation->initializeExtensions();

    SegmentationSList createdSegmentations;
    createdSegmentations << m_segmentation;
    m_model->emitSegmentationAdded(createdSegmentations);

    ViewManager::Selection selectedSegmentations;
    selectedSegmentations << pickableItemPtr(m_segmentation.get());
    m_viewManager->setSelection(selectedSegmentations);

    m_rasterized = true;
  }

  //----------------------------------------------------------------------------
  SegmentationSPtr ContourAddSegmentation::getCreatedSegmentation() const
  {
    return m_segmentation;
  }

} /* namespace EspINA */

