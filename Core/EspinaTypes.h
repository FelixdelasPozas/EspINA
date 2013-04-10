#ifndef ESPINATYPES_H
#define ESPINATYPES_H

#include <itkImage.h>

#include <QList>
//#define QT_SHAREDPOINTER_TRACK_POINTERS
#include <QSharedPointer>
#include <QSet>

namespace EspINA
{
  class   ModelItem;
  typedef ModelItem *               ModelItemPtr;
  typedef QList<ModelItemPtr>       ModelItemList;

  class   Taxonomy;

  class   TaxonomyElement;
  typedef TaxonomyElement *               TaxonomyElementPtr;
  typedef QList<TaxonomyElementPtr>       TaxonomyElementList;

  class   Sample;
  typedef Sample *               SamplePtr;
  typedef QList<SamplePtr>       SampleList;

  class   Channel;
  typedef Channel *         ChannelPtr;
  typedef QList<ChannelPtr> ChannelList;

  class   Segmentation;
  typedef Segmentation *         SegmentationPtr;
  typedef QList<SegmentationPtr> SegmentationList;
  typedef QSet<SegmentationPtr>  SegmentationSet;

  class   Filter;
  typedef Filter *         FilterPtr;
  typedef QList<FilterPtr> FilterList;

  class   PickableItem;
  typedef PickableItem *               PickableItemPtr;

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
    TAXONOMY,
    SAMPLE,
    CHANNEL,
    SEGMENTATION,
    FILTER
  };

  enum RelationType
  {
    IN,
    OUT,
    INOUT
  };

  enum PlaneType
  {
    AXIAL = 2,    //XY
    CORONAL = 1,  //ZX
    SAGITTAL = 0, //YZ
    VOLUME = 3    //3D
  };

  typedef QPair<QString, QByteArray> SnapshotEntry;
  typedef QList<SnapshotEntry>       Snapshot;

  const unsigned int MAX_UNDO_SIZE = 400*400*400; // In volume pixels

  QString condition(const QString &icon, const QString &description);

} // namespace EspINA

#endif// ESPINATYPES_H
