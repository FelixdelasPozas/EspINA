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

int bounding_box( int argc, char** argv )
{
  bool error = false;

  Bounds b1{-1.5, 0.5, -1.5, 0.5, -1.5, 0.5};
  Bounds b2{0.5, 1.5, 0.5, 1.5, 0.5, 1.5};
  Bounds bb{-1.5, 1.5, -1.5, 1.5, -1.5, 1.5};

  VolumeBounds vb1{b1};
  VolumeBounds vb2{b2};
  VolumeBounds vbb{bb};

  auto bb1_1 = boundingBox(vb1, vb1);
  if (vb1 != bb1_1)
  {
    cerr << bb1_1 << " is equal to " << vb1 << endl;
    error = true;
  }

  auto bb1_2 = boundingBox(vb1, vb2);
  if (!isEquivalent(bb1_2, vbb))
  {
    cerr << bb1_2 << " is equivalent to " << vbb << endl;
    error = true;
  }

  try
  {
    auto bb1_invalid = boundingBox(vb1, VolumeBounds());
    cerr << "Expected exception due to invalid bounds: " << bb1_invalid.toString() << endl;
    error = true;
  } catch (Incompatible_Volume_Bounds_Exception e)
  {
  }

  VolumeBounds incompatible{b1, NmVector3{2, 2, 2}};
  try
  {
    auto bb1_incompatible = boundingBox(vb1, incompatible);
    cerr << "Expected exception due to incompatible bounds: " << bb1_incompatible.toString() << endl;
    error = true;
  } catch (Incompatible_Volume_Bounds_Exception e)
  {
  }

  return error;
}