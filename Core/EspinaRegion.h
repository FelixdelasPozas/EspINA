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


#ifndef ESPINAREGION_H
#define ESPINAREGION_H

#include "EspinaTypes.h"

namespace EspINA
{
  /// Normalized region, i.e. origin (0,0,0)
  class EspinaRegion
  {
  public:
    explicit EspinaRegion();
    explicit EspinaRegion(const Nm  bounds[6]);
    explicit EspinaRegion(const Nm minX, const Nm maxX,
                          const Nm minY, const Nm maxY,
                          const Nm minZ, const Nm maxZ);

    Nm &operator[](int idx) { return m_bounds[idx]; }
    const Nm &operator[](int idx) const { return m_bounds[idx]; }

    Nm xMin() const {return m_bounds[0];}
    Nm xMax() const {return m_bounds[1];}
    Nm yMin() const {return m_bounds[2];}
    Nm yMax() const {return m_bounds[3];}
    Nm zMin() const {return m_bounds[4];}
    Nm zMax() const {return m_bounds[5];}

    const Nm *bounds(            ) const { return m_bounds; }
    void      bounds(Nm bounds[6]) const { memcpy(bounds, m_bounds, 6*sizeof(Nm)); }

    /// Return whether this is inside @region
    bool isInside(const EspinaRegion &region) const;
    /// Check region intersection
    bool intersect(const EspinaRegion &region) const;
    /// Return intersection region
    EspinaRegion intersection(const EspinaRegion &region) const;

  private:
    Nm m_bounds[6];
  };

  EspinaRegion BoundingBox(EspinaRegion r1, EspinaRegion r2);


} // namespace EspINA

#endif // ESPINAREGION_H
