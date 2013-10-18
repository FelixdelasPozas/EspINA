#ifndef ESPINATYPES_H
#define ESPINATYPES_H

#include "EspinaCore_Export.h"
#include <itkImage.h>

#include <QList>
//#define QT_SHAREDPOINTER_TRACK_POINTERS
#include <QSet>
#include <boost/graph/graph_concepts.hpp>

namespace EspINA
{
  class Scheduler;
  using SchedulerSPtr = std::shared_ptr<Scheduler>;

  class Category;
  using CategoryPtr  = Category *;
  using CategoryList = QList<CategoryPtr>;
  using CategorySPtr = std::shared_ptr<Category>;

  class Classification;
  using ClassificationPtr  = Classification *;
  using ClassificationSPtr = std::shared_ptr<Classification>;

  class AnalysisItem;
  using AnalysisItemPtr  = AnalysisItem *;
  using AnalysisItemSPtr = std::shared_ptr<AnalysisItem>;

  class Sample;
  using SamplePtr   = Sample *;
  using SampleList  = QList<SamplePtr>;
  using SampleSPtr  = std::shared_ptr<Sample> ;
  using SampleSList = QList<SampleSPtr>;

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

  using RelationName = QString;

  class CoreFactory;
  using CoreFactorySPtr = std::shared_ptr<CoreFactory>;

  using itkVolumeType = itk::Image<unsigned char , 3>;

  using Nm = double;

  const itkVolumeType::PixelType SEG_VOXEL_VALUE = 255;
  const itkVolumeType::PixelType SEG_BG_VALUE = 0;

  enum RelationType
  {
    RELATION_IN,
    RELATION_OUT,
    RELATION_INOUT
  };

  using TimeStamp = unsigned long long;

  typedef QPair<QString, QByteArray> SnapshotEntry;
  typedef QList<SnapshotEntry>       Snapshot;
} // namespace EspINA

#endif// ESPINATYPES_H
