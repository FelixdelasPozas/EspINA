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


bool Test_Create_Region_From_Bounds(const Bounds& bounds, itkVolumeType::Pointer image, bool passIfEquivalent = true) {
  bool pass = true;

  auto expectedRegion = image->GetLargestPossibleRegion();
  auto region = equivalentRegion<itkVolumeType>(image, bounds);
  if ((region == expectedRegion) != passIfEquivalent) {
    cerr << "Bounds: " << bounds << ":" << endl;
    cerr << "Expected Region:" << endl;
    expectedRegion.Print(cerr);
    cerr << "Equivalent Region:" << endl;
    region.Print(cerr);

    pass = false;
  }

  return pass;
}

int volumetric_utils_bounds_to_region( int argc, char** argv )
{
  bool pass = true;

  itkVolumeType::SizeType size;
  size.Fill(1);
  itkVolumeType::RegionType region;
  region.SetSize(size);
  itkVolumeType::Pointer image = itkVolumeType::New();
  image->SetLargestPossibleRegion(region);
  itkVolumeType::PointType origin;
  origin.Fill(0);
  image->SetOrigin(origin);
  itkVolumeType::SpacingType spacing;
  spacing.Fill(1);
  image->SetSpacing(spacing);

  pass &= Test_Create_Region_From_Bounds({'[',-0.5,0,-0.5,0,-0.5,0,')'}, image);
  pass &= Test_Create_Region_From_Bounds({'[',0,0,0,0,0,0,')'}, image);
  pass &= Test_Create_Region_From_Bounds({'[',0,0,0,0,0,0,']'}, image);
  pass &= Test_Create_Region_From_Bounds({'[',0,1,0,1,0,1,')'}, image, false);
  pass &= Test_Create_Region_From_Bounds({'[',0,1,0,1,0,1,']'}, image, false);

  return !pass;
}