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
#include "Tests/Unitary/Core/Data/Volumetric/Testing_Support.h"

#include <vtkSmartPointer.h>

using namespace ESPINA;
using namespace std;

typedef unsigned char VoxelType;
typedef itk::Image<VoxelType, 3> ImageType;


int sparse_volume_resize_expand_volume( int argc, char** argv )
{
  bool pass = true;

  // TODO: update
//  VoxelType bg = 0;
//  VoxelType fg = 255;
//
//  Bounds initialBounds{0, 20, 0, 20, 0, 20};
//  SparseVolume<ImageType> volume(initialBounds);
//  volume.draw(vtkSmartPointer<vtkNaiveFunction>::New(), initialBounds, fg);
//
//  if (!Testing_Support<ImageType>::Test_Pixel_Values(volume.itkImage(), fg)) {
//    cerr << "Initial values are not initialized to " << fg << endl;
//    pass = false;
//  }
//
//  Bounds resizedBounds{0, 40, 0, 40, 0, 40};
//  volume.resize(resizedBounds);
//
//  if (volume.bounds() != resizedBounds) {
//    cerr << "Resized bounds " << volume.bounds() << " don't match requested bounds " << resizedBounds << endl;
//  }
//
//  if (!Testing_Support<ImageType>::Test_Pixel_Values(volume.itkImage(initialBounds), fg)) {
//    cerr << "Initial pixel values have been modified" << endl;
//    pass = false;
//  }
//
//  Bounds expandedBounds{20, 40, 20, 40, 20, 40};
//  if (!Testing_Support<ImageType>::Test_Pixel_Values(volume.itkImage(expandedBounds), volume.backgroundValue())) {
//    cerr << "Expanded pixel values are different from backround value" << volume.backgroundValue() << endl;
//    pass = false;
//  }

  return !pass;
}
