/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "EspinaVolume.h"

#include <itkExtractImageFilter.h>
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkStatisticsLabelObject.h>

typedef itk::StatisticsLabelObject<unsigned int, 3> LabelObjectType;
typedef itk::LabelMap<LabelObjectType> LabelMapType;
typedef itk::LabelImageToShapeLabelMapFilter<EspinaVolume, LabelMapType> Image2LabelFilterType;
typedef itk::ExtractImageFilter<EspinaVolume, EspinaVolume> ExtractType;

//----------------------------------------------------------------------------
EspinaVolume::Pointer strechToFitContent(EspinaVolume *volume)
{
  Image2LabelFilterType::Pointer image2label = Image2LabelFilterType::New();
  image2label->ReleaseDataFlagOn();
  image2label->SetInput(volume);
  image2label->Update();

  // Get segmentation's Bounding Box
  LabelMapType            *labelMap = image2label->GetOutput();
  LabelObjectType     *segmentation = labelMap->GetLabelObject(SEG_VOXEL_VALUE);
  LabelObjectType::RegionType segBB = segmentation->GetBoundingBox();

  // Extract the volume corresponding to the Bounding Box
  ExtractType::Pointer extractor = ExtractType::New();
  extractor->SetInput(volume);
  extractor->SetExtractionRegion(segBB);
  extractor->Update();

  EspinaVolume::Pointer res = extractor->GetOutput();
  res->DisconnectPipeline();
  return res;
}
