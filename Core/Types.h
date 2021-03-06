#ifndef Types_H
#define Types_H

#include "Core/EspinaCore_Export.h"

// ITK
#include <itkImage.h>

// Qt
#include <QList>
#include <QSet>

namespace ESPINA
{
  class Scheduler;
  using SchedulerSPtr = std::shared_ptr<Scheduler>;

  class Category;
  using CategoryPtr  = Category *;
  using CategoryList = QList<CategoryPtr>;
  using CategorySPtr = std::shared_ptr<Category>;

  class Persistent;
  using PersistentPtr   = Persistent *;
  using PersistentSPtr  = std::shared_ptr<Persistent>;
  using PersistentSList = QList<PersistentSPtr>;

  class Sample;
  using SamplePtr   = Sample *;
  using SampleList  = QList<SamplePtr>;
  using SampleSPtr  = std::shared_ptr<Sample> ;
  using SampleSList = QList<SampleSPtr>;

  class ViewItem;

  class Channel;
  using ChannelPtr   = Channel *;
  using ChannelList  = QList<ChannelPtr>;
  using ChannelSPtr  = std::shared_ptr<Channel>;
  using ChannelSList = QList<ChannelSPtr>;

  class Segmentation;
  using SegmentationPtr   = Segmentation *;
  using SegmentationList  = QList<SegmentationPtr>;
  using SegmentationSet   = QSet<SegmentationPtr>;
  using SegmentationSPtr  = std::shared_ptr<Segmentation>;
  using SegmentationSList = QList<SegmentationSPtr>;

  class Filter;
  using FilterPtr   = Filter *;
  using FilterList  = QList<FilterPtr>;
  using FilterSPtr  = std::shared_ptr<Filter>;
  using FilterSList = QList<FilterSPtr>;

  class Output;
  using OutputPtr   = Output *;
  using OutputSPtr  = std::shared_ptr<Output>;
  using OutputSList = QList<OutputSPtr>;

  class CoreFactory;
  using CoreFactorySPtr = std::shared_ptr<CoreFactory>;

  using itkVolumeType = itk::Image<unsigned char , 3>;

  const itkVolumeType::PixelType SEG_VOXEL_VALUE = 255;
  const itkVolumeType::PixelType SEG_BG_VALUE = 0;

  enum RelationType
  {
    RELATION_IN,
    RELATION_OUT,
    RELATION_INOUT
  };

  using RelationName = QString;

  using TimeStamp = unsigned long long;
  using TimeRange = QList<TimeStamp>;
  const TimeStamp MAX_TIMESTAMP = std::numeric_limits<TimeStamp>::max();

  using Hue = unsigned short int;

  class RawMesh;
  using RawMeshPtr = RawMesh *;
  using RawMeshSPtr = std::shared_ptr<RawMesh>;
} // namespace ESPINA

#endif// Types_H
