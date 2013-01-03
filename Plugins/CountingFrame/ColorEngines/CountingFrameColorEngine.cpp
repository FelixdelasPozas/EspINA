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
#include "Extensions/CountingFrameSegmentationExtension.h"

#include <Core/Extensions/ModelItemExtension.h>
#include <Core/Model/Segmentation.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
CountingFrameColorEngine::CountingFrameColorEngine()
{
  m_discardedLUT = LUTPtr::New();
  m_discardedLUT->Allocate();
  m_discardedLUT->SetNumberOfTableValues(2);
  m_discardedLUT->Build();
  m_discardedLUT->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  m_discardedLUT->SetTableValue(1, 1.0, 0.0, 0.0, 0.2);
  m_discardedLUT->Modified();

  m_nonDiscartedLUT = LUTPtr::New();
  m_nonDiscartedLUT->Allocate();
  m_nonDiscartedLUT->SetNumberOfTableValues(2);
  m_nonDiscartedLUT->Build();
  m_nonDiscartedLUT->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  m_nonDiscartedLUT->SetTableValue(1, 0.0, 1.0, 0.0, 1.0);
  m_nonDiscartedLUT->Modified();
}


//-----------------------------------------------------------------------------
QColor CountingFrameColorEngine::color(SegmentationPtr seg)
{
  ModelItemExtensionPtr ext = seg->extension(CountingFrameSegmentationExtension::ID);
  Q_ASSERT(ext);
  CountingFrameSegmentationExtension *segExt = dynamic_cast<CountingFrameSegmentationExtension *>(ext);

  if (segExt->isDiscarded())
    return QColor(255, 0, 0, 50);
  else
    return QColor(0, 255, 0, 255);
}

//-----------------------------------------------------------------------------
LUTPtr CountingFrameColorEngine::lut(SegmentationPtr seg)
{
  ModelItemExtensionPtr ext = seg->extension(CountingFrameSegmentationExtension::ID);
  Q_ASSERT(ext);
  CountingFrameSegmentationExtension *segExt = dynamic_cast<CountingFrameSegmentationExtension *>(ext);

  if (segExt->isDiscarded())
    return m_discardedLUT;
  else
    return m_nonDiscartedLUT;
}


