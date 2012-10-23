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


#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "EspinaTypes.h"

class BoundingBox
{
public:
  explicit BoundingBox(Nm bounds[6]);
  explicit BoundingBox(EspinaVolume *image);

  Nm xMin() const {return m_bounds[0];}
  Nm xMax() const {return m_bounds[1];}
  Nm yMin() const {return m_bounds[2];}
  Nm yMax() const {return m_bounds[3];}
  Nm zMin() const {return m_bounds[4];}
  Nm zMax() const {return m_bounds[5];}

  Nm *bounds(){return m_bounds;}

  bool intersect(BoundingBox &bb);
  BoundingBox intersection(BoundingBox &bb);

private:
  explicit BoundingBox();
  Nm m_bounds[6];
};
#endif // BOUNDINGBOX_H
