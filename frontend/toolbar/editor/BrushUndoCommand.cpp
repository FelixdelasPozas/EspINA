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

#include "common/model/Filter.h"
#include <EspinaRegions.h>

#include <itkExtractImageFilter.h>

typedef itk::ExtractImageFilter<EspinaVolume, EspinaVolume> ExtractType;

//-----------------------------------------------------------------------------
Brush::DrawCommand::DrawCommand(Filter* source,
                                OutputNumber output,
                                BrushList brushes,
                                EspinaVolume::PixelType value)
: m_source(source)
, m_output(output)
, m_brushes(brushes)
, m_value(value)
{
  for (int i = 0; i < m_brushes.size(); i++)
  {
    Brush &brush = m_brushes[i];
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
}

//-----------------------------------------------------------------------------
void Brush::DrawCommand::redo()
{
  if (m_newVolume.IsNotNull())
  {
    m_source->restoreOutput(m_output, m_newVolume);
  }else
  {
    if (m_source->numberOutputs() > 0)
      backup(m_prevVolume, m_source->output(m_output));

    for (int i=0; i<m_brushes.size(); i++)
    {
      Brush &brush = m_brushes[i];
      if (0 == i) // Prevent resizing on each brush
        m_source->draw(m_output, brush.first, m_strokeBounds, m_value);
      else
        m_source->draw(m_output, brush.first, brush.second.bounds(), m_value);
    }
    if (m_source->numberOutputs() > 0)
      backup(m_newVolume, m_source->output(m_output));
  }
}


//-----------------------------------------------------------------------------
void Brush::DrawCommand::undo()
{
  if (m_prevVolume.IsNotNull())
    m_source->restoreOutput(m_output, m_prevVolume);
}

//-----------------------------------------------------------------------------
void Brush::DrawCommand::backup(EspinaVolume::Pointer &output, EspinaVolume* volume)
{
  ExtractType::Pointer extractor = ExtractType::New();
  extractor->SetNumberOfThreads(1);
  extractor->SetInput(volume);
  extractor->SetExtractionRegion(volume->GetLargestPossibleRegion());
  extractor->Update();

  output = extractor->GetOutput();
//   // Old version storing only stroke volume
//   double volumeBounds[6];
//   VolumeBounds(volume, volumeBounds);
// 
//   BoundingBox strokeBB(m_strokeBounds);
//   BoundingBox volumeBB(volumeBounds);
// 
//   if (strokeBB.intersect(volumeBB))
//   {
//     BoundingBox interectionBB = strokeBB.intersection(volumeBB);
// 
//     typedef EspinaVolume::RegionType Region;
//     Region strokeRegion = BoundsToRegion(interectionBB.bounds(), volume->GetSpacing());
//     Region subVolumeRegion = VolumeRegion(volume, strokeRegion);
// 
//     ExtractType::Pointer extractor = ExtractType::New();
//     extractor->SetNumberOfThreads(1);
//     extractor->SetInput(volume);
//     extractor->SetExtractionRegion(subVolumeRegion);
//     extractor->Update();
// 
//     output = extractor->GetOutput();
//   }
}
