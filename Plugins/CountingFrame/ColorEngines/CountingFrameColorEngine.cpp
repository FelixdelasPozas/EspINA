/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "CountingFrameColorEngine.h"
#include "Extensions/StereologicalInclusion.h"
#include <Extensions/ExtensionUtils.h>

using namespace ESPINA;
using namespace ESPINA::CF;

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
  int a = 255;

  if (segmentation->hasExtension(StereologicalInclusion::TYPE))
  {
    auto extension = retrieveExtension<StereologicalInclusion>(segmentation);

    if (extension->isExcluded())
    {
      r = 255;
      a =  50;
    } else
    {
      g = 255;
      a = 255;
    }
  }

  return QColor(r, g, b, a);
}

//-----------------------------------------------------------------------------
LUTSPtr CountingFrameColorEngine::lut(SegmentationAdapterPtr segmentation)
{
  auto res = m_includedLUT;

  if (segmentation->hasExtension(StereologicalInclusion::TYPE))
  {
    auto extension = retrieveExtension<StereologicalInclusion>(segmentation);

    if (extension->isExcluded())
      res = m_excludedLUT;
  }
  return res;
}

