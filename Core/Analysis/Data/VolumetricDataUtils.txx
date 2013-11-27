/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <Core/Utils/Spatial.h>
#include <Core/Utils/Bounds.h>

namespace EspINA {

//-----------------------------------------------------------------------------
template<typename T>
typename T::RegionType equivalentRegion(const T* image, const Bounds& bounds)
{
  typename T::SpacingType s = image->GetSpacing();

  typename T::PointType p0, p1;
  for (int i = 0; i < 3; ++i) {
    Axis dir = toAxis(i);

    p0[i] = bounds[2*i];
    p1[i] = bounds[2*i+1];

    int startIndex = p0[i]/s[i];
    double voxelStart = startIndex*s[i]+s[i]/2;

    double startValue = fabs(p0[i]-voxelStart);
    bool isVoxelStart =  startValue < 0.0001;
    if (isVoxelStart) {
      p0[i] += s[i]/2.0;
    }

    int index = p1[i]/s[i];
    double voxelEnd = index*s[i]+s[i]/2;

    double value = fabs(p1[i]-voxelEnd);
    bool isVoxelEnd =  value < 0.0001;
    if (!bounds.areUpperIncluded(dir) && isVoxelEnd) {
      p1[i] -= s[i]/2.0;
    }
  }

  typename T::IndexType i0, i1;
  image->TransformPhysicalPointToIndex(p0, i0);
  image->TransformPhysicalPointToIndex(p1, i1);

  typename T::RegionType region;
  region.SetIndex(i0);
  region.SetUpperIndex(i1);

  return region;
}

//-----------------------------------------------------------------------------
template<typename T>
Bounds equivalentBounds(const typename T::Pointer image, const typename T::RegionType& region)
{
  Bounds bounds;

  typename T::PointType p0, p1;

  image->TransformIndexToPhysicalPoint(region.GetIndex(), p0);
  image->TransformIndexToPhysicalPoint(region.GetUpperIndex(), p1);

  typename T::SpacingType s = image->GetSpacing();

  for (int i = 0; i < 3; ++i)
  {
    bounds[2*i]   = p0[i] - s[i]/2;
    bounds[2*i+1] = p1[i] + s[i]/2;
  }

  return bounds;
}

// //-----------------------------------------------------------------------------
// double memory_size_in_MB(int number_of_pixels)
// {
//   return number_of_pixels / 1024.0 / 1024.0;
// }

template<typename T>
//-----------------------------------------------------------------------------
typename T::Pointer create_itkImage(const Bounds&                bounds,
                                    const typename T::ValueType  value,
                                    const NmVector3             &spacing,
                                    const NmVector3             &origin)
{
  typename T::PointType   itkOrigin;
  typename T::SpacingType itkSpacing;

  for(int i = 0; i < 3; ++i)
  {
    itkOrigin[i]  = origin[i];
    itkSpacing[i] = spacing[i];
  }

  typename T::Pointer image = T::New();
  // Origin and spacing must be set before calling equivalentRegion on image
  image->SetOrigin(itkOrigin);
  image->SetSpacing(itkSpacing);
  image->SetRegions(equivalentRegion<T>(image, bounds));
  image->Allocate();
  image->FillBuffer(value);

  return image;
}

}
