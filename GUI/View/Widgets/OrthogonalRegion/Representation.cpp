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

#include "Representation.h"

using namespace ESPINA;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::OrthogonalRegion;

//----------------------------------------------------------------------------
Representation::Representation()
: Representation{{1,1,1},{0, 1, 0, 1, 0, 1}}
{
}

//----------------------------------------------------------------------------
Representation::Representation(const NmVector3 &resolution, const Bounds &bounds)
: m_mode(Mode::RESIZABLE)
, m_resolution(resolution)
, m_bounds(bounds)
, m_color(Qt::yellow)
, m_pattern(0xFFFF)
{

}

//----------------------------------------------------------------------------
void Representation::setMode(const Representation::Mode mode)
{
  if (m_mode != mode)
  {
    m_mode = mode;

    emit modeChanged(mode);
  }
}

//----------------------------------------------------------------------------
Representation::Mode Representation::mode() const
{
  return m_mode;
}

//----------------------------------------------------------------------------
void Representation::setResolution(const NmVector3 &resolution)
{
  if (m_resolution != resolution)
  {
    m_resolution = resolution;

    emit resolutionChanged(resolution);
  }
}

//----------------------------------------------------------------------------
NmVector3 Representation::resolution() const
{
  return m_resolution;
}

//----------------------------------------------------------------------------
void Representation::setBounds(const Bounds &bounds)
{
  if (m_bounds != bounds)
  {
    m_bounds = bounds;

    emit boundsChanged(bounds);
  }
}

//----------------------------------------------------------------------------
Bounds Representation::bounds() const
{
  return m_bounds;
}

//----------------------------------------------------------------------------
void Representation::setColor(const QColor &color)
{
  if (m_color != color)
  {
    m_color = color;

    emit colorChanged(color);
  }
}

//----------------------------------------------------------------------------
QColor Representation::representationColor() const
{
  return m_color;
}

//----------------------------------------------------------------------------
void Representation::setRepresentationPattern(int pattern)
{
  if (m_pattern != pattern)
  {
    m_pattern = pattern;

    emit patternChanged(pattern);
  }
}

//----------------------------------------------------------------------------
int Representation::representationPattern() const
{
  return m_pattern;
}
