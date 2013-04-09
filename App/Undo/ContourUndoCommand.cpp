/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

namespace EspINA
{
  //-----------------------------------------------------------------------------
  ContourUndoCommand::ContourUndoCommand(SegmentationSPtr seg,
                                         vtkPolyData* contour,
                                         Nm pos,
                                         PlaneType plane,
                                         itkVolumeType::PixelType value,
                                         ViewManager *vm,
                                         FilledContour *tool)
  : m_segmentation(seg)
  , m_contour(contour)
  , m_plane(plane)
  , m_pos(pos)
  , m_value(value)
  , m_viewManager(vm)
  , m_needReduction(false)
  , m_tool(tool)
  , m_abortOperation(false)
  {
    contour->ComputeBounds();
    contour->GetBounds(m_contourBounds);
  }
  
  //-----------------------------------------------------------------------------
  ContourUndoCommand::~ContourUndoCommand()
  {
  }

  //-----------------------------------------------------------------------------
  void ContourUndoCommand::redo()
  {
    // we musn't abort the operation the first time is called
    if (m_abortOperation)
      m_tool->abortOperation();
    else
      m_abortOperation = true;

    if (m_newVolume.IsNotNull())
    {
      m_segmentation->filter()->draw(0, m_newVolume);
    }
    else
    {
      EspinaRegion contourRegion(m_contourBounds);
      EspinaVolume::Pointer volume = m_segmentation->volume();

      m_prevRegions = m_segmentation->filter()->output(0).editedRegions;

      if (!contourRegion.isInside(volume->espinaRegion()))
      {
        if (contourRegion.intersect(volume->espinaRegion()))
          m_prevVolume = volume->cloneVolume(contourRegion.intersection(volume->espinaRegion()));

        m_needReduction = true;
      }
      else
        m_prevVolume = volume->cloneVolume(contourRegion);

      m_segmentation->filter()->draw(0, m_contour, m_pos, m_plane, m_value, true);

      m_newVolume = m_segmentation->volume()->cloneVolume(contourRegion);
    }

    m_viewManager->updateSegmentationRepresentations(m_segmentation.data());
  }
  
  //-----------------------------------------------------------------------------
  void ContourUndoCommand::undo()
  {
    m_tool->abortOperation();

    Q_ASSERT(m_newVolume.IsNotNull());

    EspinaRegion contourRegion(m_contourBounds);
    m_segmentation->filter()->fill(0, contourRegion, SEG_BG_VALUE, false);

    if (m_prevVolume.IsNotNull())
      m_segmentation->filter()->draw(0, m_prevVolume, !m_needReduction);

    if (m_needReduction)
      m_segmentation->volume()->fitToContent();

    // Restore previous edited regions
    m_segmentation->filter()->output(0).editedRegions = m_prevRegions;

    m_viewManager->updateSegmentationRepresentations(m_segmentation.data());
  }

  //----------------------------------------------------------------------------
  ContourAddSegmentation::ContourAddSegmentation(ChannelSPtr channel,
                                                 FilterSPtr filter,
                                                 SegmentationSPtr seg,
                                                 TaxonomyElementSPtr taxonomy,
                                                 EspinaModel *model,
                                                 FilledContour *tool)
  : m_model         (model)
  , m_channel       (channel)
  , m_filter        (filter)
  , m_seg           (seg)
  , m_taxonomy      (taxonomy)
  , m_tool          (tool)
  , m_abortOperation(false)
  {
    m_sample = m_channel->sample();
    Q_ASSERT(m_sample);
  }

  //----------------------------------------------------------------------------
  void ContourAddSegmentation::redo()
  {
    if (m_abortOperation)
      m_tool->abortOperation();
    else
      m_abortOperation = true;

    m_model->addFilter(m_filter);
    m_model->addRelation(m_channel, m_filter, Channel::LINK);
    m_seg->setTaxonomy(m_taxonomy);
    m_model->addSegmentation(m_seg);
    m_model->addRelation(m_filter , m_seg, Filter::CREATELINK);
    m_model->addRelation(m_sample , m_seg, Relations::LOCATION);
    m_model->addRelation(m_channel, m_seg, Channel::LINK);
    m_seg->initializeExtensions();
  }

  //----------------------------------------------------------------------------
  void ContourAddSegmentation::undo()
  {
    m_tool->abortOperation();
    m_model->removeRelation(m_channel, m_seg, Channel::LINK);
    m_model->removeRelation(m_sample , m_seg, Relations::LOCATION);
    m_model->removeRelation(m_filter , m_seg, Filter::CREATELINK);
    m_model->removeSegmentation(m_seg);
    m_model->removeRelation(m_channel, m_filter, Channel::LINK);
    m_model->removeFilter(m_filter);
  }

} /* namespace EspINA */
