#ifndef ESPINATYPES_H
#define ESPINATYPES_H

#include <itkImage.h>

const unsigned int SEG_VOXEL_VALUE = 255;
typedef itk::Image<unsigned short,3> SegmentationLabelMap;
typedef itk::Image<unsigned char, 3> EspinaVolume;
typedef unsigned int OutputNumber;

typedef double Nm;

enum PlaneType
{
  AXIAL = 2,
  SAGITTAL = 0,
  CORONAL = 1
};

void VolumeExtent(EspinaVolume *volume, int extent[6]);

void VolumeBounds(EspinaVolume *volume, double bounds[6]);

EspinaVolume::RegionType ExtentToRegion(int extent[6]);

bool isExtentPixel(int x, int y, int z, int extent[6]);

#endif// ESPINATYPES_H
