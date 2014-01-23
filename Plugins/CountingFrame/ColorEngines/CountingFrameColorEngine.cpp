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


#include "CountingFrameColorEngine.h"
#include "Extensions/StereologicalInclusion.h"

using namespace EspINA;
using namespace EspINA::CF;

//-----------------------------------------------------------------------------
CountingFrameColorEngine::CountingFrameColorEngine()
{
  m_excludedLUT = LUTSPtr::New();
  m_excludedLUT->Allocate();
  m_excludedLUT->SetNumberOfTableValues(2);
  m_excludedLUT->Build();
  m_excludedLUT->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  m_excludedLUT->SetTableValue(1, 1.0, 0.0, 0.0, 0.2);
  m_excludedLUT->Modified();

  m_includedLUT = LUTSPtr::New();
  m_includedLUT->Allocate();
  m_includedLUT->SetNumberOfTableValues(2);
  m_includedLUT->Build();
  m_includedLUT->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  m_includedLUT->SetTableValue(1, 0.0, 1.0, 0.0, 1.0);
  m_includedLUT->Modified();
}


//-----------------------------------------------------------------------------
QColor CountingFrameColorEngine::color(SegmentationAdapterPtr segmentation)
{
  int r = 0;
  int g = 0;
  int b = 0;

  auto extension = stereologicalInclusionExtension(segmentation);

  if (extension->isExcluded())
  {
    r = 255;
  } else
  {
    g = 255;
  }

  return QColor(r, g, b, 255);
}

//-----------------------------------------------------------------------------
LUTSPtr CountingFrameColorEngine::lut(SegmentationAdapterPtr segmentation)
{
//    if (!seg->channel()) //TODO Change to assert
//     return m_includedLUT;
  auto extension = stereologicalInclusionExtension(segmentation);

  return extension->isExcluded()?m_excludedLUT:m_includedLUT;
}

//-----------------------------------------------------------------------------
StereologicalInclusionSPtr CountingFrameColorEngine::stereologicalInclusionExtension(SegmentationAdapterPtr segmentation)
{
  StereologicalInclusionSPtr stereologicalExtentsion;

  if (segmentation->hasExtension(StereologicalInclusion::TYPE))
  {
    auto extension = segmentation->extension(StereologicalInclusion::TYPE);
    stereologicalExtentsion = stereologicalInclusion(extension);
  }
  else
  {
    stereologicalExtentsion = StereologicalInclusionSPtr(new StereologicalInclusion());
    segmentation->addExtension(stereologicalExtentsion);
  }
  Q_ASSERT(stereologicalExtentsion);

  return stereologicalExtentsion;
}
