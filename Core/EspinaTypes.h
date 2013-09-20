#ifndef ESPINATYPES_H
#define ESPINATYPES_H

#include "EspinaCore_Export.h"
#include <itkImage.h>

#include <QList>
//#define QT_SHAREDPOINTER_TRACK_POINTERS
#include <QSet>
#include <boost/shared_ptr.hpp>

namespace EspINA
{
  class   ModelItem;
  typedef ModelItem *                  ModelItemPtr;
  typedef QList<ModelItemPtr   >       ModelItemList;
  typedef boost::shared_ptr<ModelItem> ModelItemSPtr;
  typedef QList<ModelItemSPtr>         ModelItemSList;

  class   Taxonomy;
  typedef boost::shared_ptr<Taxonomy> TaxonomySPtr;

  class   TaxonomyElement;
  typedef TaxonomyElement *                  TaxonomyElementPtr;
  typedef QList<TaxonomyElementPtr>          TaxonomyElementList;
  typedef boost::shared_ptr<TaxonomyElement> TaxonomyElementSPtr;

  class   Sample;
  typedef Sample *                  SamplePtr;
  typedef QList<SamplePtr>          SampleList;
  typedef boost::shared_ptr<Sample> SampleSPtr;
  typedef QList<SampleSPtr>         SampleSList;

  class   Channel;
  typedef Channel *                  ChannelPtr;
  typedef QList<ChannelPtr>          ChannelList;
  typedef boost::shared_ptr<Channel> ChannelSPtr;
  typedef QList<ChannelSPtr>         ChannelSList;


  class   Segmentation;
  typedef Segmentation *                  SegmentationPtr;
  typedef QList<SegmentationPtr>          SegmentationList;
  typedef QSet<SegmentationPtr>           SegmentationSet;
  typedef boost::shared_ptr<Segmentation> SegmentationSPtr;
  typedef QList<SegmentationSPtr>         SegmentationSList;


  class   Filter;
  typedef Filter *         FilterPtr;
  typedef QList<FilterPtr> FilterList;
  typedef boost::shared_ptr<Filter> FilterSPtr;
  typedef QList<FilterSPtr>      FilterSList;

  class   PickableItem;
  typedef PickableItem *                  PickableItemPtr;
  typedef boost::shared_ptr<PickableItem> PickableItemSPtr;

//   class   ModelItemExtension;
//   typedef ModelItemExtension *ModelItemExtensionPtr;
//
//   class   SampleExtension;
//   typedef SampleExtension           *SampleExtensionPtr;
//   typedef QList<SampleExtensionPtr>  SampleExtensionList;
//
//   class   ChannelExtension;
//   typedef ChannelExtension           *ChannelExtensionPtr;
//   typedef QList<ChannelExtensionPtr>  ChannelExtensionList;
//
//   class   SegmentationExtension;
//   typedef SegmentationExtension           *SegmentationExtensionPtr;
//   typedef QList<SegmentationExtensionPtr>  SegmentationExtensionList;

  class   EspinaFactory;

  class   IFilterCreator;
  typedef IFilterCreator *IFilterCreatorPtr;

  class   IFileReader;
  typedef IFileReader *IFileReaderPtr;

  class   IRenderer;

  typedef itk::Image<unsigned short, 3> SegmentationLabelMap;
  typedef itk::Image<unsigned char , 3> itkVolumeType;

  typedef double Nm;

  const itkVolumeType::PixelType SEG_VOXEL_VALUE = 255;
  const itkVolumeType::PixelType SEG_BG_VALUE = 0;

  enum ModelItemType
  {
    TAXONOMY     = 0x1,
    SAMPLE       = 0x2,
    CHANNEL      = 0x4,
    SEGMENTATION = 0x8,
    FILTER       = 0x16
  };

  enum RelationType
  {
    RELATION_IN,
    RELATION_OUT,
    RELATION_INOUT
  };

  enum PlaneType
  {
    AXIAL = 2,    //XY
    CORONAL = 1,  //ZX
    SAGITTAL = 0, //YZ
    VOLUME = 3,    //3D
    UNDEFINED = 255
  };

  typedef unsigned long long EspinaTimeStamp;

  typedef QPair<QString, QByteArray> SnapshotEntry;
  typedef QList<SnapshotEntry>       Snapshot;

  const unsigned int MAX_UNDO_SIZE = 400*400*400; // In volume pixels

  QString EspinaCore_EXPORT condition(const QString &icon, const QString &description);

} // namespace EspINA

#endif// ESPINATYPES_H
