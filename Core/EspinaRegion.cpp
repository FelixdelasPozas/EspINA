/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "EspinaRegion.h"
#include <vtkMath.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
EspinaRegion::EspinaRegion()
{
  vtkMath::UninitializeBounds(m_bounds);
}

//-----------------------------------------------------------------------------
EspinaRegion::EspinaRegion(const Nm bounds[6])
{
  memcpy(m_bounds, bounds, 6*sizeof(Nm));
}

//-----------------------------------------------------------------------------
EspinaRegion::EspinaRegion(const Nm minX, const Nm maxX,
                           const Nm minY, const Nm maxY,
                           const Nm minZ, const Nm maxZ)
{
  m_bounds[0] = minX;
  m_bounds[1] = maxX;
  m_bounds[2] = minY;
  m_bounds[3] = maxY;
  m_bounds[4] = minZ;
  m_bounds[5] = maxZ;
}


//-----------------------------------------------------------------------------
bool EspinaRegion::intersect(const EspinaRegion &region) const
{
  bool xOverlap = xMin() <= region.xMax() && xMax() >= region.xMin();
  bool yOverlap = yMin() <= region.yMax() && yMax() >= region.yMin();
  bool zOverlap = zMin() <= region.zMax() && zMax() >= region.zMin();

  return xOverlap && yOverlap && zOverlap;
}

//-----------------------------------------------------------------------------
bool EspinaRegion::isInside(const EspinaRegion &region) const
{
  bool xInside = region.xMin() <= xMin() && xMax() <= region.xMax();
  bool yInside = region.yMin() <= yMin() && yMax() <= region.yMax();
  bool zInside = region.zMin() <= zMin() && zMax() <= region.zMax();

  return xInside && yInside && zInside;
}

//-----------------------------------------------------------------------------
EspinaRegion EspinaRegion::intersection(const EspinaRegion &region) const
{
  Nm res[6];

  res[0] = std::max(xMin(), region.xMin());
  res[1] = std::min(xMax(), region.xMax());
  res[2] = std::max(yMin(), region.yMin());
  res[3] = std::min(yMax(), region.yMax());
  res[4] = std::max(zMin(), region.zMin());
  res[5] = std::min(zMax(), region.zMax());

  return EspinaRegion(res);
}


namespace EspINA
{
  //-----------------------------------------------------------------------------
  EspinaRegion BoundingBox(EspinaRegion r1, EspinaRegion r2)
  {
    Nm bounds[6];

    for(unsigned int min = 0, max = 1; min < 6; min += 2, max +=2)
    {
      bounds[min] = std::min(r1[min], r2[min]);
      bounds[max] = std::max(r1[max], r2[max]);
    }

    return EspinaRegion(bounds);
  }
} // namespace EspINA