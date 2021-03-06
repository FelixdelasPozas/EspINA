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

#ifndef ESPINA_SPATIAL_H
#define ESPINA_SPATIAL_H

// C++
#include <iostream>
#include <cstdint>
#include <cmath>

// Qt
#include <QList>
#include <QString>
#include <QObject>

namespace ESPINA
{
  /** \class Axis
   * \brief Axis direction enumeration.
   *
   */
  enum class Axis: std::int8_t
  {
    X=0,
    Y=1,
    Z=2,
    UNDEFINED=3
  };

  /** \brief Returns the numerical index equivalent of the specified axis.
   * \param[in] axis.
   *
   */
  constexpr int idx(const Axis axis)
  {
    return (axis == Axis::X ? 0 : (axis == Axis::Y ? 1 : (axis == Axis::Z ? 2 : 3)));
  }

  /** \brief Converts the numerical value to its Axis equivalent.
   * \param[in] idx, value [0,2]
   *
   */
  constexpr Axis toAxis(int idx)
  {
    return (idx == 0 ? Axis::X : (idx == 1 ? Axis::Y : (idx == 2 ? Axis::Z : Axis::UNDEFINED)));
  }

  /** \brief Returns the text of the given direction
   * \param[in] direction axis direction).
   *
   */
  inline const QString toText(const Axis direction)
  {
    switch(direction)
    {
      case Axis::X: return QObject::tr("X");
      case Axis::Y: return QObject::tr("Y");
      case Axis::Z: return QObject::tr("Z");
      default: break;
    }

    return QObject::tr("Undefined");
  }

  /** \class Plane
   * \brief Plane surface enumeration.
   *
   */
  enum class Plane: std::int8_t
  {
    XY = 0,  // AXIAL
    XZ = 1,  // CORONAL
    YZ = 2,  // SAGITTAL
    UNDEFINED = 3
  };

  /** \brief Returns the numerical index equivalent of the specified plane.
   * \param[in] plane.
   *
   */
  constexpr int idx(const Plane plane)
  {
    return plane == Plane::XY? 2:(plane == Plane::XZ?1:(plane == Plane::YZ?0:-1));
  }

  /** \brief Returns the numerical index equivalent of the specified plane.
   * \param[in] plane.
   *
   */
  constexpr int normalCoordinateIndex(const Plane plane)
  { return idx(plane); }

  /** \brief Converts the numerical value to its Plane equivalent.
   * \param[in] idx.
   *
   */
  constexpr Plane toPlane(int idx)
  {
    return idx == 0?Plane::YZ:(idx == 1?Plane::XZ:( idx == 2?Plane::XY:Plane::UNDEFINED));
  }

  /** \brief Returns the text of the given orthogonal plane
   * \param[in] direction axis direction).
   *
   */
  inline const QString toText(const Plane plane)
  {
    switch(plane)
    {
      case Plane::XY: return QObject::tr("Axial");
      case Plane::XZ: return QObject::tr("Coronal");
      case Plane::YZ: return QObject::tr("Sagittal");
      default: return QObject::tr("Undefined");
    }
  }

  using Nm = double;

  /** \brief Returns true if both values are equal to some degree given by the delta and the spacing.
   * \param[in] lhs.
   * \param[in] rhs.
   * \param[in] spacing.
   *
   * NOTE: The spacing parameter is here to change the DELTA
   * when checking VolumeBounds values, as their values should be
   * in fixed points (like in a grid) but aren't.
   * TODO: Solve the problem with VolumeBounds and remove this ASAP.
   */
  inline bool areEqual(const Nm lhs, const Nm rhs, const Nm spacing = 1.0)
  {
    const double DELTA = 0.01 * spacing;
    return std::fabs(lhs - rhs) < DELTA;
  }

  /** \brief Returns true if the value is outside the given limits.
   * \param[in] value
   * \param[in] limitA
   * \param[in] limitB
   */
  inline bool isOutsideLimits(double value, double limitA, double limitB)
  {
    double lower = std::min(limitA, limitB);
    double upper = std::max(limitA, limitB);
    return (value < lower || upper < value);
  };

}


#endif // ESPINA_SPATIAL_H
