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

using namespace ESPINA;
using namespace std;

int intersection( int argc, char** argv )
{
  bool error = false;

  Bounds b1{-1.5, 0.5, -1.5, 0.5, -1.5, 0.5};
  Bounds b2{-0.5, 1.5, -0.5, 1.5, -0.5, 1.5};

  VolumeBounds vb1{b1};
  VolumeBounds vb2{b2};

  if (!intersect(vb1, vb1)) {
    cerr << "Every bound should intersect with itself" << endl;
    error = true;
  }

  if (!intersect(vb1, vb2)) {
    cerr << vb1 << " intersects " << vb2 << endl;
    error = true;
  }

  if (!intersect(vb2, vb1)) {
    cerr << vb2 << " intersects " << vb1 << ". Intersection is symmetric" << endl;
    error = true;
  }

  Bounds ib{-0.5, 0.5, -0.5, 0.5, -0.5, 0.5};
  VolumeBounds expectedIntersection{ib};

  auto actualInteresction = ESPINA::intersection(vb1, vb2);
  if (!isEquivalent(expectedIntersection, actualInteresction)) {
    cerr << "Expected bounds intersection " << expectedIntersection << " but got" << actualInteresction << " instead" << endl;
    error = true;
  }

  if (!actualInteresction.areValid()) {
    cerr << vb1 << " and " << vb2 << " intersection should be a valid bounds" << endl;
    error = true;
  }

  Bounds b3{0.5, 1.5, 0.5, 1.5, 0.5, 1.5};
  VolumeBounds vb3{b3};

  if (intersect(vb1, vb3)) {
    cerr << vb1 << " doesn't intersects " << vb3 << endl;
    error = true;
  }

  Bounds b4{-1.5, -0.5, -1.5, -0.5, -1.5, -0.5};
  VolumeBounds vb4{b4};

  if (intersect(vb2, vb4)) {
    cerr << vb2 << " doesn't intersects " << vb4 << endl;
    error = true;
  }

  if (intersect(vb1, VolumeBounds()))
  {
    cerr << "Unexpected bounds intersection between invalid bounds" << endl;
    error = true;
  }

  try
  {
    auto invalidIntersection = ESPINA::intersection(vb1, VolumeBounds());
    error = true;
  } catch (Incompatible_Volume_Bounds_Exception e)
  {
  }

  VolumeBounds incompatible{b1, NmVector3{2, 2, 2}};

  if (intersect(vb1, incompatible))
  {
    cerr << "Unexpected bounds intersection between incompatible bounds" << endl;
    error = true;
  }

  try
  {
    auto incompatibleIntersection = ESPINA::intersection(vb1, incompatible);
    cerr << "incompatible intersection: " << incompatibleIntersection << endl;
    error = true;
  } catch (Incompatible_Volume_Bounds_Exception e)
  {
  }

  return error;
}