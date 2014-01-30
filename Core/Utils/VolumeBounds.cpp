/*
 * Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include "VolumeBounds.h"

#include <Core/EspinaTypes.h>
#include <Core/Analysis/Data/VolumetricDataUtils.h>
#include <vtkMath.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
VolumeBounds::VolumeBounds(const Bounds& bounds, const NmVector3& spacing, const NmVector3& origin)
: m_origin(origin)
, m_spacing(spacing)
{
  if (bounds.areValid() && spacing[0] > 0 && spacing[1] > 0 && spacing[2] > 0)
  {
    auto region = equivalentRegion<itkVolumeType>(m_origin, m_spacing, bounds);

    m_bounds = equivalentBounds<itkVolumeType>(m_origin, m_spacing, region);
  }
}

//-----------------------------------------------------------------------------
void VolumeBounds::exclude(int idx, Nm value)
{
  int i = idx/2;

  NmVector3 point{value, value, value};

  VolumeBounds voxelBounds{Bounds(point), m_spacing, m_origin};

  if (idx % 2 == 0)
  {
    if (areEqual(m_bounds[idx], value))
    {
      m_bounds[idx] += m_spacing[i];

      if (areEqual(m_bounds[2*i], m_bounds[2*i+1]))
      {
        qWarning() << "WARNING: Empty bounds";
      }
    } else if (m_bounds[idx] < value)
    {
      m_bounds[idx] = voxelBounds[idx] + m_spacing[i];
    }
  } else if (value < m_bounds[idx])
  {
    m_bounds[idx] = voxelBounds[idx] - m_spacing[i];
  }
}

//-----------------------------------------------------------------------------
void VolumeBounds::include(int idx, Nm value)
{
  int i = idx/2;

  NmVector3 point{value, value, value};

  VolumeBounds voxelBounds{Bounds(point), m_spacing, m_origin};

  if (idx % 2 != 0)
  {
    if (areEqual(m_bounds[idx], value))
    {
      m_bounds[idx] += m_spacing[i];

      if (areEqual(m_bounds[2*i], m_bounds[2*i+1]))
      {
        qWarning() << "WARNING: Empty bounds";
      }
    } else if (m_bounds[idx] < value)
    {
      m_bounds[idx] = voxelBounds[idx];
    }
  } else if (value < m_bounds[idx])
  {
    m_bounds[idx] = voxelBounds[idx];
  }
}

//-----------------------------------------------------------------------------
QString VolumeBounds::toString() const
{
  return m_bounds.toString() + "@(" + m_origin.toString() + "," + m_spacing.toString() + ")";
}

//-----------------------------------------------------------------------------
bool EspINA::isMultiple(Nm point, Nm spacing)
{
  Nm indexValue = point / spacing;

  Nm delta      = fabs(vtkMath::Round(indexValue) - indexValue);

  return areEqual(delta, 0);
}

//-----------------------------------------------------------------------------
bool EspINA::isAligned(Nm point, Nm origin, Nm spacing)
{
  Nm indexValue = (point - spacing/2.0 - origin) / spacing;

  Nm delta      = fabs(vtkMath::Round(indexValue) - indexValue);

  return areEqual(delta, 0);
}

//-----------------------------------------------------------------------------
bool EspINA::isCompatible(const VolumeBounds &lhs, const VolumeBounds &rhs)
{
  if (!lhs.areValid() || !rhs.areValid()) return false;

  if (lhs.spacing() != rhs.spacing()) return false;

  if (lhs.origin() == rhs.origin()) return true;

  for (int i = 0; i < 3; ++i)
  {
    if (!isMultiple(lhs.origin()[i], lhs.spacing()[i])) return false;
    if (!isMultiple(rhs.origin()[i], rhs.spacing()[i])) return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
bool EspINA::isEquivalent(const VolumeBounds &lhs, const VolumeBounds &rhs)
{
  if (!isCompatible(lhs, rhs)) return false;

  for (int i = 0; i < 6; ++i) {
    if (!areEqual(lhs[i], rhs[i])) return false;
  }

  if (lhs.origin() != rhs.origin())
  {
    qWarning() << "WARNING: Equivalent Volume Bounds have different origins";
    qWarning() << lhs.toString() << rhs.toString();
  }

  return true;
}

//-----------------------------------------------------------------------------
bool EspINA::intersect(const VolumeBounds& lhs, const VolumeBounds& rhs)
{
  if (!isCompatible(lhs, rhs)) return false;

  auto lessThan    = [](double a, double b){return a <  b;};
  auto greaterThan = [](double a, double b){return a >  b;};

  bool overlap = true;

  int lo = 0, up = 1;
  auto spacing = lhs.spacing();
  Q_ASSERT(spacing == rhs.spacing());
  for (auto dir : {Axis::X, Axis::Y, Axis::Z})
  {
    overlap &= lessThan(lhs[lo], rhs[up]) && greaterThan(lhs[up], rhs[lo]);

    overlap &= !areEqual(lhs[lo], rhs[up], spacing[idx(dir)]);

    overlap &= !areEqual(lhs[up], rhs[lo], spacing[idx(dir)]);

    lo += 2;
    up += 2;
  }

  return overlap;
  //return intersect(lhs.bounds(), rhs.bounds());
}

//-----------------------------------------------------------------------------
VolumeBounds EspINA::intersection(const VolumeBounds& lhs, const VolumeBounds& rhs)
throw (Incompatible_Volume_Bounds_Exception)
{
  if (!isCompatible(lhs, rhs)) throw Incompatible_Volume_Bounds_Exception();

  Bounds bounds;

  int lo = 0, up = 1;
  for (Axis dir : {Axis::X, Axis::Y, Axis::Z})
  {
    bounds[lo] = std::max(lhs[lo], rhs[lo]);
    bounds[up] = std::min(lhs[up], rhs[up]);

    lo += 2;
    up += 2;
  }

  return VolumeBounds(bounds, lhs.spacing(), rhs.origin());
  //return VolumeBounds(intersection(lhs.bounds(), rhs.bounds()), lhs.spacing(), rhs.origin());
}

//-----------------------------------------------------------------------------
VolumeBounds EspINA::boundingBox(const VolumeBounds &lhs, const VolumeBounds &rhs)
throw (Incompatible_Volume_Bounds_Exception)
{
  if (!isCompatible(lhs, rhs)) throw Incompatible_Volume_Bounds_Exception();

  return VolumeBounds(boundingBox(lhs.bounds(), rhs.bounds()), lhs.spacing(), lhs.origin());
}

//-----------------------------------------------------------------------------
bool EspINA::contains(const VolumeBounds &container, const VolumeBounds &contained)
{
  if (!isCompatible(container, contained)) return false;

  return contains(container.bounds(), contained.bounds());
}

//-----------------------------------------------------------------------------
bool EspINA::contains(const VolumeBounds &bounds, const NmVector3 &point)
{
  for (int i = 0; i < 3; ++i)
  {
    int lo = 2*i;
    int up = 2*i+1;

    if (!areEqual(bounds[lo], point[i]) && point[i] < bounds[lo]) return false;

    if ( areEqual(bounds[up], point[i]) || point[i] > bounds[up]) return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
std::ostream &EspINA::operator<<(std::ostream &os, const VolumeBounds &bounds)
{
  os << bounds.toString().toStdString();

  return os;
}

//-----------------------------------------------------------------------------
QDebug EspINA::operator<< (QDebug d, const VolumeBounds &bounds)
{
  d << QString(bounds.toString());

  return d;
}

//-----------------------------------------------------------------------------
bool EspINA::operator==(const VolumeBounds &lhs, const VolumeBounds &rhs)
{
  return isEquivalent(lhs, rhs) && lhs.origin() == rhs.origin();
/*
  if (lhs.origin() != rhs.origin()) return false;

  if (lhs.spacing() != rhs.spacing()) return false;

  for (int i = 0; i < 6; ++i) {
    if (!areEqual(lhs[i], rhs[i])) return false;
  }

  return true; */
}

//-----------------------------------------------------------------------------
bool EspINA::operator!=(const VolumeBounds &lhs, const VolumeBounds &rhs)
{
  return !(lhs == rhs);
}
