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


#include "CountingRegionColorEngine.h"
#include <extensions/CountingRegionSegmentationExtension.h>

#include <Core/Extensions/ModelItemExtension.h>
#include <Core/Model/Segmentation.h>

//-----------------------------------------------------------------------------
CountingRegionColorEngine::CountingRegionColorEngine()
{
  m_discartedLUT = LUTPtr::New();
  m_discartedLUT->Allocate();
  m_discartedLUT->SetNumberOfTableValues(2);
  m_discartedLUT->Build();
  m_discartedLUT->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  m_discartedLUT->SetTableValue(1, 1.0, 0.0, 0.0, 0.2);
  m_discartedLUT->Modified();

  m_nonDiscartedLUT = LUTPtr::New();
  m_nonDiscartedLUT->Allocate();
  m_nonDiscartedLUT->SetNumberOfTableValues(2);
  m_nonDiscartedLUT->Build();
  m_nonDiscartedLUT->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  m_nonDiscartedLUT->SetTableValue(1, 0.0, 1.0, 0.0, 1.0);
  m_nonDiscartedLUT->Modified();
}


//-----------------------------------------------------------------------------
QColor CountingRegionColorEngine::color(Segmentation* seg)
{
  ModelItemExtension *ext = seg->extension(CountingRegionSegmentationExtension::ID);
  Q_ASSERT(ext);
  CountingRegionSegmentationExtension *segExt = dynamic_cast<CountingRegionSegmentationExtension *>(ext);

  if (segExt->isDiscarted())
    return QColor(255, 0, 0, 50);
  else
    return QColor(0, 255, 0, 255);
}

//-----------------------------------------------------------------------------
LUTPtr CountingRegionColorEngine::lut(Segmentation* seg)
{
  ModelItemExtension *ext = seg->extension(CountingRegionSegmentationExtension::ID);
  Q_ASSERT(ext);
  CountingRegionSegmentationExtension *segExt = dynamic_cast<CountingRegionSegmentationExtension *>(ext);

  if (segExt->isDiscarted())
    return m_discartedLUT;
  else
    return m_nonDiscartedLUT;
}


