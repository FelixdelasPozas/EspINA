#ifndef ESPINATYPES_H
#define ESPINATYPES_H

#include <itkImage.h>

#include <QList>
#include <QString>
#include <QSharedPointer>

namespace EspINA
{
  class   ModelItem;
  typedef QSharedPointer<ModelItem> ModelItemPtr;
  typedef QList<ModelItemPtr>       ModelItemList;

  class   Taxonomy;
  typedef QSharedPointer<Taxonomy> TaxonomyPtr;

  class   TaxonomyElement;
  typedef QSharedPointer<TaxonomyElement> TaxonomyElementPtr;
  typedef QList<TaxonomyElementPtr>       TaxonomyElementList;

  class   Filter;
  typedef QSharedPointer<Filter> FilterPtr;
  typedef QList<FilterPtr>       FilterList;

  class   Sample;
  typedef QSharedPointer<Sample> SamplePtr;
  typedef QList<SamplePtr>       SampleList;

  class   Channel;
  typedef QSharedPointer<Channel> ChannelPtr;
  typedef QList<ChannelPtr>       ChannelList;

  class   Segmentation;
  typedef QSharedPointer<Segmentation> SegmentationPtr;
  typedef QList<SegmentationPtr>       SegmentationList;

  class   RelationshipGraph;
  typedef QSharedPointer<RelationshipGraph> RelationshipGraphPtr;

  class   PickableItem;
  typedef QSharedPointer<PickableItem> PickableItemPtr;

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

  class   EspinaModel;
  typedef QSharedPointer<EspinaModel> EspinaModelPtr;

  class   IFilterCreator;
  typedef IFilterCreator *IFilterCreatorPtr;

  class   IFileReader;
  typedef IFileReader *IFileReaderPtr;

  class   IRenderer;
  typedef QSharedPointer<IRenderer> IRendererPtr;
  typedef QList<IRendererPtr>       IRendererList;

  class   ISettingsPanel;
  typedef QSharedPointer<ISettingsPanel> ISettingsPanelPtr;
  typedef QList<ISettingsPanelPtr>       ISettingsPanelList;

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
