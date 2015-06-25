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

#include "OrthogonalRepresentation.h"

using namespace ESPINA;
using namespace ESPINA::GUI::View::Widgets::OrthogonalRegion;

//----------------------------------------------------------------------------
OrthogonalRepresentation::OrthogonalRepresentation()
: OrthogonalRepresentation({1,1,1},{0, 1, 0, 1, 0, 1})
{
}

//----------------------------------------------------------------------------
OrthogonalRepresentation::OrthogonalRepresentation(const NmVector3 &resolution, const Bounds &bounds)
: m_mode(Mode::RESIZABLE)
, m_resolution(resolution)
, m_bounds(bounds)
, m_color(Qt::yellow)
, m_pattern(0xFFFF)
{

}

//----------------------------------------------------------------------------
void OrthogonalRepresentation::setMode(const OrthogonalRepresentation::Mode mode)
{
  if (m_mode != mode)
  {
    m_mode = mode;

    emit modeChanged(mode);
  }
}

//----------------------------------------------------------------------------
OrthogonalRepresentation::Mode OrthogonalRepresentation::mode() const
{
  return m_mode;
}

//----------------------------------------------------------------------------
void OrthogonalRepresentation::setResolution(const NmVector3 &resolution)
{
  if (m_resolution != resolution)
  {
    m_resolution = resolution;

    emit resolutionChanged(resolution);
  }
}

//----------------------------------------------------------------------------
NmVector3 OrthogonalRepresentation::resolution() const
{
  return m_resolution;
}

//----------------------------------------------------------------------------
void OrthogonalRepresentation::setBounds(const Bounds &bounds)
{
  if (m_bounds != bounds)
  {
    m_bounds = bounds;

    emit boundsChanged(bounds);
  }
}

//----------------------------------------------------------------------------
Bounds OrthogonalRepresentation::bounds() const
{
  return m_bounds;
}

//----------------------------------------------------------------------------
void OrthogonalRepresentation::setColor(const QColor &color)
{
  if (m_color != color)
  {
    m_color = color;

    emit colorChanged(color);
  }
}

//----------------------------------------------------------------------------
QColor OrthogonalRepresentation::representationColor() const
{
  return m_color;
}

//----------------------------------------------------------------------------
void OrthogonalRepresentation::setRepresentationPattern(int pattern)
{
  if (m_pattern != pattern)
  {
    m_pattern = pattern;

    emit patternChanged(pattern);
  }
}

//----------------------------------------------------------------------------
int OrthogonalRepresentation::representationPattern() const
{
  return m_pattern;
}
