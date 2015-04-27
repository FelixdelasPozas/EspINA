/*
 * Copyright 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
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
