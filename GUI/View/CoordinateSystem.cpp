/*

 Copyright (C) 2015 Jorge Peña Pastor <jpena@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
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

#include "CoordinateSystem.h"

#include "vtkMath.h"

using namespace ESPINA;
using namespace ESPINA::GUI::View;

//-----------------------------------------------------------------------------
CoordinateSystem::CoordinateSystem(const NmVector3 &resolution)
: CoordinateSystem(resolution, NmVector3{0, 0, 0})
{

}

//-----------------------------------------------------------------------------
CoordinateSystem::CoordinateSystem(const NmVector3 &resolution, const NmVector3 &origin)
: CoordinateSystem(resolution,
                   origin,
                   Bounds{origin[0] - resolution[0]/2.0, origin[0] + resolution[0]/2.0,
                          origin[1] - resolution[1]/2.0, origin[1] + resolution[1]/2.0,
                          origin[2] - resolution[2]/2.0, origin[2] + resolution[2]/2.0})
{

}

//-----------------------------------------------------------------------------
CoordinateSystem::CoordinateSystem(const NmVector3 &resolution, const NmVector3 &origin, const Bounds &bounds)
: m_origin    {origin}
, m_resolution{resolution}
, m_bounds    {bounds}
{

}

//-----------------------------------------------------------------------------
void CoordinateSystem::setOrigin(const NmVector3 &origin)
{
  m_origin = origin;
}


//-----------------------------------------------------------------------------
NmVector3 CoordinateSystem::origin() const
{
  return m_origin;
}

//-----------------------------------------------------------------------------
void CoordinateSystem::setResolution(const NmVector3 &resolution)
{
  if (resolution != m_resolution)
  {
    m_resolution = resolution;

    emit resolutionChanged(resolution);
  }
}

//-----------------------------------------------------------------------------
NmVector3 CoordinateSystem::resolution() const
{
  return m_resolution;
}

//-----------------------------------------------------------------------------
void CoordinateSystem::setBounds(const Bounds &bounds)
{
  if (m_bounds != bounds)
  {
    m_bounds = bounds;

    emit boundsChanged(bounds);
  }
}

//-----------------------------------------------------------------------------
Bounds CoordinateSystem::bounds() const
{
  return m_bounds;
}

//-----------------------------------------------------------------------------
Nm CoordinateSystem::voxelBottom(const int sliceIndex, const Plane plane) const
{
  int index = normalCoordinateIndex(plane);

  return m_bounds[2*index] + sliceIndex * m_resolution[index];
}

//-----------------------------------------------------------------------------
Nm CoordinateSystem::voxelBottom(const Nm position, const Plane plane) const
{
  return voxelBottom(voxelSlice(position, plane), plane);
}

//-----------------------------------------------------------------------------
Nm CoordinateSystem::voxelCenter(const int sliceIndex, const Plane plane) const
{
  int index = normalCoordinateIndex(plane);

  return m_bounds[2*index] + ((sliceIndex + 0.5)* m_resolution[index]);
}

//-----------------------------------------------------------------------------
Nm CoordinateSystem::voxelCenter(const Nm position, const Plane plane) const
{
  return voxelCenter(voxelSlice(position, plane), plane);
}

//-----------------------------------------------------------------------------
Nm CoordinateSystem::voxelTop(const int sliceIndex, const Plane plane) const
{
  int index = normalCoordinateIndex(plane);
  return m_bounds[2*index] + (sliceIndex + 1.0) * m_resolution[index];
}

//-----------------------------------------------------------------------------
Nm CoordinateSystem::voxelTop(const Nm position, const Plane plane) const
{
  return voxelTop(voxelSlice(position, plane), plane);
}

//-----------------------------------------------------------------------------
int CoordinateSystem::voxelSlice(const Nm position, const Plane plane) const
{
  int index = normalCoordinateIndex(plane);

  return vtkMath::Floor((position-m_bounds[2*index])/m_resolution[index]);
}

//-----------------------------------------------------------------------------
NmVector3 CoordinateSystem::voxelCenter(const int xIndex, const int yIndex, const int zIndex) const
{
  return NmVector3{voxelCenter(xIndex, Plane::YZ),
                   voxelCenter(yIndex, Plane::XZ),
                   voxelCenter(zIndex, Plane::XZ)};
}

//-----------------------------------------------------------------------------
NmVector3 CoordinateSystem::voxelCenter(const NmVector3 &point) const
{
  return NmVector3{voxelCenter(point[0], Plane::YZ),
                   voxelCenter(point[1], Plane::XZ),
                   voxelCenter(point[2], Plane::XY)};
}
