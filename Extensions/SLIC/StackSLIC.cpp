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
}

//--------------------------------------------------------------------
State StackSLIC::state() const
{
  //Save:
  //- Snapshot name for the calculated supervoxels
  //- Parameters used for the last computation
  return State();
}

//--------------------------------------------------------------------
Snapshot StackSLIC::snapshot() const
{
  //Save REL as a bytearray or whatever compression we end up using
  return Snapshot();
}

//--------------------------------------------------------------------
StackExtension::InformationKeyList StackSLIC::availableInformation() const
{
  return InformationKeyList();
}

//--------------------------------------------------------------------
void StackSLIC::onComputeSLIC(unsigned int parameter_m_s, unsigned int parameter_m_c, SLICVariant variant, unsigned int max_iterations, double tolerance)
{
  if(task != NULL)
    return;
  /*auto */task = std::make_shared<SLICComputeTask>(m_extendedItem, m_scheduler, m_factory, parameter_m_s, parameter_m_c, variant, max_iterations, tolerance);

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
    task->abort();
    disconnect(task.get(), SIGNAL(finished()), this, SLOT(onSLICComputed()));
    task = NULL;
  }
}


//--------------------------------------------------------------------
StackSLIC::SLICComputeTask::SLICComputeTask(ChannelPtr stack, SchedulerSPtr scheduler, CoreFactory *factory, unsigned int parameter_m_s, unsigned int parameter_m_c, SLICVariant variant, unsigned int max_iterations, double tolerance)
: Task(scheduler)
, m_stack{stack}
, m_factory{factory}
, parameter_m_s(parameter_m_s)
, parameter_m_c(parameter_m_c)
, variant(variant)
, max_iterations(max_iterations)
, tolerance(tolerance)
, voxels(NULL)
{
}

//--------------------------------------------------------------------
void StackSLIC::SLICComputeTask::run()
{
  BlockTimer<std::chrono::milliseconds> timer{"SLIC"};

  auto edgesExtension = retrieveOrCreateStackExtension<ChannelEdges>(m_stack, m_factory);
  auto sliceRegion = edgesExtension->sliceRegion(0);


  Bounds bounds = m_stack->bounds();
  OutputSPtr output = m_stack->output();
  NmVector3 spacing = output->spacing();
  //Calculate dimensions using voxels as unit
  int max_x = bounds.lenght(Axis::X)/spacing[0];
  int max_y = bounds.lenght(Axis::Y)/spacing[1];
  int max_z = bounds.lenght(Axis::Z)/spacing[2];
  //Calculate total number of voxels
  unsigned long long n_voxels = max_x*max_y*max_z;
  unsigned long long voxel_index;

  qDebug() << QString("Size: %1 %2 %3").arg(max_x).arg(max_y).arg(max_z);

  //Get ITK image from extended item
  auto inputVolume = readLockVolume(output);
  if (!inputVolume->isValid())
  {
    auto what    = QObject::tr("Invalid input volume");
    auto details = QObject::tr("StackSLIC::onComputeSLIC() ->Invalid input volume.");

    throw Utils::EspinaException(what, details);
  }

  //Testing parameters
  parameter_m_s = 10;
  parameter_m_c = 20;
  variant = ASLIC;
  max_iterations = 10;
  tolerance = 0;

  //Square tolerance to avoid having to calculate roots later
  if(tolerance>0) tolerance *= tolerance;

  QList<Label> labels;
  //unsigned long int *voxels = (unsigned long int*) calloc(n_voxels, sizeof(unsigned long int));
  voxels = (unsigned long int*) malloc(n_voxels * sizeof(unsigned long int));
  memset(voxels, ULONG_MAX, n_voxels);
  auto image = inputVolume->itkImage();
  typedef itk::Image<unsigned char, 3> ImageType;
  typedef ImageType::IndexType IndexType;

  //Find centers for the supervoxels
  IndexType cur_index;
  for(int z=parameter_m_s/2;z<max_z;z+=parameter_m_s)
  {
    sliceRegion = edgesExtension->sliceRegion(z);
    /*top_left = sliceRegion.GetIndex();
    size = sliceRegion.GetSize();*/
    for(int y=parameter_m_s/2;y<max_y;y+=parameter_m_s)
    {
      for(int x=parameter_m_s/2;x<max_x;x+=parameter_m_s)
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

        labels.append((Label) {{x,y,z}, parameter_m_c, parameter_m_s, parameter_m_c / parameter_m_s});
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
  bool converged=false, cropped = false;;
  qDebug() << "start: max iterations" << max_iterations;
  int progress = 0;
  try
  {
    for(int iteration = 0; iteration<max_iterations && !converged; iteration++)
    {
      auto newProgress = (iteration * 100.0) / max_iterations;
      if(newProgress != progress) qDebug() << "progress" << newProgress;
      progress = newProgress;

      for(unsigned long long int label_index=0;label_index<labels.size();label_index++)
      {
        //qDebug() << QString("Starting iteration: %1").arg(iteration);
        if(label_index%5000 == 0)
          qDebug() << QString("Label %1: %2s").arg(label_index).arg(timer.elapsed()/1000);
        label=&labels[label_index];
        //Set the voxel region to iterate
        region_size = 2*label->m_s;
        /*if(label_index == 0) {
          qDebug() << QString("M_S: %1").arg(label->m_s);
          qDebug() << QString("M_C: %1").arg(label->m_c);
        }*/

        int region_x, region_y, region_z, region_size_x = region_size, region_size_y = region_size, region_size_z = region_size;
        region_x = label->center.m_Index[0] - label->m_s;
        region_y = label->center.m_Index[1] - label->m_s;
        region_z = label->center.m_Index[2] - label->m_s;
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
            label->norm_quotient = label->m_c*label->m_c/(label->m_s<1?1:label->m_s*label->m_s);
            label->m_s = 0;
            label->m_c = 0;
            break;
          case SLICO:
            //label->norm_quotient = label->m_c/(label->m_s<1?1:label->m_s);
            label->norm_quotient = label->m_c*label->m_c/(label->m_s<1?1:label->m_s*label->m_s);
            label->m_s = 0;
            break;
          case SLIC:
            label->m_s = 0;
            break;
        }

        center_color = image->GetPixel(label->center);

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
            color_distance *= color_distance;
            spatial_distance = pow(cur_index[0]-label->center[0],2) + pow(cur_index[1]-label->center[1],2) + pow(cur_index[2]-label->center[2],2);
            distance = color_distance + label->norm_quotient * spatial_distance;

            //Check if voxel is closer to this supervoxel than to its current paired supervoxel
            voxel_index = cur_index[0]+cur_index[1]*max_x+cur_index[2]*max_x*max_y;
            if(voxels[voxel_index] == ULONG_MAX || voxels[voxel_index] == label_index) {
              //First iteration, voxels not assigned yet
              distance_old = DBL_MAX;
            } else {
              label_old = &labels[voxels[voxel_index]];
              distance_old = (voxel_color - image->GetPixel(label_old->center))*(voxel_color - image->GetPixel(label_old->center)) + label_old->norm_quotient *
                            (pow(cur_index[0]-label_old->center[0],2) + pow(cur_index[1]-label_old->center[1],2) + pow(cur_index[2]-label_old->center[2],2));
            }
            if(distance < distance_old) {
              voxels[voxel_index] = label_index;
              //qDebug() << QString("Old: %1 New: %2").arg(distance_old).arg(distance);
            }

            //Update maximum distances if voxel is asigned to label
            if(label_index == voxels[voxel_index]) {
              switch(variant) {
                case ASLIC:
                  if(label->m_c < color_distance)
                    label->m_c = color_distance;
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
      } //label

      //Recalculate centers, check convergence
      IndexType newCenter;

      //If convergence test is enabled, assume it converged until proven otherwise
      if(tolerance > 0) converged = true;
      for(unsigned long long int label_index=0;label_index<labels.size();label_index++)
      {
        label=&labels[label_index];
        //TODO: Recalculate centers skipping unassigned voxels (label == ULONG_MAX)
        //maximum area for assigned voxels = m_s*m_s
        newCenter = label->center;
        //If convergence test is enabled, check for convergence
        if(tolerance > 0 && converged)
        {
          spatial_distance = pow(newCenter[0]-label->center[0],2) + pow(newCenter[1]-label->center[1],2) + pow(newCenter[2]-label->center[2],2);
          if(spatial_distance > tolerance) converged = false;
        }

        //Square root of squared maximum distances saved
        switch(variant) {
          case ASLIC:
            //Update max. distances calculating their roots as we were
            //using squared distances for the simplified equations
            label->m_c = std::sqrt(label->m_c);
          case SLICO:
            label->m_s = std::sqrt(label->m_s);
          case SLIC:
            break;
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

  //TODO: Enforce connectivity

  //TODO: Save to stack
  if(voxels != NULL)
    delete voxels;
}

//--------------------------------------------------------------------
void StackSLIC::SLICComputeTask::onAbort()
{
  if(voxels != NULL)
    delete voxels;
}
