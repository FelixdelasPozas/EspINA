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

#include <sstream>

using namespace EspINA;
using namespace std;

int stream_operator( int argc, char** argv )
{

  int error = 0;

  Bounds b1{1,2,3,4,5,6};

  stringstream b1_stream;
  
  string expected_b1_stream = "{[1,2),[3,4),[5,6)}";
  
  b1_stream << b1;
  
  if (b1_stream.str() != expected_b1_stream) {
    cerr << b1_stream.str() << " is not equal to " << expected_b1_stream << endl;
    error = EXIT_FAILURE;
  }
  
  Bounds b2{'(',1,2,3,4,5,6,']'};

  stringstream b2_stream;
  
  string expected_b2_stream = "{(1,2],(3,4],(5,6]}";
  
  b2_stream << b2;
  
  if (b2_stream.str() != expected_b2_stream) {
    cerr << "_" << b2_stream.str() << "_ is not equal to _" << expected_b2_stream << "_"<< endl;
    error = EXIT_FAILURE;
  }

  return error;
}