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

using namespace std;
using namespace EspINA;

int inclusion_exclusion( int argc, char** argv )
{
  bool error = false;

  Bounds b1{-1.5, 1.5, -1.5, 1.5, -1.5, 1.5};

  VolumeBounds bounds1(b1);
  bounds1.exclude(0, -2);
  if (bounds1[0] != -1.5)
  {
    cerr << bounds1 <<  " doesn't exclude -2 "<< endl;
    error = true;
  }

  bounds1.include(0, -0.5);
  if (bounds1[0] != -1.5)
  {
    cerr << bounds1 <<  " doesn't include -0.5 "<< endl;
    error = true;
  }

  bounds1.exclude(0, -1.5);
  if (bounds1[0] != -0.5)
  {
    cerr << bounds1 <<  " doesn't exclude -1.5 "<< endl;
    error = true;
  }

  bounds1.include(0, -1.5);
  if (bounds1[0] != -1.5)
  {
    cerr << bounds1 <<  " doesn't include -1.5 "<< endl;
    error = true;
  }

  bounds1.exclude(0, -1);
  if (bounds1[0] != -0.5)
  {
    cerr << bounds1 <<  " doesn't exclude -1 "<< endl;
    error = true;
  }

  bounds1.include(0, -0.5);
  if (bounds1[0] != -0.5)
  {
    cerr << bounds1 <<  " doesn't include -0.5 "<< endl;
    error = true;
  }

  bounds1.exclude(1, 2);
  if (bounds1[1] != 1.5)
  {
    cerr << bounds1 <<  " doesn't exclude 2 "<< endl;
    error = true;
  }

  bounds1.include(1, 0.5);
  if (bounds1[1] != 1.5)
  {
    cerr << bounds1 <<  " doesn't include 0.5 "<< endl;
    error = true;
  }

  bounds1.exclude(1, 1.5);
  if (bounds1[1] != 1.5)
  {
    cerr << bounds1 <<  " doesn't exclude 1.5 "<< endl;
    error = true;
  }

  bounds1.include(1, 1.5);
  if (bounds1[1] != 2.5)
  {
    cerr << bounds1 <<  " doesn't include 1.5 "<< endl;
    error = true;
  }

  bounds1.exclude(1, 1.5);
  if (bounds1[1] != 1.5)
  {
    cerr << bounds1 <<  " doesn't exclude 1.5 "<< endl;
    error = true;
  }

  bounds1.exclude(1, 1);
  if (bounds1[1] != 0.5)
  {
    cerr << bounds1 <<  " doesn't exclude 1 "<< endl;
    error = true;
  }

  bounds1.include(1, 0.5);
  if (bounds1[1] != 1.5)
  {
    cerr << bounds1 <<  " doesn't include 0.5 "<< endl;
    error = true;
  }

  Bounds b2{-3.5, -1.5, -3.5, -1.5, -3.5, -1.5};

  VolumeBounds bounds2(b2);
  bounds2.exclude(0, -4);
  if (bounds2[0] != -3.5)
  {
    cerr << bounds2 <<  " doesn't exclude -4 "<< endl;
    error = true;
  }

  bounds2.include(0, -2.5);
  if (bounds2[0] != -3.5)
  {
    cerr << bounds2 << " doesn't include -2.5 "<< endl;
    error = true;
  }

  bounds2.exclude(0, -3.5);
  if (bounds2[0] != -2.5)
  {
    cerr << bounds2 <<  " doesn't exclude -3.5 "<< endl;
    error = true;
  }

  bounds2.include(0, -3.5);
  if (bounds2[0] != -3.5)
  {
    cerr << bounds2 <<  " doesn't include -3.5 "<< endl;
    error = true;
  }

  bounds2.exclude(0, -3);
  if (bounds2[0] != -2.5)
  {
    cerr << bounds2 << " doesn't exclude -3 "<< endl;
    error = true;
  }

  bounds2.include(0, -2.5);
  if (bounds2[0] != -2.5)
  {
    cerr << bounds2 <<  " doesn't include -2.5 "<< endl;
    error = true;
  }

  bounds2.exclude(1, 0);
  if (bounds2[1] != -1.5)
  {
    cerr << bounds2 << " doesn't exclude 0 "<< endl;
    error = true;
  }

  bounds2.include(1, -2.5);
  if (bounds2[1] != -1.5)
  {
    cerr << bounds2 << " doesn't include -2.5 "<< endl;
    error = true;
  }

  bounds2.exclude(1, -1.5);
  if (bounds2[1] != -1.5)
  {
    cerr << bounds2 << " doesn't exclude -1.5 "<< endl;
    error = true;
  }

  bounds2.include(1, -1.5);
  if (bounds2[1] != -0.5)
  {
    cerr << bounds2 <<  " doesn't include -1.5 "<< endl;
    error = true;
  }

  bounds2.exclude(1, -1.5);
  if (bounds2[1] != -1.5)
  {
    cerr << bounds2 <<  " doesn't exclude -1.5 "<< endl;
    error = true;
  }

  bounds2.exclude(1, -2);
  if (bounds2[1] != -2.5)
  {
    cerr << bounds2 <<  " doesn't exclude -2 "<< endl;
    error = true;
  }

  bounds2.include(1, -2.5);
  if (bounds2[1] != -1.5)
  {
    cerr << bounds2 << " doesn't include -2.5 "<< endl;
    error = true;
  }


  Bounds b3{1.5, 3.5, 1.5, 3.5, 1.5, 3.5};

  VolumeBounds bounds3(b3);
  bounds3.exclude(0, 0);
  if (bounds3[0] != 1.5)
  {
    cerr << bounds3 <<  " doesn't exclude 0 "<< endl;
    error = true;
  }

  bounds3.include(0, 2);
  if (bounds3[0] != 1.5)
  {
    cerr << bounds3 <<  " doesn't include 2 "<< endl;
    error = true;
  }

  bounds3.exclude(0, 1.5);
  if (bounds3[0] != 2.5)
  {
    cerr << bounds3 <<  " doesn't exclude 1.5 "<< endl;
    error = true;
  }

  bounds3.include(0, 1.5);
  if (bounds3[0] != 1.5)
  {
    cerr << bounds3 <<  " doesn't include 1.5 "<< endl;
    error = true;
  }

  bounds3.exclude(0, 2);
  if (bounds3[0] != 2.5)
  {
    cerr << bounds3 <<  " doesn't exclude 2.5 "<< endl;
    error = true;
  }

  bounds3.include(0, 1.5);
  if (bounds3[0] != 1.5)
  {
    cerr << bounds3 <<  " doesn't include 1.5 "<< endl;
    error = true;
  }

  bounds3.exclude(1, 4);
  if (bounds3[1] != 3.5)
  {
    cerr << bounds3 <<  " doesn't exclude 2 "<< endl;
    error = true;
  }

  bounds3.include(1, 2.5);
  if (bounds3[1] != 3.5)
  {
    cerr << bounds3 <<  " doesn't include 2.5 "<< endl;
    error = true;
  }

  bounds3.exclude(1, 2.5);
  if (bounds3[1] != 2.5)
  {
    cerr << bounds3 <<  " doesn't exclude 2.5 "<< endl;
    error = true;
  }

  bounds3.include(1, 2.5);
  if (bounds3[1] != 3.5)
  {
    cerr << bounds3 <<  " doesn't include 3.5 "<< endl;
    error = true;
  }

  bounds3.exclude(1, 2.5);
  if (bounds3[1] != 2.5)
  {
    cerr << bounds3 <<  " doesn't exclude 2.5 "<< endl;
    error = true;
  }

  bounds3.exclude(1, 3);
  if (bounds3[1] != 2.5)
  {
    cerr << bounds3 <<  " doesn't exclude 3 "<< endl;
    error = true;
  }

  bounds3.include(1, 3);
  if (bounds3[1] != 3.5)
  {
    cerr << bounds3 <<  " doesn't include 0.5 "<< endl;
    error = true;
  }

  return error;
}