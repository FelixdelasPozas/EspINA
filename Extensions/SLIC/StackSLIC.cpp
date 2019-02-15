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

// TODO /////////////////////////////////////////
// Test other compression methods apart from RLE.
// SLIC tested, more testing with SLICO and ASLIC.
// Delete supervoxels without voxels (empty labels).
// Use of itk regions instead of computing with Bounds(), some parts can crash due to bad rounding (getSuperVoxel?).
// Use of slice regions from Edges? (check, this is done already?)
// Compute SLIC of only a region of the stack, this changes save and load methods if we want to save results.
// More code cleanups and commenting.
// Additional pass for memory reduction, if(bounds(supervoxel_i).intersect(bounds(supervoxel_j) == false) both can have same color, using only unsigned char.
////////////////////////////////////////////////

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
#include <itkImage.hxx>
#include <itkEuclideanDistanceMetric.hxx>
#include <itkImageConstIteratorWithIndex.hxx>
#include <itkImageRegion.hxx>
#include <itkGradientMagnitudeImageFilter.h>

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
  if(result.computed) return true;

  Snapshot snapshot;

  auto dataName = snapshotName(DATA_FILE);
  auto labelsName = snapshotName(LABELS_FILE);
  QFileInfo dataFileInfo = m_extendedItem->storage()->absoluteFilePath(dataName);
  QFileInfo labelsFileInfo = m_extendedItem->storage()->absoluteFilePath(labelsName);

  if(!dataFileInfo.exists() || !labelsFileInfo.exists())
  {
    return false;
  }

  QFile labelsFile(labelsFileInfo.absoluteFilePath());
  QFile dataFile(dataFileInfo.absoluteFilePath());
  if(!labelsFile.open(QIODevice::ReadOnly) || !dataFile.open(QIODevice::ReadOnly))
  {
    return false;
  }

  QWriteLocker lock(&result.m_dataMutex);

  QByteArray labelBuffer = labelsFile.readAll();
  QDataStream labelStream(&labelBuffer, QIODevice::ReadOnly);
  labelStream.setVersion(QDataStream::Qt_4_0);
  result.supervoxels.clear();
  labelStream >> result.supervoxels;
  QByteArray data = dataFile.readAll();

  QDataStream stream(&data, QIODevice::ReadOnly);
  stream.setVersion(QDataStream::Qt_4_0);

  stream >> result.supervoxel_count;
  stream >> result.slice_count;
  stream >> result.bounds[0] >> result.bounds[1] >> result.bounds[2] >> result.bounds[3] >> result.bounds[4] >> result.bounds[5];

  int variant;
  stream >> variant;
  result.variant = static_cast<StackSLIC::SLICVariant>(variant);
  stream >> result.m_s;
  stream >> result.m_c;
  stream >> result.iterations;
  stream >> result.tolerance;

  result.modified = false;
  result.computed = true;

  return true;
}

//-----------------------------------------------------------------------------
void StackSLIC::onComputeSLIC(unsigned char parameter_m_s, unsigned char parameter_m_c, Extensions::StackSLIC::SLICVariant variant, unsigned int max_iterations, double tolerance)
{
  if(m_task && m_task->isRunning()) return;

  result.computed     = false;
  result.m_s          = parameter_m_s;
  result.m_c          = parameter_m_c;
  result.variant      = variant;
  result.iterations   = max_iterations;
  result.tolerance    = tolerance;
  result.modified     = false;
  result.bounds       = m_extendedItem->output()->bounds();

  m_task = std::make_shared<SLICComputeTask>(m_extendedItem, m_scheduler, m_factory, result);

  connect(m_task.get(), SIGNAL(finished()), this, SLOT(onSLICComputed()));
  connect(m_task.get(), SIGNAL(progress(int)), this, SIGNAL(progress(int)));

  Task::submit(m_task);
}

//-----------------------------------------------------------------------------
void StackSLIC::onSLICComputed()
{
  if(m_task != nullptr)
  {
    disconnect(m_task.get(), SIGNAL(finished()), this, SLOT(onSLICComputed()));
    disconnect(m_task.get(), SIGNAL(progress(int)), this, SIGNAL(progress(int)));
    m_task = nullptr;
  }

  emit computeFinished();
}

//-----------------------------------------------------------------------------
void StackSLIC::onAbortSLIC()
{
  if(m_task != nullptr)
  {
    disconnect(m_task.get(), SIGNAL(finished()), this, SLOT(onSLICComputed()));
    disconnect(m_task.get(), SIGNAL(progress(int)), this, SIGNAL(progress(int)));

    if(m_task->isRunning()) m_task->abort();
    m_task->thread()->wait(-1);

    m_task = nullptr;
    qDebug() << "SLIC execution aborted. Shutting down threads...";
    emit computeAborted();
  }
}

//-----------------------------------------------------------------------------
StackSLIC::SLICComputeTask::SLICComputeTask(ChannelPtr stack, SchedulerSPtr scheduler, CoreFactory *factory, SLICResult &result)
: Task{scheduler}
, m_stack{stack}
, m_factory{factory}
, result(result)
, spacing(stack->output()->spacing())
, bounds(stack->bounds())
, voxels{nullptr}
, n_voxels(0)
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

  description += QString("of") + m_stack->name();
  setDescription(description);
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::run()
{
  BlockTimer<std::chrono::milliseconds> timer{"SLIC"};
  reportProgress(0);

  //Square tolerance to avoid having to calculate roots later
  const auto tolerance = std::pow(result.tolerance, 2);

  //Initialize channel edges extension
  auto edgesExtension = retrieveOrCreateStackExtension<ChannelEdges>(m_stack, m_factory);
  auto sliceRegion = edgesExtension->sliceRegion(0);

  //Calculate dimensions using voxels as unit
  max_x = bounds.lenght(Axis::X)/spacing[0];
  max_y = bounds.lenght(Axis::Y)/spacing[1];
  max_z = bounds.lenght(Axis::Z)/spacing[2];

  //Calculate total number of voxels
  n_voxels = max_x*max_y*max_z;

  qDebug() << QString("Size: %1 %2 %3").arg(max_x).arg(max_y).arg(max_z);
  qDebug() << QString("Spatial: %1 - Color: %2 - Iterations: %3 - Tolerance: %4").arg(result.m_s).arg(result.m_c).arg(result.iterations).arg(tolerance);

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

  //Variables
  float spatial_distance;
  ImageRegion region;
  IndexType cur_index;
  Label *label;
  bool converged = false;
  int progressValue = 0;
  unsigned int label_index = 0, voxel_index;
  long long int sum_x = 0, sum_y = 0, sum_z = 0;
  unsigned long long int sum_color = 0, sum_voxels = 0;
  double newProgress = 0;
  int region_position[3];
  int region_size[3];

  //Find centers for the supervoxels
  QList<Label> labels;
  if(!initSupervoxels(image.GetPointer(), labels, edgesExtension.get()))
    return;
  qDebug() << QString("Created %1 labels").arg(labels.size());

  //Reserve enough memory for all voxels
  voxels = new unsigned int[n_voxels];
  std::memset(voxels, std::numeric_limits<unsigned int>::max(), n_voxels * sizeof(unsigned int));

  //Extract constants from the loops.
  const auto scan_size = 2. * result.m_s;

  const double iterationPercentage = 100.0 / result.iterations;

  try
  {
    for(unsigned int iteration = 0; iteration< result.iterations && !converged; iteration++)
    {
      if(!canExecute())
      {
        delete[] voxels;
        voxels = nullptr;
        return;
      }
      qDebug() << QString("Starting iteration: %1 - %2s").arg(iteration+1).arg(timer.elapsed()/1000);

      newProgress = iterationPercentage * iteration;
      if(newProgress != progressValue)
      {
        progressValue = newProgress;
        reportProgress(progressValue);
      }

      watcher.setFuture(QtConcurrent::map(labels, std::bind(&SLICComputeTask::computeLabel, this, std::placeholders::_1, edgesExtension, image, &labels)));
      watcher.waitForFinished();

      if(!canExecute())
      {
        delete[] voxels;
        voxels = nullptr;
        return;
      }

      //If convergence test is enabled, assume it converged until proven otherwise
      if(tolerance > 0) converged = true;

      //Recalculate centers, check convergence
      qDebug() << "Recalculating centers";
      for(label_index = 0; label_index < static_cast<unsigned int>(labels.size()); label_index++)
      {
        if(!canExecute())
        {
          delete[] voxels;
          voxels = nullptr;
          return;
        }

        label=&labels[label_index];
        findCandidateRegion(label->center, scan_size, region_position, region_size);

        //Make sure that the region is inside the bounds of the image
        fitRegionToBounds(region_position, region_size);

        region_size[0] += region_position[0];
        region_size[1] += region_position[1];
        region_size[2] += region_position[2];

        sum_color = sum_voxels = sum_x = sum_y = sum_z = 0;
        for (auto z = region_position[2]; z < region_size[2]; z++)
        {
          for (auto y = region_position[1]; y < region_size[1]; y++)
          {
            voxel_index = region_position[0] + (y * max_x) + (z * max_x * max_y);
            if(voxel_index > n_voxels)
            {
              qWarning() << "invalid index" << voxel_index;
              qWarning() << "region index" << region_position[0] << region_position[1] << region_position[2];
              qWarning() << "region size " << region_size[0] << region_size[1] << region_size[2];
              Q_ASSERT(false); // segmentation fault when indexing 'voxels' pointer.
            }
            for (auto x = region_position[0]; x < region_size[0]; x++)
            {
              voxel_index++;
              if (label->index == voxels[voxel_index])
              {
                sum_voxels++;
                sum_x += x; sum_y += y; sum_z += z;
                cur_index = {x,y,z};
                sum_color += image->GetPixel(cur_index);
              }
            }
          }
        }

        if(sum_voxels > 0)
        {
          //Calculate averaged coordinates
          sum_x = round(sum_x/sum_voxels);
          sum_y = round(sum_y/sum_voxels);
          sum_z = round(sum_z/sum_voxels);
          //Calculate displacement for the tolerance test
          if(tolerance > 0 && converged)
          {
            cur_index = {sum_x, sum_y, sum_z};
            spatial_distance = calculateDistance(label->center, cur_index, 0, 0, 0, nullptr, nullptr, true);
            if(spatial_distance > tolerance)
              converged = false;
          }
          label->center = {sum_x, sum_y, sum_z};
          label->color = sum_color/sum_voxels;
        }

        //Update weights with maximum observed results and reset them
        switch(result.variant)
        {
          case SLICVariant::ASLIC:
            //Squared constants to prevent having to use expensive roots
            label->norm_quotient = std::pow(label->m_c,2)/pow(label->m_s,2);
            label->m_c = 1;
            label->m_s = 1.0;
            break;
          case SLICVariant::SLICO:
            label->norm_quotient = std::pow(result.m_c,2)/pow(label->m_s,2);
            label->m_s = 1.0;
            break;
          case SLICVariant::SLIC:
            break;
          default:
            Q_ASSERT(false);
            break;
        }

      } //label

      qDebug() << QString("Finishing iteration: %1 - %2").arg(iteration+1).arg(timer.elapsed()/1000);
    } //iteration
  }
  catch(std::exception &e)
  {
     std::cerr << e.what() << std::endl << std::flush;
     exit(1);
  }

  if(!canExecute())
  {
    delete[] voxels;
    voxels = nullptr;
    return;
  }

  qDebug() << QString("Starting post-process connectivity algorithm: %1").arg(timer.elapsed()/1000);
  ensureConnectivity(labels);

  saveResults(labels);

  delete [] voxels;
  voxels = nullptr;

  result.computed = true;
  result.modified = true;
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::onAbort()
{
  watcher.cancel();
}

//-----------------------------------------------------------------------------
const unsigned long int StackSLIC::getSupervoxel(const itkVolumeType::IndexType position) const
{
  //TODO: Check if x,y,z is in bounds
  if(!result.computed) return 0;

  auto slice = getUncompressedSlice(position[2]).get();
  return slice[position[0]+position[1]*(int)result.bounds.lenght(Axis::X)];
}

//-----------------------------------------------------------------------------
const unsigned char StackSLIC::getSupervoxelColor(const unsigned int supervoxel) const
{
  if(!result.computed) return 0;

  return result.supervoxels[supervoxel].color;
}

//-----------------------------------------------------------------------------
const itkVolumeType::IndexType StackSLIC::getSupervoxelCenter(const unsigned int supervoxel) const
{
  if(!result.computed) return {0,0,0};

  return result.supervoxels[supervoxel].center;
}

//-----------------------------------------------------------------------------
bool StackSLIC::drawVoxelCenters(const unsigned int slice, vtkSmartPointer<vtkPoints> data)
{
  if(isRunning()) return false;

  QReadLocker lock(&result.m_dataMutex);

  if(!result.computed) return false;

  data->Reset();
  auto spacing = m_extendedItem->output()->spacing();

  auto testAndAddOp = [this, &spacing, &data, slice](const SuperVoxel &superVoxel)
  {
    if(superVoxel.center.GetElement(2) == slice)
    {
      data->InsertNextPoint(superVoxel.center.GetElement(0) * spacing[0],
                            superVoxel.center.GetElement(1) * spacing[1],
                            superVoxel.center.GetElement(2) * spacing[2]);
    }
  };
  std::for_each(result.supervoxels.constBegin(), result.supervoxels.constEnd(), testAndAddOp);
  data->Modified();

  return true;
}

//-----------------------------------------------------------------------------
bool StackSLIC::drawSliceInImageData(const unsigned int slice, vtkSmartPointer<vtkImageData> data)
{
  if(isRunning()) return false;

  QReadLocker lock(&result.m_dataMutex);

  if(!result.computed || slice < 0 || slice >= result.slice_count) return false;

  Bounds bounds = m_extendedItem->bounds();
  OutputSPtr output = m_extendedItem->output();
  NmVector3 spacing = output->spacing();
  unsigned int max_x = bounds.lenght(Axis::X)/spacing[0];
  unsigned int max_y = bounds.lenght(Axis::Y)/spacing[1];
  unsigned long long int pixel_count = max_x*max_y;

  data->SetExtent(0, max_x-1, 0, max_y-1, slice, slice);
  data->SetSpacing(spacing[0], spacing[1], spacing[2]);
  data->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  auto buffer = reinterpret_cast<unsigned char *>(data->GetScalarPointer());

  memcpy(buffer, getUncompressedSlice(slice).get(), pixel_count);

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

  //TODO: Use Core/Utils/SpatialUtils.hxx & Core/Analysis/Data/VolumetricDataUtils.hxx
  using ImageType = itk::Image<unsigned int, 3>;

  auto spacing = m_extendedItem->output()->spacing();
  unsigned int max_x = bounds.lenght(Axis::X)/spacing[0];

  auto image = create_itkImage<ImageType>(bounds, SEG_BG_VALUE, spacing);
  auto data_offset = image->GetBufferPointer();

  QReadLocker lock(&result.m_dataMutex);
  for (int z = bounds[4]; z < bounds[4] + bounds[5]; z++)
  {
    auto s = getUncompressedLabeledSlice(z);
    for (int y = bounds[2]; y < bounds[2] + bounds[3]; y++)
    {
      auto slice_buffer = s.get();
      slice_buffer += y * max_x;
      for (int x = bounds[0]; x < bounds[0] + bounds[1]; x++)
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
  return result.computed;
}

//-----------------------------------------------------------------------------
const bool StackSLIC::isRunning() const
{
  return m_task && m_task->isRunning();
}

//-----------------------------------------------------------------------------
StackSLIC::SLICVariant StackSLIC::getVariant()
{
  return result.variant;
}

//-----------------------------------------------------------------------------
unsigned char StackSLIC::getSupervoxelSize()
{
  return result.m_s;
}

//-----------------------------------------------------------------------------
unsigned char StackSLIC::getColorWeight()
{
  return result.m_c;
}

//-----------------------------------------------------------------------------
unsigned int StackSLIC::getIterations()
{
  return result.iterations;
}

//-----------------------------------------------------------------------------
double StackSLIC::getTolerance()
{
  return result.tolerance;
}

//-----------------------------------------------------------------------------
unsigned int StackSLIC::getSupervoxelCount()
{
  return result.supervoxel_count;
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
inline const bool StackSLIC::SLICComputeTask::isInBounds(const int x, const int y, const int z) const
{
  return !(x < min_x || y < min_y || z < min_z || x > max_x || y > max_y || z > max_z);
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::fitRegionToBounds(int region_position[], int region_size[])
{
  if(region_position[0] < min_x)
  {
    region_size[0] -= min_x - region_position[0]; //Substract from size by adding a negative value
    region_position[0] = min_x;
  }

  if(region_position[1] < min_y)
  {
    region_size[1] -= min_y - region_position[1];
    region_position[1] = min_y;
  }

  if(region_position[2] < min_z)
  {
    region_size[2] += min_z - region_position[2];
    region_position[2] = min_z;
  }

  if(region_position[0]+region_size[0] > max_x) region_size[0] = max_x-region_position[0];
  if(region_position[1]+region_size[1] > max_y) region_size[1] = max_y-region_position[1];
  if(region_position[2]+region_size[2] > max_z) region_size[2] = max_z-region_position[2];
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::fitRegionToBounds(long long int region_position[], long long int region_size[])
{
  if(region_position[0] < min_x)
  {
    region_size[0] -= min_x - region_position[0]; //Substract from size by adding a negative value
    region_position[0] = min_x;
  }

  if(region_position[1] < min_y)
  {
    region_size[1] -= min_y - region_position[1];
    region_position[1] = min_y;
  }

  if(region_position[2] < min_z)
  {
    region_size[2] += min_z - region_position[2];
    region_position[2] = min_z;
  }

  if(region_position[0]+region_size[0] > max_x) region_size[0] = max_x-region_position[0];
  if(region_position[1]+region_size[1] > max_y) region_size[1] = max_y-region_position[1];
  if(region_position[2]+region_size[2] > max_z) region_size[2] = max_z-region_position[2];
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::saveResults(QList<Label> labels)
{
  QWriteLocker lock(&result.m_dataMutex);

  result.slice_count = max_z;
  result.supervoxel_count = labels.size();
  result.supervoxels.clear();

  std::for_each(labels.constBegin(), labels.constEnd(), [this](const Label &label) { result.supervoxels.append({label.center, label.color});});
  labels.clear();

  for (int z = 0; z < max_z; z++)
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

    if(data.size() != file.write(data))
    {
      qDebug() << "failed to save data of" << fileName;
      continue;
    }

    file.flush();
    file.close();
  }
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::compressSlice(QDataStream &stream, unsigned int z)
{
  //Compression algorithm goes here
  unsigned long long voxel_index = 0;
  unsigned int current_label;
  unsigned char same_label_count = 0;

  for (int y = 0; y < max_y; y++)
  {
    voxel_index = y * max_x + z * max_x * max_y;
    for (int x = 0; x < max_x; x++)
    {
      if (x != 0)
      {
        if (voxels[voxel_index] != current_label)
        {
          //Write RLE
          stream << current_label;
          stream << same_label_count;
          current_label = voxels[voxel_index];
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

          same_label_count++;
        }
      }
      else
      {
        current_label = voxels[voxel_index];
        same_label_count = 1;
      }
      voxel_index++;
    }
    //Write last supervoxel in row
    stream << current_label;
    stream << same_label_count;
  }
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::findCandidateRegion(itkVolumeType::IndexType &center, double scan_size,  int region_position[], int region_size[])
{
  union vec4f
  {
    __m128 vec;
    float f[4];
  };

  //Vector division: scan_size / spacing
  vec4f srca, srcb;
  //srca.f[0] = scan_size;
  //srca.f[1] = scan_size;
  //srca.f[2] = scan_size;
  //srca.vec = _mm_load_ps(srca.f);
  srca.vec = _mm_set_ps1(scan_size);
  srcb.f[0] = spacing[0];
  srcb.f[1] = spacing[1];
  srcb.f[2] = spacing[2];
  srcb.vec = _mm_load_ps(srcb.f);
  srca.vec = _mm_div_ps(srca.vec, srcb.vec);
  _mm_store_ps(srcb.f, srca.vec);

  region_size[0] = srcb.f[0];
  region_size[1] = srcb.f[1];
  region_size[2] = srcb.f[2];

  //Vector divide by 2
  //srcb.f[0] = 2;
  //srcb.vec = _mm_load1_ps(&srcb.f[0]);
  srcb.vec = _mm_set_ps1(2);
  srcb.vec = _mm_div_ps(srca.vec, srcb.vec);

  //Substract center - previous result
  srca.f[0] = center[0];
  srca.f[1] = center[1];
  srca.f[2] = center[2];
  srcb.vec = _mm_sub_ps(srca.vec, srcb.vec);
  _mm_store_ps(srcb.f, srcb.vec);

  region_position[0] = srcb.f[0];
  region_position[1] = srcb.f[1];
  region_position[2] = srcb.f[2];
}

//-----------------------------------------------------------------------------
bool StackSLIC::SLICComputeTask::initSupervoxels(itkVolumeType *image, QList<Label> &labels, ChannelEdges *edgesExtension)
{
  using IndexType = itkVolumeType::IndexType;
  using GradientType = itk::Image<float,3>;
  using ImageRegion = itk::ImageRegion<3>;
  using GradientFilterType = itk::GradientMagnitudeImageFilter<itkVolumeType, GradientType>;
  using GradientRegionIterator = itk::ImageRegionConstIteratorWithIndex<GradientType>;

  ImageRegion region;
  IndexType cur_index;
  int region_position[3];
  int region_size[3];

  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();

  for(int z=result.m_s/2+min_z;z<max_z;z+=result.m_s/spacing[2])
  {
    if(!canExecute()) return false;

    auto sliceRegion = edgesExtension->sliceRegion(static_cast<unsigned int>(z));

    for(int y = result.m_s / 2 + min_y; y < max_y; y += result.m_s / spacing[1])
    {
      for(int x = result.m_s / 2 + min_x; x < max_x; x += result.m_s / spacing[0])
      {
        if(!canExecute()) return false;

        //Skip if out of the region of interest or image bounds
        if(!isInBounds(x, y, z)) continue;

        //Check if inside bounds using ChannelEdges, else skip this label
        cur_index[0] = x;
        cur_index[1] = y;
        cur_index[2] = z;

        //Don't create supervoxel centers outside of the calculated edges
        if(!sliceRegion.IsInside(cur_index)) continue;

        //Check lowest gradient voxel in a 3x3x3 area around voxel
        region_position[0] = x-2; region_position[1] = y-2; region_position[2] = z-2;
        region_size[0] = 5; region_size[1] = 5; region_size[2] = 5;
        fitRegionToBounds(region_position, region_size);
        //qDebug() << QString("Region: %1 %2 %3 / %4 %5 %6").arg(region_position[0]).arg(region_position[1])
        //                    .arg(region_position[2]).arg(region_size[0]).arg(region_size[1]).arg(region_size[2]);
        region.SetIndex((IndexType) {region_position[0], region_position[1], region_position[2]});
        region.SetSize(0, region_size[0]);
        region.SetSize(1, region_size[1]);
        region.SetSize(2, region_size[2]);

        //Calculate gradient in area
        image->SetRequestedRegion(region);
        gradientFilter->SetNumberOfThreads(1);
        gradientFilter->SetInput(image);
        gradientFilter->GetOutput()->SetRequestedRegion(region);
        gradientFilter->Update();

        //Loop gradient image and find lowest gradient
        region_position[0] = x-1; region_position[1] = y-1; region_position[2] = z-1;
        region_size[0] = 3; region_size[1] = 3; region_size[2] = 3;
        fitRegionToBounds(region_position, region_size);
        region.SetIndex((IndexType) {region_position[0], region_position[1], region_position[2]});
        region.SetSize(0, region_size[0]);
        region.SetSize(1, region_size[1]);
        region.SetSize(2, region_size[2]);
        GradientRegionIterator it(gradientFilter->GetOutput(), region);
        it.GoToBegin();
        float lowestGradient = std::numeric_limits<float>::max();
        while(!it.IsAtEnd())
        {
          //qDebug() << QString("Center/Checking: %2 %3 %4 -> %5 %6 %7").arg(x).arg(y).arg(z).arg(it.GetIndex()[0]).arg(it.GetIndex()[1]).arg(it.GetIndex()[2]);
          auto value = it.Get();
          if(value < lowestGradient)
          {
            cur_index = it.GetIndex();
            lowestGradient = value;
            //qDebug() << QString("New value: %1 | Center/New: %2 %3 %4 -> %5 %6 %7")
            //                   .arg(value).arg(x).arg(y).arg(z).arg(cur_index[0]).arg(cur_index[1]).arg(cur_index[2]);
          }
          ++it;
        }

        labels.append((Label) {std::pow(result.m_c,2) / std::pow(result.m_s,2), 1.0, (unsigned int) labels.size(), cur_index, image->GetPixel(cur_index), 1 });
      }
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
float StackSLIC::SLICComputeTask::calculateDistance(IndexType &voxel_index, IndexType &center_index,
                                                   unsigned char voxel_color, unsigned char center_color,
                                                   float norm_quotient, float *color_distance,
                                                   float *spatial_distance, bool only_spatial)
{
  union vec4f
  {
    __m128 vec;
    float f[4];
  };

  //Vector substraction of cur_index - label->center
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
  srcb.f[1] = spacing[0];
  srcb.f[2] = spacing[1];
  srcb.f[3] = spacing[2];
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
void StackSLIC::SLICComputeTask::computeLabel(Label &label, std::shared_ptr<ChannelEdges> edgesExtension, itkVolumeType::Pointer image, QList<Label> *labels)
{
  //TODO: Add progress tracking through signals
  int region_position[3];
  int region_size[3];
  unsigned char center_color, voxel_color;
  int current_slice = 0;
  ImageRegion region;
  bool cropped;
  unsigned int voxel_index, label_old_index;
  float spatial_distance, color_distance, distance, distance_old;
  Label *label_old;
  IndexType cur_index;

  //Find the region of voxels that are in range of this supervoxel
  findCandidateRegion(label.center, 2. * result.m_s, region_position, region_size);

  //Make sure that the region is inside the bounds of the image
  fitRegionToBounds(region_position, region_size);
  center_color = label.color;

  //Iterate over the slices
  for(current_slice = region_position[2]; current_slice < region_position[2] + region_size[2]; current_slice++)
  {
    //Get slice edges
    auto sliceRegion = edgesExtension->sliceRegion(current_slice);
    //Set region bounds around center for current slice
    region.SetIndex(0, region_position[0]);
    region.SetIndex(1, region_position[1]);
    region.SetIndex(2, current_slice);
    region.SetSize(0, region_size[0]);
    region.SetSize(1, region_size[1]);
    region.SetSize(2, 1);

    //Crop to region inside edges
    cropped = region.Crop(sliceRegion);

    //If the region was completely outside the edges, skip to next slice
    //This mean either the stack was displaced too much or the z index
    //was too high/low
    if(!cropped)
      continue;


    RegionIterator it(image, region);
    it.GoToBegin();

    while(!it.IsAtEnd())
    {
      cur_index = it.GetIndex();
      voxel_color = it.Get();
      distance = calculateDistance(cur_index, label.center, voxel_color, center_color, label.norm_quotient, &color_distance, &spatial_distance);
      voxel_index = cur_index[0]+cur_index[1]*max_x+cur_index[2]*max_x*max_y;
      label_old_index = voxels[voxel_index];
      if(label_old_index == std::numeric_limits<unsigned int>::max() || label_old_index == label.index)
      {
        //If voxel is unassigned or assigned to the current label, assume it's an infinite distance away
        distance_old = std::numeric_limits<double>::max();
      }
      else
      {
        label_old = &(*labels)[label_old_index];
        if(label_old == nullptr)
        {
          qDebug() << QString("Null label_old: %1/%2 labels %3").arg(label_old_index).arg((*labels).size());
          label_old = &(*labels)[label_old_index];
          qDebug() << QString("Label: %1").arg((*labels)[label_old_index].index);
        }
        distance_old = calculateDistance(cur_index, label_old->center, voxel_color, image->GetPixel(label_old->center), label_old->norm_quotient, nullptr, nullptr);
      }

      if(distance < distance_old)
      {
        QMutexLocker locker(&labelListMutex);
        if(label_old_index != voxels[voxel_index])
        {
          label_old = &(*labels)[voxels[voxel_index]];
          distance_old = calculateDistance(cur_index, label_old->center, voxel_color, image->GetPixel(label_old->center), label_old->norm_quotient, nullptr, nullptr);
        }
        if(distance < distance_old)
        {
          voxels[voxel_index] = label.index;
          //Update maximum distances
          switch(result.variant)
          {
            case SLICVariant::ASLIC:
              if(label.m_c < color_distance)
                label.m_c = color_distance;
            //no break
            case SLICVariant::SLICO:
            case SLICVariant::SLIC:
              if(label.m_s < spatial_distance)
                label.m_s = spatial_distance;
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

  //if(!canExecute()) return;

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
void StackSLIC::SLICComputeTask::createSupervoxel(IndexType cur_index, ChannelEdges *edgesExtension, itkVolumeType *image, QList<Label> *labels)
{
  using GradientType = itk::Image<float,3>;
  using GradientFilterType = itk::GradientMagnitudeImageFilter<itkVolumeType, GradientType>;
  //using GradientFilterType = itk::GradientMagnitudeRecursiveGaussianImageFilter<itkVolumeType, GradientType>;
  using GradientRegionIterator = itk::ImageRegionConstIteratorWithIndex<GradientType>;


  ImageRegion region;
  int region_position[3];
  int region_size[3];

  auto sliceRegion = edgesExtension->sliceRegion(cur_index[2]);

  //Don't create supervoxel centers outside of the calculated edges
  if(!sliceRegion.IsInside(cur_index)) return;

  //Check lowest gradient voxel in a 3x3x3 area around voxel
  region_position[0] = cur_index[0]-2; region_position[1] = cur_index[1]-2; region_position[2] = cur_index[2]-2;
  region_size[0] = 5; region_size[1] = 5; region_size[2] = 5;
  fitRegionToBounds(region_position, region_size);
  //qDebug() << QString("Region: %1 %2 %3 / %4 %5 %6").arg(region_position[0]).arg(region_position[1])
  //                    .arg(region_position[2]).arg(region_size[0]).arg(region_size[1]).arg(region_size[2]);
  region.SetIndex((IndexType) {region_position[0], region_position[1], region_position[2]});
  region.SetSize(0, region_size[0]);
  region.SetSize(1, region_size[1]);
  region.SetSize(2, region_size[2]);

  //Calculate gradient in area
  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  //gradientFilter->SetNumberOfThreads(1);
  image->SetRequestedRegion(region);
  gradientFilter->SetInput(image);
  gradientFilter->GetOutput()->SetRequestedRegion(region);
  gradientFilter->Update();

  //Loop gradient image and find lowest gradient
  region_position[0] = cur_index[0]-1; region_position[1] = cur_index[1]-1; region_position[2] = cur_index[2]-1;
  region_size[0] = 3; region_size[1] = 3; region_size[2] = 3;
  fitRegionToBounds(region_position, region_size);
  region.SetIndex((IndexType) {region_position[0], region_position[1], region_position[2]});
  region.SetSize(0, region_size[0]);
  region.SetSize(1, region_size[1]);
  region.SetSize(2, region_size[2]);
  GradientRegionIterator it(gradientFilter->GetOutput(), region);
  it.GoToBegin();
  float lowestGradient = std::numeric_limits<float>::max();
  while(!it.IsAtEnd())
  {
    //qDebug() << QString("Center/Checking: %2 %3 %4 -> %5 %6 %7").arg(x).arg(y).arg(z).arg(it.GetIndex()[0]).arg(it.GetIndex()[1]).arg(it.GetIndex()[2]);
    auto value = it.Get();
    if(value < lowestGradient)
    {
      cur_index = it.GetIndex();
      lowestGradient = value;
      //qDebug() << QString("New value: %1 | Center/New: %2 %3 %4 -> %5 %6 %7")
      //                   .arg(value).arg(x).arg(y).arg(z).arg(cur_index[0]).arg(cur_index[1]).arg(cur_index[2]);
    }
    ++it;
  }

  {
    QMutexLocker locker(&labelListMutex);
    labels->append((Label) {std::pow(result.m_c,2) / std::pow(result.m_s,2), 1.0, (unsigned int) labels->size(), cur_index, image->GetPixel(cur_index), 1 });
  }
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::ensureConnectivity(QList<Label> &labels)
{
  std::for_each(labels.begin(), labels.end(), [this](Label &label) { labelConnectivity(label); });
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::labelConnectivity(Label &label)
{
  const int slice_area = max_x * max_y;
  const int distance = label.m_s;
  const int range = distance*2+1;
  IndexType center = label.center;
  IndexType current = center;
  IndexType contiguous[6];//[26];
  long long int region[3] = {center[0] - distance, center[1] - distance, center[2] - distance};
  long long int length[3] = {range, range, range};
  fitRegionToBounds(region, length);
  long long int offsetImage, offsetRegion;
  const int size = length[0] * length[1] * length[2];
  std::vector<bool> visited(size, false);
  std::queue<IndexType> search_queue;

  search_queue.push(current);
  offsetRegion = current[0] - region[0] + (current[1]-region[1]) * length[0] + (current[2] - region[2])*length[0]*length[1];
  offsetImage = current[0] + current[1]*max_x + current[2]*slice_area;
  //Edge case where a supervoxel is too irregular and the center doesn't land inside its volume
  bool centerFound = voxels[offsetImage] == label.index;
  //if(!centerFound) {
    //qDebug() << QString("Label %1 - Center not inside supervoxel!").arg(label.index);
  //}

  if(offsetRegion > size) qDebug() << QString("Error calculating offset!\n");

  visited[offsetRegion] = true;

  while(!search_queue.empty())
  {
    current = search_queue.front();
    search_queue.pop();
    offsetImage = current[0] + current[1]*max_x + current[2]*slice_area;

    contiguous[0] = {current[0]+1, current[1], current[2]};
    contiguous[1] = {current[0]-1, current[1], current[2]};
    contiguous[2] = {current[0], current[1]+1, current[2]};
    contiguous[3] = {current[0], current[1]-1, current[2]};
    contiguous[4] = {current[0], current[1], current[2]+1};
    contiguous[5] = {current[0], current[1], current[2]-1};

    for(IndexType next : contiguous)
    {
      if(isInBounds(next[0], next[1], next[2]))
      {
        //Skip voxels outside the valid subregion
        if(next[0] < region[0] || next[0] >= region[0] + length[0] ||
            next[1] < region[1] || next[1] >= region[1] + length[1] ||
            next[2] < region[2] || next[2] >= region[2] + length[2])
        {
          continue;
        }

        offsetRegion = next[0] - region[0] + (next[1]-region[1]) * length[0] + (next[2] - region[2])*length[0]*length[1];

        if(offsetRegion > size) qDebug() << QString("Error calculating offset!");

        if(!visited[offsetRegion])
        {
          offsetImage = next[0] + next[1]*max_x + next[2]*slice_area;
          //If this is an irregular supervoxel (center isn't inside) do a breadth first
          //search for the closest voxel belonging to the supervoxel
          if(!centerFound) {
            //If a voxel with this label is found, restart the search from that voxel
            //keeping the visited list to avoid double-checking previous voxels
            if(voxels[offsetImage] == label.index)
            {
              std::queue<IndexType> empty;
              std::swap(search_queue, empty);
              centerFound = true;
              visited[offsetRegion] = true;
              search_queue.push(next);
              break;
            }
            search_queue.push(next);
          }
          else
          {
            if(voxels[offsetImage] == label.index) search_queue.push(next);
          }
          visited[offsetRegion] = true;
        }
      }
    }
  }

  //TODO: If centerFound is false at this point the supervoxel has no voxels,
  //decide what to do. Possibly remove the label entirely.
  if(!centerFound)
  {
    qDebug() << QString("Label %1 - Center not found!").arg(label.index);
    return;
  }

  int region_xy_len = length[0] * length[1];
  std::vector<unsigned int> adjacent_labels;
  int unconnected_count = 0;
  for(int i = 0; i < size; i++)
  {
    if(visited[i])
      continue;
    int z = i / (region_xy_len);
    int r = i - z*region_xy_len;
    int y = r / length[0];
    int x = r - y * length[0];
    offsetImage = (x+region[0]) + (y+region[1])*max_x + (z+region[2])*slice_area;
    if(voxels[offsetImage] == label.index)
    {
      int adjacentOffset;
      if (x < max_x-1)
      {
        adjacentOffset = offsetImage + 1;
        if(adjacentOffset >= 0 && adjacentOffset < static_cast<int>(n_voxels) && voxels[adjacentOffset] != label.index)
          adjacent_labels.push_back(voxels[adjacentOffset]);
      }
      if (x > 0)
      {
        adjacentOffset = offsetImage - 1;
        if(adjacentOffset >= 0 && adjacentOffset < static_cast<int>(n_voxels) && voxels[adjacentOffset] != label.index)
          adjacent_labels.push_back(voxels[adjacentOffset]);
      }
      if (y < max_y-1)
      {
        adjacentOffset = offsetImage + max_x;
        if(adjacentOffset >= 0 && adjacentOffset < static_cast<int>(n_voxels) && voxels[adjacentOffset] != label.index)
          adjacent_labels.push_back(voxels[adjacentOffset]);
      }
      if (y > 0)
      {
        adjacentOffset = offsetImage - max_x;
        if(adjacentOffset >= 0 && adjacentOffset < static_cast<int>(n_voxels) && voxels[adjacentOffset] != label.index)
          adjacent_labels.push_back(voxels[adjacentOffset]);
      }
      if (z < max_z-1)
      {
        adjacentOffset = offsetImage + max_y;
        if(adjacentOffset >= 0 && adjacentOffset < static_cast<int>(n_voxels) && voxels[adjacentOffset] != label.index)
          adjacent_labels.push_back(voxels[adjacentOffset]);
      }
      if (z > 0)
      {
        adjacentOffset = offsetImage - max_y;
        if(adjacentOffset >= 0 && adjacentOffset < static_cast<int>(n_voxels) && voxels[adjacentOffset] != label.index)
          adjacent_labels.push_back(voxels[adjacentOffset]);
      }

      std::sort(adjacent_labels.begin(), adjacent_labels.end());
      int max_occurrences = 0;
      int occurrences = 1;
      unsigned int l = adjacent_labels.size() > 0?adjacent_labels[0]:label.index;
      for (unsigned int j = 0; j < adjacent_labels.size() - 1; j++)
      {
        if (adjacent_labels[j] == adjacent_labels[j + 1])
        {
          occurrences++;
          if (occurrences > max_occurrences)
          {
            max_occurrences = occurrences;
            l = adjacent_labels[j];
          }
        }
        else
        {
          occurrences = 1;
        }
      }
      //TODO: Add critical section for multithreaded writes to voxels[]
      if(l==label.index)
      {
        qDebug() << QString("Label %1 - Can't find new supervoxel for: %2 %3 %4\n").arg(label.index).arg(x).arg(y).arg(z);
      }
      voxels[offsetImage] = l;
      unconnected_count++;
    }
  }

//  if(unconnected_count>0)
//    qDebug() << QString("Label %1 - %2 unconnected voxels").arg(label.index).arg(unconnected_count);
}

//-----------------------------------------------------------------------------
std::unique_ptr<char[]> StackSLIC::getUncompressedSlice(const int slice) const
{
  auto compressed = getSlice(slice);

  QDataStream stream(&compressed, QIODevice::ReadOnly);
  stream.setVersion(QDataStream::Qt_4_0);

  Bounds bounds = m_extendedItem->bounds();
  OutputSPtr output = m_extendedItem->output();
  NmVector3 spacing = output->spacing();
  unsigned int max_x = bounds.lenght(Axis::X)/spacing[0];
  unsigned int max_y = bounds.lenght(Axis::Y)/spacing[1];
  unsigned long long int pixel_count = max_x*max_y;
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
      *buffer_ptr++ = label == std::numeric_limits<unsigned int>::max() ? 0 : result.supervoxels[label].color;
    }
    pixel += voxel_count;
  }

  return buffer;
}

//-----------------------------------------------------------------------------
std::unique_ptr<long long[]> StackSLIC::getUncompressedLabeledSlice(const int slice) const
{
  auto compressed = getSlice(slice);

  QDataStream stream(&compressed, QIODevice::ReadOnly);
  stream.setVersion(QDataStream::Qt_4_0);

  Bounds bounds = m_extendedItem->bounds();
  OutputSPtr output = m_extendedItem->output();
  NmVector3 spacing = output->spacing();
  unsigned int max_x = bounds.lenght(Axis::X)/spacing[0];
  unsigned int max_y = bounds.lenght(Axis::Y)/spacing[1];
  unsigned long long int pixel_count = max_x*max_y;
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
  if(!result.computed)
  {
    auto message = QObject::tr("SLIC not computed");
    auto details = QObject::tr("SLICResult::getSlice() -> ") + message;

    throw EspinaException(message, details);
  }

  QReadLocker lock(&result.m_dataMutex);

  QFileInfo filePath;
  if(result.modified)
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

  QReadLocker lock(&result.m_dataMutex);

  if(result.computed)
  {
    for(unsigned int i = 0; i < result.slice_count; ++i)
    {
      snapshot << SnapshotData(snapshotName(VOXELS_FILE).arg(i), getSlice(i));
    }

    QByteArray labelBuffer;
    QDataStream labelStream(&labelBuffer, QIODevice::WriteOnly);
    labelStream.setVersion(QDataStream::Qt_4_0);
    labelStream << result.supervoxels;
    snapshot << SnapshotData(snapshotName(LABELS_FILE), labelBuffer);

    QByteArray dataBuffer;
    QDataStream stream(&dataBuffer, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_0);

    stream << result.supervoxel_count;
    stream << result.slice_count;
    stream << result.bounds[0] << result.bounds[1] << result.bounds[2] << result.bounds[3] << result.bounds[4] << result.bounds[5];

    stream << static_cast<int>(result.variant);
    stream << result.m_s;
    stream << result.m_c;
    stream << result.iterations;
    stream << result.tolerance;

    snapshot << SnapshotData(snapshotName(DATA_FILE), dataBuffer);
  }

  return snapshot;
}
