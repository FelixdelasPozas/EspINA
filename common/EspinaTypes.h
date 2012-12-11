#ifndef ESPINATYPES_H
#define ESPINATYPES_H

#include <itkImage.h>

#include <QList>
#include <QString>

class Filter;
class Channel;
class Segmentation;

typedef QList<Channel *>      ChannelList;
typedef QList<Segmentation *> SegmentationList;

typedef itk::Image<unsigned short,3> SegmentationLabelMap;
typedef itk::Image<unsigned char, 3> EspinaVolume;


typedef double Nm;

const EspinaVolume::PixelType SEG_VOXEL_VALUE = 255;
const EspinaVolume::PixelType SEG_BG_VALUE = 0;

enum PlaneType
{
  AXIAL = 2,    //XY
  CORONAL = 1,  //ZX
  SAGITTAL = 0  //YZ
};

#endif// ESPINATYPES_H
