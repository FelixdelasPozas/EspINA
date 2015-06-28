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

//-----------------------------------------------------------------------------
PropertyColorEngine::PropertyColorEngine()
: m_minValue(0)
, m_maxValue(10000)
, m_minColor(Qt::blue)
, m_maxColor(Qt::red)
, m_property("Size")
{
}

//-----------------------------------------------------------------------------
void PropertyColorEngine::setProperty(const QString &property, double min, double max)
{
  m_property = property;
  m_minValue = min;
  m_maxValue = max;
}

//-----------------------------------------------------------------------------
QColor PropertyColorEngine::color(SegmentationAdapterPtr seg)
{
  Q_ASSERT(seg);

  QColor color(Qt::gray);

  auto info = seg->information(m_property);

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
LUTSPtr PropertyColorEngine::lut(SegmentationAdapterPtr seg)
{
  auto c = color(seg);
  auto  seg_lut = LUTSPtr::New();
  seg_lut->Allocate();
  seg_lut->SetNumberOfTableValues(2);
  seg_lut->Build();
  seg_lut->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  seg_lut->SetTableValue(1, c.redF(), c.greenF(), c.blueF(), 1.0);
  seg_lut->Modified();

  return seg_lut;
}

//-----------------------------------------------------------------------------
double PropertyColorEngine::adjustRange(double value) const
{
  return qMax(m_minValue, qMin(value, m_maxValue));
}

double PropertyColorEngine::interpolateFactor(double value) const
{
  return (value - m_minValue) / (m_maxValue - m_minValue);
}
