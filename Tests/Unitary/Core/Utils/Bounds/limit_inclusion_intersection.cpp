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

#include "Bounds.h"

using namespace ESPINA;
using namespace std;

int limit_inclusion_intersection( int argc, char** argv )
{
  int error = 0;

  Bounds b1{0,10,0,10,0,10};
  Bounds b2{10,15,10,15,10,15};

  if (intersect(b1, b2)) {
    cerr << b1 << " doesn't intersect " << b2 << endl;
    error = EXIT_FAILURE;
  }

  if (intersect(b2, b1)) {
    cerr << b2 << " doesn't intersect " << b1 << ". Intersection is symmetric" << endl;
    error = EXIT_FAILURE;
  }

  for (auto dir : {Axis::X, Axis::Y, Axis::Z}) {
    b1.setUpperInclusion(dir, true);
  }

  if (!intersect(b1, b2)) {
    cerr << b1 << " intersects " << b2 << endl;
    error = EXIT_FAILURE;
  }

  if (!intersect(b2, b1)) {
    cerr << b2 << " intersects " << b1 << ". Intersection is symmetric" << endl;
    error = EXIT_FAILURE;
  }

  Bounds expectedIntersection{'[',10,10,10,10,10,10,']'};
  Bounds actualInteresction = intersection(b1, b2);
  if (expectedIntersection != actualInteresction) {
    cerr << b1 << b2 << endl;
    cerr << "Expected bounds intersection " << expectedIntersection << " but got" << actualInteresction << " instead" << endl;
    error = EXIT_FAILURE;
  }

  if (!actualInteresction.areValid()) {
    cerr << b1 << " and " << b2 << " intersection should be a valid bounds" << endl;
    error = EXIT_FAILURE;
  }

  for (auto dir : {Axis::X, Axis::Y, Axis::Z}) {
    b2.setLowerInclusion(dir, false);
  }

  if (intersect(b1, b2)) {
    cerr << b1 << " doesn't intersect " << b2 << endl;
    error = EXIT_FAILURE;
  }

  if (intersect(b2, b1)) {
    cerr << b2 << " doesn't intersect " << b1 << ". Intersection is symmetric" << endl;
    error = EXIT_FAILURE;
  }

  return error;
}