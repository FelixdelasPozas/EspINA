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
#include "SegmentationCollision.h"

// itk
#include <itkImageRegionIteratorWithIndex.h>

// Qt
#include <QDebug>

using namespace EspINA;

//-----------------------------------------------------------------------------
bool EspINA::checkCollision(SegmentationVolumeSPtr seg1, SegmentationVolumeSPtr seg2)
{
  if (!seg1->espinaRegion().intersect(seg2->espinaRegion()))
    return false;

  itkVolumeType::RegionType region1 = seg1->toITK()->GetLargestPossibleRegion();
  itkVolumeType::RegionType region2 = seg2->toITK()->GetLargestPossibleRegion();
  itkVolumeType::RegionType::IndexType index;
  int extent[6];

  // some regions need to get their indexes adapted, some not. If the segmentation was created in the
  // current session, the index doesn't need to be adapted, otherwise the index is [0,0,0] and doesn't
  // match the current bounds so we need to adapt it. We need the regions transformed to get the correct
  // itk region intersection
  index = region1.GetIndex();
  seg1->extent(extent);
  bool adaptRegion1 = !((index[0] == extent[0]) && (index[1] == extent[2]) && (index[2] == extent[4]));
  if (adaptRegion1)
  {
    seg1->extent(extent);
    index[0] = extent[0];
    index[1] = extent[2];
    index[2] = extent[4];
    region1.SetIndex(index);
  }

  index = region2.GetIndex();
  seg2->extent(extent);
  bool adaptRegion2 = !((index[0] == extent[0]) && (index[1] == extent[2]) && (index[2] == extent[4]));
  if (adaptRegion2)
  {
    seg2->extent(extent);
    index[0] = extent[0];
    index[1] = extent[2];
    index[2] = extent[4];
    region2.SetIndex(index);
  }

  // both regions should be equal in index and size after cropping
  if (!region1.Crop(region2) || !region2.Crop(region1))
    Q_ASSERT(false);

  Q_ASSERT(region1 == region2);

  // re-adapt region indexes to be compatible with the original itk images
  if (adaptRegion1)
  {
    index = region1.GetIndex();
    seg1->extent(extent);
    index[0] -= extent[0];
    index[1] -= extent[2];
    index[2] -= extent[4];
    region1.SetIndex(index);
  }

  if (adaptRegion2)
  {
    index = region2.GetIndex();
    seg2->extent(extent);
    index[0] -= extent[0];
    index[1] -= extent[2];
    index[2] -= extent[4];
    region2.SetIndex(index);
  }

  itk::ImageRegionIteratorWithIndex<itkVolumeType> it1(seg1->toITK(), region1);
  itk::ImageRegionIteratorWithIndex<itkVolumeType> it2(seg2->toITK(), region2);
  it1.GoToBegin();
  it2.GoToBegin();

  for (; !it1.IsAtEnd(); ++it1, ++it2)
    if ((it1.Get() == SEG_VOXEL_VALUE) && (it2.Get() == SEG_VOXEL_VALUE))
      return true;

  return false;
}
