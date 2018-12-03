/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include <GUI/Model/ModelAdapter.h>
#include <GUI/ColorEngines/ConnectionsColorEngine.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Utils;
using namespace ESPINA::GUI::ColorEngines;

//--------------------------------------------------------------------
ConnectionsColorEngine::ConnectionsColorEngine()
: ColorEngine{"ConnectionsColorEngine", tr("Colors segmentations according to its number of connections.")}
, m_HUERange {new RangeHSV(0, 100)}
{
  m_HUERange->setRangeToTruncatedHUERange(310);
}

//--------------------------------------------------------------------
ConnectionsColorEngine::~ConnectionsColorEngine()
{
  delete m_HUERange;
}

//--------------------------------------------------------------------
QColor ConnectionsColorEngine::color(ConstSegmentationAdapterPtr segmentation)
{
  auto smartPtr    = segmentation->model()->smartPointer(const_cast<SegmentationAdapter *>(segmentation));
  auto connections = segmentation->model()->connections(smartPtr);

  return m_HUERange->color(connections.size());
}

//--------------------------------------------------------------------
LUTSPtr ConnectionsColorEngine::lut(ConstSegmentationAdapterPtr segmentation)
{
  auto  segColor = color(segmentation);
  auto  segLUT   = LUTSPtr::New();

  segLUT->Allocate();
  segLUT->SetNumberOfTableValues(2);
  segLUT->Build();
  segLUT->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  segLUT->SetTableValue(1, segColor.redF(), segColor.greenF(), segColor.blueF(), 1.0);
  segLUT->Modified();

  return segLUT;
}

//--------------------------------------------------------------------
void ConnectionsColorEngine::setRange(const unsigned int minimum, const unsigned int maximum)
{
  m_HUERange->setMinimumValue(minimum);
  m_HUERange->setMaximumValue(maximum);
}
