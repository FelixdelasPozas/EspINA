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

using namespace EspINA;
using namespace std;

int bounds_invalid_list_constructor( int argc, char** argv )
{
  int error = 0;
  try {
    Bounds bounds{0,1,2,3};
    error = EXIT_FAILURE;
    cerr << bounds << endl;
  } catch (Wrong_number_initial_values& e) {
  }

  try {
    Bounds bounds{0,1,2,3,4,5,6,7,8};
    error = EXIT_FAILURE;
    cerr << bounds << endl;
  } catch (Wrong_number_initial_values& e) {
  }

  try {
    Bounds bounds{0,1,2,3,4,5,6,7};
    error = EXIT_FAILURE;
    cerr << "Invalid Token" << bounds << endl;
  } catch (Invalid_bounds_token& e) {
  }

  try {
    Bounds bounds{0,1,2,3,4,5,6,7,8,9,10,11};
    error = EXIT_FAILURE;
    cerr << "Invalid Token" << bounds << endl;
  } catch (Invalid_bounds_token& e) {
  }

  try {
    Bounds bounds{'(',1,2,3,4,5,6,7,8,9,10,11};
    error = EXIT_FAILURE;
    cerr << "Invalid Token" << bounds << endl;
  } catch (Invalid_bounds_token& e) {
  }

  return error;
}