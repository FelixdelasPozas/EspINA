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


#include "BoundingBox.h"

#include "EspinaRegions.h"

//-----------------------------------------------------------------------------
BoundingBox::BoundingBox()
{
  m_bounds[0] = m_bounds[2] = m_bounds[4] = 0;
  m_bounds[1] = m_bounds[3] = m_bounds[5] = -1;
}

//-----------------------------------------------------------------------------
BoundingBox::BoundingBox(Nm bounds[6])
{
  memcpy(m_bounds, bounds, 6*sizeof(Nm));
}

//-----------------------------------------------------------------------------
BoundingBox::BoundingBox(EspinaVolume* image)
{
  VolumeBounds(image, m_bounds);
}


//-----------------------------------------------------------------------------
bool BoundingBox::intersect(BoundingBox& bb)
{
  bool xOverlap = xMin() <= bb.xMax() && xMax() >= bb.xMin();
  bool yOverlap = yMin() <= bb.yMax() && yMax() >= bb.yMin();
  bool zOverlap = zMin() <= bb.zMax() && zMax() >= bb.zMin();

  return xOverlap && yOverlap && zOverlap;
}

//-----------------------------------------------------------------------------
BoundingBox BoundingBox::intersection(BoundingBox& bb)
{
  BoundingBox res;
  res.m_bounds[0] = std::max(xMin(), bb.xMin());
  res.m_bounds[1] = std::min(xMax(), bb.xMax());
  res.m_bounds[2] = std::max(yMin(), bb.yMin());
  res.m_bounds[3] = std::min(yMax(), bb.yMax());
  res.m_bounds[4] = std::max(zMin(), bb.zMin());
  res.m_bounds[5] = std::min(zMax(), bb.zMax());
  return res;
}