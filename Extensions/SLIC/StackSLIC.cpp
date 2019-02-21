/*
 * Copyright (C) 2018, Álvaro Muñoz Fernández <golot@golot.es>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

//ESPINA
#include <Extensions/SLIC/StackSLIC.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Factory/CoreFactory.h>
#include <Core/Utils/Spatial.h>
#include <Core/Utils/BlockTimer.hxx>
#include <Core/Utils/EspinaException.h>
#include <Core/Types.h>
#include <Extensions/EdgeDistances/ChannelEdges.h>

//ITK
#include <itkImage.h>
#include <itkEuclideanDistanceMetric.h>
#include <itkImageConstIteratorWithIndex.h>
#include <itkImageRegion.h>
#include <itkGradientMagnitudeImageFilter.h>
#include <itkNeighborhoodIterator.h>

// VTK
#include <vtkUnsignedCharArray.h>
#include <vtkImageData.h>
#include <vtkPointData.h>

// C++
#include <limits>
#include <cmath>
#include <memory>
#include <functional>
#include <queue>
#include <utility>

// Qt
#include <QDataStream>
#include <QtCore>

//Intrinsics
#include <xmmintrin.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Extensions;

const StackExtension::Type StackSLIC::TYPE = "StackSLIC";

const QString StackSLIC::VOXELS_FILE = "voxels_%1.slic";
const QString StackSLIC::LABELS_FILE = "labels.slic";
const QString StackSLIC::DATA_FILE   = "data.slic";

const QStringList UNITS{ "bytes", "KB", "MB", "GB", "TB"};

//-----------------------------------------------------------------------------
StackSLIC::StackSLIC(SchedulerSPtr scheduler, CoreFactory* factory, const InfoCache &cache)
: StackExtension{cache}
, m_scheduler   {scheduler}
, m_factory     {factory}
, m_task        {nullptr}
{
}

//-----------------------------------------------------------------------------
StackSLIC::~StackSLIC()
{
  if(m_task != nullptr)
  {
      if(m_task->isRunning()) m_task->abort();
      m_task->thread()->wait(-1);
      m_task = nullptr;
  }
}

//-----------------------------------------------------------------------------
bool StackSLIC::loadFromSnapshot()
{
  if(m_result.computed) return true;

  Snapshot snapshot;

  auto dataName = snapshotName(DATA_FILE);
  auto labelsName = snapshotName(LABELS_FILE);
  QFileInfo dataFileInfo = m_extendedItem->storage()->absoluteFilePath(dataName);
  QFileInfo labelsFileInfo = m_extendedItem->storage()->absoluteFilePath(labelsName);

  if(!dataFileInfo.exists() || !labelsFileInfo.exists()) return false;

  QFile labelsFile(labelsFileInfo.absoluteFilePath());
  QFile dataFile(dataFileInfo.absoluteFilePath());

  if(!labelsFile.open(QIODevice::ReadOnly) || !dataFile.open(QIODevice::ReadOnly))  return false;

  QWriteLocker lock(&m_result.dataMutex);

  QByteArray labelBuffer = labelsFile.readAll();
  auto labelData = qUncompress(labelBuffer);
  labelBuffer.clear();

  QDataStream labelStream(&labelData, QIODevice::ReadOnly);
  labelStream.setVersion(QDataStream::Qt_4_0);
  m_result.supervoxels.clear();
  labelStream >> m_result.supervoxels;

  QByteArray data = dataFile.readAll();
  auto opData = qUncompress(data);
  data.clear();

  QDataStream stream(&opData, QIODevice::ReadOnly);
  stream.setVersion(QDataStream::Qt_4_0);

  long long x,y,z;
  stream >> x >> y >> z;
  m_result.region.SetIndex(itkVolumeType::IndexType{x,y,z});
  long long sx,sy,sz;
  stream >> sx >> sy >> sz;
  m_result.region.SetSize(itkVolumeType::SizeType{static_cast<long unsigned>(sx),static_cast<long unsigned>(sy),static_cast<long unsigned>(sz)});

  int variant;
  stream >> variant;
  m_result.variant = static_cast<StackSLIC::SLICVariant>(variant);
  stream >> m_result.m_s;
  stream >> m_result.m_c;
  stream >> m_result.iterations;
  stream >> m_result.tolerance;

  m_result.modified = false;
  m_result.computed = true;

  return true;
}

//-----------------------------------------------------------------------------
void StackSLIC::onComputeSLIC(unsigned char m_s, unsigned char m_c, Extensions::StackSLIC::SLICVariant variant, unsigned int max_iterations, double tolerance)
{
  if(m_task && m_task->isRunning()) return;

  m_result.computed     = false;
  m_result.m_s          = m_s;
  m_result.m_c          = m_c;
  m_result.variant      = variant;
  m_result.iterations   = max_iterations;
  m_result.tolerance    = tolerance;
  m_result.modified     = false;

  m_task = std::make_shared<SLICComputeTask>(m_extendedItem, m_scheduler, m_factory, m_result);

  connect(m_task.get(), SIGNAL(finished()),    this, SLOT(onSLICComputed()));
  connect(m_task.get(), SIGNAL(aborted()),     this, SLOT(onSLICComputed()));
  connect(m_task.get(), SIGNAL(progress(int)), this, SIGNAL(progress(int)));

  Task::submit(m_task);
}

//-----------------------------------------------------------------------------
void StackSLIC::onSLICComputed()
{
  if(m_task != nullptr)
  {
    disconnect(m_task.get(), SIGNAL(finished()),    this, SLOT(onSLICComputed()));
    disconnect(m_task.get(), SIGNAL(aborted()),     this, SLOT(onSLICComputed()));
    disconnect(m_task.get(), SIGNAL(progress(int)), this, SIGNAL(progress(int)));

    if(!m_task->isAborted())
    {
      emit computeFinished();
    }
    else
    {
      emit computeAborted();
    }

    m_task = nullptr;
  }
}

//-----------------------------------------------------------------------------
void StackSLIC::onAbortSLIC()
{
  if(m_task != nullptr)
  {
    disconnect(m_task.get(), SIGNAL(finished()),    this, SLOT(onSLICComputed()));
    disconnect(m_task.get(), SIGNAL(aborted()),     this, SLOT(onSLICComputed()));
    disconnect(m_task.get(), SIGNAL(progress(int)), this, SIGNAL(progress(int)));

    if(m_task->isRunning()) m_task->abort();
    m_task->thread()->wait(-1);
    m_task = nullptr;

    emit computeAborted();
  }

  // load old results if present
  m_result.computed = false;
  loadFromSnapshot();
}

//-----------------------------------------------------------------------------
StackSLIC::SLICComputeTask::SLICComputeTask(ChannelPtr stack, SchedulerSPtr scheduler, CoreFactory *factory, SLICResult &result)
: Task      {scheduler}
, m_stack   {stack}
, m_factory {factory}
, result    (result)
, voxels    {nullptr}
{
  QString description = "Computing ";
  switch(result.variant)
  {
    case SLICVariant::ASLIC:
      description += "ASLIC";
      break;
    case SLICVariant::SLIC:
      description += "SLIC";
      break;
    case SLICVariant::SLICO:
      description += "SLICO";
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  description += QString(" of ") + m_stack->name();
  setDescription(description);
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::run()
{
  BlockTimer<std::chrono::milliseconds> timer{"SLIC"};
  reportProgress(0);
  m_errorMessage.clear();

  //Square tolerance to avoid having to calculate roots later
  const auto TOLERANCE = std::pow(result.tolerance, 2);

  //Initialize channel edges extension
  auto edgesExtension = retrieveOrCreateStackExtension<ChannelEdges>(m_stack, m_factory);

  //Get ITK image from extended item
  OutputSPtr output = m_stack->output();
  auto inputVolume = readLockVolume(output);
  if (!inputVolume->isValid())
  {
    auto what    = QObject::tr("Invalid input volume");
    auto details = QObject::tr("StackSLIC::onComputeSLIC() ->Invalid input volume.");

    throw Utils::EspinaException(what, details);
  }

  auto image = inputVolume->itkImage();

  result.region = image->GetLargestPossibleRegion();

  result.converged = false;

  //Find centers for the supervoxels
  QList<Label> labels;
  if(!initLabels(image, labels, edgesExtension.get())) return;

  qDebug() << QString("Created %1 labels").arg(labels.size());

  // try to do it in memory, notify if there isn't enough memory.
  double totalSize = result.region.GetNumberOfPixels() * sizeof(unsigned int);

  try
  {
    voxels = std::unique_ptr<unsigned int[]>(new unsigned int[result.region.GetNumberOfPixels()]);
    std::memset(voxels.get(), std::numeric_limits<unsigned int>::max(), totalSize);
  }
  catch(...)
  {
    int unit = 0;
    while(totalSize > 1024)
    {
      ++unit;
      totalSize /= 1024.;
    }

    m_errorMessage = tr("Not enough memory. Processing the stack '%1' requires a total of %2 %3 of memory.").arg(m_stack->name()).arg(QString::number(totalSize, 'f', 1)).arg(UNITS.at(unit));
    abort();
    return;
  }

  try
  {
    for(unsigned int iteration = 0; iteration < result.iterations && !result.converged; ++iteration)
    {
      // asumme all converge if we are going to check for convergence. Set to false when recomputing supervoxel centers.
      if(TOLERANCE > 0) result.converged = true;

      if(!canExecute()) break;

      qDebug() << QString("Starting iteration: %1 - %2s").arg(iteration+1).arg(timer.elapsed()/1000);

      int newProgress = (100 * iteration) / result.iterations;
      if(newProgress != progress())
      {
        reportProgress(newProgress);
      }

      qDebug() << "Computing labels.";
      watcher.setFuture(QtConcurrent::map(labels, std::bind(&SLICComputeTask::computeLabel, this, std::placeholders::_1, edgesExtension, image, labels)));
      watcher.waitForFinished();

      if(!canExecute()) break;

      qDebug() << "Recalculating centers.";
      watcher.setFuture(QtConcurrent::map(labels, std::bind(&SLICComputeTask::recalculateCenter, this, std::placeholders::_1, image, TOLERANCE)));
      watcher.waitForFinished();

      if(!canExecute()) break;

      qDebug() << QString("Finishing iteration: %1 - %2").arg(iteration+1).arg(timer.elapsed()/1000);
    } //iteration
  }
  catch(const EspinaException &e)
  {
    std::cerr << "Exception " << e.what() << std::endl;
    std::cerr << "Details " << e.details() << std::endl;
    exit(1);
  }
  catch(const std::exception &e)
  {
    std::cerr << "std::exception " << e.what() << std::endl << std::flush;
    exit(1);
  }

  if(canExecute())
  {
    // add supervoxels as valid. Invalid labels (empty) will be marked as such when computing connectivity.
    std::for_each(labels.constBegin(), labels.constEnd(), [this](const Label &label) { result.supervoxels.append({label.center, label.color, label.valid});});

    saveResults(labels);

    result.computed = true;
    result.modified = true;
  }

  voxels = nullptr;
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::onAbort()
{
  watcher.cancel();
}

//-----------------------------------------------------------------------------
const unsigned long int StackSLIC::getSupervoxel(const itkVolumeType::IndexType position) const
{
  if(!m_result.computed || !m_result.region.IsInside(position)) return 0;

  auto slice = getUncompressedSlice(position[2]).get();
  auto pos = position[0]-m_result.region.GetIndex(0) + (position[1]-m_result.region.GetIndex(1))*m_result.region.GetSize(0);
  return slice[pos];
}

//-----------------------------------------------------------------------------
const unsigned char StackSLIC::getSupervoxelColor(const unsigned int supervoxel) const
{
  if(!m_result.computed) return 0;

  return m_result.supervoxels[supervoxel].color;
}

//-----------------------------------------------------------------------------
const itkVolumeType::IndexType StackSLIC::getSupervoxelCenter(const unsigned int supervoxel) const
{
  if(!m_result.computed) return {0,0,0};

  return m_result.supervoxels[supervoxel].center;
}

//-----------------------------------------------------------------------------
bool StackSLIC::drawVoxelCenters(const unsigned int slice, vtkSmartPointer<vtkPoints> data)
{
  if(isRunning()) return false;

  QReadLocker lock(&m_result.dataMutex);

  if(!m_result.computed) return false;

  data->Reset();
  auto spacing = m_extendedItem->output()->spacing();

  auto testAndAddOp = [this, &spacing, &data, slice](const SuperVoxel &superVoxel)
  {
    if(superVoxel.valid && superVoxel.center.GetElement(2) == slice)
    {
      data->InsertNextPoint(superVoxel.center.GetElement(0) * spacing[0],
                            superVoxel.center.GetElement(1) * spacing[1],
                            superVoxel.center.GetElement(2) * spacing[2]);
    }
  };
  std::for_each(m_result.supervoxels.constBegin(), m_result.supervoxels.constEnd(), testAndAddOp);
  data->Modified();

  return true;
}

//-----------------------------------------------------------------------------
bool StackSLIC::drawSliceInImageData(const unsigned int slice, vtkSmartPointer<vtkImageData> data)
{
  if(isRunning()) return false;

  QReadLocker lock(&m_result.dataMutex);

  if(!m_result.computed) return false;

  itkVolumeType::IndexType index{m_result.region.GetIndex(0), m_result.region.GetIndex(1), slice};
  if(!m_result.region.IsInside(index)) return false;

  OutputSPtr output = m_extendedItem->output();
  NmVector3 spacing = output->spacing();
  const long int max_x = m_result.region.GetSize(0);
  const long int max_y = m_result.region.GetSize(1);
  unsigned long long int pixel_count = max_x*max_y;

  data->SetExtent(m_result.region.GetIndex(0), m_result.region.GetIndex(0) + max_x-1, m_result.region.GetIndex(1), m_result.region.GetIndex(1) + max_y-1, slice, slice);
  data->SetSpacing(spacing[0], spacing[1], spacing[2]);
  data->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  auto buffer = reinterpret_cast<unsigned char *>(data->GetScalarPointer());

  std::memcpy(buffer, getUncompressedSlice(slice).get(), pixel_count);

  return true;
}

//-----------------------------------------------------------------------------
itk::SmartPointer<itk::Image<unsigned int, 3>> StackSLIC::getLabeledImageFromBounds(const Bounds bounds) const
{
  //Partial results shouldn't ever be useful outside of the preview
  //that is handled already in drawSliceInImageData()
  //Only returns something if SLIC has been computed
  if(isRunning())
  {
    auto message = QObject::tr("SLIC results called during computation");
    auto details = QObject::tr("StackSLIC::getLabeledImageFromBounds() -> Results are not available while SLIC computation is running.");

    throw Utils::EspinaException(message, details);
  }

  if(!bounds.areValid() || !contains(m_extendedItem->output()->bounds(), bounds))
  {
    auto message = QObject::tr("invalid bounds");
    auto details = QObject::tr("StackSLIC::getLabeledImageFromBounds() -> Requested area is outside the image bounds.");

    throw Utils::EspinaException(message, details);
  }

  using ImageType = itk::Image<unsigned int, 3>;

  auto spacing = m_extendedItem->output()->spacing();
  auto image = create_itkImage<ImageType>(bounds, SEG_BG_VALUE, spacing);
  auto data_offset = image->GetBufferPointer();
  auto region = image->GetLargestPossibleRegion();

  QReadLocker lock(&m_result.dataMutex);
  for (int z = region.GetIndex(2); z < region.GetIndex(2) + static_cast<long int>(region.GetSize(2)); ++z)
  {
    auto s = getUncompressedLabeledSlice(z);
    for (int y = region.GetIndex(1); y < region.GetIndex(1) + static_cast<long int>(region.GetSize(1)); ++y)
    {
      auto slice_buffer = s.get();
      slice_buffer += y * region.GetSize(0);
      for (int x = region.GetIndex(0); x < region.GetIndex(0) + static_cast<long int>(region.GetSize(0)); ++x)
      {
        *(data_offset++) = *(slice_buffer++);
      }
    }
  }

  image->Modified();
  return image;
}

//-----------------------------------------------------------------------------
const bool StackSLIC::isComputed() const
{
  return m_result.computed;
}

//-----------------------------------------------------------------------------
const bool StackSLIC::isRunning() const
{
  return m_task && m_task->isRunning();
}

//-----------------------------------------------------------------------------
StackSLIC::SLICVariant StackSLIC::getVariant()
{
  return m_result.variant;
}

//-----------------------------------------------------------------------------
unsigned char StackSLIC::getSupervoxelSize()
{
  return m_result.m_s;
}

//-----------------------------------------------------------------------------
unsigned char StackSLIC::getColorWeight()
{
  return m_result.m_c;
}

//-----------------------------------------------------------------------------
unsigned int StackSLIC::getIterations()
{
  return m_result.iterations;
}

//-----------------------------------------------------------------------------
double StackSLIC::getTolerance()
{
  return m_result.tolerance;
}

//-----------------------------------------------------------------------------
unsigned int StackSLIC::getSupervoxelCount()
{
  auto isValid = [](const SuperVoxel &voxel) { return voxel.valid; };
  return std::count_if(m_result.supervoxels.constBegin(), m_result.supervoxels.constEnd(), isValid);;
}

//-----------------------------------------------------------------------------
double StackSLIC::getSliceSpacing()
{
  return m_extendedItem ? m_extendedItem->output()->spacing()[2] : -1;
}

//-----------------------------------------------------------------------------
QDataStream &operator>>(QDataStream &in, StackSLIC::SuperVoxel &label)
{
  long long int center[3];
  in >> center[0] >> center[1] >> center[2];
  label.center[0] = static_cast<long int>(center[0]);
  label.center[1] = static_cast<long int>(center[1]);
  label.center[2] = static_cast<long int>(center[2]);
  in >> label.color;
  return in;
}

//-----------------------------------------------------------------------------
QDataStream &operator<<(QDataStream &out, const StackSLIC::SuperVoxel &label)
{
  long long int center[3]{label.center[0], label.center[1], label.center[2]};
  out << center[0] << center[1] << center[2];
  out << label.color;
  return out;
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::saveResults(QList<Label> labels)
{
  QWriteLocker lock(&result.dataMutex);

  for (auto z = result.region.GetIndex(2); z < result.region.GetIndex(2) + static_cast<long int>(result.region.GetSize(2)); ++z)
  {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_0);

    compressSlice(stream, z);

    auto fileName =  m_factory->defaultStorage()->absoluteFilePath(QString(VOXELS_FILE).arg(z));

    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Unbuffered))
    {
      qDebug() << "failed to create" << fileName;
      continue;
    }

    auto compressedData = qCompress(data, 9);
    data.clear();

    if(compressedData.size() != file.write(compressedData))
    {
      qDebug() << "failed to save data of" << fileName;
      continue;
    }

    file.flush();
    file.close();
  }
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::compressSlice(QDataStream &stream, const long int z)
{
  //Compression algorithm goes here
  unsigned long long voxel_index = 0;
  unsigned int current_label;
  unsigned char same_label_count = 0;

  voxel_index = offsetOfIndex(IndexType{result.region.GetIndex(0), result.region.GetIndex(1), z});

  for(auto y = result.region.GetIndex(1); y < result.region.GetIndex(1) + static_cast<long int>(result.region.GetSize(1)); ++y)
  {
    for(auto x = result.region.GetIndex(0); x < result.region.GetIndex(0) + static_cast<long int>(result.region.GetSize(0)); ++x)
    {
      auto voxelValue = voxels[voxel_index];

      if (x != result.region.GetIndex(0))
      {
        if (voxelValue != current_label)
        {
          //Write RLE
          stream << current_label;
          stream << same_label_count;
          current_label = voxelValue;
          same_label_count = 1;
        }
        else
        {
          if(same_label_count == std::numeric_limits<unsigned char>::max())
          {
            stream << current_label;
            stream << same_label_count;

            same_label_count = 0;
          }

          ++same_label_count;
        }
      }
      else
      {
        current_label = voxelValue;
        same_label_count = 1;
      }
      ++voxel_index;
    }
    //Write last supervoxel in row
    stream << current_label;
    stream << same_label_count;
  }
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::findCandidateRegion(itkVolumeType::IndexType &center, double scan_size, ImageRegion &region) const
{
  union vec4f
  {
    __m128 vec;
    float f[4];
  };

  region.SetSize(0, scan_size);
  region.SetSize(1, scan_size);
  region.SetSize(2, scan_size);

  //Vector divide by 2
  vec4f srca, srcb;
  srca.vec = _mm_set_ps1(scan_size);
  srcb.vec = _mm_set_ps1(2);
  srcb.vec = _mm_div_ps(srca.vec, srcb.vec);

  //Substract center - previous result
  srca.f[0] = center[0];
  srca.f[1] = center[1];
  srca.f[2] = center[2];
  srcb.vec = _mm_sub_ps(srca.vec, srcb.vec);
  _mm_store_ps(srcb.f, srcb.vec);

  region.SetIndex(0, srcb.f[0]);
  region.SetIndex(1, srcb.f[1]);
  region.SetIndex(2, srcb.f[2]);

  region.Crop(result.region);
}

//-----------------------------------------------------------------------------
bool StackSLIC::SLICComputeTask::initLabels(itkVolumeType *image, QList<Label> &labels, ChannelEdges *edgesExtension)
{
  using GradientType = itk::Image<float,3>;

  ImageRegion region;
  itkVolumeType::IndexType cur_index;

  for (auto z = (result.m_s / 2) + result.region.GetIndex(2); z < result.region.GetIndex(2) + static_cast<long int>(result.region.GetSize(2)); z += result.m_s)
  {
    if(!canExecute()) return false;

    auto sliceRegion = edgesExtension->sliceRegion(static_cast<unsigned int>(z));

    for(auto y = (result.m_s / 2) + result.region.GetIndex(1); y < result.region.GetIndex(1)+static_cast<long int>(result.region.GetSize(1)); y += result.m_s)
    {
      for(auto x = (result.m_s / 2) + result.region.GetIndex(0); x < result.region.GetIndex(0)+static_cast<long int>(result.region.GetSize(0)); x += result.m_s)
      {
        if(!canExecute()) return false;

        cur_index[0] = x;
        cur_index[1] = y;
        cur_index[2] = z;

        //Skip if out of the region of interest or image bounds
        if(!result.region.IsInside(cur_index)) continue;

        //Don't create supervoxel centers outside of the calculated edges
        if(!sliceRegion.IsInside(cur_index)) continue;

        //Check lowest gradient voxel in a 3x3x3 area around voxel
        region.SetIndex(0, x-2);
        region.SetIndex(1, y-2);
        region.SetIndex(2, z-2);
        region.SetSize(0, 5);
        region.SetSize(1, 5);
        region.SetSize(2, 5);
        region.Crop(result.region);

        //Calculate gradient in area
        auto gradientFilter = itk::GradientMagnitudeImageFilter<itkVolumeType, GradientType>::New();
        gradientFilter->SetNumberOfThreads(1);
        gradientFilter->SetUseImageSpacingOff();
        gradientFilter->SetInput(image);
        gradientFilter->GetOutput()->SetRequestedRegion(region);
        gradientFilter->Update();

        //Loop gradient image and find lowest gradient
        region.SetIndex(0,x-1);
        region.SetIndex(1,y-1);
        region.SetIndex(2,z-1);
        region.SetSize(0, 3);
        region.SetSize(1, 3);
        region.SetSize(2, 3);
        region.Crop(result.region);

        itk::ImageRegionConstIteratorWithIndex<GradientType> it(gradientFilter->GetOutput(), region);
        it.GoToBegin();
        auto lowestGradient = std::numeric_limits<float>::max();
        while(!it.IsAtEnd())
        {
          auto value = it.Get();
          if(value < lowestGradient)
          {
            cur_index = it.GetIndex();
            lowestGradient = value;
          }
          ++it;
        }

        labels.append(Label{std::pow(result.m_c,2) / std::pow(result.m_s,2), 1.0, static_cast<unsigned int>(labels.size()), cur_index, image->GetPixel(cur_index), 1 , true});
      }
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
float StackSLIC::SLICComputeTask::calculateDistance(const IndexType &voxel_index, const IndexType &center_index,
                                                   const unsigned char voxel_color, const unsigned char center_color,
                                                   const float norm_quotient, float *color_distance,
                                                   float *spatial_distance, bool only_spatial)
{
  union vec4f
  {
    __m128 vec;
    float f[4];
  };

  //Vector subtraction of cur_index - label->center
  vec4f srca, srcb;
  srca.f[1] = voxel_index[0];
  srca.f[2] = voxel_index[1];
  srca.f[3] = voxel_index[2];
  srca.f[0] = voxel_color;
  srcb.f[1] = center_index[0];
  srcb.f[2] = center_index[1];
  srcb.f[3] = center_index[2];
  srcb.f[0] = center_color;
  srca.vec = _mm_load_ps(srca.f);
  srcb.vec = _mm_load_ps(srcb.f);
  srca.vec = _mm_sub_ps(srca.vec, srcb.vec);

  //Vector multiply of previous result * axis spacing
  srcb.f[1] = 1;
  srcb.f[2] = 1;
  srcb.f[3] = 1;
  srcb.f[0] = color_normalization_constant;
  srcb.vec = _mm_load_ps(srcb.f);
  srca.vec = _mm_mul_ps(srca.vec, srcb.vec);

  //Calculate powers of 2 of results
  srcb.vec = _mm_mul_ps(srca.vec, srca.vec);
  _mm_store_ps(srcb.f, srcb.vec);

  double spatial = srcb.f[1] + srcb.f[2] + srcb.f[3];
  if(only_spatial)
    //Return spatial distance squared
    return spatial;
  //Return normalized distance squared
  if(color_distance != nullptr)
    *color_distance = srcb.f[0];
  if(spatial_distance != nullptr)
    *spatial_distance = spatial;

  return srcb.f[0] + norm_quotient * spatial;
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::computeLabel(Label &label, ChannelEdgesSPtr edgesExtension, itkVolumeType *image, QList<Label> &labels)
{
  ImageRegion subRegion;
  ImageRegion region;
  unsigned char voxel_color;
  long int current_slice = 0;
  unsigned long long voxel_index;
  unsigned int label_old_index;
  float spatial_distance, color_distance, distance, distance_old;
  IndexType cur_index;

  //Find the region of voxels that are in range of this supervoxel
  findCandidateRegion(label.center, 2. * result.m_s, subRegion);

  const auto center_color = label.color;

  //Iterate over the slices
  for(current_slice = subRegion.GetIndex(2); current_slice < subRegion.GetIndex(2) + static_cast<long int>(subRegion.GetSize(2)); ++current_slice)
  {
    //Get slice edges
    auto sliceRegion = edgesExtension->sliceRegion(current_slice);
    //Set region bounds around center for current slice
    region.SetIndex(0, subRegion.GetIndex(0));
    region.SetIndex(1, subRegion.GetIndex(1));
    region.SetIndex(2, current_slice);
    region.SetSize(0, subRegion.GetSize(0));
    region.SetSize(1, subRegion.GetSize(1));
    region.SetSize(2, 1);

    //If the region was completely outside the edges, skip to next slice
    //This mean either the stack was displaced too much or the z index
    //was too high/low
    if(!region.Crop(sliceRegion)) continue;

    RegionIterator it(image, region);
    it.GoToBegin();

    while(!it.IsAtEnd())
    {
      cur_index   = it.GetIndex();
      voxel_color = it.Get();
      voxel_index = offsetOfIndex(cur_index);
      auto voxelLabel = voxels[voxel_index];

      label_old_index = voxelLabel;
      if(label_old_index == std::numeric_limits<unsigned int>::max() || label_old_index == label.index)
      {
        //If voxel is unassigned or assigned to the current label, assume it's an infinite distance away
        distance_old = std::numeric_limits<double>::max();
      }
      else
      {
        Q_ASSERT(static_cast<int>(label_old_index) < labels.size());
        const auto &label_old = labels.at(label_old_index);
        distance_old = calculateDistance(cur_index, label_old.center, voxel_color, image->GetPixel(label_old.center), label_old.norm_quotient, nullptr, nullptr);
      }

      distance = calculateDistance(cur_index, label.center, voxel_color, center_color, label.norm_quotient, &color_distance, &spatial_distance);

      if(distance < distance_old)
      {
        QMutexLocker locker(&labelListMutex);
        if(label_old_index != voxelLabel)
        {
          Q_ASSERT(static_cast<int>(voxelLabel) < labels.size());
          const auto &label_old = labels.at(voxelLabel);
          distance_old = calculateDistance(cur_index, label_old.center, voxel_color, image->GetPixel(label_old.center), label_old.norm_quotient, nullptr, nullptr);
        }

        if(distance < distance_old)
        {
          voxels[voxel_index] = label.index;

          //Update maximum distances
          switch(result.variant)
          {
            case SLICVariant::ASLIC:
              if(label.m_c < color_distance) label.m_c = color_distance;
            //no break
            case SLICVariant::SLICO:
            case SLICVariant::SLIC:
              if(label.m_s < spatial_distance) label.m_s = spatial_distance;
              break;
            default:
              Q_ASSERT(false);
              break;
          }
        }
      }

      ++it;
    } //RegionIterator
  }//current_slice

  QMutexLocker locker(&labelListMutex);
  switch(result.variant)
  {
    case SLICVariant::ASLIC:
      //Update max. distances calculating their roots as we were
      //using squared distances for the simplified equations
      label.m_c = std::sqrt(label.m_c);
    //no break
    case SLICVariant::SLICO:
    case SLICVariant::SLIC:
      label.m_s = std::sqrt(label.m_s);
      break;
    default:
      Q_ASSERT(false);
      break;
  }
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::recalculateCenter(Label& label, itkVolumeType *image, const double tolerance)
{
  //Recalculate centers, check convergence
  const auto SCAN_SIZE = 2. * result.m_s;

  ImageRegion subRegion;
  findCandidateRegion(label.center, SCAN_SIZE, subRegion);

  unsigned long long sum_color = 0;
  unsigned long long sum_voxels = 0;
  long long int sum_x = 0;
  long long int sum_y = 0;
  long long int sum_z = 0;

  auto it = itk::ImageRegionConstIterator<itkVolumeType>(image, subRegion);
  it.GoToBegin();

  while(!it.IsAtEnd())
  {
    const auto index = it.GetIndex();
    const auto voxel_index = offsetOfIndex(index);
    const auto voxelLabel = voxels[voxel_index];

    Q_ASSERT(voxel_index < result.region.GetNumberOfPixels());

    if (label.index == voxelLabel)
    {
      ++sum_voxels;
      sum_x += index.GetElement(0); sum_y += index.GetElement(1); sum_z += index.GetElement(2);
      sum_color += image->GetPixel(index);
    }

    ++it;
  }

  if(sum_voxels > 0)
  {
    //Calculate averaged coordinates
    sum_x = std::round(sum_x/sum_voxels);
    sum_y = std::round(sum_y/sum_voxels);
    sum_z = std::round(sum_z/sum_voxels);

    //Calculate displacement for the tolerance test
    {
      QWriteLocker lock(&result.dataMutex);
      if(tolerance > 0 && result.converged)
      {
        auto cur_index = IndexType{sum_x, sum_y, sum_z};
        auto spatial_distance = calculateDistance(label.center, cur_index, 0, 0, 0, nullptr, nullptr, true);

        if(spatial_distance > tolerance) result.converged = false;
      }
    }

    QMutexLocker locker(&labelListMutex);
    label.center = {sum_x, sum_y, sum_z};
    label.color = sum_color/sum_voxels;
  }

  {
    QMutexLocker locker(&labelListMutex);
    label.valid = (sum_voxels != 0);
  }

  //Update weights with maximum observed results and reset them
  switch(result.variant)
  {
    case SLICVariant::ASLIC:
      {
        QMutexLocker locker(&labelListMutex);
        //Squared constants to prevent having to use expensive roots
        label.norm_quotient = std::pow(label.m_c,2)/std::pow(label.m_s,2);
        label.m_c = 1;
        label.m_s = 1.0;
      }
      break;
    case SLICVariant::SLICO:
      {
        QMutexLocker locker(&labelListMutex);
        label.norm_quotient = std::pow(result.m_c,2)/std::pow(label.m_s,2);
        label.m_s = 1.0;
      }
      break;
    case SLICVariant::SLIC:
      break;
    default:
      Q_ASSERT(false);
      break;
  }
}

//-----------------------------------------------------------------------------
std::unique_ptr<char[]> StackSLIC::getUncompressedSlice(const int slice) const
{
  auto data = getSlice(slice);
  auto RLEdata = qUncompress(data);
  data.clear();

  QDataStream stream(&RLEdata, QIODevice::ReadOnly);
  stream.setVersion(QDataStream::Qt_4_0);

  unsigned long long int pixel_count = m_result.region.GetSize(0)*m_result.region.GetSize(1);
  unsigned long long int pixel = 0;

  std::unique_ptr<char[]> buffer(new char[pixel_count]);
  auto buffer_ptr = buffer.get();

  unsigned int label;
  unsigned char voxel_count;

  while(pixel < pixel_count)
  {
    stream >> label;
    stream >> voxel_count;

    for(unsigned int i = 0; i < voxel_count; i++)
    {
      *buffer_ptr++ = label == std::numeric_limits<unsigned int>::max() ? 0 : m_result.supervoxels[label].color;
    }
    pixel += voxel_count;
  }

  return buffer;
}

//-----------------------------------------------------------------------------
std::unique_ptr<long long[]> StackSLIC::getUncompressedLabeledSlice(const int slice) const
{
  auto data = getSlice(slice);
  auto RLEdata = qUncompress(data);
  data.clear();

  QDataStream stream(&RLEdata, QIODevice::ReadOnly);
  stream.setVersion(QDataStream::Qt_4_0);

  unsigned long long int pixel_count = m_result.region.GetSize(0)*m_result.region.GetSize(1);
  unsigned long long int pixel = 0;

  std::unique_ptr<long long[]> buffer(new long long[pixel_count]);
  auto buffer_ptr = buffer.get();

  unsigned int label;
  unsigned char voxel_count;

  while(pixel < pixel_count)
  {
    stream >> label;
    stream >> voxel_count;
    for(int i = 0; i < voxel_count; i++)
    {
      *buffer_ptr++ = label;
    }
    pixel += voxel_count;
  }

  return buffer;
}

//-----------------------------------------------------------------------------
const QByteArray StackSLIC::getSlice(const int slice) const
{
  if(!m_result.computed)
  {
    auto message = QObject::tr("SLIC not computed");
    auto details = QObject::tr("SLICResult::getSlice() -> ") + message;

    throw EspinaException(message, details);
  }

  QReadLocker lock(&m_result.dataMutex);

  QFileInfo filePath;
  if(m_result.modified)
  {
    filePath = m_factory->defaultStorage()->absoluteFilePath(QString(VOXELS_FILE).arg(slice));
  }
  else
  {
    filePath = m_extendedItem->storage()->absoluteFilePath(snapshotName(VOXELS_FILE).arg(slice));
  }

  if (!filePath.exists())
  {
    qDebug() << filePath.absoluteFilePath();
    auto message = QObject::tr("Slice file not found");
    auto details = QObject::tr("SLICResult::getSlice() -> The path to the slice file doesn't exist.");

    throw Utils::EspinaException(message, details);
  }

  QFile file(filePath.absoluteFilePath());
  if (!file.open(QIODevice::ReadOnly))
  {
    auto message = QObject::tr("Slice file couldn't be read");
    auto details = QObject::tr("SLICResult::getSlice() -> The slice file couldn't be read.");

    throw Utils::EspinaException(message, details);
  }

  return file.readAll();
}

//--------------------------------------------------------------------
void StackSLIC::onExtendedItemSet(ChannelPtr stack)
{
  loadFromSnapshot();
}

//--------------------------------------------------------------------
Snapshot ESPINA::Extensions::StackSLIC::snapshot() const
{
  Snapshot snapshot;

  QReadLocker lock(&m_result.dataMutex);

  if(m_result.computed)
  {
    for(unsigned int i = m_result.region.GetIndex(2); i < m_result.region.GetIndex(2)+m_result.region.GetSize(2); ++i)
    {
      snapshot << SnapshotData(snapshotName(VOXELS_FILE).arg(i), getSlice(i));
    }

    QByteArray labelBuffer;
    QDataStream labelStream(&labelBuffer, QIODevice::WriteOnly);
    labelStream.setVersion(QDataStream::Qt_4_0);
    labelStream << m_result.supervoxels;
    snapshot << SnapshotData(snapshotName(LABELS_FILE), qCompress(labelBuffer,9));
    labelBuffer.clear();

    QByteArray dataBuffer;
    QDataStream stream(&dataBuffer, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_0);

    stream << static_cast<long long>(m_result.region.GetIndex(0)) << static_cast<long long>(m_result.region.GetIndex(1)) << static_cast<long long>(m_result.region.GetIndex(2));
    stream << static_cast<long long>(m_result.region.GetSize(0)) << static_cast<long long>(m_result.region.GetSize(1)) << static_cast<long long>(m_result.region.GetSize(2));

    stream << static_cast<int>(m_result.variant);
    stream << m_result.m_s;
    stream << m_result.m_c;
    stream << m_result.iterations;
    stream << m_result.tolerance;

    snapshot << SnapshotData(snapshotName(DATA_FILE), qCompress(dataBuffer, 9));
    dataBuffer.clear(); // not really needed, end of scope.
  }

  return snapshot;
}

//--------------------------------------------------------------------
inline const unsigned long long int StackSLIC::SLICComputeTask::offsetOfIndex(const IndexType &index)
{
  auto &region = result.region;

  return  (index.GetElement(0) - region.GetIndex(0)) +
         ((index.GetElement(1) - region.GetIndex(1)) * region.GetSize(0)) +
         ((index.GetElement(2) - region.GetIndex(2)) * region.GetSize(0)*region.GetSize(1));
}

//--------------------------------------------------------------------
const int StackSLIC::taskProgress() const
{
  return (m_task ? m_task->progress() : 0);
}

//--------------------------------------------------------------------
const QStringList StackSLIC::SLICComputeTask::errors() const
{
  QStringList result;

  if(!m_errorMessage.isEmpty())
  {
    result << m_errorMessage;
  }

  return result;
}

//--------------------------------------------------------------------
const QStringList StackSLIC::errors() const
{
  return (m_task && m_task->hasErrors() ? m_task->errors() : QStringList());
};
