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

#include <Core/Extensions/ModelItemExtension.h>
#include <Core/Model/Segmentation.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
CountingFrameColorEngine::CountingFrameColorEngine()
{
  m_excludedLUT = LUTPtr::New();
  m_excludedLUT->Allocate();
  m_excludedLUT->SetNumberOfTableValues(2);
  m_excludedLUT->Build();
  m_excludedLUT->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  m_excludedLUT->SetTableValue(1, 1.0, 0.0, 0.0, 0.2);
  m_excludedLUT->Modified();

  m_includedLUT = LUTPtr::New();
  m_includedLUT->Allocate();
  m_includedLUT->SetNumberOfTableValues(2);
  m_includedLUT->Build();
  m_includedLUT->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  m_includedLUT->SetTableValue(1, 0.0, 1.0, 0.0, 1.0);
  m_includedLUT->Modified();
}


//-----------------------------------------------------------------------------
QColor CountingFrameColorEngine::color(SegmentationPtr seg)
{
  if (seg->channel().isNull())
    return QColor(0, 0, 0, 255);

  StereologicalInclusion *stereologicalExtentsion;
  Segmentation::InformationExtension extension = seg->informationExtension(StereologicalInclusion::ID);
  if (extension)
  {
    stereologicalExtentsion = stereologicalInclusionPtr(extension);
  }
  else
  {
    stereologicalExtentsion = new StereologicalInclusion();
    seg->addExtension(stereologicalExtentsion);
  }
  Q_ASSERT(stereologicalExtentsion);

  if (stereologicalExtentsion->isExcluded())
    return QColor(255, 0, 0, 50);
  else
    return QColor(0, 255, 0, 255);
}

//-----------------------------------------------------------------------------
LUTPtr CountingFrameColorEngine::lut(SegmentationPtr seg)
{
  if (seg->channel().isNull())
    return m_includedLUT;

  StereologicalInclusion *stereologicalExtentsion;
  Segmentation::InformationExtension extension = seg->informationExtension(StereologicalInclusion::ID);
  if (extension)
  {
    stereologicalExtentsion = stereologicalInclusionPtr(extension);
  }
  else
  {
    stereologicalExtentsion = new StereologicalInclusion();
    seg->addExtension(stereologicalExtentsion);
  }
  Q_ASSERT(stereologicalExtentsion);

  if (stereologicalExtentsion->isExcluded())
    return m_excludedLUT;
  else
    return m_includedLUT;
}
