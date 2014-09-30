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
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>


using namespace std;
using namespace ESPINA;


int volumetric_utils_create_itk_image( int argc, char** argv )
{
  bool pass = true;

  itkVolumeType::Pointer e1 = itkVolumeType::New();
  itkVolumeType::RegionType region1;
  for(int i = 0; i < 3; ++i) region1.SetSize(i, 1);
  e1->SetLargestPossibleRegion(region1);

  Bounds b1{-0.5, 0.5, -0.5, 0.5, -0.5, 0.5};
  auto i1 = create_itkImage<itkVolumeType>(b1, 0);

  if (i1->GetLargestPossibleRegion() != e1->GetLargestPossibleRegion())
  {
    cerr << "Expeceted Region:" << endl;
    e1->GetLargestPossibleRegion().Print(std::cout);
    cerr << "Created Region:" << endl;
    i1->GetLargestPossibleRegion().Print(std::cout);
    pass = false;
  }

  if (e1->GetLargestPossibleRegion() != equivalentRegion<itkVolumeType>(i1, b1))
  {
    cerr << "Expeceted Region:" << endl;
    e1->GetLargestPossibleRegion().Print(std::cout);
    cerr << "Created Region:" << endl;
    equivalentRegion<itkVolumeType>(i1, b1).Print(std::cout);
  }

  itkVolumeType::Pointer e2 = itkVolumeType::New();
  itkVolumeType::RegionType region2;
  region2.SetSize(0, 402);
  region2.SetSize(1, 254);
  region2.SetSize(2, 21);
  e2->SetLargestPossibleRegion(region2);
  itkVolumeType::PointType origin;
  origin[0] = 2549.3;
  origin[1] = 1783.4;
  origin[2] = 0;
  e2->SetOrigin(origin);

  itkVolumeType::SpacingType spacing;
  spacing[0] = 3.7;
  spacing[1] = 3.7;
  spacing[2] = 20;
  e2->SetSpacing(spacing);


  Bounds b2;
  for(int i = 0; i < 3; ++i)
  {
    b2[2*i]   = e2->GetOrigin()[i]-e2->GetSpacing()[i]/2.0;
    b2[2*i+1] = b2[2*i]+ region2.GetSize(i)*e2->GetSpacing()[i];
  }

  auto i2 = create_itkImage<itkVolumeType>(b2, 0, {3.7, 3.7, 20}, {2549.3, 1783.4, 0});

  if (i2->GetLargestPossibleRegion() != e2->GetLargestPossibleRegion())
  {
    cerr << "Expeceted Region:" << endl;
    e2->GetLargestPossibleRegion().Print(std::cout);
    cerr << "Created Region:" << endl;
    i2->GetLargestPossibleRegion().Print(std::cout);
    pass = false;
  }

  itkVolumeType::Pointer e3 = itkVolumeType::New();
  itkVolumeType::RegionType region3;
  for (int i = 0; i < 3; ++i)
  {
    region3.SetIndex(i, (origin[i]+spacing[i]/2.0)/spacing[i]);
  }
  region3.SetSize(0, 402);
  region3.SetSize(1, 254);
  region3.SetSize(2, 21);
  e3->SetLargestPossibleRegion(region3);
  e3->SetSpacing(spacing);

  Bounds b3;
  for(int i = 0; i < 3; ++i)
  {
    b3[2*i]   = (region3.GetIndex()[i]-0.5) * e3->GetSpacing()[i];
    b3[2*i+1] = b3[2*i]+ region3.GetSize(i)*e3->GetSpacing()[i];
  }

  auto i3 = create_itkImage<itkVolumeType>(b3, 0, {3.7, 3.7, 20});

  if (i3->GetLargestPossibleRegion() != e3->GetLargestPossibleRegion())
  {
    cerr << "Expeceted Region:" << endl;
    e3->GetLargestPossibleRegion().Print(std::cout);
    cerr << "Created Region:" << endl;
    i3->GetLargestPossibleRegion().Print(std::cout);
    pass = false;
  }

  Bounds b4 = equivalentBounds<itkVolumeType>(i2, i2->GetLargestPossibleRegion());

  auto i4 = create_itkImage<itkVolumeType>(b4, 0, {3.7, 3.7, 20});

  if (i3->GetLargestPossibleRegion() != i4->GetLargestPossibleRegion())
  {
    cerr << "Expeceted Region:" << endl;
    i3->GetLargestPossibleRegion().Print(std::cout);
    cerr << "Created Region:" << endl;
    i4->GetLargestPossibleRegion().Print(std::cout);
    pass = false;
  }

  // This one has non 0 index
  Bounds b5 = equivalentBounds<itkVolumeType>(i3, i3->GetLargestPossibleRegion());

  auto i5 = create_itkImage<itkVolumeType>(b5, 0, {3.7, 3.7, 20});

  if (i3->GetLargestPossibleRegion() != i5->GetLargestPossibleRegion())
  {
    cerr << "Expeceted Region:" << endl;
    i3->GetLargestPossibleRegion().Print(std::cout);
    cerr << "Created Region:" << endl;
    i5->GetLargestPossibleRegion().Print(std::cout);
    pass = false;
  }

  return !pass;
}
