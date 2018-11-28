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
#include <Core/Factory/CoreFactory.h>
#include <Core/Utils/Spatial.h>
#include <Core/Utils/BlockTimer.hxx>
#include <Core/Utils/EspinaException.h>
#include <Extensions/EdgeDistances/ChannelEdges.h>
#include <Core/Types.h>

//ITK
#include <itkImage.hxx>
#include <itkEuclideanDistanceMetric.hxx>
#include <itkImageConstIteratorWithIndex.hxx>
#include <itkImageRegion.hxx>
#include <vtkUnsignedCharArray.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include "itkGradientMagnitudeImageFilter.h"
//#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"

// C++
#include <limits>
#include <math.h>
#include <memory>
#include <functional>
#include <queue>

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
const QString StackSLIC::DATA_FILE = "data.slic";

//-----------------------------------------------------------------------------
StackSLIC::StackSLIC(SchedulerSPtr scheduler, CoreFactory* factory, const InfoCache &cache)
: StackExtension{cache}
, m_scheduler{scheduler}
, m_factory  {factory}
, task       {nullptr}
{
  qDebug() << "Constructor StackSLIC";
}

//-----------------------------------------------------------------------------
StackSLIC::~StackSLIC()
{
  if(task != nullptr)
  {
      if(task->isRunning())
        task->abort();
  }
}

//-----------------------------------------------------------------------------
State StackSLIC::state() const
{
  return State();
}

//-----------------------------------------------------------------------------
Snapshot StackSLIC::snapshot() const
{
  if(!result.computed)
    return Snapshot();

  QReadLocker lock(&result.m_dataMutex);

  Snapshot snapshot;

  QString voxelsName;
  auto dataName = snapshotName(DATA_FILE);
  auto labelsName = snapshotName(LABELS_FILE);
  for(int i = 0; i<result.slice_count; i++) {
    voxelsName = snapshotName(QString(VOXELS_FILE).arg(i));
    snapshot << SnapshotData(voxelsName, result.voxels[i]);
  }

  QByteArray labelBuffer;
  QDataStream labelStream(&labelBuffer, QIODevice::WriteOnly);
  labelStream.setVersion(QDataStream::Qt_4_0);
  labelStream << result.supervoxels;
  snapshot << SnapshotData(labelsName, labelBuffer);

  QByteArray dataBuffer;
  QDataStream stream(&dataBuffer, QIODevice::WriteOnly);
  stream.setVersion(QDataStream::Qt_4_0);

  stream << result.supervoxel_count;
  stream << result.slice_count;
  stream << result.bounds[0] << result.bounds[1] << result.bounds[2];

  stream << (int) result.variant;
  stream << result.m_s;
  stream << result.m_c;
  stream << result.iterations;
  stream << result.tolerance;

  snapshot << SnapshotData(dataName, dataBuffer);

  return snapshot;
}

//-----------------------------------------------------------------------------
bool StackSLIC::loadFromSnapshot()
{
  if(result.computed)
    return true;

  QWriteLocker lock(&result.m_dataMutex);

  Snapshot snapshot;

  auto dataName = snapshotName(DATA_FILE);
  auto labelsName = snapshotName(LABELS_FILE);
  QFileInfo dataFileInfo = m_extendedItem->storage()->absoluteFilePath(dataName);
  QFileInfo labelsFileInfo = m_extendedItem->storage()->absoluteFilePath(labelsName);

  if(!dataFileInfo.exists() || !labelsFileInfo.exists())
    return false;

  QFile labelsFile(labelsFileInfo.absoluteFilePath());
  QFile dataFile(dataFileInfo.absoluteFilePath());
  if(!labelsFile.open(QIODevice::ReadOnly) || !dataFile.open(QIODevice::ReadOnly))
    return false;


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
  stream >> result.bounds[0] >> result.bounds[1] >> result.bounds[2];

  int variant;
  stream >> variant;
  result.variant = (StackSLIC::SLICVariant) variant;
  stream >> result.m_s;
  stream >> result.m_c;
  stream >> result.iterations;
  stream >> result.tolerance;

  result.voxels.clear();
  QString voxelsName;
  for(int i = 0; i<result.slice_count; i++) {
    voxelsName = snapshotName(QString(VOXELS_FILE).arg(i));
    QFileInfo voxelsFileInfo = m_extendedItem->storage()->absoluteFilePath(voxelsName);
    if(!voxelsFileInfo.exists() ) {
      result.voxels.clear();
      result.supervoxels.clear();
      return false;
    }
    QFile voxelsFile(voxelsFileInfo.absoluteFilePath());
    if(!voxelsFile.open(QIODevice::ReadOnly)) {
      result.voxels.clear();
      result.supervoxels.clear();
      return false;
    }
    result.voxels.append(voxelsFile.readAll());
  }

  result.computed = true;

  return true;
}

//-----------------------------------------------------------------------------
StackExtension::InformationKeyList StackSLIC::availableInformation() const
{
  return InformationKeyList();
}

//-----------------------------------------------------------------------------
void StackSLIC::onComputeSLIC(unsigned char parameter_m_s, unsigned char parameter_m_c, Extensions::StackSLIC::SLICVariant variant, unsigned int max_iterations, double tolerance)
{
  if(task != nullptr) return;

  /*auto */task = std::make_shared<SLICComputeTask>(m_extendedItem, m_scheduler, m_factory, &result, parameter_m_s, parameter_m_c, variant, max_iterations, tolerance);

  connect(task.get(), SIGNAL(finished()), this, SLOT(onSLICComputed()));
  connect(task.get(), SIGNAL(progress(int)), this, SIGNAL(progress(int)));
  connect(this, SLOT(onAbortSLIC()), &task->watcher, SLOT(cancel()));

  Task::submit(task);
}

//-----------------------------------------------------------------------------
void StackSLIC::onSLICComputed()
{
  if(task != nullptr)
  {
    disconnect(task.get(), SIGNAL(finished()), this, SLOT(onSLICComputed()));
    disconnect(task.get(), SIGNAL(progress(int)), this, SIGNAL(progress(int)));
    task = nullptr;
  }

  emit computeFinished();
}

//-----------------------------------------------------------------------------
void StackSLIC::onAbortSLIC()
{
  if(task != nullptr)
  {
    disconnect(task.get(), SIGNAL(finished()), this, SLOT(onSLICComputed()));
    disconnect(task.get(), SIGNAL(progress(int)), this, SIGNAL(progress(int)));

    if(task->isRunning()) task->abort();

    task = nullptr;
    qDebug() << "SLIC execution aborted. Shutting down threads...";
    emit computeAborted();
  }
}

//-----------------------------------------------------------------------------
StackSLIC::SLICComputeTask::SLICComputeTask(ChannelPtr stack, SchedulerSPtr scheduler, CoreFactory *factory, SLICResult *result, unsigned int parameter_m_s, unsigned int parameter_m_c, SLICVariant variant, unsigned int max_iterations, double tolerance)
: Task(scheduler)
, m_stack{stack}
, m_factory{factory}
, result(result)
, parameter_m_s(parameter_m_s)
, parameter_m_c(parameter_m_c)
, variant(variant)
, max_iterations(max_iterations)
, tolerance(tolerance)
, voxels(nullptr)
, label_list(nullptr)
, bounds(stack->bounds())
, scan_size(0)
{

  QString description = "Computing ";
  switch(variant)
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

  setDescription(description);
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::run()
{
  BlockTimer<std::chrono::milliseconds> timer{"SLIC"};
  emit progress(0);

  //Square tolerance to avoid having to calculate roots later
  if(tolerance>0) tolerance *= tolerance;

  //Initialize channel edges extension
  auto edgesExtension = retrieveOrCreateStackExtension<ChannelEdges>(m_stack, m_factory);
  auto sliceRegion = edgesExtension->sliceRegion(0);

  //const Bounds bounds = m_stack->bounds();
  OutputSPtr output = m_stack->output();
  spacing = output->spacing();

  //Calculate dimensions using voxels as unit
  max_x = bounds.lenght(Axis::X)/spacing[0];
  max_y = bounds.lenght(Axis::Y)/spacing[1];
  max_z = bounds.lenght(Axis::Z)/spacing[2];

  //TODO: Set up minimums and maximums if ROI enabled
  //max_x/y/z = min(ROI_max_x/y/z, max_x/y/z)
  //min_x/y/z = min(ROI_min_x/y/z, min_x/y/z)

  //Calculate total number of voxels
  const unsigned long int n_voxels = max_x*max_y*max_z;

  qDebug() << QString("Size: %1 %2 %3").arg(max_x).arg(max_y).arg(max_z);
  qDebug() << QString("Spatial: %1 - Color: %2 - Iterations: %3 - Tolerance: %4").arg(parameter_m_s).arg(parameter_m_c).arg(max_iterations).arg(tolerance);

  //Get ITK image from extended item
  auto inputVolume = readLockVolume(output);
  if (!inputVolume->isValid())
  {
    auto what    = QObject::tr("Invalid input volume");
    auto details = QObject::tr("StackSLIC::onComputeSLIC() ->Invalid input volume.");

    throw Utils::EspinaException(what, details);
  }
  auto image = inputVolume->itkImage();

  //Variables
  float spatial_distance, color_distance, distance, distance_old;
  ImageRegion region;
  IndexType cur_index;
  Label *label, *label_old;
  unsigned char center_color, voxel_color;
  bool converged = false, cropped = false;
  int progressValue = 0, current_slice = 0;
  unsigned int label_index = 0, voxel_index;
  long long int sum_x = 0, sum_y = 0, sum_z = 0;
  unsigned long long int sum_color = 0, sum_voxels = 0;
  unsigned int x, y, z;
  double newProgress = 0;
  int region_position[3];
  int region_size[3];

  //Find centers for the supervoxels
  QList<Label> labels;
  label_list = &labels;
  if(!initSupervoxels(image.GetPointer(), labels, edgesExtension.get()))
    return;
  qDebug() << QString("Created %1 labels").arg(labels.size());

  //Reserve enough memory for all voxels
  voxels = new unsigned int[n_voxels];
  std::memset(voxels, std::numeric_limits<unsigned int>::max(), n_voxels * sizeof(unsigned int));

  //Extract constants from the loops.
  scan_size = 2. * parameter_m_s;

  const double iterationPercentage = 100.0 / max_iterations;

  try
  {
    for(unsigned int iteration = 0; iteration<max_iterations && !converged; iteration++)
    {
      if(!canExecute())
      {
        delete[] voxels;
        return;
      }
      qDebug() << QString("Starting iteration: %1 - %2s").arg(iteration+1).arg(timer.elapsed()/1000);

      newProgress = iterationPercentage * iteration;
      if(newProgress != progressValue)
      {
        progressValue = newProgress;
        emit progress(progressValue);
      }

      watcher.setFuture(QtConcurrent::map(labels, std::bind(&SLICComputeTask::computeLabel, this, std::placeholders::_1, edgesExtension, image, &labels)));
      watcher.waitForFinished();
      /*if(concurrentMap == nullptr) {
        delete[] voxels;
        return;
      } else {
        concurrentMap = nullptr;
      }*/

      if(!canExecute())
      {
        delete[] voxels;
        return;
      }

      //If convergence test is enabled, assume it converged until proven otherwise
      if(tolerance > 0) converged = true;

      //Recalculate centers, check convergence
      qDebug() << "Recalculating centers";
      for(label_index=0;label_index<labels.size();label_index++)
      {
        if(!canExecute())
        {
          delete[] voxels;
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
        for (z = region_position[2]; z < region_size[2]; z++)
        {
          for (y = region_position[1]; y < region_size[1]; y++)
          {
            voxel_index = region_position[0] + (y * max_x) + (z * max_x * max_y);
            if(voxel_index > n_voxels)
            {
              qWarning() << "invalid index" << voxel_index;
              qWarning() << "region index" << region_position[0] << region_position[1] << region_position[2];
              qWarning() << "region size " << region_size[0] << region_size[1] << region_size[2];
              Q_ASSERT(false); // segmentation fault when indexing 'voxels' pointer.
            }
            for (x = region_position[0]; x < region_size[0]; x++)
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
        switch(variant)
        {
          case ASLIC:
            //Squared constants to prevent having to use expensive roots
            label->norm_quotient = pow(label->m_c,2)/pow(label->m_s,2);
            label->m_c = 1;
            label->m_s = 1.0;
            break;
          case SLICO:
            label->norm_quotient = pow(parameter_m_c,2)/pow(label->m_s,2);
            label->m_s = 1.0;
            break;
          case SLIC:
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
    return;
  }

  qDebug() << QString("Starting post-process connectivity algorithm: %1").arg(timer.elapsed()/1000);
  ensureConnectivity();

  if(!canExecute())
  {
    delete[] voxels;
    return;
  }

  qDebug() << QString("Finished computing SLIC in %1s").arg(timer.elapsed()/1000);

  qDebug() << "Generating and compressing results";

  //Compress and save results
  saveResults(labels, voxels);
  result->variant = variant;
  result->m_s = parameter_m_s;
  result->m_c = parameter_m_c;
  result->iterations = max_iterations;
  result->tolerance = (double) sqrt(tolerance);

  if(voxels != nullptr) delete[] voxels;
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::onAbort()
{
  watcher.cancel();
}

//-----------------------------------------------------------------------------
unsigned long int StackSLIC::getSupervoxel(unsigned int x, unsigned int y, unsigned int z)
{
  //TODO: Check if x,y,z is in bounds

  if(!result.computed)
    return 0;

  QDataStream stream(&result.voxels[z], QIODevice::ReadOnly);
  stream.setVersion(QDataStream::Qt_4_0);

  unsigned int current_x = 0;
  unsigned int current_y = 0;
  unsigned int label;
  unsigned short voxel_count;
  unsigned int max_x = result.bounds.lenght(Axis::X);

  while(current_y < y && current_x < x) {
    stream >> label;
    stream >> voxel_count;
    current_x += voxel_count;
    if(current_x >= max_x) {
      current_x = 0;
      current_y++;
    }
  }

  return label;
}

//-----------------------------------------------------------------------------
unsigned char StackSLIC::getSupervoxelColor(unsigned int supervoxel)
{
  if(!result.computed)
      return 0;

  return result.supervoxels[supervoxel].color;
}

//-----------------------------------------------------------------------------
itkVolumeType::IndexType StackSLIC::getSupervoxelCenter(unsigned int supervoxel)
{
  if(!result.computed)
      return {0,0,0};

  return result.supervoxels[supervoxel].center;
}

//-----------------------------------------------------------------------------
bool StackSLIC::drawVoxelCenters(unsigned int slice, vtkSmartPointer<vtkPoints> data)
{
  if(isRunning() && task->voxels != nullptr) {
    auto spacing = m_extendedItem->output()->spacing();
    data->Reset();
    for(auto superVoxel: (*task->label_list))
    {
      if(superVoxel.center.GetElement(2) == slice)
      {
        data->InsertNextPoint(superVoxel.center.GetElement(0) * spacing[0],
                              superVoxel.center.GetElement(1) * spacing[1],
                              superVoxel.center.GetElement(2) * spacing[2]);
      }
    }
    data->Modified();
    return true;
  }

  loadFromSnapshot();

  QReadLocker lock(&result.m_dataMutex);

  if(!result.computed) return false;

  auto spacing = m_extendedItem->output()->spacing();

  data->Reset();
  for(auto superVoxel: result.supervoxels)
  {
    if(superVoxel.center.GetElement(2) == slice)
    {
      data->InsertNextPoint(superVoxel.center.GetElement(0) * spacing[0],
                            superVoxel.center.GetElement(1) * spacing[1],
                            superVoxel.center.GetElement(2) * spacing[2]);
    }
  }
  data->Modified();

  return true;
}

//-----------------------------------------------------------------------------
bool StackSLIC::drawSliceInImageData(unsigned int slice, vtkSmartPointer<vtkImageData> data)
{
  if(isRunning() && task->voxels != nullptr) {
    Bounds bounds = m_extendedItem->bounds();
    OutputSPtr output = m_extendedItem->output();
    NmVector3 spacing = output->spacing();
    unsigned int max_x = bounds.lenght(Axis::X)/spacing[0];
    unsigned int max_y = bounds.lenght(Axis::Y)/spacing[1];
    unsigned int max_z = bounds.lenght(Axis::Z)/spacing[2];
    if(slice < 0 || slice >= max_z) return false;

    unsigned long long int pixel_count = max_x*max_y;
    unsigned long long int pixel = 0;

    data->SetExtent(0, max_x-1, 0, max_y-1, slice, slice);
    data->SetSpacing(spacing[0], spacing[1], spacing[2]);
    data->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    auto buffer = reinterpret_cast<unsigned char *>(data->GetScalarPointer());

    unsigned int label;
    unsigned char color;

    for(unsigned int i = pixel_count * slice; i < pixel_count * (slice+1); i++) {
      label = task->voxels[i];
      if(label == std::numeric_limits<unsigned int>::max())
        color = 0;
      else
        color = task->label_list->at(label).color;
      *buffer = color;
      ++buffer;
    }

    return true;
  }

  loadFromSnapshot();

  QReadLocker lock(&result.m_dataMutex);

  if(!result.computed || slice < 0 || slice >= result.slice_count) return false;

  Bounds bounds = m_extendedItem->bounds();
  OutputSPtr output = m_extendedItem->output();
  NmVector3 spacing = output->spacing();
  unsigned int max_x = bounds.lenght(Axis::X)/spacing[0];
  unsigned int max_y = bounds.lenght(Axis::Y)/spacing[1];
  unsigned long long int pixel_count = max_x*max_y;
  unsigned long long int pixel = 0;

  data->SetExtent(0, max_x-1, 0, max_y-1, slice, slice);
  data->SetSpacing(spacing[0], spacing[1], spacing[2]);
  data->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  auto buffer = reinterpret_cast<unsigned char *>(data->GetScalarPointer());

  QDataStream stream(&result.voxels[slice], QIODevice::ReadOnly);
  stream.setVersion(QDataStream::Qt_4_0);

  unsigned int label;
  unsigned short voxel_count;
  unsigned char color;

  while(pixel < pixel_count)
  {
    stream >> label;
    stream >> voxel_count;
    //Check for voxels out of edges (no label assigned)
    if(label == std::numeric_limits<unsigned int>::max())
      color = 0;
    else
      color = result.supervoxels[label].color;
    for(int i = 0; i < voxel_count; i++)
    {
      *buffer = color;
      ++buffer;
    }
    pixel += voxel_count;
  }

  return true;
}

//-----------------------------------------------------------------------------
bool StackSLIC::isComputed()
{
  return result.computed;
}

//-----------------------------------------------------------------------------
bool StackSLIC::isRunning()
{
  return task != nullptr && task->isRunning();
}

//-----------------------------------------------------------------------------
StackSLIC::SLICVariant StackSLIC::getVariant()
{
  loadFromSnapshot();
  return result.computed?result.variant:SLICVariant::SLIC;
}

//-----------------------------------------------------------------------------
unsigned char StackSLIC::getSupervoxelSize()
{
  loadFromSnapshot();
  return result.computed?result.m_s:10;
}

//-----------------------------------------------------------------------------
unsigned char StackSLIC::getColorWeight()
{
  loadFromSnapshot();
  return result.computed?result.m_c:20;
}

//-----------------------------------------------------------------------------
unsigned int StackSLIC::getIterations()
{
  loadFromSnapshot();
  return result.computed?result.iterations:10;
}

//-----------------------------------------------------------------------------
double StackSLIC::getTolerance()
{
  loadFromSnapshot();
  return result.computed?result.tolerance:0.0;
}

//-----------------------------------------------------------------------------
unsigned int StackSLIC::getSupervoxelCount()
{
  loadFromSnapshot();
  return result.computed?result.supervoxel_count:0;
}

//-----------------------------------------------------------------------------
double StackSLIC::getSliceSpacing()
{
  return m_extendedItem->output()->spacing()[2];
}

//-----------------------------------------------------------------------------
QString StackSLIC::snapshotName(const QString& file) const
{
  auto channelName = m_extendedItem->name();

  return QString("%1/%2/%3_%4").arg(Path())
                               .arg(type())
                               .arg(channelName.remove(' ').replace('.','_'))
                               .arg(file);
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
inline bool StackSLIC::SLICComputeTask::isInBounds(int x, int y, int z)
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
void StackSLIC::SLICComputeTask::saveResults(QList<Label> labels, unsigned int *voxels)
{
  unsigned int voxel_index = 0, current_label, label_index;
  unsigned short same_label_count = 0;
  Label *label;

  QWriteLocker lock(&result->m_dataMutex);

  result->slice_count = max_z;
  result->supervoxel_count = labels.size();
  result->supervoxels.clear();
  result->voxels.clear();

  for(label_index=0;label_index<labels.size();label_index++)
  {
    label=&labels[label_index];
    result->supervoxels.append({label->center, label->color});
  }
  labels.clear();

  for (unsigned int z = 0; z < max_z; z++)
  {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_0);

    for (unsigned int y = 0; y < max_y; y++)
    {
      voxel_index = y * max_x + z * max_x * max_y;
      for (unsigned int x = 0; x < max_x; x++)
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
    result->voxels.append(data);
  }

  result->computed = true;
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::findCandidateRegion(itkVolumeType::IndexType &center, double scan_size,  int region_position[], int region_size[])
{
  union vec4f {
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
  //QFutureSynchronizer<void> synchronizer;
  /*QList<IndexType> centers;

  for(unsigned int z=parameter_m_s/2+min_z;z<max_z;z+=parameter_m_s/spacing[2])
  {
    //auto sliceRegion = edgesExtension->sliceRegion(z);

    for(unsigned int y=parameter_m_s/2+min_y;y<max_y;y+=parameter_m_s/spacing[1])
    {
      for(unsigned int x=parameter_m_s/2+min_x;x<max_x;x+=parameter_m_s/spacing[0])
      {

        //Skip if out of the region of interest or image bounds
        if(!isInBounds(x, y, z)) continue;
        centers.append((IndexType) {x,y,z});
      }
    }
  }

  if(!canExecute()) return false;
  auto func = std::bind(&StackSLIC::SLICComputeTask::createSupervoxel, this, std::placeholders::_1, edgesExtension, image, &labels);
  watcher.setFuture(QtConcurrent::map(centers, func));
  watcher.waitForFinished();*/

  //synchronizer.waitForFinished();
  //synchronizer.clearFutures();


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

  for(unsigned int z=parameter_m_s/2+min_z;z<max_z;z+=parameter_m_s/spacing[2])
  {
    if(!canExecute()) return false;

    auto sliceRegion = edgesExtension->sliceRegion(z);

    for(unsigned int y=parameter_m_s/2+min_y;y<max_y;y+=parameter_m_s/spacing[1])
    {
      for(unsigned int x=parameter_m_s/2+min_x;x<max_x;x+=parameter_m_s/spacing[0])
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
          if(value < lowestGradient) {
            cur_index = it.GetIndex();
            lowestGradient = value;
            //qDebug() << QString("New value: %1 | Center/New: %2 %3 %4 -> %5 %6 %7")
            //                   .arg(value).arg(x).arg(y).arg(z).arg(cur_index[0]).arg(cur_index[1]).arg(cur_index[2]);
          }
          ++it;
        }

        labels.append((Label) {pow(parameter_m_c,2) / pow(parameter_m_s,2), 1.0, (unsigned int) labels.size(), cur_index, image->GetPixel(cur_index), 1 });
      }
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
float StackSLIC::SLICComputeTask::calculateDistance(IndexType &voxel_index, IndexType &center_index,
                                                   unsigned char voxel_color, unsigned char center_color, float norm_quotient, float *color_distance, float *spatial_distance, bool only_spatial)
{
  union vec4f {
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

  //if(!canExecute()) return;

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
  findCandidateRegion(label.center, scan_size, region_position, region_size);

  //Make sure that the region is inside the bounds of the image
  fitRegionToBounds(region_position, region_size);
  center_color = label.color;

  //Iterate over the slices
  for(current_slice = region_position[2]; current_slice < region_position[2] + region_size[2]; current_slice++)
  {
    //if(!canExecute()) return;

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
      //if(!canExecute()) return;

      cur_index = it.GetIndex();
      voxel_color = it.Get();
      distance = calculateDistance(cur_index, label.center, voxel_color, center_color, label.norm_quotient, &color_distance, &spatial_distance);
      voxel_index = cur_index[0]+cur_index[1]*max_x+cur_index[2]*max_x*max_y;
      label_old_index = voxels[voxel_index];
      if(label_old_index == std::numeric_limits<unsigned int>::max() || label_old_index == label.index) {
        //If voxel is unassigned or assigned to the current label, assume it's an infinite distance away
        distance_old = std::numeric_limits<double>::max();
      } else {
        label_old = &(*labels)[label_old_index];
        if(label_old == nullptr) {
          qDebug() << QString("Null label_old: %1/%2 labels %3").arg(label_old_index).arg((*labels).size());
          label_old = &(*labels)[label_old_index];
          qDebug() << QString("Label: %1").arg((*labels)[label_old_index].index);
        }
        distance_old = calculateDistance(cur_index, label_old->center, voxel_color, image->GetPixel(label_old->center), label_old->norm_quotient, nullptr, nullptr);
      }

      if(distance < distance_old) {
        QMutexLocker locker(&labelListMutex);
        if(label_old_index != voxels[voxel_index]) {
          label_old = &(*labels)[voxels[voxel_index]];
          distance_old = calculateDistance(cur_index, label_old->center, voxel_color, image->GetPixel(label_old->center), label_old->norm_quotient, nullptr, nullptr);
        }
        if(distance < distance_old) {
          voxels[voxel_index] = label.index;
          //Update maximum distances
          switch(variant)
          {
            case ASLIC:
              if(label.m_c < color_distance)
                label.m_c = color_distance;
            //no break
            case SLICO:
            case SLIC:
              if(label.m_s < spatial_distance)
                label.m_s = spatial_distance;
              break;
          }
        }
      }

      ++it;
    } //RegionIterator
  }//current_slice

  //if(!canExecute()) return;

  switch(variant)
  {
    case ASLIC:
      //Update max. distances calculating their roots as we were
      //using squared distances for the simplified equations
      label.m_c = std::sqrt(label.m_c);
    //no break
    case SLICO:
    case SLIC:
      label.m_s = std::sqrt(label.m_s);
      break;
  }

}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::createSupervoxel(IndexType cur_index, ChannelEdges *edgesExtension, itkVolumeType *image, QList<Label> *labels) {
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
    if(value < lowestGradient) {
      cur_index = it.GetIndex();
      lowestGradient = value;
      //qDebug() << QString("New value: %1 | Center/New: %2 %3 %4 -> %5 %6 %7")
      //                   .arg(value).arg(x).arg(y).arg(z).arg(cur_index[0]).arg(cur_index[1]).arg(cur_index[2]);
    }
    ++it;
  }
  {
    QMutexLocker locker(&labelListMutex);
    labels->append((Label) {pow(parameter_m_c,2) / pow(parameter_m_s,2), 1.0, (unsigned int) labels->size(), cur_index, image->GetPixel(cur_index), 1 });
  }
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::ensureConnectivity() {
  //watcher.setFuture(QtConcurrent::map(*label_list, std::bind(&SLICComputeTask::labelConnectivity, this, std::placeholders::_1)));
  //watcher.waitForFinished();
  int size = (*label_list).size();
  for(int label_index=0;label_index<size;label_index++){
    labelConnectivity((*label_list)[label_index]);
  }
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::labelConnectivity(Label &label) {

  const int slice_area = max_x * max_y;
  int distance = label.m_s;
  int range = distance*2+1;
  IndexType center = label.center;
  IndexType current = center;
  IndexType contiguous[6];
  long long int region[3] = {center[0] - distance, center[1] - distance, center[2] - distance};
  long long int length[3] = {range, range, range};
  fitRegionToBounds(region, length);
  long long int offsetImage, offsetRegion;
  const int size = length[0] * length[1] * length[2];
  bool visited[size] = { false };
  std::queue<IndexType> search_queue;

  search_queue.push(current);
  offsetRegion = current[0] - region[0] + (current[1]-region[1]) * length[0] + (current[2] - region[2])*length[0]*length[1];
  offsetImage = current[0] + current[1]*max_x + current[2]*slice_area;
  if(voxels[offsetImage] != label.index) {
    //qDebug() << QString("Label %1 - Center not inside supervoxel!").arg(label.index);
    return;
  }
  if(offsetRegion > size)
    qDebug() << QString("Error calculating offset!\n");
  visited[offsetRegion] = true;

  while(!search_queue.empty()) {

    current = search_queue.front();
    search_queue.pop();
    offsetImage = current[0] + current[1]*max_x + current[2]*slice_area;

    contiguous[0] = {current[0]+1, current[1], current[2]};
    contiguous[1] = {current[0]-1, current[1], current[2]};
    contiguous[2] = {current[0], current[1]+1, current[2]};
    contiguous[3] = {current[0], current[1]-1, current[2]};
    contiguous[4] = {current[0], current[1], current[2]+1};
    contiguous[5] = {current[0], current[1], current[2]-1};
    for(IndexType next : contiguous) {
      if(isInBounds(next[0], next[1], next[2])) {
        //Skip voxels outside the valid subregion
        if(next[0] < region[0] || next[0] > region[0] + length[0] ||
            next[1] < region[1] || next[1] > region[1] + length[1] ||
            next[2] < region[2] || next[2] > region[2] + length[2])
          continue;
        offsetRegion = current[0] - region[0] + (current[1]-region[1]) * length[0] + (current[2] - region[2])*length[0]*length[1];
        if(offsetRegion > size)
            qDebug() << QString("Error calculating offset!\n");
        if(!visited[offsetRegion]) {
          offsetImage = next[0] + next[1]*max_x + next[2]*slice_area;
          if(voxels[offsetImage] == label.index) {
            search_queue.push(next);
          }
          visited[offsetRegion] = true;
        }
      }
    }

  }

  std::vector<unsigned int> adjacent_labels;
  int unconnected_count = 0;
  for(int i = 0, mod = 0; i < size; i++) {
    if(visited[i])
      continue;
    int z = i / (length[0] * length[1]);
    int r = i - z*length[0]*length[1];
    int y = r / length[0];
    int x = r - y * length[0];
    offsetImage = x + y*max_x + z*slice_area;
    if(voxels[offsetImage] == label.index) {
      int adjacentOffset = offsetImage+1;
      if(adjacentOffset >= 0 && adjacentOffset < (*label_list).size() && voxels[adjacentOffset] != label.index)
        adjacent_labels.push_back(voxels[adjacentOffset]);
      adjacentOffset = offsetImage-1;
      if(adjacentOffset >= 0 && adjacentOffset < (*label_list).size() && voxels[adjacentOffset] != label.index)
        adjacent_labels.push_back(voxels[adjacentOffset]);
      adjacentOffset = offsetImage+max_x;
      if(adjacentOffset >= 0 && adjacentOffset < (*label_list).size() && voxels[adjacentOffset] != label.index)
        adjacent_labels.push_back(voxels[adjacentOffset]);
      adjacentOffset = offsetImage-max_x;
      if(adjacentOffset >= 0 && adjacentOffset < (*label_list).size() && voxels[adjacentOffset] != label.index)
        adjacent_labels.push_back(voxels[adjacentOffset]);
      adjacentOffset = offsetImage+max_y;
      if(adjacentOffset >= 0 && adjacentOffset < (*label_list).size() && voxels[adjacentOffset] != label.index)
        adjacent_labels.push_back(voxels[adjacentOffset]);
      adjacentOffset = offsetImage-max_y;
      if(adjacentOffset >= 0 && adjacentOffset < (*label_list).size() && voxels[adjacentOffset] != label.index)
        adjacent_labels.push_back(voxels[adjacentOffset]);

      std::sort(adjacent_labels.begin(), adjacent_labels.end());
      int max_occurrences = 0;
      int occurrences = 1;
      unsigned int l = label.index;
      for(int j = 0; j < adjacent_labels.size()-1; j++) {
        if(adjacent_labels[j] == adjacent_labels[j+1]) {
          occurrences ++;
          if(occurrences > max_occurrences) {
            max_occurrences = occurrences;
            l = adjacent_labels[j];
          }
        } else
          occurrences = 1;
      }
      //TODO: Add critical section for multithreaded writes to voxels[]
      voxels[offsetImage] = l;
      unconnected_count++;
    }
  }

  if(unconnected_count>0)
    qDebug() << QString("Label %1 - %2 unconnected voxels").arg(label.index).arg(unconnected_count);
}
