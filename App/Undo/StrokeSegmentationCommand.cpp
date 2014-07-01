/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "StrokeSegmentationCommand.h"
#include <Core/Relations.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/Sample.h>
#include <GUI/Representations/BasicGraphicalRepresentationFactory.h>

using namespace EspINA;

//------------------------------------------------------------------------
StrokeSegmentationCommand::StrokeSegmentationCommand(ChannelPtr channel,
                                                     TaxonomyElementPtr taxonomy,
                                                     Brush::BrushShapeList brushes,
                                                     SegmentationSPtr &segmentation,
                                                     EspinaModel *model)
: QUndoCommand()
, m_model(model)
, m_sample(channel->sample())
, m_channel(model->findChannel(channel))
, m_taxonomy(model->findTaxonomyElement(taxonomy))
{
  double strokeBounds[6];
  for (int i = 0; i < brushes.size(); i++)
  {
    Brush::BrushShape &brush = brushes[i];
    if (0 == i)
      memcpy(strokeBounds, brush.second.bounds(), 6*sizeof(double));
    else
    {
      for (int i=0; i < 6; i+=2)
        strokeBounds[i] = std::min(brush.second.bounds()[i], strokeBounds[i]);
      for (int i=1; i < 6; i+=2)
        strokeBounds[i] = std::max(brush.second.bounds()[i], strokeBounds[i]);
    }
  }

  m_filter = FilterSPtr(new FreeFormSource(EspinaRegion(strokeBounds),
                                           channel->volume()->spacing(),
                                           Brush::FREEFORM_SOURCE_TYPE));
  SetBasicGraphicalRepresentationFactory(m_filter);

  SegmentationVolumeSPtr volume = segmentationVolume(m_filter->output(0));
  for (int i = 0; i < brushes.size(); i++)
  {
    Brush::BrushShape &brush = brushes[i];
    volume->draw(brush.first, brush.second.bounds(), SEG_VOXEL_VALUE, brushes.size()-1 == i);
  }

  m_segmentation = m_model->factory()->createSegmentation(m_filter, 0);
  segmentation   = m_segmentation;
}

//------------------------------------------------------------------------
void StrokeSegmentationCommand::redo()
{
  m_segmentation->setTaxonomy(m_taxonomy);

  m_model->addFilter(m_filter);
  m_model->addRelation(m_channel, m_filter, Channel::LINK);
  m_model->addSegmentation(m_segmentation);
  m_model->addRelation(m_filter , m_segmentation, Filter::CREATELINK);
  m_model->addRelation(m_sample , m_segmentation, Relations::LOCATION);
  m_model->addRelation(m_channel, m_segmentation, Channel::LINK);
}

//------------------------------------------------------------------------
void StrokeSegmentationCommand::undo()
{
  m_model->removeRelation(m_channel, m_segmentation, Channel::LINK);
  m_model->removeRelation(m_sample , m_segmentation, Relations::LOCATION);
  m_model->removeRelation(m_filter , m_segmentation, Filter::CREATELINK);
  m_model->removeSegmentation(m_segmentation);
  m_model->removeRelation(m_channel, m_filter, Channel::LINK);
  m_model->removeFilter(m_filter);
}
