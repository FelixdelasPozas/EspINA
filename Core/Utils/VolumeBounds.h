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

#ifndef ESPINA_VOLUME_BOUNDS_H
#define ESPINA_VOLUME_BOUNDS_H

#include "Core/EspinaCore_Export.h"

#include "Core/Utils/Spatial.h"
#include "Core/Utils/NmVector3.h"
#include "Bounds.h"

#include <QList>
#include <QString>

namespace ESPINA
{
  /** \brief Minimal bounds that enclose all voxels of any volumetric data
   */
  class VolumeBounds
  {
    public:
      explicit VolumeBounds(const Bounds &bounds = Bounds(), const NmVector3 &spacing = NmVector3{1,1,1}, const NmVector3 &origin = NmVector3());

      /** \brief Return whether or not Bounds define a valid region of the 3D space
       *
       *  Any region whose lower bounds are greater than its upper bounds is considered to be an
       *  empty region, and thus invalid.
       */
      bool areValid() const
      { return m_spacing[0] > 0 && m_spacing[1] > 0 && m_spacing[2] > 0 && m_bounds.areValid(); }

      const double& operator[](int idx) const
      { return m_bounds[idx]; }

      /** \brief Return the distance between both sides of the bounds in a given direction
       *
       */
      double lenght(const Axis dir) const
      { return m_bounds.lenght(dir); }

      void setOrigin(const NmVector3& origin)
      { m_origin = origin; }

      NmVector3 origin() const
      { return m_origin; }

      void setSpacing(const NmVector3& spacing)
      { m_spacing = spacing; }

      NmVector3 spacing() const
      { return m_spacing; }

      /** \brief Exclude the voxel corresponging to value
       *
       */
      void exclude(int idx, Nm value);

      /** \brief Include the voxel correspongind to value
       *
       */
      void include(int idx, Nm value);

      Bounds bounds() const
      { return m_bounds; }

      QString toString() const;

    private:
      NmVector3 m_origin;
      NmVector3 m_spacing;
      Bounds    m_bounds;
  };

  using VolumeBoundsList = QList<VolumeBounds>;

  struct Incompatible_Volume_Bounds_Exception {};

  bool isMultiple(Nm point, Nm spacing);

  bool isAligned(Nm point, Nm origin, Nm spacing);

  /** \brief Two volume bounds are compatible if they can contain the same voxels in the reference frame
   *
   *  NOTE: They actually don't need to conatain the same voxels, they bounds may differ.
   */
  bool isCompatible(const VolumeBounds& lhs, const VolumeBounds& rhs);

  /** \brief Two volume bounds are equivalent if they contain the same voxels in the reference frame
   *
   */
  bool isEquivalent(const VolumeBounds& lhs, const VolumeBounds& rhs);

  /** \brief Return wether b1 intersects b2 or not
   *
   */
  bool intersect(const VolumeBounds& lhs, const VolumeBounds& rhs);

  /** \brief Return the bounds which belong both to b1 and b2
   *
   */
  VolumeBounds intersection(const VolumeBounds& lhs, const VolumeBounds& rhs)
  throw (Incompatible_Volume_Bounds_Exception);

  /** \brief Return the minimum bounds containing b1 and b2
   *
   *  If bounds are not compatible an exception will be thrown
   */
  VolumeBounds boundingBox(const VolumeBounds &lhs, const VolumeBounds& rhs)
  throw (Incompatible_Volume_Bounds_Exception);

  /** \brief Return whether a bound is contained inside another
   *
   *  Boundaires are inside if and only if both boundaries
   *  are equally included
   */
  bool contains(const VolumeBounds& container, const VolumeBounds& contained);

  bool contains(const VolumeBounds& bounds, const NmVector3& point);

  std::ostream& operator<<(std::ostream& os, const VolumeBounds& bounds);

  QDebug operator<< (QDebug d, const VolumeBounds &bounds);

  /**
   *
   * Equality is true if and only if, they define the same bounds using the same origin and spacing
   */
  bool operator==(const VolumeBounds& lhs, const VolumeBounds& rhs);

  bool operator!=(const VolumeBounds& lhs, const VolumeBounds& rhs);

  /* \brief Returns true if the blocks are adjacent and the bounding box is the union of both.
   * For that to happen the blocks must have equal sizes on two sides and be adjacent on the
   * third side.
   */
  bool areAdjacent(const VolumeBounds &lhs, const VolumeBounds &rhs);
}


#endif // ESPINA_VOLUME_BOUNDS_H
