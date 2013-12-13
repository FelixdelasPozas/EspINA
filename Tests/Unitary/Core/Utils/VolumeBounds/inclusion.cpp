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

int inclusion( int argc, char** argv )
{
  bool error = false;

  Bounds containerBounds{-1.5, 1.5, -1.5, 1.5, -1.5, 1.5};
  Bounds containedBounds{-0.5, 0.5, -0.5, 0.5, -0.5, 0.5};

  VolumeBounds container(containerBounds);
  VolumeBounds contained(containedBounds);

  if (!contains(container, container)) {
    cerr << container <<  " are contained itself" << endl;
    error = true;
  }

  if (!contains(container, contained)) {
    cerr << contained <<  " are contained inside " << container << endl;
    error = true;
  }

  if (contains(contained, container)) {
    cerr << container <<  " are not contained inside " << contained << endl;
    error = true;
  }

  VolumeBounds invalid(containedBounds, NmVector3{2, 2, 2});
  if (contains(container, invalid)) {
    cerr << invalid <<  " are not contained inside " << container << endl;
    error = true;
  }

  NmVector3 inside;
  if (!contains(container, inside)) {
    cerr << inside <<  " is contained inside " << container << endl;
    error = true;
  }

  NmVector3 leftCorner{-1.5, -1.5, -1.5};
  if (!contains(container, leftCorner)) {
    cerr << leftCorner <<  " is contained inside " << container << endl;
    error = true;
  }

  NmVector3 rightCorner{1.5, 1.5, 1.5};
  if (contains(container, rightCorner)) {
    cerr << rightCorner <<  " is not contained inside " << container << endl;
    error = true;
  }

  NmVector3 outerLetf{-2.5, -2.5, -2.5};
  if (contains(container, outerLetf)) {
    cerr << outerLetf <<  " is not contained inside " << container << endl;
    error = true;
  }

  NmVector3 outerRight{2.5, 2.5, 2.5};
  if (contains(container, outerRight)) {
    cerr << outerRight <<  " is not contained inside " << container << endl;
    error = true;
  }

  return error;
}