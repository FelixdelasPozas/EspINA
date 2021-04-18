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

#include "Core/Analysis/Data/Volumetric/SparseVolume.hxx"
#include "Tests/Testing_Support.h"

#include <vtkSmartPointer.h>

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;

using VoxelType = unsigned char;
using ImageType = itk::Image<VoxelType, 3>;

int sparse_volume_resize_reduce_volume( int argc, char** argv )
{
  bool pass = true;

  VoxelType fg = 255;

  Bounds initialBounds{0, 20, 0, 20, 0, 20};
  SparseVolume<ImageType> volume(initialBounds);
  volume.draw(initialBounds, fg);

  if (!Testing_Support<ImageType>::Test_Pixel_Values(volume.itkImage(), fg)) {
    cerr << "Initial values are not initialized to " << fg << endl;
    pass = false;
  }

  Bounds reducedBounds{0, 10, 0, 10, 0, 10};
  volume.resize(reducedBounds);

  Bounds expectedBounds{-0.5, 10.5, -0.5, 10.5, -0.5, 10.5};
  if (volume.bounds() !=  expectedBounds)
  {
    cerr << "Reduced bounds " << volume.bounds() << " don't match requested bounds " << expectedBounds << endl;
    pass = false;
  }

  if (!Testing_Support<ImageType>::Test_Pixel_Values(volume.itkImage(reducedBounds), fg)) {
    cerr << "Initial pixel values have been modified" << endl;
    pass = false;
  }

  return !pass;
}
