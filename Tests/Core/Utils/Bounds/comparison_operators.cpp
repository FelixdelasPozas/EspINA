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

int comparison_operators( int argc, char** argv )
{
  int error = 0;

  Bounds b1{1,2,3,4,5,6};
  Bounds b2{2,4,3,4,5,6};
  Bounds b3{'[',1,2,3,4,5,6,')'};
  Bounds b4{'(',1,2,3,4,5,6,']'};

  if (!(b1 == b1)) {
    cerr << b1 << " is equal to " << b1 << endl;
    error = EXIT_FAILURE;
  }

  if (b1 != b1) {
    cerr << b1 << " is equal to " << b1 << endl;
    error = EXIT_FAILURE;
  }

  if (b1 == b2) {
    cerr << b1 << " is not equal to " << b2 << endl;
    error = EXIT_FAILURE;
  }

  if (!(b1 != b2)) {
    cerr << b1 << " is not equal to " << b2  << endl;
    error = EXIT_FAILURE;
  }

  if (!(b1 == b3)) {
    cerr << b1 << " is equal to " << b3 << endl;
    error = EXIT_FAILURE;
  }

  if (b1 != b3) {
    cerr << b1 << " is equal to " << b3 << endl;
    error = EXIT_FAILURE;
  }

  if (b1 == b4) {
    cerr << b1 << " is not equal to " << b4 << endl;
    error = EXIT_FAILURE;
  }

  if (!(b1 != b4)) {
    cerr << b1 << " is not equal to " << b4 << endl;
    error = EXIT_FAILURE;
  }

  return error;
}