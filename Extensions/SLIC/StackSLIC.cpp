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
#include "itkImage.h"
#include <itkEuclideanDistanceMetric.h>
#include "itkImageConstIteratorWithIndex.h"
#include "itkImageRegion.h"

#include <limits>
#include <math.h>
#include <memory>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Extensions;

const StackExtension::Type StackSLIC::TYPE = "Stack SLIC";

//--------------------------------------------------------------------
StackSLIC::StackSLIC(SchedulerSPtr scheduler, CoreFactory* factory, const InfoCache &cache)
: StackExtension{cache}
, m_scheduler{scheduler}
, m_factory  {factory}
, task(NULL)
{
  //Bounds bounds = m_extendedItem->bounds();
  //qDebug() << bounds.toString();
  qDebug() << "Constructor StackSLIC";
  qDebug() << QString("%1").arg(result.supervoxel_count);

  result.slice_offset = NULL;
}

//--------------------------------------------------------------------
StackSLIC::~StackSLIC()
{
  if(task != NULL) {
      if(task->isRunning())
        task->abort();
  }
  if(result.slice_offset != NULL)
      delete result.slice_offset;
}

//--------------------------------------------------------------------
State StackSLIC::state() const
{
  //Save:
  //- Parameters used for the last computation
  return State();
}

//--------------------------------------------------------------------
Snapshot StackSLIC::snapshot() const
{
  qDebug() << "Snapshot";
  return Snapshot();
}

//--------------------------------------------------------------------
StackExtension::InformationKeyList StackSLIC::availableInformation() const
{
  return InformationKeyList();
}

//--------------------------------------------------------------------
void StackSLIC::onComputeSLIC(unsigned char parameter_m_s, unsigned char parameter_m_c, Extensions::StackSLIC::SLICVariant variant, unsigned int max_iterations, double tolerance)
{
  if(task != NULL)
    return;
  /*auto */task = std::make_shared<SLICComputeTask>(m_extendedItem, m_scheduler, m_factory, &result, parameter_m_s, parameter_m_c, variant, max_iterations, tolerance);

  connect(task.get(), SIGNAL(finished()), this, SLOT(onSLICComputed()));

  Task::submit(task);
}

//--------------------------------------------------------------------
void StackSLIC::onSLICComputed()
{
  qDebug() << "finished";
  if(task != NULL) {
    disconnect(task.get(), SIGNAL(finished()), this, SLOT(onSLICComputed()));
    task = NULL;
  }
}

void StackSLIC::onAbortSLIC() {
  if(task != NULL) {
    if(task->isRunning())
      task->abort();
    disconnect(task.get(), SIGNAL(finished()), this, SLOT(onSLICComputed()));
    task = NULL;
  }
}


//--------------------------------------------------------------------
StackSLIC::SLICComputeTask::SLICComputeTask(ChannelPtr stack, SchedulerSPtr scheduler, CoreFactory *factory, SLICResult *result, unsigned int parameter_m_s, unsigned int parameter_m_c, SLICVariant variant, unsigned int max_iterations, double tolerance)
: Task(scheduler)
, m_stack{stack}
, m_factory{factory}
, parameter_m_s(parameter_m_s)
, parameter_m_c(parameter_m_c)
, variant(variant)
, max_iterations(max_iterations)
, tolerance(tolerance)
, voxels(NULL)
, result(result)
{
}

//--------------------------------------------------------------------
void StackSLIC::SLICComputeTask::run()
{
  BlockTimer<std::chrono::milliseconds> timer{"SLIC"};

  typedef StackSLIC::Label Label;

  auto edgesExtension = retrieveOrCreateStackExtension<ChannelEdges>(m_stack, m_factory);
  auto sliceRegion = edgesExtension->sliceRegion(0);


  Bounds bounds = m_stack->bounds();
  OutputSPtr output = m_stack->output();
  NmVector3 spacing = output->spacing();
  //Calculate dimensions using voxels as unit
  unsigned int max_x = bounds.lenght(Axis::X)/spacing[0];
  unsigned int max_y = bounds.lenght(Axis::Y)/spacing[1];
  unsigned int max_z = bounds.lenght(Axis::Z)/spacing[2];
  //Calculate total number of voxels
  unsigned long int n_voxels = max_x*max_y*max_z;
  unsigned long int voxel_index;

  //Used to avoid dividing when switching from grayscale space (0-255)
  //to CIELab intensity (0-100)
  double color_normalization_constant = 100/255;

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

  //Testing parameters
  /*parameter_m_s = 10;
  parameter_m_c = 20;
  variant = ASLIC;
  max_iterations = 10;
  tolerance = 0;*/

  //Square tolerance to avoid having to calculate roots later
  if(tolerance>0) tolerance *= tolerance;

  QList<Label> labels;
  //unsigned long int *voxels = (unsigned long int*) calloc(n_voxels, sizeof(unsigned long int));
  voxels = (unsigned int*) malloc(n_voxels * sizeof(unsigned int));
  memset(voxels, std::numeric_limits<unsigned int>::max(), n_voxels);
  auto image = inputVolume->itkImage();
  typedef itk::Image<unsigned char, 3> ImageType;
  typedef ImageType::IndexType IndexType;

  //Find centers for the supervoxels
  IndexType cur_index;
  for(unsigned int z=parameter_m_s/2;z<max_z;z+=parameter_m_s)
  {
    sliceRegion = edgesExtension->sliceRegion(z);
    /*top_left = sliceRegion.GetIndex();
    size = sliceRegion.GetSize();*/
    for(unsigned int y=parameter_m_s/2;y<max_y;y+=parameter_m_s)
    {
      for(unsigned int x=parameter_m_s/2;x<max_x;x+=parameter_m_s)
      {
        //Check if inside bounds using ChannelEdges, else skip this label
        cur_index[0] = x;
        cur_index[1] = y;
        cur_index[2] = z;

        //Don't create supervoxel centers outside of the calculated edges
        if(!sliceRegion.IsInside(cur_index)) {
          continue;
        }
        //TODO: Check lowest gradient voxel in a 3x3x3 area around voxel

        //Following code discarded as SLIC's original implementation doesn't set this up, it is done during the first iteration:
        //Assign voxels in a m_s*m_s*m_s area to this supervoxel to start a grid in the first iteration
        //Attempt to prevent first iteration max distance issues, not working
        /*for(int it_x=x-parameter_m_s*2;it_x<x+parameter_m_s&&it_x<=max_x;it_x++)
          for(int it_y=y-parameter_m_s*2;it_y<y+parameter_m_s&&it_y<=max_y;it_y++)
            for(int it_z=z-parameter_m_s*2;it_z<z+parameter_m_s&&it_z<=max_z;it_z++) {
              if(it_x<0||it_y<0||it_z<0)
                continue;
              long long int v_index = it_x+it_y*max_x+it_z*max_x*max_y;
              if(voxels[v_index] == ULONG_MAX)
                voxels[v_index] = labels.size();
            }*/
        cur_index = {x,y,z};
        //labels.append((Label) {{x,y,z}, image->GetPixel(cur_index), parameter_m_c, parameter_m_s, pow(parameter_m_c,2) / pow(parameter_m_s,2)});
        labels.append((Label) {pow(parameter_m_c,2) / pow(parameter_m_s,2), parameter_m_s, {x,y,z}, image->GetPixel(cur_index), parameter_m_c });
      }
    }
  }
  qDebug() << QString("Created %1 labels").arg(labels.size());

  typedef itk::ImageRegionConstIteratorWithIndex<ImageType> RegionIterator;
  typedef itk::ImageRegion<3> ImageRegion;
  //ImageRegion::SizeType size;
  double region_size, spatial_distance, color_distance, distance, distance_old;
  ImageRegion region;
  Label *label, *label_old;
  unsigned char center_color, voxel_color;
  bool converged = false, cropped = false;
  qDebug() << "start: max iterations" << max_iterations;
  int progress = 0;
  try
  {
    for(int iteration = 0; iteration<max_iterations && !converged; iteration++)
    {
      auto newProgress = (iteration * 100.0) / max_iterations;
      if(newProgress != progress) qDebug() << "progress" << newProgress;
      progress = newProgress;

      int region_x, region_y, region_z, region_size_x, region_size_y, region_size_z;
      for(unsigned int label_index=0;label_index<labels.size();label_index++)
      {
        //qDebug() << QString("Starting iteration: %1").arg(iteration);
        if(label_index%5000 == 0)
          qDebug() << QString("Label %1: %2s").arg(label_index).arg(timer.elapsed()/1000);
        label=&labels[label_index];
        //Set the voxel region to iterate
        region_size = 2*parameter_m_s;
        /*if(label_index == 0) {
          qDebug() << QString("M_S: %1").arg(label->m_s);
          qDebug() << QString("M_C: %1").arg(label->m_c);
        }*/
        region_size_x = region_size_y = region_size_z = region_size;
        region_x = label->center[0] - parameter_m_s;
        region_y = label->center[1] - parameter_m_s;
        region_z = label->center[2] - parameter_m_s;
        //Make sure that the region is inside the bounds of the image
        if(region_x<0)
        {
          region_size_x+=region_x; //Substract from size by adding a negative value
          region_x=0;
        }
        if(region_y<0)
        {
          region_size_y+=region_y;
          region_y=0;
        }
        if(region_z<0)
        {
          region_size_z+=region_z;
          region_z=0;
        }
        if(region_x+region_size_x > max_x) region_size_x = max_x-region_x;
        if(region_y+region_size_y > max_y) region_size_y = max_y-region_y;
        if(region_z+region_size_z > max_z) region_size_z = max_z-region_z;
        /*region.SetIndex(0, region_x);
        region.SetIndex(1, region_y);
        region.SetIndex(2, region_z);
        region.SetSize(0, region_size_x);
        region.SetSize(1, region_size_y);
        region.SetSize(2, region_size_z);*/

        switch(variant) {
          case ASLIC:
            //label->norm_quotient = label->m_c/(label->m_s<1?1:label->m_s);
            //Squared constants to prevent having to use expensive roots
            label->norm_quotient = pow(label->m_c,2)/(label->m_s<1?1:pow(label->m_s,2));
            //First iteration keeps original parameter values to avoid overextending
            if(iteration == 0) {
              label->m_s = parameter_m_s;
              label->m_c = parameter_m_c;
            } else {
              label->m_s = 0;
              label->m_c = 0;
            }
            break;
          case SLICO:
            //label->norm_quotient = label->m_c/(label->m_s<1?1:label->m_s);
            label->norm_quotient = pow(label->m_c,2)/(label->m_s<1?1:pow(label->m_s,2));
          //no break
          case SLIC:
            if(iteration == 0) {
              label->m_s = parameter_m_s;
            } else {
              label->m_s = 0;
            }
            break;
        }

        //center_color = image->GetPixel(label->center);
        center_color = label->color;

        //Iterate over the slices
        for(int current_slice = region_z; current_slice < region_z + region_size_z; current_slice++) {
          //Get slice edges
          sliceRegion = edgesExtension->sliceRegion(current_slice);
          //qDebug() << QString("Edge: %1 %2 %3 / %4 %5 %6").arg(sliceRegion.GetIndex()[0]).arg(sliceRegion.GetIndex()[1]).arg(sliceRegion.GetIndex()[2])
          //                                                .arg(sliceRegion.GetSize()[0]).arg(sliceRegion.GetSize()[1]).arg(sliceRegion.GetSize()[2]);

          //Set region bounds around center for current slice
          region.SetIndex(0, region_x);
          region.SetIndex(1, region_y);
          region.SetIndex(2, current_slice);
          region.SetSize(0, region_size_x);
          region.SetSize(1, region_size_y);
          region.SetSize(2, 1);

          //Crop to region inside edges
          cropped = region.Crop(sliceRegion);

          //If the region was completely outside the edges, skip to next slice
          //This mean either the stack was displaced too much or the z index
          //was too high/low
          if(!cropped)
            continue;

          /*qDebug() << QString("Edge: %1 %2 %3 / %4 %5 %6").arg(region.GetIndex()[0]).arg(region.GetIndex()[1]).arg(region.GetIndex()[2])
                                                                    .arg(region.GetSize()[0]).arg(region.GetSize()[1]).arg(region.GetSize()[2]);*/

          image->SetRequestedRegion(region);
          RegionIterator it(image, image->GetRequestedRegion());
          //qDebug() << QString("Created RegionIterator");
          //RegionIterator it(image, region);
          it.GoToBegin();
          //qDebug() << QString("Starting RegionIterator");
          while(!it.IsAtEnd())
          {
            cur_index = it.GetIndex();
            voxel_color = it.Get();

            //Calculate distance between current voxel and supervoxel center
            color_distance = voxel_color-center_color;
            color_distance *= color_normalization_constant;
            color_distance *= color_distance;
            spatial_distance = pow(cur_index[0]-label->center[0],2) + pow(cur_index[1]-label->center[1],2) + pow(cur_index[2]-label->center[2],2);
            distance = color_distance + label->norm_quotient * spatial_distance;

            //Check if voxel is closer to this supervoxel than to its current paired supervoxel
            voxel_index = cur_index[0]+cur_index[1]*max_x+cur_index[2]*max_x*max_y;
            if(voxels[voxel_index] == std::numeric_limits<unsigned int>::max() || voxels[voxel_index] == label_index) {
              //First iteration (voxels not assigned yet) or voxel belongs to label already
              distance_old = std::numeric_limits<double>::max();
            } else {
              label_old = &labels[voxels[voxel_index]];
              distance_old = pow((voxel_color - image->GetPixel(label_old->center))*color_normalization_constant,2) + label_old->norm_quotient *
                            (pow(cur_index[0]-label_old->center[0],2) + pow(cur_index[1]-label_old->center[1],2) + pow(cur_index[2]-label_old->center[2],2));
            }
            if(distance < distance_old) {
              //if(iteration > 0 && voxels[voxel_index] != label_index) qDebug() << QString("Old: %1 New: %2").arg(distance_old).arg(distance);
              voxels[voxel_index] = label_index;
            }

            //Update maximum distances if voxel is asigned to label
            //Skip in first iteration to avoid overextending to previously unassigned voxels
            if(iteration > 0 && label_index == voxels[voxel_index]) {
              switch(variant) {
                case ASLIC:
                  if(label->m_c < color_distance)
                    label->m_c = color_distance;
                //no break
                case SLICO:
                case SLIC: //update m_s in SLIC to use when recalculating centers
                  if(label->m_s < spatial_distance)
                    label->m_s = spatial_distance;
                  break;
              }
            }

            ++it;
          } //RegionIterator
        }//current_slice

        if(iteration > 0) {
          switch(variant) {
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
        }
      } //label

      //If convergence test is enabled, assume it converged until proven otherwise
      if(tolerance > 0)
        converged = true;

      //Recalculate centers, check convergence
      qDebug() << "Recalculating centers";
      for(unsigned int label_index=0;label_index<labels.size();label_index++)
      {
        if(label_index%5000 == 0)
          qDebug() << QString("Label %1: %2s").arg(label_index).arg(timer.elapsed()/1000);
        label=&labels[label_index];
        //region_size = 2.5*label->m_s;
        region_size = 2*parameter_m_s;
        region_size_x = region_size_y = region_size_z = region_size;
        region_x = label->center[0] - parameter_m_s;
        region_y = label->center[1] - parameter_m_s;
        region_z = label->center[2] - parameter_m_s;
        if(region_x<0)
        {
          region_size_x+=region_x;
          region_x=0;
        }
        if(region_y<0)
        {
          region_size_y+=region_y;
          region_y=0;
        }
        if(region_z<0)
        {
          region_size_z+=region_z;
          region_z=0;
        }
        if(region_x+region_size_x > max_x) region_size_x = max_x-region_x;
        if(region_y+region_size_y > max_y) region_size_y = max_y-region_y;
        if(region_z+region_size_z > max_z) region_size_z = max_z-region_z;

        region_size_x += region_x;
        region_size_y += region_y;
        region_size_z += region_z;

        long long int sum_x = 0, sum_y = 0, sum_z = 0;
        unsigned long long int sum_color = 0, sum_voxels = 0;
        for(unsigned int z = region_z; z < region_size_z; z++) {
          for(unsigned int y = region_y; y < region_size_y; y++) {
            voxel_index = region_x-1+y*max_x+z*max_x*max_y;
            for(unsigned int x = region_x; x < region_size_x; x++) {
              //voxel_index = x+y*max_x+z*max_x*max_y;
              voxel_index++;
              if(label_index == voxels[voxel_index]) {
                sum_voxels++;
                sum_x += x;
                sum_y += y;
                sum_z += z;
                cur_index = {x,y,z};
                sum_color += image->GetPixel(cur_index);
              }
            }
          }
        }

        if(sum_voxels > 0) {
          //Calculate averaged coordinates
          sum_x = round(sum_x/sum_voxels);
          sum_y = round(sum_y/sum_voxels);
          sum_z = round(sum_z/sum_voxels);
          //Calculate displacement for the tolerance test
          if(tolerance > 0 && converged) {
            spatial_distance = pow(label->center[0]-sum_x,2)+pow(label->center[1]-sum_y,2)+pow(label->center[2]-sum_z,2);
            if(spatial_distance > tolerance)
              converged = false;
          }
          //DEBUG
          //spatial_distance = pow(label->center[0]-sum_x,2)+pow(label->center[1]-sum_y,2)+pow(label->center[2]-sum_z,2);
          //if (spatial_distance != 27) qDebug() << QString("%1 displacement: %2 ( %3 , %4 , %5 )").arg(label_index).arg(spatial_distance).arg(label->center[0]-sum_x).arg(label->center[1]-sum_y).arg(label->center[2]-sum_z);
          //EO DEBUG
          label->center = {sum_x, sum_y, sum_z};
          label->color = sum_color/sum_voxels;
        }
      } //label

      qDebug() << QString("Finishing iteration: %1").arg(iteration);
    } //iteration
  }
  catch(std::exception &e)
  {
     std::cerr << e.what() << std::endl << std::flush;
     exit(1);
  }

  qDebug() << QString("Finished computing SLIC in %1s").arg(timer.elapsed()/1000);


  //Free memory used by old results
  /*if(supervoxels != NULL)
    delete supervoxels;
  supervoxels_size = labels.size();
  supervoxels = malloc(supervoxels_size*sizeof(SuperVoxel));
  for(unsigned int label_index=0;label_index<labels.size();label_index++)
  {
    supervoxels[label_index].color = labels[label_index].color;
    supervoxels[label_index].center = labels[label_index].center;
  }*/

  qDebug() << "Generating and compressing results";

  result->slice_count = max_z;
  result->supervoxel_count = labels.size();
  result->supervoxels.clear();
  result->voxels.clear();
  result->bounds = bounds;
  if(result->slice_offset != NULL)
    delete result->slice_offset;
  result->slice_offset = (unsigned int*) malloc(max_z * sizeof(unsigned int));

  for(unsigned int label_index=0;label_index<labels.size();label_index++)
  {
    label=&labels[label_index];
    result->supervoxels.append({label->center, label->color});
  }
  labels.clear();

  unsigned short same_label_count = 0;
  unsigned int current_label;

  QDataStream stream(&result->voxels, QIODevice::WriteOnly);

  for(unsigned int z = 0; z < max_z; z++) {
    result->slice_offset[z] = result->supervoxels.size();
    for(unsigned int y = 0; y < max_y; y++) {
      voxel_index = y*max_x+z*max_x*max_y;
      for(unsigned int x = 0; x < max_x; x++) {
        if(x!=0) {
          if(voxels[voxel_index] != current_label) {
            //Write RLE
            stream << current_label;
            stream << same_label_count;
            current_label = voxels[voxel_index];
            same_label_count = 1;
          } else {
            same_label_count++;
          }
        } else {
          current_label = voxels[voxel_index];
          same_label_count = 1;
        }
        voxel_index++;
      }
      //Write RLE
      stream << current_label;
      stream << same_label_count;
    }
  }

  result->computed = true;

  if(voxels != NULL)
    delete voxels;
}

//--------------------------------------------------------------------
void StackSLIC::SLICComputeTask::onAbort()
{
  if(voxels != NULL)
    delete voxels;
}

//--------------------------------------------------------------------
unsigned long int StackSLIC::getSupervoxel(unsigned int x, unsigned int y, unsigned int z) {
  //TODO: Check if SLIC is computed
  /*if(supervoxels == NULL)
      return std::numeric_limits<unsigned int>::max();

  //TODO: Check if x,y,z is in bounds

  //Loop through supervoxels and find the closest
  OutputSPtr output = m_extendedItem->output();
  auto inputVolume = readLockVolume(output);
  if (!inputVolume->isValid())
  {
    auto what    = QObject::tr("Invalid input volume");
    auto details = QObject::tr("StackSLIC::onComputeSLIC() ->Invalid input volume.");

    throw Utils::EspinaException(what, details);
  }
  auto image = inputVolume->itkImage();*///no break

  /*typedef itk::Image<unsigned char, 3> ImageType;
  typedef ImageType::IndexType IndexType;
  IndexType cur_index;
  cur_index = {x,y,z};
  unsigned char voxel_color = image->GetPixel(cur_index);
  double color_normalization_constant = 100/255;

  for(unsigned long long int label_index=0;label_index<supervoxels_size;label_index++)
  {
    color_distance = voxel_color-center_color;
    color_distance *= color_normalization_constant;
    color_distance *= color_distance;
    spatial_distance = pow(cur_index[0]-label->center[0],2) + pow(cur_index[1]-label->center[1],2) + pow(cur_index[2]-label->center[2],2);
    distance = color_distance + label->norm_quotient * spatial_distance;
  }*/

  if(!result.computed)
    return 0;

  QDataStream stream(&result.voxels, QIODevice::ReadOnly);
  if(stream.skipRawData(result.slice_offset[z]) != result.slice_offset[z]) {
    //TODO: Exception
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

//--------------------------------------------------------------------
unsigned char StackSLIC::getSupervoxelColor(unsigned int supervoxel) {
  //TODO: Check if SLIC is computed
  /*if(supervoxels == NULL || supervoxel > supervoxels_size)
    return std::numeric_limits<unsigned long int>::max();
  //Return supervoxel color
  return supervoxels[supervoxel*sizeof(SuperVoxel)].color;*/

  if(!result.computed)
      return 0;

  return result.supervoxels[supervoxel].color;
}

//--------------------------------------------------------------------
itk::Image<unsigned char, 3>::IndexType StackSLIC::getSupervoxelCenter(unsigned int supervoxel) {
  //TODO: Check if SLIC is computed
  /*if(supervoxels == NULL || supervoxel > supervoxels_size)
    return std::numeric_limits<unsigned long int>::max();
  //Return supervoxel center coordinates
  return supervoxels[supervoxel*sizeof(SuperVoxel)].center;*/

  if(!result.computed)
      return {0,0,0};

  return result.supervoxels[supervoxel].center;
}

/*//--------------------------------------------------------------------
StackSLIC::SLICResult::SLICResult()
: slice_count(0)
, supervoxel_count(0)
, slice_offset(NULL)
, computed(false)
, bounds({0,0,0})
{
  qDebug() << "Constructor SLICResult";
}

//--------------------------------------------------------------------
StackSLIC::SLICResult::~SLICResult()
{
  if(slice_offset != NULL)
    delete slice_offset;
}*/
