/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "EspinaRegions.h"


//-----------------------------------------------------------------------------
void VolumeExtent(EspinaVolume *volume, int extent[6])
{
  EspinaVolume::SpacingType spacing = volume->GetSpacing();
  EspinaVolume::RegionType region = volume->GetLargestPossibleRegion();
  for(int i=0; i<3; i++)
  {
    int min = 2*i, max = 2*i+1;
    extent[min] = int(volume->GetOrigin()[i]/spacing[i] + 0.5) + region.GetIndex(i);
    extent[max] = extent[min] + region.GetSize(i) - 1;
  }
}

//-----------------------------------------------------------------------------
void VolumeBounds(EspinaVolume *volume, double bounds[6])
{
  EspinaVolume::SpacingType spacing = volume->GetSpacing();
  EspinaVolume::RegionType region   = volume->GetLargestPossibleRegion();
  for(int i=0; i<3; i++)
  {
    int min = 2*i, max = 2*i+1;
    bounds[min] = volume->GetOrigin()[i] + region.GetIndex()[i]*spacing[i];
    bounds[max] = bounds[min] + (region.GetSize()[i] - 1)*spacing[i];
//     int extentMin = int(volume->GetOrigin()[i]/spacing[i]+0.5) + region.GetIndex(i);
//     int extentMax = extentMin + region.GetSize(i) - 1;
//     bounds[min] = extentMin*spacing[i];
//     bounds[max] = extentMax*spacing[i];
  }
}

//-----------------------------------------------------------------------------
EspinaVolume::RegionType ExtentToRegion(int extent[6])
{
  EspinaVolume::RegionType region;
  for(int i=0; i<3; i++)
  {
    region.SetIndex(i, extent[2*i]);
    region.SetSize (i, extent[2*i+1] - extent[2*i]+1);
  }
  return region;
}

//-----------------------------------------------------------------------------
EspinaVolume::RegionType BoundsToRegion(double bounds[6],
					EspinaVolume::SpacingType spacing)
{
  EspinaVolume::RegionType region;

  EspinaVolume::IndexType min, max;
  for (int i = 0; i < 3; i++)
  {
    min[i] = bounds[2*i]/spacing[i]+0.5;
    max[i] = bounds[2*i+1]/spacing[i]+0.5;

    region.SetIndex(i, min[i]);
    region.SetSize (i, max[i] - min[i] + 1);
  }
  return region;
}

//-----------------------------------------------------------------------------
EspinaVolume::RegionType BoundingBoxRegion(EspinaVolume::RegionType r1,
					   EspinaVolume::RegionType r2)
{
  EspinaVolume::RegionType boundingRegion;
  EspinaVolume::IndexType minIndex, maxIndex;
  for(unsigned int i = 0; i < 3; i++)
  {
    minIndex.SetElement(i, std::min(r1.GetIndex(i),r2.GetIndex(i)));
    maxIndex.SetElement(i, std::max(r1.GetIndex(i)+r1.GetSize(i) - 1,
				    r2.GetIndex(i)+r2.GetSize(i) - 1));
    boundingRegion.SetIndex(i, minIndex[i]);
    boundingRegion.SetSize (i, maxIndex[i] - minIndex[i] + 1);
  }
  return boundingRegion;
}

//-----------------------------------------------------------------------------
EspinaVolume::RegionType NormalizedRegion(EspinaVolume* volume)
{
  EspinaVolume::RegionType region = volume->GetLargestPossibleRegion();
  EspinaVolume::PointType  origin = volume->GetOrigin();

  if (origin[0] != 0 || origin[1] != 0 || origin[2] != 0)
  {
    EspinaVolume::SpacingType spacing = volume->GetSpacing();
    for (int i = 0; i < 3; i++)
      region.SetIndex(i, region.GetIndex(i)+(origin[i]/spacing[i]+0.5));
  }

  return region;
}

// //-----------------------------------------------------------------------------
// bool isExtentPixel(int x, int y, int z, int extent[6])
// {
//   return (extent[0] <= x && x <= extent[1]
//        && extent[2] <= y && y <= extent[3]
//        && extent[4] <= z && z <= extent[5]);
// }