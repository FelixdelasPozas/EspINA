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

#include <iostream>
#include <QList>

namespace EspINA {

  struct Wrong_number_initial_values {};

  struct Invalid_bounds_token {};

  enum class Axis { X=0, Y=1, Z=2 };

  constexpr int idx(const Axis axis) {
    return axis == Axis::X? 0:(axis == Axis::Y?1:2);
  }

  constexpr Axis toAxis(int i) {
    return i == 0?Axis::X:(i == 1?Axis::Y:Axis::Z);
  }

  enum class Plane { XY, YZ, XZ };

  constexpr int idx(const Plane plane) {
    return plane == Plane::XY? 2:(plane == Plane::YZ?0:1);
  }

  /** \brief Set of values defining a region in the 3D space
   * 
   * Boundary values can be defined to belong or not to the region itself
   */
  class Bounds {
  public:
    /**
     * Create invalid bounds
     */
    Bounds();

    /** \brief Create Bounds from an initial list of values
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

    /** \brief Return whether or not Bounds define a valid region of the 3D space 
     * 
     *  Any region whose lower bounds are greater than its upper bounds is considered to be an
     *  empty region, and thus invalid.
     */
    bool areValid() const { return m_bounds[0] <= m_bounds[1] && m_bounds[2] <= m_bounds[3] &&m_bounds[4] <= m_bounds[5]; }

    double& operator[](int idx) { return m_bounds[idx]; }
    const double& operator[](int idx) const { return m_bounds[idx]; }

    /** \brief Set wheter or not lower bounds in the given direction should be included in the region defined by the bounds
     */
    void setLowerInclusion(const Axis dir, const bool value) { m_lowerInclusion[idx(dir)] = value; }

    /** \brief Return wheter or not lower bounds in the given direction should be included in the region defined by the bounds
     */
    bool areLowerIncluded(const Axis dir) const { return m_lowerInclusion[idx(dir)]; }

    /** \brief Set wheter or not upper bounds in the given direction should be included in the region defined by the bounds
     */
    void setUpperInclusion(const Axis dir, const bool value) { m_upperInclusion[idx(dir)] = value; }

    /** \brief Return wheter or not upper bounds in the given direction should be included in the region defined by the bounds
     */
    bool areUpperIncluded(const Axis dir) const { return m_upperInclusion[idx(dir)]; }

    /** \brief Return the distance between both sides of the bounds in a given direction
     *
     */
    double lenght(const Axis dir) const { return m_bounds[2*idx(dir)+1] - m_bounds[2*idx(dir)]; }

  private:
    double m_bounds[6];
    bool   m_lowerInclusion[3];
    bool   m_upperInclusion[3];
  };

  using BoundsList = QList<Bounds>;


  /** \brief Return wether b1 intersects b2 or notice
   *
   *  In case just the bounds have one value in common, both
   *  bounds must be inclussive
   */
  bool intersect(const Bounds& b1, const Bounds& b2);

  /** \brief Return the bounds which belong both to b1 and b2
   *
   *  In case just the bounds have one value in common, both
   *  bounds must be inclussive
   */
  Bounds intersection(const Bounds& b1, const Bounds& b2);

  /** \brief Return the minimum bouds containing b1 and b2
   *
   */
  Bounds boundingBox(const Bounds &b1, const Bounds& b2);

  /** \brief Return whether a bound is contained inside another
   *
   *  Boundaires are inside if and only if both boundaries
   *  are equally included
   */
  bool areInside(const Bounds& contained, const Bounds& container);


  std::ostream& operator<<(std::ostream& os, const Bounds& bounds);

  bool operator==(const Bounds& lhs, const Bounds& rhs);

  bool operator!=(const Bounds& lhs, const Bounds& rhs);
}


#endif // ESPINA_BOUNDS_H
