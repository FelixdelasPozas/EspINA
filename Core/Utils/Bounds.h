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

#ifndef ESPINA_BOUNDS_H
#define ESPINA_BOUNDS_H

#include <Core/Utils/Vector3.hxx>
#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Core/Utils/Spatial.h"
#include <iostream>

// Qt
#include <QDebug>
#include <QList>
#include <QString>

namespace ESPINA
{
  /** \class Bounds
   * \brief Set of values defining a region in the 3D space.
   *
   * Boundary values can be defined to belong or not to the region itself
   */
  class EspinaCore_EXPORT Bounds
  {
    public:
      /** \brief Bounds class constructor.
       *
       * Create invalid bounds
       */
      explicit Bounds();

      /** \brief Bounds class constructor.
       * \param[in] bounds initial list of values.
       *
       * Create Bounds from an initial list of values
       *
       *  There are three formats to initialize Bounds instances:
       *  - {min_x, max_x, min_y, max_y, min_z, max_z} \n
       *    Lower values are inlcuded.\n
       *    Uppaer values are excluded.\n
       *  - {'LI',min_x, max_x, min_y, max_y, min_z, max_z,'UI'}.
       *  - {'LI',min_x, max_x,'UI','LI', min_y, max_y,'UI', 'LI', min_z, max_z,'UI'}.\n
       *    Lower values are included or not according to LI value.\n
       *    '[' includes values and '(' excludes them.\n
       *    Upper values are included or not according to UI value.\n
       *    ']' includes values and ')' excludes them.
       */
      Bounds(std::initializer_list<double> bounds);

      /** \brief Bounds class constructor.
       * \param[in] point.
       *
       * Constructs the bounds of a point.
       *
       */
      explicit Bounds(const NmVector3 &point);


      /** \brief Bounds class constructor.
       * \param[in] bounds values in a dim 6 array.
       *
       * Constructs the bounds of a point.
       *
       */
      explicit Bounds(Nm *bounds);

      /** \brief Bounds class constructor.
       * \param[in] strimg bounds serialization
       *
       * Constructs the bounds from its serialization
       *
       */
      explicit Bounds(const QString &string);

      /** \brief Returns whether or not Bounds define a valid region of the 3D space
       *
       *  Any region whose lower bounds are greater than its upper bounds is considered to be an
       *  empty region, and thus invalid.
       */
      bool areValid() const
      {
        bool valid = true;
        int i = 0;
        while (valid && i < 3)
        {
          auto lb = m_bounds[2*i];
          auto ub = m_bounds[2*i+1];

          valid = lb <= ub && !(lb == ub && !m_lowerInclusion[i] && !m_upperInclusion[i]);

          ++i;
        }

        return valid;
      }

      /** \brief Bounds operator[int]
       *
       */
      double& operator[](int idx)
      { return m_bounds[idx]; }

      /** \brief Bounds operator[int] const
       *
       */
      const double& operator[](int idx) const
      { return m_bounds[idx]; }

      /** \brief Set wheter or not lower bounds in the given direction should be included in the region defined by the bounds
       * \param[in] dir axis direction.
       * \param[in] value true to include lower value, false otherwise.
       *
       */
      void setLowerInclusion(const Axis dir, const bool value)
      { m_lowerInclusion[idx(dir)] = value; }

      /** \brief Set wheter or not lower bounds should be included in the region defined by the bounds.
       * \param[in] value true to include all lower values, false otherwise.
       *
       */
      void setLowerInclusion(const bool value)
      { m_lowerInclusion[idx(Axis::X)] = m_lowerInclusion[idx(Axis::Y)] = m_lowerInclusion[idx(Axis::Z)] = value; }

      /** \brief Returns wheter or not lower bounds in the given direction should be included in the region defined by the bounds
       * \param[in] dir axis direction.
       *
       */
      bool areLowerIncluded(const Axis dir) const
      { return m_lowerInclusion[idx(dir)]; }

      /** \brief Set wheter or not upper bounds in the given direction should be included in the region defined by the bounds
       * \param[in] dir axis direction.
       * \param[in] value true to include lower value, false otherwise.
       *
       */
      void setUpperInclusion(const Axis dir, const bool value)
      { m_upperInclusion[idx(dir)] = value; }

      /** \brief Set wheter or not upper bounds should be included in the region defined by the bounds
       * \param[in] value true to include all upper values, false otherwise.
       *
       */
      void setUpperInclusion(const bool value)
      { m_upperInclusion[idx(Axis::X)] = m_upperInclusion[idx(Axis::Y)] = m_upperInclusion[idx(Axis::Z)] = value; }

      /** \brief Returns wheter or not upper bounds in the given direction should be included in the region defined by the bounds
       * \param[in] dir axis direction.
       */
      bool areUpperIncluded(const Axis dir) const
      { return m_upperInclusion[idx(dir)]; }

      /** \brief Returns the distance between both sides of the bounds in a given direction
       * \param[in] dir axis direction.
       *
       */
      double lenght(const Axis dir) const
      { return m_bounds[2*idx(dir)+1] - m_bounds[2*idx(dir)]; }

      /** \brief Dumps the contents of the bounds to a formatted QString.
       *
       */
      QString toString() const;

    private:
      double m_bounds[6];
      bool   m_lowerInclusion[3];
      bool   m_upperInclusion[3];
  };

  using BoundsList = QList<Bounds>;

  /** \brief Returns wether b1 intersects b2 or not.
   * \param[in] b1 bounds object.
   * \param[in] b2 bounds object.
   * \param[in] spacing.
   *
   *  In case just the bounds have one value in common, both
   *  bounds must be inclussive
   */
  bool EspinaCore_EXPORT intersect(const Bounds& b1, const Bounds& b2, NmVector3 spacing=NmVector3{1.0, 1.0, 1.0});

  /** \brief Returns the bounds which belong both to b1 and b2.
   * \param[in] b1 bounds object.
   * \param[in] b2 bounds object.
   * \param[in] spacing.
   *
   *  In case just the bounds have one value in common, both
   *  bounds must be inclussive
   */
  Bounds EspinaCore_EXPORT intersection(const Bounds& b1, const Bounds& b2, NmVector3 spacing=NmVector3{1.0, 1.0, 1.0});

  /** \brief Returns the minimum bouds containing b1 and b2
   * \param[in] b1 bounds object.
   * \param[in] b2 bounds object.
   * \param[in] spacing.
   *
   */
  Bounds EspinaCore_EXPORT boundingBox(const Bounds &b1, const Bounds& b2, NmVector3 spacing=NmVector3{1.0, 1.0, 1.0});

  /** \brief Returns the centroid of bounds
   * \param[in] bounds to compute its centroid
   *
   */
  NmVector3 EspinaCore_EXPORT centroid(const Bounds &bounds);

  /** \brief Returns the start point of the lower voxel
   * \param[in] bounds to compute its lower bound
   *
   *  It returns NmVector3{bounds[0], bounds[2], bounds[4]}
   */
  NmVector3 EspinaCore_EXPORT lowerPoint(const Bounds &bounds);

  /** \brief Returns the end point of the upper voxel
   * \param[in] bounds to compute its lower bound
   *
   *  It returns NmVector3{bounds[0], bounds[2], bounds[4]}
   */
  NmVector3 EspinaCore_EXPORT upperrPoint(const Bounds &bounds);

  /** \brief Add bounds to boundingBox
   * \param[in] boundingBox to be updated
   * \param[in] bounds to update boundingBox with
   *
   */
  void EspinaCore_EXPORT updateBoundingBox(Bounds &boundingBox, const Bounds &bounds);

  /** \brief Returns whether a bound is contained inside another
   * \param[in] container bounds object.
   * \param[in] contained bounds object.
   * \param[in] spacing
   *
   *  Boundaires are inside if and only if both boundaries
   *  are equally included
   */
  bool EspinaCore_EXPORT contains(const Bounds& container, const Bounds& contained, const NmVector3 &spacing=NmVector3{1,1,1});

  /** \brief Returns whether a bound contains a point or not.
   * \param[in] bounds bounds object.
   * \param[in] point point object.
   * \param[in] spacing.
   *
   *  Boundaires are inside if and only if both boundaries
   *  are equally included
   */
  bool EspinaCore_EXPORT contains(const Bounds& bounds, const NmVector3& point, const NmVector3 &spacing=NmVector3{1,1,1});

  /** \brief Returns whether position is inside bounds for the given direction
   * \param[in] bounds bounds object.
   * \param[in] axis axis direction
   * \param[in] pos axis spatial position
   *
   *  Boundaires are inside if and only if both boundaries
   *  are equally included
   */
  bool EspinaCore_EXPORT contains(const Bounds &bounds, const Axis axis, const Nm pos);

  /** \brief Bounds operator<< for streams.
   *
   */
  EspinaCore_EXPORT std::ostream& operator<<(std::ostream& os, const Bounds& bounds);

  /** \brief Bounds operator<< for QDebug.
   *
   */
  QDebug EspinaCore_EXPORT operator<< (QDebug d, const Bounds &bounds);

  /** \brief Bounds equality operator.
   *
   */
  bool EspinaCore_EXPORT operator==(const Bounds& lhs, const Bounds& rhs);

  /** \brief Bounds inequality operator.
   *
   */
  bool EspinaCore_EXPORT operator!=(const Bounds& lhs, const Bounds& rhs);

  /** \brief Returns true if the blocks are adjacent and the bounding box is the union of both.
   * \param[in] lhs bounds object.
   * \param[in] rhs bounds object.
   *
   * To be adjacent the blocks must have equal sizes on two sides and be adjacent on the
   * third side.
   */
  bool EspinaCore_EXPORT areAdjacent(const Bounds &lhs, const Bounds &rhs);
}

#endif // ESPINA_BOUNDS_H
