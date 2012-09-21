#ifndef ESPINATYPES_H
#define ESPINATYPES_H

#include <itkImage.h>
#include <QString>

typedef itk::Image<unsigned short,3> SegmentationLabelMap;
typedef itk::Image<unsigned char, 3> EspinaVolume;
typedef unsigned int OutputNumber;

typedef double Nm;

const EspinaVolume::PixelType SEG_VOXEL_VALUE = 255;

enum PlaneType
{
  AXIAL = 2,
  SAGITTAL = 0,
  CORONAL = 1
};

#endif// ESPINATYPES_H
