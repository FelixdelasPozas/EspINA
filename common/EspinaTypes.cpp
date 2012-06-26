#include "EspinaTypes.h"


void VolumeExtent(EspinaVolume *volume, int extent[6])
{
  EspinaVolume::RegionType region = volume->GetBufferedRegion();
  for(int i=0; i<3; i++)
  {
    int min = 2*i, max = 2*i+1;
    extent[min] = region.GetIndex(i);
    extent[max] = extent[min] + region.GetSize(i) - 1;
  }
}

void VolumeBounds(EspinaVolume *volume, double bounds[6])
{
  EspinaVolume::SpacingType spacing = volume->GetSpacing();
  EspinaVolume::RegionType region   = volume->GetBufferedRegion();
  for(int i=0; i<3; i++)
  {
    int min = 2*i, max = 2*i+1;
    int extentMin = region.GetIndex(i);
    int extentMax = extentMin + region.GetSize(i) - 1;
    bounds[min] = extentMin*spacing[i];
    bounds[max] = extentMax*spacing[i];
  }
}