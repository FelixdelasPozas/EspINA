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

bool Test_Bounds(const Bounds& bounds, itkVolumeType::Pointer image, bool passIfEquivalent = true) {
  bool pass = true;

  Bounds equivalent = equivalentBounds<itkVolumeType>(image, image->GetLargestPossibleRegion());
  if ((equivalent == bounds) != passIfEquivalent) {
    cerr << "Region:" << endl;
    image->GetLargestPossibleRegion().Print(cerr);
    cerr << "Unexpected equivalent bounds:" << equivalent << ". Expected: " << bounds << endl;

    pass = false;
  }

  return pass;
}


int volumetric_utils_region_to_bounds( int argc, char** argv )
{
  bool pass = true;

  itkVolumeType::Pointer image1 = itkVolumeType::New();
  itkVolumeType::RegionType region1;
  for(int i = 0; i < 3; ++i) region1.SetSize(i, 1);
  image1->SetLargestPossibleRegion(region1);

  pass &= Test_Bounds({-0.5, 0.5, -0.5, 0.5, -0.5, 0.5}, image1);
  pass &= Test_Bounds({'[',-0.5, 0.5, -0.5, 0.5, -0.5, 0.5,']'}, image1, false);
  pass &= Test_Bounds({0, 1, 0, 1, 0, 1}, image1, false);

  itkVolumeType::Pointer image2 = itkVolumeType::New();
  itkVolumeType::RegionType region2;
  region2.SetSize(0, 402);
  region2.SetSize(1, 254);
  region2.SetSize(2, 21);
  image2->SetLargestPossibleRegion(region2);
  itkVolumeType::PointType origin;
  origin[0] = 2549.3;
  origin[1] = 1783.4;
  origin[2] = 0;
  image2->SetOrigin(origin);

  itkVolumeType::SpacingType spacing;
  spacing[0] = 3.7;
  spacing[1] = 3.7;
  spacing[2] = 20;
  image2->SetSpacing(spacing);

  Bounds bounds2;
  for(int i = 0; i < 3; ++i)
  {
    bounds2[2*i]   = image2->GetOrigin()[i]-image2->GetSpacing()[i]/2.0;
    bounds2[2*i+1] = bounds2[2*i]+ region2.GetSize(i)*image2->GetSpacing()[i];
  }
  pass &= Test_Bounds(bounds2, image2);

  itkVolumeType::Pointer image3 = itkVolumeType::New();
  itkVolumeType::RegionType region3;
  for (int i = 0; i < 3; ++i)
  {
    region3.SetIndex(i, (origin[i]+spacing[i]/2.0)/spacing[i]);
  }
  region3.SetSize(0, 402);
  region3.SetSize(1, 254);
  region3.SetSize(2, 21);
  image3->SetLargestPossibleRegion(region3);
  image3->SetSpacing(spacing);

  Bounds bounds3;
  for(int i = 0; i < 3; ++i)
  {
    bounds3[2*i]   = (region3.GetIndex()[i]-0.5) * image3->GetSpacing()[i];
    bounds3[2*i+1] = bounds3[2*i]+ region3.GetSize(i)*image3->GetSpacing()[i];
  }
  pass &= Test_Bounds(bounds3, image3);

  return !pass;
}
