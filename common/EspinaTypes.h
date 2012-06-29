#ifndef ESPINATYPES_H
#define ESPINATYPES_H

#include <itkImage.h>

typedef itk::Image<unsigned char, 3> EspinaVolume;
typedef unsigned int OutputNumber;

void VolumeExtent(EspinaVolume *volume, int extent[6]);

void VolumeBounds(EspinaVolume *volume, double bounds[6]);

EspinaVolume::RegionType ExtentToRegion(int extent[6]);

bool isExtentPixel(int x, int y, int z, int extent[6]);

#endif// ESPINATYPES_H
