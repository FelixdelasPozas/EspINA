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
#include <Tests/Testing_Support.h>


using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;

int volumetric_utils_equivalent_bounds( int argc, char** argv )
{
  bool pass = true;

  itkVolumeType::PointType origin1;
  origin1.Fill(0);

  itkVolumeType::PointType origin2;
  origin2.Fill(5);

  itkVolumeType::SpacingType spacing;
  spacing.Fill(2.5);

  itkVolumeType::Pointer img1 = Testing_Support<itkVolumeType>::Create_Test_Image(origin1, {2,2,2}, {10,20,30}, spacing, 0);
  itkVolumeType::Pointer img2 = Testing_Support<itkVolumeType>::Create_Test_Image(origin2, {0,0,0}, {10,20,30}, spacing, 0);

  if (img1->GetLargestPossibleRegion() == img2->GetLargestPossibleRegion()) {
    cerr << "Region1 should be different from Region2" << endl;
    pass = false;
  }

  Bounds bounds1 = equivalentBounds<itkVolumeType>(img1, img1->GetLargestPossibleRegion());
  Bounds bounds2 = equivalentBounds<itkVolumeType>(img2, img2->GetLargestPossibleRegion());
  if (bounds1 != bounds2) {
    cerr << bounds1 << " should be equal to " << bounds2 << endl;
    pass = false;
  }

  return !pass;
}