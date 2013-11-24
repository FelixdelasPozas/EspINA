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

#include "Core/Analysis/Data/Volumetric/SparseVolume.h"
#include <Core/Utils/Spatial.h>

using namespace EspINA;
using namespace std;

typedef unsigned char VoxelType;
typedef itk::Image<VoxelType, 3> ImageType;

int sparse_volume_default_constructor( int argc, char** argv )
{
  int error = 0;

 SparseVolume<ImageType> volume;

 Bounds defaultBounds = volume.bounds();

 if (defaultBounds.areValid()) {
   cerr << "Default constructed ItkVolume bounds: " << defaultBounds << ". Expected invalid bounds" << endl;
   error = EXIT_FAILURE;
 }

 for (Axis dir : {Axis::X, Axis::Y, Axis::Z}) {
   if (!defaultBounds.areLowerIncluded(dir)) {
     cerr << "Default volume bounds must have lower bounds included" << endl;
     error = EXIT_FAILURE;
   }

   if (defaultBounds.areUpperIncluded(dir)) {
     cerr << "Default volume bounds must have upper bounds excluded" << endl;
     error = EXIT_FAILURE;
   }
 }

 if (volume.memoryUsage() != 0) {
   cerr << "Default constructed ItkVolume memory usage must be 0" << endl;
   error = EXIT_FAILURE;
 }

 if (volume.backgroundValue() != 0) {
   cerr << "Default background value must be 0" << endl;
   error = EXIT_FAILURE;
 }

  return error;
}
