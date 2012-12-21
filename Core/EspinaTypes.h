#ifndef ESPINATYPES_H
#define ESPINATYPES_H

#include <itkImage.h>

#include <QList>
#define QT_SHAREDPOINTER_TRACK_POINTERS
#include <QSharedPointer>

namespace EspINA
{
  class   ModelItem;
  typedef ModelItem *               ModelItemPtr;
  typedef QList<ModelItemPtr>       ModelItemList;
  typedef QSharedPointer<ModelItem> SharedModelItemPtr;
  typedef QList<SharedModelItemPtr> SharedModelItemList;

  class   Taxonomy;
  typedef QSharedPointer<Taxonomy> SharedTaxonomyPtr;

  class   TaxonomyElement;
  typedef TaxonomyElement *               TaxonomyElementPtr;
  typedef QList<TaxonomyElementPtr>       TaxonomyElementList;
  typedef QSharedPointer<TaxonomyElement> SharedTaxonomyElementPtr;
  typedef QList<SharedTaxonomyElementPtr> SharedTaxonomyElementList;

  class   Sample;
  typedef Sample *               SamplePtr;
  typedef QList<SamplePtr>       SampleList;

  class   Channel;
  typedef Channel *         ChannelPtr;
  typedef QList<ChannelPtr> ChannelList;

  class   Segmentation;
  typedef Segmentation *         SegmentationPtr;
  typedef QList<SegmentationPtr> SegmentationList;

  class   Filter;
  typedef Filter *         FilterPtr;
  typedef QList<FilterPtr> FilterList;

  class   RelationshipGraph;
  typedef QSharedPointer<RelationshipGraph> RelationshipGraphPtr;

  class   PickableItem;
  typedef PickableItem *               PickableItemPtr;
  typedef QSharedPointer<PickableItem> SharedPickableItemPtr;

  class   ModelItemExtension;
  typedef QSharedPointer<ModelItemExtension> ModelItemExtensionPtr;

  class   SampleExtension;
  typedef QSharedPointer<SampleExtension> SampleExtensionPtr;
  typedef QList<SampleExtensionPtr>       SampleExtensionList;

  class   ChannelExtension;
  typedef QSharedPointer<ChannelExtension> ChannelExtensionPtr;
  typedef QList<ChannelExtensionPtr>       ChannelExtensionList;

  class   SegmentationExtension;
  typedef QSharedPointer<SegmentationExtension> SegmentationExtensionPtr;
  typedef QList<SegmentationExtensionPtr>       SegmentationExtensionList;

  class   EspinaFactory;
  typedef QSharedPointer<EspinaFactory> EspinaFactoryPtr;

  struct Relation
  {
    SharedModelItemPtr ancestor;
    SharedModelItemPtr succesor;
    QString relation;
  };
  typedef QList<Relation> RelationList;


  class   IFilterCreator;
  typedef IFilterCreator *IFilterCreatorPtr;

  class   IFileReader;
  typedef IFileReader *IFileReaderPtr;


  class   IRenderer;
  typedef QSharedPointer<IRenderer> IRendererPtr;
  typedef QList<IRendererPtr>       IRendererList;

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

} // EspINA

#endif// ESPINATYPES_H
