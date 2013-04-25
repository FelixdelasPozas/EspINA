/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#include "BrushUndoCommand.h"
#include <Core/Model/Filter.h>
#include <GUI/ViewManager.h>

#include <itkExtractImageFilter.h>

using namespace EspINA;

typedef itk::ExtractImageFilter<itkVolumeType, itkVolumeType> ExtractType;

//-----------------------------------------------------------------------------
Brush::DrawCommand::DrawCommand(SegmentationSPtr seg,
                                BrushShapeList brushes,
                                itkVolumeType::PixelType value,
                                ViewManager *vm,
                                Brush *parent)
: m_seg(seg)
, m_output(seg->outputId())
, m_brushes(brushes)
, m_viewManager(vm)
, m_value(value)
, m_needReduction(false)
{
  for (int i = 0; i < m_brushes.size(); i++)
  {
    BrushShape &brush = m_brushes[i];
    if (0 == i)
      memcpy(m_strokeBounds, brush.second.bounds(), 6*sizeof(double));
    else
    {
      for (int i=0; i < 6; i+=2)
        m_strokeBounds[i] = std::min(brush.second.bounds()[i], m_strokeBounds[i]);
      for (int i=1; i < 6; i+=2)
        m_strokeBounds[i] = std::max(brush.second.bounds()[i], m_strokeBounds[i]);
    }
  }

  if (parent)
    connect(this, SIGNAL(initBrushTool()), parent, SLOT(initBrushTool()));
}

//-----------------------------------------------------------------------------
void Brush::DrawCommand::redo()
{
  if (m_newVolume.IsNotNull())
  {
    m_seg->filter()->draw(m_output, m_newVolume);
  }
  else
  {
    EspinaRegion strokeRegion(m_strokeBounds);

    if (m_seg->filter()->validOutput(m_output))
    {
      SegmentationVolumeTypeSPtr volume = boost::dynamic_pointer_cast<SegmentationVolumeType>(m_seg->output()->data(VolumeOutputType::TYPE));

      m_prevRegions = m_seg->output()->editedRegions();

      if (!strokeRegion.isInside(volume->espinaRegion()))
      {
        if (strokeRegion.intersect(volume->espinaRegion()))
          m_prevVolume = volume->cloneVolume(strokeRegion.intersection(volume->espinaRegion()));
        m_needReduction = true;
      } else
      {
        m_prevVolume = volume->cloneVolume(strokeRegion);
      }

    }

    for (int i=0; i < m_brushes.size(); i++)
    {
      bool lastBrush    = m_brushes.size() - 1 == i;
      BrushShape &brush = m_brushes[i];

      if (0 == i) // Prevent resizing on each brush
        m_seg->filter()->draw(m_output, brush.first, m_strokeBounds, m_value, lastBrush);
      else
        m_seg->filter()->draw(m_output, brush.first, brush.second.bounds(), m_value, lastBrush);
    }

    if (m_seg->filter()->validOutput(m_output))
    {
      SegmentationVolumeTypeSPtr volume = boost::dynamic_pointer_cast<SegmentationVolumeType>(m_seg->output()->data(VolumeOutputType::TYPE));
      m_newVolume = volume->cloneVolume(strokeRegion);
    }
  }

  m_viewManager->updateSegmentationRepresentations(m_seg.data());
}

//-----------------------------------------------------------------------------
void Brush::DrawCommand::undo()
{
  if (m_prevVolume.IsNotNull() || m_needReduction)
  {
    EspinaRegion strokeRegion(m_strokeBounds);

    m_seg->filter()->fill(m_output, strokeRegion, SEG_BG_VALUE, false);

    if (m_prevVolume.IsNotNull())
    m_seg->filter()->draw(m_output, m_prevVolume, !m_needReduction);

    if (m_needReduction)
    {
      SegmentationVolumeTypeSPtr volume = boost::dynamic_pointer_cast<SegmentationVolumeType>(m_seg->output()->data(VolumeOutputType::TYPE));
      volume->fitToContent();
    }

    // Restore previous edited regions
    m_seg->output()->setEditedRegions(m_prevRegions);
  }
  else
    emit initBrushTool();

  m_viewManager->updateSegmentationRepresentations(m_seg.data());
}

//-----------------------------------------------------------------------------
Brush::SnapshotCommand::SnapshotCommand(SegmentationSPtr seg,
                                        FilterOutputId output,
                                        ViewManager *vm)
: m_seg(seg)
, m_output(output)
, m_viewManager(vm)
{
  SegmentationVolumeTypeSPtr segVolume = boost::dynamic_pointer_cast<SegmentationVolumeType>(m_seg->output()->data(VolumeOutputType::TYPE));
  m_prevVolume = segVolume->cloneVolume();
}

//-----------------------------------------------------------------------------
void Brush::SnapshotCommand::redo()
{
  if (m_newVolume.IsNotNull())
    m_seg->filter()->restoreOutput(m_output, m_newVolume);
}

//-----------------------------------------------------------------------------
void Brush::SnapshotCommand::undo()
{
  if (m_newVolume.IsNull())
  {
    SegmentationVolumeTypeSPtr segVolume = boost::dynamic_pointer_cast<SegmentationVolumeType>(m_seg->output()->data(VolumeOutputType::TYPE));
    m_newVolume = segVolume->cloneVolume();
  }

  if (m_prevVolume.IsNotNull())
    m_seg->filter()->restoreOutput(m_output, m_prevVolume);
}
