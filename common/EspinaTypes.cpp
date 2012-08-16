#include "EspinaTypes.h"


//-----------------------------------------------------------------------------
void VolumeExtent(EspinaVolume *volume, int extent[6])
{
  EspinaVolume::SpacingType spacing = volume->GetSpacing();
  EspinaVolume::RegionType region = volume->GetBufferedRegion();
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
  EspinaVolume::RegionType region   = volume->GetBufferedRegion();
  for(int i=0; i<3; i++)
  {
    int min = 2*i, max = 2*i+1;
    int extentMin = int(volume->GetOrigin()[i]/spacing[i]+0.5) + region.GetIndex(i);
    int extentMax = extentMin + region.GetSize(i) - 1;
    bounds[min] = extentMin*spacing[i];
    bounds[max] = extentMax*spacing[i];
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
bool isExtentPixel(int x, int y, int z, int extent[6])
{
  return (extent[0] <= x && x <= extent[1]
       && extent[2] <= y && y <= extent[3]
       && extent[4] <= z && z <= extent[5]);
}
