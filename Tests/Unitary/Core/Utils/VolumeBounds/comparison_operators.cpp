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

using namespace ESPINA;
using namespace std;

int comparison_operators( int argc, char** argv )
{
  bool error = false;

  Bounds b1{-0.5, 0.5, -0.5, 0.5, -0.5, 0.6};
  Bounds b2{-1.5, 1.5, -1.5, 1.5, -1.5, 1.5};

  NmVector3 spacing1{1,1,1};
  NmVector3 spacing3{3,3,3};

  NmVector3 origin1{1,1,1};

  VolumeBounds vb1{b1, spacing1};
  VolumeBounds vb2{b2, spacing1};
  VolumeBounds vb3{b2, spacing3};
  VolumeBounds vb4{b2, spacing1, origin1};

  if (!(vb1 == vb1)) {
    cerr << vb1 << " is equal to " << vb1 << endl;
    error = true;
  }

  if (vb1 != vb1) {
    cerr << vb1 << " is equal to " << vb1 << endl;
    error = true;
  }

  if (vb1 == vb2) {
    cerr << vb1 << " is not equal to " << vb2 << endl;
    error = true;
  }

  if (!(vb1 != vb2)) {
    cerr << b1 << " is not equal to " << b2  << endl;
    error = true;
  }

  if (vb2.bounds() != vb3.bounds()) {
    cerr << vb2.bounds() << " is equal to " << vb3.bounds() << endl;
    error = true;
  }

  if (vb2 == vb3) {
    cerr << vb2 << " is not equal to " << vb3 << endl;
    error = true;
  }

  if (!(vb2 != vb3)) {
    cerr << vb2 << " is not equal to " << vb3 << endl;
    error = true;
  }

  if (vb2.bounds() != vb4.bounds()) {
    cerr << vb2.bounds() << " is equal to " << vb4.bounds() << endl;
    error = true;
  }

  if (vb2 == vb4) {
    cerr << vb2 << " is not equal to " << vb4 << endl;
    error = true;
  }

  if (!(vb2 != vb4)) {
    cerr << vb2 << " is not equal to " << vb4 << endl;
    error = true;
  }

  return error;
}