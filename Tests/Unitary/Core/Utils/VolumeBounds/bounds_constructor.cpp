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

#include <VolumeBounds.h>

#include "TestSupport.h"

using namespace EspINA;
using namespace std;

// bool Test_Bounds(const Bounds& bounds) {
//   bool pass = true;
//
//   if (!bounds.areValid()) {
//     cerr << "List constructed bounds: " << bounds << ". Expected valid bounds" << endl;
//     pass = false;
//   }
//
//   for (int i = 0; i < 6; ++i) {
//     if (bounds[i] != i + 0.5) {
//       cerr << "Wrong value initialization at " << i << " bounds: " << bounds << endl;
//       pass = false;
//     }
//   }
//
//   return pass;
// }

bool Test_Bounds_Constructor(NmVector3 origin, NmVector3 spacing)
{
  bool error = false;

  Nm o0 = origin[0];
  Nm o1 = origin[1];
  Nm o2 = origin[2];

  Nm s0 = spacing[0];
  Nm s1 = spacing[1];
  Nm s2 = spacing[2];

  Bounds ideal{o0-s0/2, o0+s0/2, o1-s1/2, o1+s1/2, o2-s2/2, o2+s2/2};
  VolumeBounds reference(ideal, spacing, origin);

  if (reference != ideal) // Uses equal comparisson defined in TestSupport
  {
    cerr << "Wrong reference bounds initialization: " << reference << endl;
    error = true;
  }

  Bounds bounds{o0-s0/2, o0, o1, o1+s1/2, o2-0.1, o2+0.1};
  VolumeBounds volume(bounds, spacing, origin);

  VolumeBounds copy{volume.bounds(), spacing, origin};
  if (copy != volume)
  {
    cerr << "Unexpected volume bounds: " << copy << ". Expected value: " << volume << endl;
    error = true;
  }


  if (volume != reference)
  {
    cerr << "Unexpected volume bounds: " << volume << ". Expected value: " << reference << endl;
    error = true;
  }

  Bounds bigger;
  for (int i = 0; i < 3; i++)
  {
    bigger[2*i]   = ideal[2*i]   - spacing[i]/2;
    bigger[2*i+1] = ideal[2*i+1] + spacing[i]/2;
  }

  Bounds expected{o0-3*s0/2, o0+3*s0/2, o1+-3*s1/2, o1+3*s1/2, o2-3*s2/2, o2+3*s2/2};
  VolumeBounds biggerVolume(bigger, spacing, origin);

  if (biggerVolume != expected) // Uses equal comparisson defined in TestSupport
  {
    cerr << "Unexpected volume bounds: " << biggerVolume << ". Expected value: " << expected << endl;
    error = true;
  }

  return error;
}

int bounds_constructor( int argc, char** argv )
{
  bool error = false;

  error  = Test_Bounds_Constructor(NmVector3{0, 0, 0},       NmVector3{1, 1, 1});
  error |= Test_Bounds_Constructor(NmVector3{0, 0, 0},       NmVector3{1.5, 3.7, 2.421});
  error |= Test_Bounds_Constructor(NmVector3{1, 1, 1},       NmVector3{1, 1, 1});
  error |= Test_Bounds_Constructor(NmVector3{1.4, 1.3, 3.3}, NmVector3{1, 1, 1});

  return error;
}