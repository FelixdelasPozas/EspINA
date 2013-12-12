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

int equivalence(int argc, char** argv )
{
  bool error = false;

  Bounds b1{-1.5, 1.5, -1.5, 1.5, -1.5, 1.5};
  Bounds b2{-0.5, 0.5, -0.5, 0.5, -0.5, 0.5};

  NmVector3 spacing1{1,1,1};

  NmVector3 origin1{0,0,0};
  NmVector3 origin2{1,1,1};

  VolumeBounds vb1{b1, spacing1, origin1};
  VolumeBounds vb2{b1, spacing1, origin2};
  VolumeBounds vb3{b2, spacing1, origin1};

  if (!isEquivalent(vb1, vb1)) {
    cerr << vb1 << " is equivalent with itself" << endl;
    error = true;
  }

  if (!isEquivalent(vb1, vb2)) {
    cerr << vb1 << " is equivalent with " << vb2 << endl;
    error = true;
  }

  if (isEquivalent(vb1, vb3)) {
    cerr << vb1 << " is not equivalent with " << vb3 << endl;
    error = true;
  }

  Nm s = 1.5;
  Nm b = 3*s/2;
  NmVector3 spacing2{s,s,s};

  Bounds b3{-b, b, -b, b, -b, b};
  VolumeBounds vb4{b3, spacing2};
  VolumeBounds vb5{b3, spacing2, spacing2};

  if (!isEquivalent(vb4, vb5)) {
    cerr << vb4 << " is equivalent with " << vb5 << endl;
    error = true;
  }

  if (isEquivalent(vb1, vb4)) {
    cerr << vb4 << " is not compatibile with " << vb1 << endl;
    error = true;
  }


  return error;
}