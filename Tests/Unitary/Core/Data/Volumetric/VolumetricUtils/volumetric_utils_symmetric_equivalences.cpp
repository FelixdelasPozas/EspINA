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
#include <Core/EspinaTypes.h>
#include <Core/Analysis/Data/VolumetricDataUtils.h>


using namespace std;
using namespace EspINA;

int volumetric_utils_symmetric_equivalences( int argc, char** argv )
{
  int error = 0;

  itkVolumeType::SizeType size{1, 1, 1};

  itkVolumeType::RegionType expectedRegion;
  expectedRegion.SetSize(size);

  itkVolumeType::Pointer image = itkVolumeType::New();
  image->SetRegions(expectedRegion);
  image->Allocate();

  // This is true only for those bounds computed directly from a region
  itkVolumeType::RegionType region = equivalentRegion<itkVolumeType>(image, equivalentBounds<itkVolumeType>(image, expectedRegion));
  if (region != expectedRegion) {
    cerr << "The equivalent region of the equivalent bounds of a given region should be the same given region." << endl;
    cerr << "Unexpected Equivalent Region:" << endl;
    region.Print(cerr);
    cerr << "Expected Region:" << endl;
    expectedRegion.Print(cerr);
    error = EXIT_FAILURE;
  }

  Bounds expectedBounds{-0.5, 0.5, -0.5, 0.5, -0.5, 0.5};

  Bounds bounds = equivalentBounds<itkVolumeType>(image, equivalentRegion<itkVolumeType>(image, expectedBounds));
  if (bounds != expectedBounds) {
    cerr << "The equivalent region of the equivalent bounds of a given region should be the same given region." << endl;
    cerr << "Unexpected Equivalent Bounds: " << bounds << endl;
    cerr << "Expected Bounds: " << expectedBounds << endl;
    error = EXIT_FAILURE;
  }

  return error;
}