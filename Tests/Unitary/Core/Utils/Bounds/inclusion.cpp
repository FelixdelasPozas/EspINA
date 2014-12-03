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

#include "NmVector3.h"

using namespace ESPINA;
using namespace std;

bool Test_Bounds_Are_Inside(const Bounds& contained, const Bounds& container, bool pass_if_inside = true) {
  bool pass = true;

  if (contains(container, contained) != pass_if_inside) {
    cerr << contained << " should ";
    if (!pass_if_inside) cerr << "not ";
    cerr << "be inside of " << container << endl;

    pass = false;
  }

  return pass;
}

bool Test_Bounds_Are_Inside(const Bounds& internalBounds, const Bounds& bounds, int offset, bool same_boundary_type) {
  bool pass = true;

  for (bool edgeInclussion : {true, false})
  {
    int i = 0;
    Bounds edgeBounds;
    for (auto dir : {Axis::X, Axis::Y, Axis::Z}) {

      edgeBounds = internalBounds;
      edgeBounds[i] = bounds[i] - offset;
      pass &= Test_Bounds_Are_Inside(edgeBounds, bounds, same_boundary_type && offset == 0);
      ++i;

      edgeBounds = internalBounds;
      edgeBounds[i] = bounds[i] + offset;
      pass &= Test_Bounds_Are_Inside(edgeBounds, bounds, same_boundary_type && offset == 0);
      ++i;
    }
  }

  return pass;
}

int inclusion( int argc, char** argv )
{
  bool pass = true;

  Bounds bounds{0,10,0,10,0,10};

  if (!contains(bounds, bounds)) {
    cerr << "Every bound should be inside of itself" << endl;
    pass = false;
  }

//   for (bool internalBoundary : {true, false}) {
//     Bounds internalBounds{2, 8, 2, 8, 2, 8};
//     for (auto dir : {Axis::X, Axis::Y, Axis::Z}) {
//       internalBounds.setLowerInclusion(dir, internalBoundary);
//       internalBounds.setUpperInclusion(dir, internalBoundary);
//     }
//     for (bool externalBoundary : {true, false}) {
//       Bounds externalBounds{0,10,0,10,0,10};
//       for (auto dir : {Axis::X, Axis::Y, Axis::Z}) {
//         externalBounds.setLowerInclusion(dir, externalBoundary);
//         externalBounds.setUpperInclusion(dir, externalBoundary);
//       }
// 
//       pass &= Test_Bounds_Are_Inside(internalBounds, externalBounds,  0, true);
//     }
//   }
// 
//   for (bool internalBoundary : {true, false}) {
//     Bounds internalBounds{2, 8, 2, 8, 2, 8};
//     for (auto dir : {Axis::X, Axis::Y, Axis::Z}) {
//       internalBounds.setLowerInclusion(dir, internalBoundary);
//       internalBounds.setUpperInclusion(dir, internalBoundary);
//     }
//     for (bool externalBoundary : {true, false}) {
//       Bounds externalBounds{0,10,0,10,0,10};
//       for (auto dir : {Axis::X, Axis::Y, Axis::Z}) {
//         externalBounds.setLowerInclusion(dir, externalBoundary);
//         externalBounds.setUpperInclusion(dir, externalBoundary);
//       }
// 
//       pass &= Test_Bounds_Are_Inside(internalBounds, externalBounds, 10, false);
//     }
//   }
for (int i = 0; i < 6; ++i) 
  for (int o = 0; o < 6; o += 2)
    for (double ILB : {'[', '('})
      for (double IUB : {']', ')'})
        for (double ELB : {'[', '('})
          for (double EUB : {']', ')'}) {
            bool invalidTestOnSameBounds   = o == 2 && ((i%2 == 0 && ELB == '(' && ILB == '[') || (i%2 != 0 && EUB == ')' && IUB == ']'));
            bool invalidTestOnIntersection = o == 4;
            bool invalidTest = invalidTestOnSameBounds || invalidTestOnIntersection;
            Bounds internal{ILB,2,8,2,8,2,8,IUB};
            internal[i] += (i % 2)?o:-o;
            Bounds external{ELB,0,10,0,10,0,10,EUB};
            pass &= Test_Bounds_Are_Inside(internal, external, !invalidTest);
          }

  return !pass;
}