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

//ITK
#include <itkImage.hxx>
#include <itkEuclideanDistanceMetric.hxx>
#include <itkImageConstIteratorWithIndex.hxx>
#include <itkImageRegion.hxx>
#include <vtkUnsignedCharArray.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include "itkGradientMagnitudeImageFilter.h"

// C++
#include <limits>
#include <math.h>
#include <memory>

// Qt
#include <QDataStream>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Extensions;

const StackExtension::Type StackSLIC::TYPE = "StackSLIC";
const QString StackSLIC::VOXELS_FILE = "voxels.slic";
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
  qDebug() << QString("%1").arg(result.supervoxel_count);

  result.slice_offset = nullptr;
}

//-----------------------------------------------------------------------------
StackSLIC::~StackSLIC()
{
  if(task != nullptr)
  {
      if(task->isRunning())
        task->abort();
  }
  if(result.slice_offset != nullptr)
      delete result.slice_offset;
}

//-----------------------------------------------------------------------------
State StackSLIC::state() const
{
  //TODO: Save:
  //- Parameters used for the last computation
  return State();
}

//-----------------------------------------------------------------------------
Snapshot StackSLIC::snapshot() const
{
  if(!result.computed)
    return Snapshot();

  QReadLocker lock(&result.m_dataMutex);

  Snapshot snapshot;

  auto voxelsName = snapshotName(VOXELS_FILE);
  auto dataName = snapshotName(DATA_FILE);
  auto labelsName = snapshotName(LABELS_FILE);
  snapshot << SnapshotData(voxelsName, result.voxels);

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
  for(unsigned int i = 0; i < result.slice_count; i++)
  {
    stream << result.slice_offset[i];
  }
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

  auto voxelsName = snapshotName(VOXELS_FILE);
  auto dataName = snapshotName(DATA_FILE);
  auto labelsName = snapshotName(LABELS_FILE);
  QFileInfo voxelsFileInfo = m_extendedItem->storage()->absoluteFilePath(voxelsName);
  QFileInfo dataFileInfo = m_extendedItem->storage()->absoluteFilePath(dataName);
  QFileInfo labelsFileInfo = m_extendedItem->storage()->absoluteFilePath(labelsName);

  if(!voxelsFileInfo.exists() || !dataFileInfo.exists() || !labelsFileInfo.exists())
    return false;

  QFile voxelsFile(voxelsFileInfo.absoluteFilePath());
  QFile labelsFile(labelsFileInfo.absoluteFilePath());
  QFile dataFile(dataFileInfo.absoluteFilePath());
  if(!voxelsFile.open(QIODevice::ReadOnly) || !labelsFile.open(QIODevice::ReadOnly) || !dataFile.open(QIODevice::ReadOnly))
    return false;

  result.voxels = voxelsFile.readAll();

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
  if(result.slice_offset!=nullptr)
  {
    delete result.slice_offset;
  }
  result.slice_offset = (unsigned int*) malloc(result.slice_count * sizeof(unsigned int));
  for(unsigned int i = 0; i < result.slice_count; i++)
  {
    stream >> result.slice_offset[i];
  }

  int variant;
  stream >> variant;
  result.variant = (StackSLIC::SLICVariant) variant;
  stream >> result.m_s;
  stream >> result.m_c;
  stream >> result.iterations;
  stream >> result.tolerance;

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
, bounds(stack->bounds())
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

  //Typedefs for clearer syntax
  using ImageType = itk::Image<unsigned char, 3>;
  using IndexType = ImageType::IndexType;
  typedef StackSLIC::Label Label;
  typedef itk::ImageRegionConstIteratorWithIndex<ImageType> RegionIterator;
  typedef itk::ImageRegion<3> ImageRegion;

  //Used to avoid dividing when switching from grayscale space (0-255)
  //to CIELab intensity (0-100)
  const double color_normalization_constant = 100.0/255.0;
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
  double spatial_distance, color_distance, distance, distance_old;
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
  if(!initSupervoxels(image.GetPointer(), labels, edgesExtension.get()))
    return;
  qDebug() << QString("Created %1 labels").arg(labels.size());

  //Reserve enough memory for all voxels
  voxels = new unsigned int[n_voxels];
  std::memset(voxels, std::numeric_limits<unsigned int>::max(), n_voxels * sizeof(unsigned int));

  //Extract constants from the loops.
  const double scan_size = 2. * parameter_m_s;

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
      qDebug() << QString("Starting iteration: %1").arg(iteration+1);

      newProgress = iterationPercentage * iteration;
      if(newProgress != progressValue)
      {
        progressValue = newProgress;
        emit progress(progressValue);
      }

      for(label_index=0;label_index<labels.size();label_index++)
      {
        if(!canExecute())
        {
          delete[] voxels;
          return;
        }

        if((label_index & (unsigned int) 4095) == 0) {
          newProgress = iterationPercentage * iteration;
          newProgress += iterationPercentage * (double) label_index / (double) labels.size();
          if(newProgress != progressValue)
          {
            progressValue = newProgress;
            emit progress(progressValue);
          }
        }
        label=&labels[label_index];

        //Find the region of voxels that are in range of this supervoxel
        findCandidateRegion(label->center, scan_size, region_position, region_size);

        //Make sure that the region is inside the bounds of the image
        fitRegionToBounds(region_position, region_size);

        center_color = label->color;

        //Iterate over the slices
        for(current_slice = region_position[2]; current_slice < region_position[2] + region_size[2]; current_slice++)
        {
          if(!canExecute())
          {
            delete[] voxels;
            return;
          }

          //Get slice edges
          sliceRegion = edgesExtension->sliceRegion(current_slice);
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

            //Calculate distance between current voxel and supervoxel center
            color_distance = voxel_color-center_color;
            color_distance *= color_normalization_constant;
            color_distance *= color_distance;
            spatial_distance = pow((cur_index[0]-label->center[0])*spacing[0],2) + pow((cur_index[1]-label->center[1])*spacing[1],2) + pow((cur_index[2]-label->center[2])*spacing[2],2);
            distance = color_distance + label->norm_quotient * spatial_distance;

            //Check if voxel is closer to this supervoxel than to its current paired supervoxel
            voxel_index = cur_index[0]+cur_index[1]*max_x+cur_index[2]*max_x*max_y;
            if(voxels[voxel_index] == std::numeric_limits<unsigned int>::max() || voxels[voxel_index] == label_index) {
              //If voxel is unassigned or assigned to the current label, assume it's an infinite distance away
              distance_old = std::numeric_limits<double>::max();
            } else {
              label_old = &labels[voxels[voxel_index]];
              distance_old = pow((voxel_color - image->GetPixel(label_old->center))*color_normalization_constant,2) + label_old->norm_quotient *
                            (pow((cur_index[0]-label_old->center[0])*spacing[0],2) + pow((cur_index[1]-label_old->center[1])*spacing[1],2) + pow((cur_index[2]-label_old->center[2])*spacing[2],2));
            }
            if(distance < distance_old) {
              voxels[voxel_index] = label_index;
              //Update maximum distances
              switch(variant)
              {
                case ASLIC:
                  if(label->m_c < color_distance)
                    label->m_c = color_distance;
                //no break
                case SLICO:
                  if(label->m_s < spatial_distance)
                    label->m_s = spatial_distance;
                //no break
                case SLIC:
                  break;
              }
            }

            ++it;
          } //RegionIterator
        }//current_slice

        switch(variant)
        {
          case ASLIC:
            //Update max. distances calculating their roots as we were
            //using squared distances for the simplified equations
            label->m_c = std::sqrt(label->m_c);
          //no break
          case SLICO:
            label->m_s = std::sqrt(label->m_s);
          //no break
          case SLIC:
            break;
        }

      } //label

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

        region_size[0] = scan_size / spacing[0];
        region_size[1] = scan_size / spacing[1];
        region_size[2] = scan_size / spacing[2];
        region_position[0] = label->center[0] - region_size[0]/2;
        region_position[1] = label->center[1] - region_size[1]/2;
        region_position[2] = label->center[2] - region_size[2]/2;

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
              if (label_index == voxels[voxel_index])
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
            spatial_distance = pow((label->center[0]-sum_x)*spacing[0],2)+pow((label->center[1]-sum_y)*spacing[1],2)+pow((label->center[2]-sum_z)*spacing[2],2);
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

      qDebug() << QString("Finishing iteration: %1").arg(iteration+1);
    } //iteration
  }
  catch(std::exception &e)
  {
     std::cerr << e.what() << std::endl << std::flush;
     exit(1);
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

}

//-----------------------------------------------------------------------------
unsigned long int StackSLIC::getSupervoxel(unsigned int x, unsigned int y, unsigned int z)
{
  //TODO: Check if x,y,z is in bounds

  if(!result.computed)
    return 0;

  QDataStream stream(&result.voxels, QIODevice::ReadOnly);
  stream.setVersion(QDataStream::Qt_4_0);
  if(stream.skipRawData(result.slice_offset[z]) != result.slice_offset[z]) {
    return 0;
  }

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
itk::Image<unsigned char, 3>::IndexType StackSLIC::getSupervoxelCenter(unsigned int supervoxel)
{
  if(!result.computed)
      return {0,0,0};

  return result.supervoxels[supervoxel].center;
}

//-----------------------------------------------------------------------------
bool StackSLIC::drawVoxelCenters(unsigned int slice, vtkSmartPointer<vtkPoints> data)
{
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
  loadFromSnapshot();

  QReadLocker lock(&result.m_dataMutex);

  if(!result.computed) return false;

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

  QDataStream stream(&result.voxels, QIODevice::ReadOnly);
  stream.setVersion(QDataStream::Qt_4_0);
  if(stream.skipRawData(result.slice_offset[slice]) != result.slice_offset[slice]) {
    return false;
  }

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
bool StackSLIC::SLICComputeTask::isInBounds(int x, int y, int z)
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


  if(result->slice_offset != nullptr)
  {
    delete result->slice_offset;
  }
  result->slice_offset = (unsigned int*) malloc(max_z * sizeof(unsigned int));

  QDataStream stream(&result->voxels, QIODevice::WriteOnly);
  stream.setVersion(QDataStream::Qt_4_0);

  for (unsigned int z = 0; z < max_z; z++)
  {
    result->slice_offset[z] = result->voxels.size();
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
  }

  result->computed = true;
}

//-----------------------------------------------------------------------------
void StackSLIC::SLICComputeTask::findCandidateRegion(itk::Image<unsigned char, 3>::IndexType &center, double scan_size,  int region_position[], int region_size[])
{
  region_size[0] = scan_size / spacing[0];
  region_size[1] = scan_size / spacing[1];
  region_size[2] = scan_size / spacing[2];
  region_position[0] = center[0] - region_size[0]/2;
  region_position[1] = center[1] - region_size[1]/2;
  region_position[2] = center[2] - region_size[2]/2;
}

//-----------------------------------------------------------------------------
bool StackSLIC::SLICComputeTask::initSupervoxels(itk::Image<unsigned char, 3> *image, QList<Label> &labels, ChannelEdges *edgesExtension)
{
  using ImageType = itk::Image<unsigned char, 3>;
  using IndexType = ImageType::IndexType;
  using GradientType = itk::Image<float,3>;
  typedef itk::ImageRegion<3> ImageRegion;
  typedef itk::GradientMagnitudeImageFilter<ImageType, GradientType>  GradientFilterType;
  typedef itk::ImageRegionConstIteratorWithIndex<GradientType> GradientRegionIterator;

  ImageRegion region;
  IndexType cur_index;
  int region_position[3];
  int region_size[3];

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
        GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
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

        labels.append((Label) {pow(parameter_m_c,2) / pow(parameter_m_s,2), 1.0, cur_index, image->GetPixel(cur_index), 1 });
      }
    }
  }
  return true;
}
