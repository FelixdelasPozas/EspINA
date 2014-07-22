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

int draw_implicit_function( int argc, char** argv )
{
  bool pass = true;

 auto bg = 0;
 auto fg = 255;

 Bounds bounds{-0.5, 3.5, -0.5, 3.5, -0.5, 3.5};
 SparseVolume<ImageType> canvas(bounds);

 if (!Testing_Support<ImageType>::Test_Pixel_Values(canvas.itkImage(), bg)) {
   cerr << "Initial values are not initialized to " << bg << endl;
   pass = false;
 }

 Bounds lowerHalfVolume{-0.5, 3.5, -0.5, 3.5, -0.5, 1.5};
 if (!Testing_Support<ImageType>::Test_Pixel_Values(canvas.itkImage(lowerHalfVolume), bg, lowerHalfVolume)) {
   cerr << "Initial Pixel values inside " << lowerHalfVolume << " should be " << bg << endl;
   pass = false;
 }

 auto brush = vtkSmartPointer<vtkNaiveFunction>::New();
 canvas.draw(brush, lowerHalfVolume, fg);

 if (!Testing_Support<ImageType>::Test_Pixel_Values(canvas.itkImage(lowerHalfVolume), fg, lowerHalfVolume)) {
   cerr << "Pixel values inside " << lowerHalfVolume << " should be " << fg << endl;
   pass = false;
 }

 Bounds upperHalfVolume{-0.5, 3.5, -0.5, 3.5, 1.5, 3.5};
 if (!Testing_Support<ImageType>::Test_Pixel_Values(canvas.itkImage(upperHalfVolume), bg, upperHalfVolume)) {
   cerr << "Pixel values inside " << upperHalfVolume << " should be " << bg << endl;
   pass = false;
 }

 canvas.draw(brush, lowerHalfVolume, bg);

 if (!Testing_Support<ImageType>::Test_Pixel_Values(canvas.itkImage(lowerHalfVolume), bg, lowerHalfVolume)) {
   cerr << "Pixel values inside " << lowerHalfVolume << " should be " << bg << endl;
   pass = false;
 }

  return !pass;
}
