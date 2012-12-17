/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2012  Laura Fernandez Soria <laura.fernandez@ctb.upm.es>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// EspINA
#include <Core/EspinaTypes.h>
#include <Core/EspinaRegion.h>
#include "SegmentationCollision.h"

// itk
#include <itkImageRegionIteratorWithIndex.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
bool checkCollision(EspinaVolume::Pointer seg1, EspinaVolume::Pointer seg2)
{
  if (!seg1->espinaRegion().intersect(seg2->espinaRegion()))
    return false;

  itkVolumeType::RegionType region1 = seg1->toITK()->GetLargestPossibleRegion();
  itkVolumeType::RegionType region2 = seg2->toITK()->GetLargestPossibleRegion();
  Q_ASSERT(region1.Crop(region2));
  Q_ASSERT(region2.Crop(region1));

  itk::ImageRegionIteratorWithIndex<itkVolumeType> it1(seg1->toITK(), region1);
  itk::ImageRegionIteratorWithIndex<itkVolumeType> it2(seg2->toITK(), region2);
  it1.GoToBegin();
  it2.GoToBegin();

  for (; !it1.IsAtEnd(); ++it1, ++it2)
    if ((it1.Get() == (SEG_VOXEL_VALUE)) && (it2.Get() == (SEG_VOXEL_VALUE)))
      return true;

  return false;
}
