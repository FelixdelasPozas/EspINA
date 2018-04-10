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
#include <Core/Utils/Spatial.h>
#include <Core/Utils/EspinaException.h>

//ITK
#include "itkImage.h"
#include <itkEuclideanDistanceMetric.h>
#include "itkImageConstIteratorWithIndex.h"

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Extensions;

const StackExtension::Type StackSLIC::TYPE = "Stack SLIC";

StackSLIC::StackSLIC(SchedulerSPtr scheduler, const InfoCache &cache)
: StackExtension{cache}
, m_scheduler{scheduler}
, variant(SLICVariant::SLIC)
, parameter_m_s(0)
, parameter_m_c(20)
{
  //Bounds bounds = m_extendedItem->bounds();
  //qDebug() << bounds.toString();
}

StackSLIC::~StackSLIC()
{
}

State StackSLIC::state() const
{
  //Save:
  //- Snapshot name for the calculated supervoxels
  //- Parameters used for the last computation
  return State();
}

Snapshot StackSLIC::snapshot() const
{
  //Save REL as a bytearray or whatever compression we end up using
  return Snapshot();
}

StackExtension::InformationKeyList StackSLIC::availableInformation() const
{
  return InformationKeyList();
}

void StackSLIC::onComputeSLIC() {
  Bounds bounds = m_extendedItem->bounds();
  OutputSPtr output = m_extendedItem->output();
  NmVector3 spacing = output->spacing();
  //Calculate dimensions using voxels as unit
  uint16_t max_x = bounds.lenght(Axis::X)/spacing[0], max_y = bounds.lenght(Axis::Y)/spacing[1], max_z = bounds.lenght(Axis::Z)/spacing[2];
  //Calculate total number of voxels
  uint32_t n_voxels = max_x*max_y*max_z;
  uint32_t voxel_index;

  qDebug() << QString("%1 %2 %3").arg(max_x).arg(max_y).arg(max_z);

  //Get ITK image from extended item
  auto inputVolume = readLockVolume(output);
  if (!inputVolume->isValid()) {
    auto what    = QObject::tr("Invalid input volume");
    auto details = QObject::tr("StackSLIC::onComputeSLIC() ->Invalid input volume.");

    throw Utils::EspinaException(what, details);
  }

  //Testing parameters
  parameter_m_s = 10;
  parameter_m_c = 20;
  variant = ASLIC;
  int max_iterations = 10;
  int tolerance = 0;
  if(tolerance>0)
    tolerance *= tolerance;

  QList<Label> labels;
  QList<Voxel> voxels;
  auto image = inputVolume->itkImage();
  typedef itk::Image<unsigned char, 3> ImageType;
  typedef ImageType::IndexType IndexType;

  //Find centers for the supervoxels
  for(uint16_t z=parameter_m_s/2;z<max_z;z+=parameter_m_s) {
    for(uint16_t y=parameter_m_s/2;y<max_y;y+=parameter_m_s) {
      for(uint16_t x=parameter_m_s/2;x<max_x;x+=parameter_m_s) {
        //TODO: Check if inside bounds using ChannelEdges
        //TODO: Check lowest gradient voxel in a 3x3x3 area around voxel
        labels.append((Label) {{x,y,z}, parameter_m_c, parameter_m_s, (double) parameter_m_c/ (double) parameter_m_s});
      }
    }
  }
  qDebug() << QString("Created %1 labels").arg(labels.size());

  //Create voxels information with infinite distance and undefined label
  voxels.reserve(n_voxels);
  for(voxel_index=0;voxel_index<n_voxels;voxel_index++) {
    voxels.append((Voxel) {0, std::numeric_limits<double>::max()});
  }
  qDebug() << QString("Created %1 voxels").arg(voxels.size());

  typedef itk::ImageRegionConstIteratorWithIndex<ImageType> RegionIterator;
  typedef itk::ImageRegion<3> ImageRegion;
  //ImageRegion::SizeType size;
  double region_size, spatial_distance, color_distance, distance;
  ImageRegion region;
  Label label;
  IndexType cur_index;
  unsigned char center_color, voxel_color;
  bool converged=false;
  for(int iteration = 0; iteration<max_iterations && !converged; iteration++) {
    for(uint16_t label_index=0;label_index<labels.size();label_index++) {
      qDebug() << QString("Starting iteration: %1").arg(iteration);
      label=labels[label_index];
      //Set the voxel region to iterate
      region_size = 2*label.m_s;
      int region_x, region_y, region_z, region_size_x = region_size, region_size_y = region_size, region_size_z = region_size;
      region_x = label.center.m_Index[0] - label.m_s;
      region_y = label.center.m_Index[1] - label.m_s;
      region_z = label.center.m_Index[2] - label.m_s;
      //Make sure that the region is inside the bounds of the image
      if(region_x<0) {
        region_size_x+=region_x;
        region_x=0;
      }
      if(region_y<0) {
        region_size_y+=region_y;
        region_y=0;
      }
      if(region_z<0) {
        region_size_z+=region_z;
        region_z=0;
      }
      if(region_x+region_size_x > max_x)
        region_size_x = max_x-region_x;
      if(region_y+region_size_y > max_y)
        region_size_y = max_y-region_y;
      if(region_z+region_size_z > max_z)
        region_size_z = max_z-region_z;
      region.SetIndex({{region_x, region_y, region_z}});
      //size.Fill(region_size);
      region.SetSize({{region_size_x, region_size_y, region_size_z}});
      image->SetRequestedRegion(region);
      //Calculate supervoxel quotient and reset maximum distances
      if(variant!=SLIC) {
        label.norm_quotient = label.m_c/(label.m_s<1?1:label.m_s);
        label.m_s = 0;
      }
      if(variant==ASLIC)
        label.m_c = 0;
      center_color = image->GetPixel(label.center);
      RegionIterator it(image, image->GetRequestedRegion());
      qDebug() << QString("Created RegionIterator");
      //RegionIterator it(image, region);
      it.GoToBegin();
      qDebug() << QString("Starting RegionIterator");
      while(!it.IsAtEnd())
      {
        cur_index = it.GetIndex();
        voxel_color = it.Get();

        //Calculate distance between current voxel and supervoxel center
        //color_distance = center_color > voxel_color ? center_color-voxel_color : voxel_color-center_color;
        //spatial_distance = 0;
        //distance = color_distance*color_distance + label.norm_quotient * spatial_distance*spatial_distance;
        color_distance = voxel_color-center_color;
        color_distance *= color_distance;
        spatial_distance = pow(cur_index[0]-label.center[0],2) + pow(cur_index[1]-label.center[1],2) + pow(cur_index[2]-label.center[2],2);
        distance = color_distance + label.norm_quotient * spatial_distance;

        //Check if voxel is closer to this supervoxel than to its current paired supervoxel
        voxel_index = cur_index[0]+cur_index[1]*max_x+cur_index[2]*max_x*max_y;
        if(distance < voxels[voxel_index].distance) {
          voxels[voxel_index].label = label_index;
          voxels[voxel_index].distance = distance;
        }

        //Update maximum distances for SLICO and ASLIC
        if(variant!=SLIC)
          if(label.m_s < spatial_distance)
            label.m_s = spatial_distance;
        if(variant==ASLIC)
          if(label.m_c < color_distance)
            label.m_c = color_distance;
        ++it;
      } //RegionIterator
    } //label

    //Recalculate centers, check convergence
    IndexType newCenter;

    //If convergence test is enabled, assume it converged until proven otherwise
    if(tolerance > 0) converged = true;
    for(Label label : labels) {
      //TODO: Recalculate centers
      newCenter = label.center;
      //If convergence test is enabled, check for convergence
      if(tolerance > 0 && converged) {
        spatial_distance = pow(newCenter[0]-label.center[0],2) + pow(newCenter[1]-label.center[1],2) + pow(newCenter[2]-label.center[2],2);
        if(spatial_distance > tolerance)
          converged = false;
      }
    } //label

    qDebug() << QString("Finishing iteration: %1").arg(iteration);
  } //iteration

  //TODO: Enforce connectivity
}
