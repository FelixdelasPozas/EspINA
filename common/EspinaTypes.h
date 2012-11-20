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

typedef int OutputNumber;
typedef QList<OutputNumber> OutputNumberList;

//TODO 2012-11-20 Crear clase y mover (seguramente a Filter)
struct FilterOutput
{
  static const int INVALID_OUTPUT_NUMBER = -1;

  bool                  isCached;
  bool                  isEdited;
  Filter               *filter;
  OutputNumber          number;
  EspinaVolume::Pointer volume;

  bool isValid() {return NULL == filter || INVALID_OUTPUT_NUMBER == number || NULL == volume.GetPointer(); }

  explicit FilterOutput(Filter *f=NULL, OutputNumber n=INVALID_OUTPUT_NUMBER, EspinaVolume::Pointer v =EspinaVolume::Pointer())
  : isCached(false), isEdited(false), filter(f), number(n), volume(v)
  {}
};
typedef QList<FilterOutput> OutputList;

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
