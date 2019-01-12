/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

// ESPINA
#include "ColorRange.h"

// Qt
#include <QColor>

using namespace ESPINA::GUI::Utils;

//------------------------------------------------------------------------
ColorRange::ColorRange(const double min, const double max)
: m_minColor(Qt::blue)
, m_maxColor(Qt::red)
, m_minValue(min)
, m_maxValue(max)
{

}

//------------------------------------------------------------------------
QColor ColorRange::color(const double value) const
{
  return color(value, m_minValue, m_maxValue);
}

//------------------------------------------------------------------------
QColor ColorRange::color(const double value, const double min, const double max) const
{
  return computeColor(value, min, max, m_minColor, m_maxColor);
}

//------------------------------------------------------------------------
void ColorRange::setMinimumValue(const double value)
{
  m_minValue = value;

  emit valueRangeChanged();
}

//------------------------------------------------------------------------
double ColorRange::minimumValue() const
{
  return m_minValue;
}

//------------------------------------------------------------------------
void ColorRange::setMaximumValue(const double value)
{
  m_maxValue = value;

  emit valueRangeChanged();
}

//------------------------------------------------------------------------
double ColorRange::maximumValue() const
{
  return m_maxValue;
}

//------------------------------------------------------------------------
void ColorRange::setMinimumColor(const QColor& color)
{
  m_minColor = color;

  emit colorRangeChanged();
}

//------------------------------------------------------------------------
QColor ColorRange::minimumColor() const
{
  return m_minColor;
}

//------------------------------------------------------------------------
void ColorRange::setMaximumColor(const QColor& color)
{
  m_maxColor = color;

  emit colorRangeChanged();
}

//------------------------------------------------------------------------
QColor ColorRange::maximumColor() const
{
  return m_maxColor;
}


//------------------------------------------------------------------------
RangeHSV::RangeHSV(const double min, const double max)
: ColorRange(min, max)
{
}

//------------------------------------------------------------------------
QColor RangeHSV::computeColor(const double  value,
                              const double  minValue,
                              const double  maxValue,
                              const QColor& minColor,
                              const QColor& maxColor) const
{
  auto inRangeValue = adjustRange(value, minValue, maxValue);

  auto f = interpolateFactor(inRangeValue, minValue, maxValue);

  auto h = minColor.hueF()        + f*(maxColor.hueF()        - minColor.hueF());
  auto s = minColor.saturationF() + f*(maxColor.saturationF() - minColor.saturationF());
  auto v = minColor.valueF()      + f*(maxColor.valueF()      - minColor.valueF());

  return QColor::fromHsvF(h, s, v);
}


//-----------------------------------------------------------------------------
double RangeHSV::adjustRange(const double value,
                             const double minValue,
                             const double maxValue) const
{
  return std::max(minValue, std::min(value, maxValue));
}

//-----------------------------------------------------------------------------
double RangeHSV::interpolateFactor(const double value,
                                   const double minValue,
                                   const double maxValue) const
{
  return (value - minValue) / (maxValue - minValue);
}

//-----------------------------------------------------------------------------
void ColorRange::setRangeToFullHUERange()
{
  m_minColor = QColor::fromHsv(0  , 255, 255);
  m_maxColor = QColor::fromHsv(359, 255, 255);
}

//-----------------------------------------------------------------------------
void ColorRange::setRangeToHalfHUERange()
{
  m_minColor = QColor::fromHsv(0  , 255, 255);
  m_maxColor = QColor::fromHsv(179, 255, 255);
}

//-----------------------------------------------------------------------------
void ColorRange::setRangeToTruncatedHUERange(const int value)
{
  m_minColor = QColor::fromHsv(                   0, 255, 255);
  m_maxColor = QColor::fromHsv(std::min(359, value), 255, 255);
}
