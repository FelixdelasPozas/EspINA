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
#include <Core/Types.h>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>


using namespace std;
using namespace ESPINA;

bool Test_Create_Region_From_Slice_Bounds(Nm w, Nm h, Nm d, bool passIfEquivalent = true) {
  bool pass = true;

  for (int i = 0;  i < 3; ++i)
  {
    Nm dim[3] = {w, h, d};

    dim[i] = 1;
    itkVolumeType::SizeType size;
    for (int j = 0; j < 3; ++j)
    {
      size.SetElement(j , dim[j]);
    }

    itkVolumeType::RegionType expectedRegion;
    expectedRegion.SetSize(size);

    itkVolumeType::Pointer image = itkVolumeType::New();
    image->SetRegions(expectedRegion);
    image->Allocate();

    dim[i] = 0;
    Bounds bounds{0, dim[0], 0, dim[1], 0, dim[2]};
    for (int j = 0; j < 6; ++j) if (j < 2*i || j > 2*i+1) bounds[j] -= 0.5;
    if ((equivalentRegion<itkVolumeType>(image, bounds) == expectedRegion) != passIfEquivalent) {
      cerr << "Expected Region:" << endl;
      expectedRegion.Print(cerr);
      cerr << "Equivalent Region From Bounds " << bounds << ":" << endl;
      equivalentRegion<itkVolumeType>(image, bounds).Print(cerr);

      pass = false;
    }
  }

  return pass;
}

int volumetric_utils_slice_bounds_to_region(int argc, char** argv)
{
  bool pass = true;

  pass &= Test_Create_Region_From_Slice_Bounds(1, 1, 1);

  pass &= Test_Create_Region_From_Slice_Bounds(10, 20, 30);

  return !pass;
}
