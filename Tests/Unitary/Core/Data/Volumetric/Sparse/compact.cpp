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
#include "Tests/Unitary/Testing_Support.h"

#include <vtkSmartPointer.h>

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;

typedef unsigned char VoxelType;
typedef itk::Image<VoxelType, 3> ImageType;

int compact( int argc, char** argv )
{
  bool pass = true;

 auto bg = 0;
 auto fg = 255;

 int size = 100;

 Bounds bounds{0, size-1, 0, size-1, 0, size-1};
 SparseVolume<ImageType> canvas(bounds);

 if (canvas.memoryUsage() != 0)
 {
   cerr << "Invalid memory usage" << endl;
   pass = false;
 }

 Bounds lowerHalfVolume{-0.5, size-0.5, -0.5, size-0.5, -0.5, size/2 - 0.5};

 auto brush = vtkSmartPointer<vtkNaiveFunction>::New();
 canvas.draw(brush, lowerHalfVolume, fg);

 auto numberOfEditedVoxels = size*size*size/2;
 auto editedSize = numberOfEditedVoxels*sizeof(ImageType::ValueType);
 auto maskSize   = editedSize/8;

 if (canvas.memoryUsage() != maskSize)
 {
   cerr << "Invalid memory usage" << endl;
   pass = false;
 }

 Bounds upperHalfVolume{-0.5, size-0.5, -0.5, size-0.5, size/2 - 0.5, size-0.5};

 canvas.draw(brush, upperHalfVolume, fg);
 canvas.draw(brush, upperHalfVolume, bg);

 if (canvas.memoryUsage() != 3*maskSize)
 {
   cerr << "Invalid memory usage" << endl;
   pass = false;
 }

 canvas.compact();

 if (canvas.memoryUsage() != editedSize)
 {
   cerr << "Invalid memory usage" << endl;
   pass = false;
 }

 return !pass;
}
