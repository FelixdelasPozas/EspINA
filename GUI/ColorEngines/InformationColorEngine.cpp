/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "InformationColorEngine.h"

#include <GUI/Utils/ColorRange.h>
#include <GUI/Model/SegmentationAdapter.h>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Utils;
using namespace ESPINA::GUI::ColorEngines;

//-----------------------------------------------------------------------------
InformationColorEngine::InformationColorEngine()
: ColorEngine("PropertyColorEngine", tr("Property"))
, m_key("MorphologicalInformation", "Size")
, m_colorRange(new RangeHSV(0, 10000))
{
}

//-----------------------------------------------------------------------------
InformationColorEngine::~InformationColorEngine()
{
  delete m_colorRange;
}

//-----------------------------------------------------------------------------
void InformationColorEngine::setInformation(const SegmentationExtension::InformationKey &key, double min, double max)
{
  m_key = key;

  m_colorRange->setMinimumValue(min);
  m_colorRange->setMaximumValue(max);

  emit modified();
}

//-----------------------------------------------------------------------------
QColor InformationColorEngine::color(ConstSegmentationAdapterPtr segmentation)
{
  Q_ASSERT(segmentation);

  QColor color(Qt::gray);

  auto extensions = segmentation->readOnlyExtensions();

  if (extensions->isReady(m_key))
  {
    auto info = extensions->information(m_key);

    if (info.isValid() && info.canConvert<double>())
    {
      color = m_colorRange->color(info.toDouble());
    }
  }

  return color;
}

//-----------------------------------------------------------------------------
LUTSPtr InformationColorEngine::lut(ConstSegmentationAdapterPtr segmentation)
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
