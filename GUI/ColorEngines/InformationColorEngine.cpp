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

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Utils;
using namespace ESPINA::GUI::ColorEngines;

//-----------------------------------------------------------------------------
InformationColorEngine::InformationColorEngine(Support::Context &context)
: ColorEngine("PropertyColorEngine", tr("Property"))
, WithContext(context)
, m_extensionType("MorphologicalInformation")
, m_measure("Size")
, m_colorRange(new RangeHSV(0, 10000))
{
}

//-----------------------------------------------------------------------------
InformationColorEngine::~InformationColorEngine()
{
  delete m_colorRange;
}

//-----------------------------------------------------------------------------
void InformationColorEngine::setInformation(const QString &information, double min, double max)
{
  m_measure = information;

  m_colorRange->setMinimumValue(min);
  m_colorRange->setMaximumValue(max);

  emit modified();
}

//-----------------------------------------------------------------------------
QColor InformationColorEngine::color(SegmentationAdapterPtr segmentation)
{
  Q_ASSERT(segmentation);

  QColor color(Qt::gray);

  auto info = segmentation->information(m_measure);

  if (info.isValid() && info.canConvert<double>())
  {
    color = m_colorRange->color(info.toDouble());
  }

  return color;
}

//-----------------------------------------------------------------------------
LUTSPtr InformationColorEngine::lut(SegmentationAdapterPtr segmentation)
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
