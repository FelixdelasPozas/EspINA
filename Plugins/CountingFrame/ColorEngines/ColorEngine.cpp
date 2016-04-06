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

// Plugin
#include "ColorEngine.h"

#include "Extensions/StereologicalInclusion.h"

// ESPINA
#include <Extensions/ExtensionUtils.h>

using namespace ESPINA;
using namespace ESPINA::CF;

//-----------------------------------------------------------------------------
ColorEngine::ColorEngine()
: GUI::ColorEngines::ColorEngine("CountingFrameColorEngine", tr("Counting Frame"))
, m_exclusionOpacity(0.5)
{
  m_excludedLUT = LUTSPtr::New();
  m_excludedLUT->Allocate();
  m_excludedLUT->SetNumberOfTableValues(2);
  m_excludedLUT->Build();
  m_excludedLUT->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  m_excludedLUT->SetTableValue(1, 1.0, 0.0, 0.0, 0.6);
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
QColor ColorEngine::color(ConstSegmentationAdapterPtr segmentation)
{
  int r = 0;
  int g = 0;
  int b = 0;
  int a = 255;

  auto extensions = segmentation->readOnlyExtensions();

  if (extensions->hasExtension(StereologicalInclusion::TYPE))
  {
    auto extension = extensions->get<StereologicalInclusion>();

    if (extension->isExcluded())
    {
      r  = 255;
      a *= m_exclusionOpacity;
    } else
    {
      g = 255;
    }
  }

  return QColor(r, g, b, a);
}

//-----------------------------------------------------------------------------
LUTSPtr ColorEngine::lut(ConstSegmentationAdapterPtr segmentation)
{
  auto res = m_includedLUT;

  auto extensions = segmentation->readOnlyExtensions();

  if (extensions->hasExtension(StereologicalInclusion::TYPE))
  {
    auto extension = extensions->get<StereologicalInclusion>();

    if (extension->isExcluded())
    {
      res = m_excludedLUT;
    }
  }
  return res;
}


//-----------------------------------------------------------------------------
double ColorEngine::exlcusionOpacity() const
{
  return m_exclusionOpacity;
}

//-----------------------------------------------------------------------------
void ColorEngine::setExclusionOpacity(const double value)
{
  m_exclusionOpacity = qMax(0.0, qMin(1.0, value));
}
