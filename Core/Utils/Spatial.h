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

#include <iostream>
#include <QList>

namespace ESPINA {

  enum class Axis { X=0, Y=1, Z=2 };

  constexpr int idx(const Axis axis)
  {
    return axis == Axis::X? 0:(axis == Axis::Y?1:2);
  }

  constexpr Axis toAxis(int i)
  {
    return i == 0?Axis::X:(i == 1?Axis::Y:Axis::Z);
  }

  enum class Plane {
    XY,  // AXIAL
    XZ,  // CORONAL
    YZ,  // SAGITTAL
    UNDEFINED
  };

  constexpr int idx(const Plane plane)
  {
    return plane == Plane::XY? 2:(plane == Plane::XZ?1:(plane == Plane::YZ?0:-1));
  }

  constexpr int normalCoordinateIndex(const Plane plane)
  { return idx(plane); }

  constexpr Plane toPlane(int i)
  {
    return i == 0?Plane::YZ:(i == 1?Plane::XZ:( i == 2?Plane::XY:Plane::UNDEFINED));
  }

  using Nm = double;

  // TODO: The spacing parameter is here to change the DELTA
  // when checking VolumeBounds values, as their values should be
  // in fixed points (like in a grid) but aren't.
  // Solve the problem with VolumeBounds and remove this ASAP.
  inline bool areEqual(Nm lhs, Nm rhs, Nm spacing = 1.0)
  {
    const double DELTA = 0.01 * spacing;
    return fabs(lhs - rhs) < DELTA;
  }

}


#endif // ESPINA_SPATIAL_H
