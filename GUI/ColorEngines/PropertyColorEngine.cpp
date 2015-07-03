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

#include "PropertyColorEngine.h"

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::ColorEngines;

//-----------------------------------------------------------------------------
PropertyColorEngine::PropertyColorEngine(Support::Context &context)
: ColorEngine("PropertyColorEngine", tr("Property"))
, WithContext(context)
, m_minValue(0)
, m_maxValue(10000)
, m_minColor(Qt::blue)
, m_maxColor(Qt::red)
, m_extensionType("MorphologicalInformation")
, m_measure("Size")
{
}

//-----------------------------------------------------------------------------
void PropertyColorEngine::setMeasure(const QString &measure, double min, double max)
{
  m_measure  = measure;
  m_minValue = min;
  m_maxValue = max;
}

//-----------------------------------------------------------------------------
QColor PropertyColorEngine::color(SegmentationAdapterPtr segmentation)
{
  Q_ASSERT(segmentation);

  QColor color(Qt::gray);

  auto info = segmentation->information(m_measure);

  if (info.isValid() && info.canConvert<double>())
  {
    auto value = adjustRange(info.toDouble());

    auto f = interpolateFactor(value);
    auto h = m_minColor.hueF()        + f*(m_maxColor.hueF()        - m_minColor.hueF());
    auto s = m_minColor.saturationF() + f*(m_maxColor.saturationF() - m_minColor.saturationF());
    auto v = m_minColor.valueF()      + f*(m_maxColor.valueF()      - m_minColor.valueF());

    color.setHsvF(h,s,v);
  }

  return color;
}

//-----------------------------------------------------------------------------
LUTSPtr PropertyColorEngine::lut(SegmentationAdapterPtr segmentation)
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

//-----------------------------------------------------------------------------
double PropertyColorEngine::adjustRange(double value) const
{
  return qMax(m_minValue, qMin(value, m_maxValue));
}

//-----------------------------------------------------------------------------
double PropertyColorEngine::interpolateFactor(double value) const
{
  return (value - m_minValue) / (m_maxValue - m_minValue);
}
